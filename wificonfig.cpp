//PhanThanhBinh-0889331222
#include "Arduino.h"
#include "wificonfig.h"

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

String add_ssid = "";
String ssid_obj = "";
String ssid_ = "";
String pass_ = "";
String url_ = "";
String port_ = "";
String user_mqtt_ = "";
String pass_mqtt_ = "";
// 123.31.105.50
char _webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8" />
        <meta http-equiv="X-UA-Compatible" content="IE=edge" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <title>Phan Thanh Binh</title>
    </head>
    <script>
      var connection = new WebSocket("ws://" + location.hostname + ":81/");
      
        connection.onopen = function () {
        connection.send("Connect " + new Date());
        };
        connection.onerror = function (error) {
          console.log("WebSocket Error ", error);
        };
          connection.onmessage = function (e) {
          console.log("Server: ", e.data);
          const msg = JSON.parse(e.data);
          console.log("Index: ", msg.index);
          console.log("SSID: ", msg.ssid);
          for(var i = 0; i< msg.index; i++){
            add_ssid(msg.ssid[i]);
          }
          document.getElementById("username").value = msg.ssid_;
          document.getElementById("password").value = msg.pass;
          document.getElementById("URL_MQTT").value = msg.url;
          document.getElementById("PORT_MQTT").value = msg.port;
          document.getElementById("USER_MQTT").value = msg.user_mqtt;
          document.getElementById("PASS_MQTT").value = msg.pass_mqtt;
          document.getElementById("Min_ATM1").value = msg.min_atm1;
          document.getElementById("Max_ATM1").value = msg.max_atm1;
          document.getElementById("Min_ATM2").value = msg.min_atm2;
          document.getElementById("Max_ATM2").value = msg.max_atm2;
          document.getElementById("number_over").value = msg.number_over;
        };

      function credentials_rec() {
        var ssid = document.getElementById("username").value;
        var pass = document.getElementById("password").value;
        var url = document.getElementById("URL_MQTT").value;
        var port = document.getElementById("PORT_MQTT").value;
        var user_mqtt = document.getElementById("USER_MQTT").value;
        var pass_mqtt = document.getElementById("PASS_MQTT").value;
        var full_command = '#{"ssid":"' + ssid + '", "pass":"' + pass + '", "url":"' + url + '", "port":"' + port + '", "user_mqtt":"' + user_mqtt + '", "pass_mqtt":"' + pass_mqtt +'"}';
        console.log(full_command);
        connection.send(full_command); 
      }
      function add_ssid(ssid) {
        const hexString = ssid;
        const bytes = new Uint8Array(hexString.match(/.{1,2}/g).map(byte => parseInt(byte, 16)));
        const utf8String = new TextDecoder().decode(bytes);
        var x = document.getElementById("username_");
        var option = document.createElement("option");
        option.text = utf8String;
        option.value = utf8String;
        x.add(option);
      }
      function upload_ssid(){
        document.getElementById("username").value = document.getElementById("username_").value;
      }
  
    </script>
    <style>
        .login-block {
            width: 320px;
            padding: 20px;
            background: #fff;
            border-radius: 5px;
            margin: 0 auto;
        }
        .login-block h1 {
            text-align: center;
            color: #000;
            font-size: 18px;
            text-transform: uppercase;
            margin-top: 0;
            margin-bottom: 20px;
        }
        .login-block input {
            width: 100%;
            height: 42px;
            box-sizing: border-box;
            border-radius: 5px;
            border: 1px solid #ccc;
            margin-bottom: 20px;
            font-size: 14px;
            font-family: Montserrat;
            padding: 0 20px 0 50px;
            outline: none;
        }
        .login-block input:active,
        .login-block input:focus {
            border: 1px solid #0011ff;
        }
        .login-block button {
            width: 100%;
            height: 40px;
            background: #0011ff;
            box-sizing: border-box;
            border-radius: 5px;
            border: 1px solid #0011ff;
            color: #fff;
            font-weight: bold;
            text-transform: uppercase;
            font-size: 14px;
            font-family: Montserrat;
            outline: none;
            cursor: pointer;
        }
        .login-block button:hover {
            background: #0011ff;
        }
        select {
            width: 100%;
            height: 42px;
            box-sizing: border-box;
            border-radius: 5px;
            border: 1px solid #ccc;
            margin-bottom: 20px;
            font-size: 14px;
            font-family: Montserrat;
            padding: 0 20px 0 50px;
            outline: none;
        }
        label {
            width: 100%;
            height: 42px;
            box-sizing: border-box;
            font-size: 14px;
            font-family: Montserrat;
            outline: none;
        }
    </style>
    <body>
    <form action="/submit" method="get">
        <div class="login-block">
            <h1>Binh Phan Config</h1>
            <label for="cars">Choose a SSID:</label>
            <select name="ssid" id="username_" onChange="upload_ssid();"> </select>
            <input type="text" value="" placeholder="SSID" id="username" oninvalid="alert('You must fill out the form SSID!');" required/>
            <input type="text" value="" placeholder="Password" id="password" oninvalid="alert('You must fill out the form Password!');" required/>
            <label for="cars">Configure MQTT broker connection:</label>
            <input type="text" value="" placeholder="URL MQTT" id="URL_MQTT" oninvalid="alert('You must fill out the form URL MQTT!');" required/>
            <input type="text" value="" placeholder="PORT MQTT" id="PORT_MQTT" oninvalid="alert('You must fill out the form Port MQTT!');" required/>
            <input type="text" value="" placeholder="USER MQTT" id="USER_MQTT" oninvalid="alert('You must fill out the form User MQTT!');" required/>
            <input type="text" value="" placeholder="PASS MQTT" id="PASS_MQTT" oninvalid="alert('You must fill out the form password MQTT!');" required/>
            <button onclick="credentials_rec();">Submit</button>
        </div>
        </form>
    </body>
