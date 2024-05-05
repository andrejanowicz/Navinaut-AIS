/*
 * Navinaut-AIS
 * Code:  https://github.com/andrejanowicz/Navinaut-AIS
 * Shop:  https://navinaut-ais.de/
 * 
 * Author:  Andre Janowicz, <aj@main-void.de>
 * License: GPLV2
 *          
 */


#include <Arduino.h>
#include "wireless.h"
#include "defs.h"
#include <WiFiClient.h>
#include <WiFi.h>
#include "LED.h"
#include "List.h"

const char* DEVICE = "NAVINAUT-AIS-";
const char* PASSWORD = "NAVINAUT";

IPAddress Ip(192, 168, 15, 1);
IPAddress NMask(255, 255, 255, 0);

const char* BROADCAST_ADDR = "192.168.15.255";
const int UDP_PORT = 2000;
const int TCP_PORT = 2222;

char friendly_name[22];
bool AP_started = false;
const size_t MAX_CLIENTS = 2;
uint8_t wireless_has_clients;

using tWiFiClientPtr = std::shared_ptr<WiFiClient>;
LinkedList<tWiFiClientPtr> clients;

WiFiUDP udp;
WiFiClient client;
WiFiServer server(TCP_PORT, MAX_CLIENTS);

void set_friendly_name() {
  
  uint64_t chipid = ESP.getEfuseMac();
  strcpy(friendly_name, DEVICE);
  sprintf(friendly_name, "%s%04X", friendly_name, (uint16_t)(chipid >> 32));
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
      wireless_has_clients = WiFi.softAPgetStationNum();
      break;
    default:
      break;
  }
}


void AddClient(WiFiClient &client) {
  
  DEBUG.print("new client:\t");
  DEBUG.println(client.remoteIP());
  clients.push_back(tWiFiClientPtr(new WiFiClient(client)));
}

void StopClient(LinkedList<tWiFiClientPtr>::iterator &it) {
  
  DEBUG.println("client disconnected.");

  (*it)->stop();
  it = clients.erase(it);

}

void WiFi_handle() {
  
  WiFiClient client = server.available();   // listen for incoming clients

  if ( client ) AddClient(client);

  for (auto it = clients.begin(); it != clients.end(); it++) {
    if ( (*it) != NULL ) {
      if ( !(*it)->connected() ) {
        StopClient(it);
      }else{
        if ( (*it)->available() ) {
          char c = (*it)->read();
          if ( c == 0x03 ) StopClient(it);
        }
      }
    }else{
      it = clients.erase(it); // Should have been erased by StopClient
    }
  }
}

void UDP_send_NMEA(char* message) {

  udp.beginPacket(BROADCAST_ADDR, UDP_PORT);
  udp.write((uint8_t*)message, strlen(message));
  udp.endPacket();

}

void TCP_IP_send_NMEA(char* message) {

  for (auto it = clients.begin() ; it != clients.end(); it++) {
    if ( (*it) != NULL && (*it)->connected() ) {
      (*it)->print(message);
    }
  }
}


void WiFi_setup() {

  WiFi.onEvent(OnWiFiEvent);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(friendly_name, PASSWORD);

  while (!AP_started) {
    LED_PWR_error_flash();
  }

  WiFi.softAPConfig(Ip, INADDR_NONE, NMask);

  IPAddress myIP = WiFi.softAPIP();

  DEBUG.print("\nUDP:\t");
  DEBUG.print(myIP);
  DEBUG.print(":");
  DEBUG.println(UDP_PORT);
  DEBUG.print("TCP/IP:\t");
  DEBUG.print(myIP);
  DEBUG.print(":");
  DEBUG.print(TCP_PORT);
  DEBUG.print("\t");
  server.begin();
}
