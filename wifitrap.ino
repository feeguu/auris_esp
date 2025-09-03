#include <WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>

#define BUFFER_SIZE 255
#define CONNECTION_MESSAGE "connect"
#define MIC_PIN 1
#define SAMPLE_RATE 8000 // 8kHz sample rate
#define BLOCK_SIZE 1024   // how many samples per UDP packet

const char *ssid = "AURIS";
const char *password = "AURIS123";

uint32_t mobileIP = 0;
bool mobileConnected = false;
bool stationConnected = false; // Flag to check if a device is connected to the AP
uint32_t mobileUdpPort = 4201;

WiFiUDP Udp;
uint32_t glassesUdpPort = 4202;
char packetBuffer[BUFFER_SIZE];

Ticker checkConnectionTicker;
Ticker waitConnectionPacketTicker;


// audio buffer
int8_t audioBuffer[BLOCK_SIZE];
int bufferIndex = 0;
Ticker sampleTicker;


// Converte IP do cliente para string
String ip_to_string(uint32_t ip)
{
  return String(ip & 0xFF) + "." + String((ip >> 8) & 0xFF) + "." + String((ip >> 16) & 0xFF) + "." + String((ip >> 24) & 0xFF);
}

void checkConnectedDevice()
{
  int numStations = WiFi.softAPgetStationNum();

  if (numStations > 0 && !stationConnected)
  {
    stationConnected = true;
    Serial.println("Device connected to AP. Waiting for connection packet...");
    Serial.printf("Number of connected stations: %d\n", numStations);
  }
  else if (numStations == 0 && stationConnected)
  {
    stationConnected = false;
    mobileConnected = false; // If station disconnects, mobile is also disconnected
    mobileIP = 0;
    Serial.println("Device disconnected from AP.");
  }
}

void handleConnectionPacket()
{
  if (stationConnected && !mobileConnected)
  {
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
      int len = Udp.read(packetBuffer, BUFFER_SIZE - 1);
      if (len > 0)
        packetBuffer[len] = 0;

      if (strcmp(packetBuffer, CONNECTION_MESSAGE) == 0)
      {
        mobileConnected = true;
        mobileIP = Udp.remoteIP();
        Serial.println("Mobile device is now connected.");
        Serial.println("Mobile IP set to: " + ip_to_string(mobileIP));
      }
    }
  }
}

void streamAudio()
{
  if (!mobileConnected)
    return;

  // read 12-bit ADC (0-4095)
  uint16_t analogValue = analogRead(MIC_PIN);

  // convert to 8-bit unsigned (0-255)
  uint8_t sample = analogValue >> 4; // divide by 16

  int8_t signedSample = (int8_t)(sample - 128);

  // add to buffer
  audioBuffer[bufferIndex++] = signedSample;

  // if buffer full, send via UDP
  if (bufferIndex >= BLOCK_SIZE)
  {
    Udp.beginPacket(mobileIP, mobileUdpPort);
    Udp.write((uint8_t*) audioBuffer, BLOCK_SIZE);
    Udp.endPacket();
    bufferIndex = 0;
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Configuring access point...");

  analogReadResolution(12); // ESP32-C3 supports up to 12-bit
  pinMode(MIC_PIN, INPUT);

  if (WiFi.softAP(ssid, password))
  {
    Serial.println("Access Point created");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }
  else
  {
    Serial.println("Failed to create Access Point");
  }

  Udp.begin(glassesUdpPort);
  Serial.printf("Listening for UDP packets on port %d\n", glassesUdpPort);

  checkConnectionTicker.attach(1, checkConnectedDevice);
  waitConnectionPacketTicker.attach(0.5, handleConnectionPacket);


  sampleTicker.attach_us(125, streamAudio);
}

void loop()
{
 
}
