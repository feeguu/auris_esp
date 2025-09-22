// ESP32/ESP32-C3 + 2x INMP441 (I2S RX estéreo) -> UDP PCM16 interleaved (L,R,...)
// Conecta em Wi-Fi doméstico (STA) e espera handshake "HELLO_AURIS" para começar a enviar.

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <cstring>
#include "driver/i2s.h"
#include "Display.h"
#include "AurisConfig.h"
#include "AurisEEPROM.h"
#include "AurisProtocol.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>

// AP

#define AP_SSID "AURIS"
#define AP_PASS "AURIS123"

// ======= Áudio / I2S =======
#define SAMPLE_RATE_HZ 16000

// Tamanho do pacote UDP em AMOSTRAS INT16 interleaved (L,R). 512 amostras = 256 frames.
#define CHUNK_SAMPLES 512 // múltiplo de 2

// Pinos do I2S (entrada)
#define PIN_I2S_DATA 4  // DIN / SD
#define PIN_I2S_LRCLK 3 // WS / LRC
#define PIN_I2S_BCLK 2  // SCK / BCLK

// ======= UDP =======
#define CONNECT_MESSAGE "HELLO_AURIS"
const int udpPort = 4210;

AurisConfig config;

WiFiUDP udp;
IPAddress clientIP;
bool clientConnected = false;

const char *languages[] = {
    "pt", // Português
    "en", // English
};

// Fala em idiomas
const char *speechs[] = {
    "Fala",     // Português
    "Speech",    // English
};

int langIndex = 0; // padrão pt

// ======= Buffers =======
static const size_t FRAMES_PER_PACKET = CHUNK_SAMPLES / 2;  // 512 int16 -> 256 frames
static const size_t I2S_IN32_COUNT = FRAMES_PER_PACKET * 2; // L32,R32,...
static const size_t I2S_IN32_BYTES = I2S_IN32_COUNT * sizeof(int32_t);

int32_t i2s_in[I2S_IN32_COUNT]; // entrada 32-bit (24b válidos)
int16_t pcm_out[CHUNK_SAMPLES]; // saída 16-bit interleaved (L,R,...)

#define HEADROOM_SHIFT 0 // 0..2 (mais alto = mais headroom, menos volume)

inline int16_t s32_to_s16(int32_t s32)
{
  // s32: 24 bits úteis em [31..8]
  int32_t v = s32 >> (10 + HEADROOM_SHIFT);
  if (v > 32767)
    v = 32767;
  if (v < -32768)
    v = -32768;
  return (int16_t)v;
}

void sendPCMUDP(int16_t *buffer, size_t sampleCount)
{
  if (!clientConnected || sampleCount == 0)
    return;

  size_t offset = 0;
  while (offset < sampleCount)
  {
    size_t toSend = min((size_t)CHUNK_SAMPLES, sampleCount - offset);
    udp.beginPacket(clientIP, udpPort);
    udp.write((uint8_t *)(buffer + offset), toSend * sizeof(int16_t));
    udp.endPacket();
    offset += toSend;
  }
}

// ======= I2S =======
static const i2s_port_t I2S_PORT = I2S_NUM_0;
Rect *speechTextRect;

void setupI2S()
{
  i2s_config_t cfg = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE_HZ,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // 24b em 32
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 256, // frames por buffer DMA
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0};

  i2s_pin_config_t pin = {
      .mck_io_num = I2S_PIN_NO_CHANGE,
      .bck_io_num = PIN_I2S_BCLK,
      .ws_io_num = PIN_I2S_LRCLK,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = PIN_I2S_DATA};

  i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin);
  i2s_set_clk(I2S_PORT, SAMPLE_RATE_HZ, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO);
}

// ======= Funções Wi-Fi =======
const char *getWiFiStatusString(wl_status_t status)
{
  switch (status)
  {
  case WL_IDLE_STATUS:
    return "IDLE";
  case WL_NO_SSID_AVAIL:
    return "NO_SSID_AVAILABLE";
  case WL_SCAN_COMPLETED:
    return "SCAN_COMPLETED";
  case WL_CONNECTED:
    return "CONNECTED";
  case WL_CONNECT_FAILED:
    return "CONNECT_FAILED";
  case WL_CONNECTION_LOST:
    return "CONNECTION_LOST";
  case WL_DISCONNECTED:
    return "DISCONNECTED";
  default:
    return "UNKNOWN";
  }
}

void setupAP()
{
  WiFi.mode(WIFI_AP);
  while (!WiFi.softAP(AP_SSID, AP_PASS))
  {
    Serial.println("Falha ao iniciar AP, tentando de novo...");
    delay(1000);
  }
  Serial.println("AP iniciado");
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());
  udp.begin(udpPort); // necessário para receber o HELLO_AURIS
}

void connectWiFiSTA(char *HOME_SSID, char *HOME_PASS)
{
  // Desconecta e limpa configurações anteriores
  WiFi.disconnect(true);
  delay(100);

  WiFi.mode(WIFI_STA);
  delay(100);

  Serial.print("Conectando a ");
  Serial.print(HOME_SSID);
  Serial.println("...");

  // Configurações adicionais para melhorar a conexão
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  WiFi.begin(HOME_SSID, HOME_PASS);

  uint32_t t0 = millis();
  int attempts = 0;
  const int maxAttempts = 3;
  const uint32_t timeoutMs = 20000; // 20 segundos

  while (millis() - t0 < timeoutMs && WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    // A cada 5 segundos, tenta reconectar se necessário
    if ((millis() - t0) % 5000 < 500 && attempts < maxAttempts)
    {
      wl_status_t status = WiFi.status();
      Serial.printf("\nStatus WiFi: %s (%d) ", getWiFiStatusString(status), status);

      if (status == WL_CONNECT_FAILED || status == WL_CONNECTION_LOST || status == WL_DISCONNECTED)
      {
        Serial.printf("(tentativa %d/%d)\n", attempts + 1, maxAttempts);
        WiFi.disconnect();
        delay(100);
        WiFi.begin(HOME_SSID, HOME_PASS);
        attempts++;
      }
    }
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nConectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    DisplayCenteredWrite("WiFi OK");

    // Inicia UDP apenas quando conectado
    udp.begin(udpPort);
  }
  else
  {
    Serial.println("\nFalha na conexão WiFi");
    Serial.printf("Status final: %s (%d)\n", getWiFiStatusString(WiFi.status()), WiFi.status());
    Serial.println("Fallback para AP");
    setupAP();
  }
}



