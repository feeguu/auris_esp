#include "driver/adc.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#define BUFFER_SIZE 8192 // adjust depending on RAM
#define SAMPLE_RATE_HZ 16000
#define SAMPLE_RATE_US (1000000 / SAMPLE_RATE_HZ)

#define CHUNK_SIZE ((size_t)512) // UDP packet size in samples

#define CONNECT_MESSAGE "HELLO_AURIS"

const char *ssid = "AURIS";
const char *password = "AURIS123";

IPAddress mobileAddress;
bool mobileConnected = false;

const int udpPort = 4210;

WiFiUDP udp;

// Set up Access Point
void setupAP()
{
  while (!WiFi.softAP(ssid, password))
  {
    Serial.println("Failed to start AP, retrying...");
    delay(1000);
  }

  Serial.println("AP started");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void waitForClient()
{
  Serial.println("Waiting for client to connect...");

  while (!mobileConnected)
  {
    int packetSize = udp.parsePacket();
    if (packetSize)
    {
      char incomingPacket[255];
      int len = udp.read(incomingPacket, 255);
      if (len > 0 && strcmp(incomingPacket, CONNECT_MESSAGE) == 0)
      {
        incomingPacket[len] = 0;
      }
      mobileAddress = udp.remoteIP();
      mobileConnected = true;
    }
    delay(100);
  }

  Serial.println("Client connected.");
}

// Send buffer function
void sendBufferUDP(int16_t *buffer, size_t length)
{
  if (mobileConnected)
  {
    // Send in chunks
    for (size_t offset = 0; offset < length; offset += CHUNK_SIZE)
    {
      size_t chunkSize = min(CHUNK_SIZE, length - offset);
      udp.beginPacket(mobileAddress, udpPort);

      // Send as raw bytes (16-bit samples)
      uint8_t *bytes = (uint8_t *)buffer;
      udp.write(bytes + offset * sizeof(int16_t), chunkSize * sizeof(int16_t));

      udp.endPacket();
    }
  }
}

hw_timer_t *timer = NULL;
volatile uint16_t writeIndex = 0;

volatile bool bufferSelected = false; // false: buffer A, true: buffer B
volatile bool bufferFull = false;     // flag to indicate full buffer

int16_t audioBufferA[BUFFER_SIZE]; // circular buffer A
int16_t audioBufferB[BUFFER_SIZE]; // circular buffer B

// ADC channel (GPIO4 on ESP32-C3)
const adc1_channel_t adcChannel = ADC1_CHANNEL_4;

void ARDUINO_ISR_ATTR onTimer()
{
  if (!mobileConnected)
  {
    // No client connected, skip sampling
    return;
  }

  // Read ADC
  int raw = adc1_get_raw(adcChannel); // 0..4095
  int16_t sample = (raw - 2048) << 4; // convert to signed 16-bit

  int16_t *audioBuffer = bufferSelected ? audioBufferB : audioBufferA;

  // Store in circular buffer
  audioBuffer[writeIndex++] = sample;

  if (writeIndex >= BUFFER_SIZE)
  {
    writeIndex = 0;
    bufferFull = true;                // flag full buffer
    bufferSelected = !bufferSelected; // switch buffer
  }
}

void setup()
{
  Serial.begin(115200);

  setupAP();

  udp.begin(udpPort);

  waitForClient();


  // Configure ADC
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(adcChannel, ADC_ATTEN_DB_11);

  timer = timerBegin(1000000);

  while (timer == NULL)
  {
    timer = timerBegin(1000000);
    Serial.println("Failed to start ticker");
  }

  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, SAMPLE_RATE_US, true, 0);

  Serial.println("Recording started...");
}

void loop()
{
  // Check if buffer is full
  if (bufferFull)
  {
    bufferFull = false;
    sendBufferUDP(bufferSelected ? audioBufferA : audioBufferB, BUFFER_SIZE);
  }

  // Main loop can do other things; sampling runs in ISR
}