</html>
)=====";

void _webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
        webSocket.sendTXT(num, add_ssid);
        if (num != 0) {
          for (byte i = 0; i < num; i++) {
            webSocket.disconnect(i);
          }
        } else {
          for (byte i = 1; i <= 4; i++) {
            webSocket.disconnect(i);
          }
        }
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);

      if (payload[0] == '#') {
        String message = String((char *)(payload));
        message = message.substring(1);
        Serial.println(message);

        // JSON part
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, message);

        String ssid = doc["ssid"];
        String pass = doc["pass"];
        String url = doc["url"];
        String port = doc["port"];
        String user_mqtt = doc["user_mqtt"];
        String pass_mqtt = doc["pass_mqtt"];
        Serial.println(ssid);
        Serial.println(pass);

        // Xóa EEPROM
        if (ssid.length() > 0 && pass.length() > 0) {
          Serial.println("Delete eeprom");
          for (int i = 0; i < 512; ++i) {
            EEPROM.write(i, 0);
          }

          // Storing in EEPROM
          Serial.println("Write eeprom ssid:");
          for (int i = 0; i < ssid.length(); ++i) {
            EEPROM.write(i, ssid[i]);
            Serial.print("Done:");
            Serial.println(ssid[i]);
          }
          Serial.println("Write pass eeprom:");
          for (int i = 0; i < pass.length(); ++i) {
            EEPROM.write(32 + i, pass[i]);
            Serial.print("Done: ");
            Serial.println(pass[i]);
          }
          Serial.println("Write eeprom user_mqtt:");
          for (int i = 0; i < user_mqtt.length(); ++i) {
            EEPROM.write(64 + i, user_mqtt[i]);
            Serial.print("Done:  ");
            Serial.println(user_mqtt[i]);
          }
          Serial.println("Write eeprom pass_mqtt:");
          for (int i = 0; i < pass_mqtt.length(); ++i) {
            EEPROM.write(96 + i, pass_mqtt[i]);
            Serial.print("Done:  ");
            Serial.println(pass_mqtt[i]);
          }
          Serial.println("Write eeprom port:");
          for (int i = 0; i < port.length(); ++i) {
            EEPROM.write(128 + i, port[i]);
            Serial.print("Done:  ");
            Serial.println(port[i]);
          }
          Serial.println("Write eeprom url:");
          for (int i = 0; i < url.length(); ++i) {
            EEPROM.write(160 + i, url[i]);
            Serial.print("Done:  ");
            Serial.println(url[i]);
          }

          delay(20);
          EEPROM.commit();
          delay(20);
          //_createWebServer();
          //delay(2000);
          //webSocket.sendTXT(num, "{\"received\": true}");
          // Serial.println("{\"received\": true}");
          Serial.println("Done");
          /* delay(1000);
          WiFi.softAPdisconnect(true);
          // Restarting ESP board
          ESP.restart();
          */
          break;
        }
      }
  }
}