void forceUpdate() {
  WiFiClient client;
  t_httpUpdate_return ret = httpUpdate.update(client, config.firmware_url);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("Falha na atualização: %s\n", httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("Nenhuma atualização disponível.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("Atualização concluída com sucesso.");
      break;
  }
}

void receiveMessage()
{
  int pktSize = udp.parsePacket();
  if (pktSize > 0)
  {
    Serial.printf("Pacote UDP recebido %d bytes\n", pktSize);
    char buf[256];
    int len = udp.read(buf, sizeof(buf) - 1);
    buf[len] = '\0';
    if (len > 0)
    {
      AurisPacket packet = ProtocolHandlePacket(buf);
      Serial.printf("Tipo do pacote: %d, Tamanho: %d\n", packet.header.type, packet.header.length);
      switch (packet.header.type)
      {
      case HANDSHAKE_TYPE:
        Serial.printf("Handshake recebido\n");
        Serial.println("Cliente conectado\n");
        clientIP = udp.remoteIP();
        clientConnected = true;

        // Envia um ack
        {
          udp.beginPacket(clientIP, udpPort);
          const char *ack = "HELLO_AURIS_ACK";
          udp.write((uint8_t *)ack, strlen(ack));
          udp.endPacket();
        }
        break;
      case LABEL_TYPE:
        DisplayClear();
        Serial.printf("Label: %s\n", (char *)packet.payload);
        DisplayCenteredWrite((char *)packet.payload);
        break;
      case CONFIG_TYPE:
        Serial.printf("Config recebido\n");
        UpdateConfig(packet);
        break;
      case RESTART_TYPE:
        Serial.printf("Reiniciando por comando\n");
        ESP.restart();
        break;
      case CAPTIONS_TYPE:
        DisplayClear();
        if(config.captions_enabled) {
          DisplayWrite(speechs[langIndex], 4, 0);
          DisplayCenteredWrite((char *)packet.payload);
        } else {
          DisplayCenteredWrite(speechs[langIndex]);
        }
        break;
      case GET_CONFIG:
      {
        // serializa config e envia
        uint32_t size = sizeof(AurisConfig);
        Serial.printf("Enviando config (%d bytes)\n", size);
        uint8_t *data = new uint8_t[size];
        memcpy(data, &config, size);
        udp.beginPacket(clientIP, udpPort);
        udp.write(data, size);
        udp.endPacket();
        delete[] data;
        break;
      }
      case UPDATE_TYPE:

        break;
      default:
        Serial.printf("packet desconhecido\n");
        break;
      }
      ProtocolDeallocatePacket(packet);
    }
  }
}

void sendAliveToBroadcast()
{
  IPAddress broadcastIP = WiFi.localIP();
  broadcastIP[3] = 255; // último octeto para 255
  udp.beginPacket(broadcastIP, udpPort);
  const char *alive = "AURIS_ALIVE";
  udp.write((uint8_t *)alive, strlen(alive));
  udp.endPacket();
}

// ======= Arduino =======
void setup()
{
  Serial.begin(115200);
  delay(200);

  EEPROMInit();

  LoadConfig(config);

  Serial.println("Configurações carregadas:");
  Serial.printf("SSID: %s\n", config.ssid);
  Serial.printf("Senha: %s\n", config.password);
  Serial.printf("Idioma: %s\n", config.language);
  Serial.printf("Tamanho da fonte: %d\n", config.font_size);
  Serial.printf("Versão: %s\n", config.version);
  Serial.printf("URL firmware: %s\n", config.firmware_url);

  DisplayInit(config.font_size);

  for (size_t i = 0; i < sizeof(languages) / sizeof(languages[0]); i++)
  {
    if (strcmp(config.language, languages[i]) == 0)
    {
      langIndex = i;
      break;
    }
  }

  Rect *speechTextRect = DisplayGetTextBounds(speechs[langIndex]);


  connectWiFiSTA(config.ssid, config.password);
  // setupAP();
  sendAliveToBroadcast();

  // UDP será iniciado automaticamente em connectWiFiSTA() se conectar
  // ou em setupAP() se fazer fallback para AP
  Serial.print("Escutando UDP porta ");
  Serial.println(udpPort);

  setupI2S();

  Serial.println("Captura I2S estéreo iniciada. Transmitindo UDP...");
}

void loop()
{
  receiveMessage();
  if (!clientConnected)
  {
    return;
  }

  // lê um "pacote" de frames do I2S e envia
  size_t bytesRead = 0;
  esp_err_t err = i2s_read(I2S_PORT, (void *)i2s_in, I2S_IN32_BYTES, &bytesRead, portMAX_DELAY);
  if (err != ESP_OK || bytesRead == 0)
    return;

  const size_t samples32 = bytesRead / sizeof(int32_t);
  for (size_t i = 0; i < samples32; i++)
  {
    pcm_out[i] = s32_to_s16(i2s_in[i]);
  }
  // sendPCMUDP(pcm_out, samples32);
}
