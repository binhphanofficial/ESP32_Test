//PhanThanhBinh-0889331222
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#else
#error "Board not found"
#endif

#include "Arduino.h"
#include "ESPAsyncWebServer.h"
#include "WebSocketsServer.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <string.h>


class credentials {
  public:
    bool credentials_get();
    bool ConectWifi(byte flag);
    void setupAP(char* softap_ssid, char* softap_pass);
    void server_loops(void);
    void EEPROM_Config(void);
    void Erase_eeprom(void);
    void Check_empty(void);
    String SSID_WIFI(void);
    String PASS_WIFI(void);
    String URL_MQTT(void);
    String PORT_MQTT(void);
    String USER_MQTT(void);
    String PASS_MQTT(void);
  private:
    void scan_wifi(void);
    bool _testWifi(void);
    void _launchWeb(void);
    void _createWebServer(void);
    String SSID_to_HEX(String ssid);
    String ssid = "";
    String pass = "";
    String url = "";
    String port = "";
    String user_mqtt = "";
    String pass_mqtt = "";
};