void credentials::Erase_eeprom() {
  EEPROM.begin(512);  // Initialasing EEPROM
  Serial.println("Deleting ...");
  Serial.println("Delete eeprom");
  for (int i = 0; i < 512; ++i) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}
void credentials::Check_empty(void) {
  EEPROM.begin(512);
  for (int i = 0; i < 512; ++i) {
    if (EEPROM.read(i) == 255) {
      EEPROM.write(i, 0);
    }
  }
  EEPROM.commit();
}
void credentials::EEPROM_Config() {
  EEPROM.begin(512);  // Khởi tạo EEPROM
  // ---------------------------------------- Đọc eeprom cho ssid
  Serial.println("READ EEPROM");
  for (int i = 0; i < 32; ++i) {
    ssid += char(EEPROM.read(i));
  }
  for (int i = 32; i < 64; ++i) {
    pass += char(EEPROM.read(i));
  }
}

String credentials::SSID_WIFI() {
  EEPROM.begin(512);
  for (int i = 0; i < 32; ++i) {
    ssid += char(EEPROM.read(i));
  }
  ssid_ = &ssid[0];
  return ssid_;
}

String credentials::PASS_WIFI() {
  EEPROM.begin(512);
  for (int i = 32; i < 64; ++i) {
    pass += char(EEPROM.read(i));
  }
  pass_ = &pass[0];
  return pass_;
}
String credentials::USER_MQTT() {
  EEPROM.begin(512);
  for (int i = 64; i < 96; ++i) {
    user_mqtt += char(EEPROM.read(i));
  }
  user_mqtt_ = &user_mqtt[0];
  return user_mqtt_;
}
String credentials::PASS_MQTT() {
  EEPROM.begin(512);
  for (int i = 96; i < 128; ++i) {
    pass_mqtt += char(EEPROM.read(i));
  }
  pass_mqtt_ = &pass_mqtt[0];
  return pass_mqtt_;
}
String credentials::PORT_MQTT() {
  EEPROM.begin(512);
  for (int i = 128; i < 160; ++i) {
    port += char(EEPROM.read(i));
  }
  port_ = &port[0];
  return port_;
}
String credentials::URL_MQTT() {
  EEPROM.begin(512);
  for (int i = 160; i < 192; ++i) {
    url += char(EEPROM.read(i));
  }
  url_ = &url[0];
  return url_;
}

bool credentials::credentials_get() {
  if (_testWifi()) {
    Serial.println("Successful connection!!!");
    return true;
  } else {
    Serial.println("Connect Wifi Fail!!!");
    return false;
  }
}
void credentials::setupAP(char *softap_ssid, char *softap_pass) {
  scan_wifi();
  WiFi.disconnect();
  delay(100);
  WiFi.softAP(softap_ssid, softap_pass);
  Serial.println("softap");
  _launchWeb();
  Serial.println("Server Started");
  webSocket.begin();
  webSocket.onEvent(_webSocketEvent);
}
bool credentials::ConectWifi(byte flag) {
  if (flag) {
    char *my_ssid = &ssid[0];
    char *my_pass = &pass[0];
    Serial.println(my_ssid);
    Serial.println(my_pass);
    WiFi.begin(my_ssid, my_pass);
  }
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  return false;
}
bool credentials::_testWifi() {
  int c = 0;
  Serial.println("Connecting to Wifi....");
  char *my_ssid = &ssid[0];
  char *my_pass = &pass[0];
  Serial.println(my_ssid);
  Serial.println(my_pass);
  WiFi.begin(my_ssid, my_pass);
  while (c < 30) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  // Serial.println("");
  // Serial.println("Timeout, open AP");
  Serial2.println("Timeout");
  return false;
}

