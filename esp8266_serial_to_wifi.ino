#include <ESP8266WiFi.h>

#define UART_BAUD 9600
#define PACKET_TIMEOUT 5
#define BUFFER_SIZE 8192

//#define MODE_AP
#define MODE_STA

#define PROTOCOL_TCP
//#define PROTOCOL_UDP


#ifdef MODE_AP
const char *ssid = "u-blox NEO-M8N";
const char *pw = "1234567890";
IPAddress ip(192, 168, 0, 1);
IPAddress netmask(255, 255, 255, 0);
const int port = 27000;
#endif

#ifdef MODE_STA
const char *ssid = "u-blox NEO-M8N";
const char *pw = "1234567890";
const int port = 27000;
#endif

#ifdef PROTOCOL_TCP
#include <WiFiClient.h>
WiFiServer server(port);
WiFiClient client;
#endif

#ifdef PROTOCOL_UDP
#include <WiFiUdp.h>
WiFiUDP udp;
IPAddress remoteIp;
#endif

uint8_t bufferReceive[BUFFER_SIZE];
uint16_t i1 = 0;

uint8_t bufferSend[BUFFER_SIZE];
uint16_t i2 = 0;

void setup()
{
  delay(500);

  Serial.begin(UART_BAUD);

#ifdef MODE_AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, ip, netmask);
  WiFi.softAP(ssid, pw);
#endif

#ifdef MODE_STA
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }
#endif

#ifdef PROTOCOL_TCP
  server.begin();
#endif

#ifdef PROTOCOL_UDP
  udp.begin(port);
#endif
}

void loop()
{
#ifdef PROTOCOL_TCP
  if (!client.connected())
  {
    client = server.available();
    return;
  }

  if (client.available())
  {
    while (client.available())
    {
      bufferReceive[i1] = (uint8_t)client.read();
      if (i1 < BUFFER_SIZE - 1)
      {
        i1++;
      }
    }

    Serial.write(bufferReceive, i1);
    i1 = 0;
  }

  if (Serial.available())
  {
    while (1) {
      if (Serial.available())
      {
        bufferSend[i2] = (char)Serial.read();
        if (i2 < BUFFER_SIZE - 1)
        {
          i2++;
        }
      }
      else
      {
        delay(PACKET_TIMEOUT);
        if (!Serial.available())
        {
          break;
        }
      }
    }

    client.write((char*)bufferSend, i2);
    i2 = 0;
  }
#endif

#ifdef PROTOCOL_UDP
  int packetSize = udp.parsePacket();
  if (packetSize > 0)
  {
    remoteIp = udp.remoteIP();
    udp.read(bufferReceive, bufferSize);

    Serial.write(bufferReceive, packetSize);
  }

  if (Serial.available())
  {
    while (1)
    {
      if (Serial.available())
      {
        bufferSend[i2] = (char)Serial.read();
        if (i2 < bufferSize - 1)
        {
          i2++;
        }
      }
      else
      {
        delay(PACKET_TIMEOUT);
        if (!Serial.available())
        {
          break;
        }
      }
    }

    udp.beginPacket(remoteIp, port);
    udp.write(bufferSend, i2);
    udp.endPacket();
    i2 = 0;
  }
#endif
}
