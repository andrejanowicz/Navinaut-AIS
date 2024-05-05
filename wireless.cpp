/*
 * Navinaut-AIS
 * Code:  https://github.com/andrejanowicz/Navinaut-AIS
 * Shop:  https://navinaut-ais.de/
 * 
 * Author:  Andre Janowicz, <aj@main-void.de>
 * License: GPLV2
 *          
 */


#include "IPAddress.h"
#include <Arduino.h>
#include "wireless.h"
#include "defs.h"
#include <WiFiClient.h>
#include <WiFi.h>
#include "LED.h"

const char* FRIENDLY_NAME = "NAVINAUT-AIS-";
const char* PASSWORD = "NAVINAUT";

IPAddress Ip(192, 168, 15, 1);
IPAddress NMask(255, 255, 255, 0);
IPAddress remote_IP;
const char* BROADCAST_ADDR = "192.168.15.255";
const int UDP_PORT = 2000;
const int TCP_PORT = 2222;

char unique_friendly_name[22];
bool AP_started = false;
const size_t MAX_CLIENTS = 2;
uint8_t wireless_has_clients;

WiFiUDP udp;
WiFiClient client;
WiFiServer server(TCP_PORT, MAX_CLIENTS);

void set_friendly_name() {
  
  uint64_t chipid = ESP.getEfuseMac();
  strcpy(unique_friendly_name, FRIENDLY_NAME);
  sprintf(unique_friendly_name, "%s%04X", FRIENDLY_NAME, (uint16_t)(chipid >> 32));
}

void OnWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info){
  switch (event) {
   case WiFiEvent_t::ARDUINO_EVENT_WIFI_READY:
      DEBUG.print("soft AP started.\t");
      AP_started = true;
      break;
    case WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_START:
      break;
    case WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      wireless_has_clients = WiFi.softAPgetStationNum();
      DEBUG.println("soft AP client connected.\t");
      break;
    case WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      DEBUG.println("soft AP client disconnected.\t");
      if(client) client.stop();
       wireless_has_clients = WiFi.softAPgetStationNum();
      break;
    default:
      break;
  }
}

enum ClientState {
  DISCONNECTED,
  CONNECTED
};

ClientState client_state = DISCONNECTED;

void WiFi_handle() {
  switch (client_state){
    case DISCONNECTED:
      client = server.available();
      if(client){
        remote_IP = client.remoteIP();
        DEBUG.print("Client ");
        DEBUG.print(remote_IP);
        DEBUG.println(" has connected");
        client_state = CONNECTED;
      }
      break;
    case CONNECTED:
      if (client.available()){
        char dummy_char = client.read();
        if (dummy_char) DEBUG.println(dummy_char);
      }
      if(!client.connected()){
        client.stop();
        DEBUG.print("Client ");
        DEBUG.print(remote_IP);
        DEBUG.println(" has disconnected");
        client_state = DISCONNECTED;
      }
    }
  }

void UDP_send_NMEA(char* message) {

  udp.beginPacket(BROADCAST_ADDR, UDP_PORT);
  udp.write((uint8_t*)message, strlen(message));
  udp.endPacket();
}

void TCP_IP_send_NMEA(char* message) {

  if (client_state == CONNECTED){
    client.print(message);
  }
}



void WiFi_setup() {

  WiFi.onEvent(OnWiFiEvent);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(unique_friendly_name, PASSWORD);

  while (!AP_started) {
    LED_PWR_error_flash();
  }

  WiFi.softAPConfig(Ip, INADDR_NONE, NMask);

  IPAddress myIP = WiFi.softAPIP();

  DEBUG.print("UDP: ");
  DEBUG.print(myIP);
  DEBUG.print(":");
  DEBUG.println(UDP_PORT);
  DEBUG.print("TCP/IP: ");
  DEBUG.print(myIP);
  DEBUG.print(":");
  DEBUG.print(TCP_PORT);
  DEBUG.print(" ");
  server.begin();
}