// This is the function which will be called when an invalid page is called from client
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void credentials::_createWebServer() {
  server.on("/", [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", _webpage);
  });
  //request->send_P(200, "text/html", _webpage);
  //request->send(SPIFFS, "/index.html");

  // Send a GET request to <IP>/get?message=<message>
  server.on("/submit", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
    message = "Reboot, please wait for WIFI connection !";
    request->send(200, "text/plain", message);
    WiFi.softAPdisconnect(true);
    ESP.restart();
  });

  server.onNotFound(notFound);
  server.begin();
}

void credentials::_launchWeb() {
  Serial.println("");
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  _createWebServer();
  // Start the server
}

void credentials::server_loops() {
  webSocket.loop();
}
void credentials::scan_wifi() {
  int error = 0;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    ssid_obj = "";
    for (int i = 0; i < n; i++) {
      if (WiFi.SSID(i).indexOf('\"') >= 0) {
        error = error + 1;
      } else {
        Serial.println(SSID_to_HEX(WiFi.SSID(i)));
        ssid_obj = ssid_obj + "\"" + SSID_to_HEX(WiFi.SSID(i)) + "\",";
      }
    }
    n = n - error;
    add_ssid = "{\"index\":" + (String)n + ",\"ssid\":[" + ssid_obj + "\"end\"],\"ssid_\":\"" + ssid_ + "\",\"pass\":\"" + pass_ + "\",\"url\":\"" + url_ + "\",\"port\":\"" + port_ + "\",\"user_mqtt\":\"" + user_mqtt_ + "\",\"pass_mqtt\":\"" + pass_mqtt_ + "\"}";
  }
  Serial.println(add_ssid);
}
String credentials::SSID_to_HEX(String ssid) {
  char DEVICE_NAME_CH[ssid.length()];
  ssid.toCharArray(DEVICE_NAME_CH, ssid.length() + 1);
  const char *wifi_ssid = DEVICE_NAME_CH;
  wchar_t *arr = (wchar_t *)(wifi_ssid);
  //Serial.println(F(arr));
  int len = ssid.length();
  //Serial.println(len);
  String a[(len / 2 + 1)];
  if (len % 2 == 1) {
    len = len + 1;
  }
  for (int j = 0; j < (len / 2); j++) {
    char tempstring[4];
    sprintf(tempstring, "%x", arr[j]);
    //Serial.println((char*)tempstring);
    a[j] = String(tempstring);
    //Serial.println(a[j]);
    //hexString = hexString + a;
  }
  String hexString = "";
  for (int j = 0; j < (len / 2); j++) {
    hexString = hexString + a[j];
    // Serial.println(a[j]);
  }
  //Serial.println(hexString);
  int len_hexString = hexString.length();
  //Serial.println(len_hexString);
  if (len_hexString % 2 == 1) {
    hexString.remove((hexString.length() - 1), 1);
  }
  //Serial.println(hexString);
  len_hexString = hexString.length();
  //Serial.println(len_hexString);
  String hexString_end = "";
  for (int i = 0; i <= len_hexString / 4; i++) {
    String index_1 = hexString.substring(0, 2);
    String index_2 = hexString.substring(2, 4);
    hexString_end = hexString_end + index_2 + index_1;
    hexString.remove(0, 4);
  }
  //Serial.println(hexString_end);
  return hexString_end;
}
