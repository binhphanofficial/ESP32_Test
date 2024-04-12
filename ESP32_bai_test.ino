#include "wificonfig.h"
#include "PubSubClient.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>

#define button_config_wifi 0
#define led_debug 2
#define waiting_time_config 5000

#define out_01 3
#define out_02 15

String model_device = "BinhPhan";
// config wifi
byte rst_wifi = 0;
byte last_rst_wifi = 0;
byte flag_wifi_config = 0;
byte flag_wifi_config_ = 1;
unsigned long millis_rst_wifi = 0;
unsigned long millis_timeout_rst_wifi = 0;
unsigned long timeout_rst_wifi = 180000;  // 3 phút
// reconnect wifi
byte flag_begin_wifi = 0;  // trạng thái sẳn sàng kết nối wifi
// name device
String mac_addr_str = "";
String imei = "";
String name_device = "";
// access point config wifi
char *esp_ssid = "";
char *esp_pass = "";
// MQTT
char *DEVICE_NAME = "";
char *MQTT_HOST = "";
int MQTT_PORT = 0;
char *MQTT_USER = "";
char *MQTT_PASS = "";
byte willQoS = 0;
boolean willRetain = true;
String DEVICE_NAME_ = "";
String MQTT_HOST_ = "";
String MQTT_USER_ = "";
String MQTT_PASS_ = "";
credentials Credentials;
WiFiClient net;
PubSubClient client_mqtt(net);
//
byte flag_wifi_mqtt = 0;
// trạng thái auto mode
byte flag_auto_mode = 0;

// MQTT topic
// Lastwill
const char *willMessage = "offline";
char *MQTT_LASTWILL_TOPIC = "";
String MQTT_LASTWILL_TOPIC_STR = "";
// SUB
char *MQTT_SUB_DATA_TOPIC = "";
String MQTT_SUB_DATA_TOPIC_STR = "";
// PUB
char *MQTT_PUB_DATA_TOPIC = "";
String MQTT_PUB_DATA_TOPIC_STR = "";

// led debug
unsigned long previousMillis = 0;
const long interval = 1000;  // thời gian sáng tắt led khi kết nối thành công wifi
// led debug wifi
unsigned long previousMillis_problem_wifi = 0;
const long interval_problem_wifi = 100;  // thời gian sáng tắt led khi kết nối không thành công wifi
// button
unsigned long previousMillis_button = 0;
const long button_expiration_time = 1000;  // thời gian hết hạn của nút khi nhấn liên tiếp

// button
int buttonPushCounter = 0;  // số lần button được nhấn
int buttonState = 0;        // trạng thái hiện tại của button
int lastButtonState = 0;    // trạng thái trước đó của button

void Status_Online(void);
void config_wifi(void);
void reconnect_wifi(void);
void processing_main(void);
byte MQTT_connection(void);
void receivedCallback(char *topic, byte *payload, unsigned int length);
void led_debug_on(void);
void led_debug_off(void);
void heart_beat_led(void);
void led_problem_disconnected_wifi(void);
void out_01_fn(byte status);
void out_02_fn(byte status);
void funtion_button(void);
void state_auto_mode(byte state_auto_mode);

void setup() {
  Serial.begin(115200);
  // Setup button config wifi
  pinMode(button_config_wifi, INPUT);
  pinMode(led_debug, OUTPUT);
  pinMode(out_01, OUTPUT);
  pinMode(out_02, OUTPUT);
  out_01_fn(0);
  out_01_fn(0);
  led_debug_off();
  // Setup name device
  mac_addr_str = WiFi.macAddress();
  imei = mac_addr_str;
  name_device = mac_addr_str;
  imei.remove(2, 1);
  imei.remove(4, 1);
  imei.remove(6, 1);
  imei.remove(8, 1);
  imei.remove(10, 1);
  name_device.remove(0, 12);
  name_device.remove(2, 1);
  name_device = model_device + "_" + name_device;
  DEVICE_NAME_ = model_device + "_" + imei;
  Serial.println(mac_addr_str);
  Serial.println(imei);
  Serial.println(name_device);
  Serial.println(DEVICE_NAME_);
  // Read data in eeprom
  Credentials.Check_empty();
  Serial.print("SSID: ");
  Serial.println(Credentials.SSID_WIFI());
  Serial.print("PASS: ");
  Serial.println(Credentials.PASS_WIFI());
  MQTT_HOST_ = Credentials.URL_MQTT();
  MQTT_PORT = Credentials.PORT_MQTT().toInt();
  MQTT_USER_ = Credentials.USER_MQTT();
  MQTT_PASS_ = Credentials.PASS_MQTT();
  Serial.print("MQTT_HOST_: ");
  Serial.println(MQTT_HOST_);
  Serial.print("MQTT_PORT: ");
  Serial.println(MQTT_PORT);
  Serial.print("MQTT_USER_: ");
  Serial.println(MQTT_USER_);
  Serial.print("MQTT_PASS_: ");
  Serial.println(MQTT_PASS_);
  // topic MQTT
  // last will
  MQTT_LASTWILL_TOPIC_STR = model_device + "/" + imei + "/lastwill";
  Serial.println(MQTT_LASTWILL_TOPIC_STR);
  // SUB
  MQTT_SUB_DATA_TOPIC_STR = model_device + "/" + imei + "/data";
  Serial.println(MQTT_SUB_DATA_TOPIC_STR);
  // PUB
  MQTT_PUB_DATA_TOPIC_STR = model_device + "/" + imei + "/data/device";
  Serial.println(MQTT_PUB_DATA_TOPIC_STR);
}
byte RST_WIFI(void) {
  return !digitalRead(button_config_wifi);
}
byte wifi_config(void) {
  byte result = flag_wifi_config;
  rst_wifi = RST_WIFI();
  if (rst_wifi != last_rst_wifi && rst_wifi == 1) {
    millis_rst_wifi = millis();
  }
  last_rst_wifi = rst_wifi;
  // Nếu nhấn giữ nút đủ 5 giây >= giá trị waiting_time_config
  if (rst_wifi == 1 && ((unsigned long)(millis() - millis_rst_wifi) >= waiting_time_config)) {
    if (flag_wifi_config_) {
      // Hiển thị 1 lần thông báo bắt đầu vào wifi config
      led_debug_on();  // sáng led báo trạng thái mode wifi config
      Serial.println("Button Begin Config!");
      Serial.println("Wifi Config !");
      config_wifi();
      flag_wifi_config = 1;
      flag_wifi_config_ = 0;  // Không hiển thị thông báo nửa
      millis_timeout_rst_wifi = millis();
    }
  }
  result = flag_wifi_config;  // 1
  return result;
}
void config_wifi(void) {
  if (!MDNS.begin("BinhPhan")) {
    Serial.println("Error starting mDNS");
    return;
  }
  char DEVICE_NAME_CH[name_device.length()];
  name_device.toCharArray(DEVICE_NAME_CH, name_device.length() + 1);
  esp_ssid = DEVICE_NAME_CH;
  Credentials.setupAP(esp_ssid, esp_pass);
}
void reconnect_wifi(void) {
  // Sẵn sàng connect wifi
  if (flag_begin_wifi == 0) {
    Serial.println("Reconnect Wifi");
    Credentials.EEPROM_Config();
    Credentials.ConectWifi(1);
    flag_begin_wifi = 1;  // Đã cấu hình kết nối xong và đưa cờ về trạng thái chờ
  } else {
    // Nếu kết nối wifi thành công thì kết nối MQTT
    if (Credentials.ConectWifi(0)) {
      led_debug_off();  // tắt led
      char MQTT_HOST_CH[MQTT_HOST_.length()];
      MQTT_HOST_.toCharArray(MQTT_HOST_CH, MQTT_HOST_.length() + 1);
      MQTT_HOST = MQTT_HOST_CH;

      Serial.println("Wifi connected");
      flag_begin_wifi = 0;  // trả cờ về trạng thái sẵn sàng
      Serial.println("Connect MQTT: ");
      client_mqtt.setServer(MQTT_HOST, MQTT_PORT);
      client_mqtt.setCallback(receivedCallback);
      if (MQTT_connection()) {
        // báo trạng thái online lên server
        Status_Online();
      } else {
        // Viết chương trình xử lý logic khi offline tại đây
        led_debug_off();  // tắt led
      }
    } else {
      led_problem_disconnected_wifi();  // nếu không kết nối được wifi thì led nháy 100ms
    }
  }
}
void loop() {
  // Nếu nhấn giữ nút 5 giây thì vào wifi config
  if (wifi_config()) {
    // Web Server local open cho phép truy cập cấu hình
    Credentials.server_loops();
    // Nếu hết thời gian 3 phút thì tự động thoát chế độ wifi config
    if ((unsigned long)(millis() - millis_timeout_rst_wifi) >= timeout_rst_wifi) {
      millis_timeout_rst_wifi = millis();
      WiFi.softAPdisconnect(true);
      Serial.println("Timeout Wifi Config!");
      flag_wifi_config = 0;
      flag_wifi_config_ = 1;
      flag_begin_wifi = 0;
    } else {
      // nhấn giữ 3 giây để thoát chế độ wifi config
      rst_wifi = RST_WIFI();
      if (rst_wifi != last_rst_wifi && rst_wifi == 1) {
        millis_rst_wifi = millis();
      }
      last_rst_wifi = rst_wifi;
      if (rst_wifi == 1 && ((unsigned long)(millis() - millis_rst_wifi) >= 3000)) {
        millis_rst_wifi = millis();
        WiFi.softAPdisconnect(true);
        Serial.println("End Wifi Config!");
        flag_wifi_config = 0;
        flag_wifi_config_ = 1;
        flag_begin_wifi = 0;
      }
    }
  } else {
    // Chường trình chức năng chính nếu không vào wifi config
    processing_main();
  }
}
void processing_main(void) {
  // Nếu kết nối wifi thành công và kết nối MQTT thành công
  if (WiFi.status() == WL_CONNECTED && flag_begin_wifi == 0 && client_mqtt.connected()) {
    client_mqtt.loop();
    // Viết chương trình xử lý logic khi online tại đây
    heart_beat_led();
    funtion_button();
  } else {
    reconnect_wifi();
  }
}
byte MQTT_connection(void) {
  int i = 0;
  while (!client_mqtt.connected()) {

    char DEVICE_NAME_CH[DEVICE_NAME_.length()];
    DEVICE_NAME_.toCharArray(DEVICE_NAME_CH, DEVICE_NAME_.length() + 1);
    DEVICE_NAME = DEVICE_NAME_CH;

    char MQTT_USER_CH[MQTT_USER_.length()];
    MQTT_USER_.toCharArray(MQTT_USER_CH, MQTT_USER_.length() + 1);
    MQTT_USER = MQTT_USER_CH;

    char MQTT_PASS_CH[MQTT_PASS_.length()];
    MQTT_PASS_.toCharArray(MQTT_PASS_CH, MQTT_PASS_.length() + 1);
    MQTT_PASS = MQTT_PASS_CH;

    char MQTT_LASTWILL_TOPIC_CH[MQTT_LASTWILL_TOPIC_STR.length()];
    MQTT_LASTWILL_TOPIC_STR.toCharArray(MQTT_LASTWILL_TOPIC_CH, MQTT_LASTWILL_TOPIC_STR.length() + 1);
    MQTT_LASTWILL_TOPIC = MQTT_LASTWILL_TOPIC_CH;

    Serial.println("MQTT connecting......");
    if (client_mqtt.connect(DEVICE_NAME, MQTT_USER, MQTT_PASS, MQTT_LASTWILL_TOPIC, willQoS, willRetain, willMessage)) {
      Serial.println("connected");

      char MQTT_SUB_DATA_TOPIC_CH[MQTT_SUB_DATA_TOPIC_STR.length()];
      MQTT_SUB_DATA_TOPIC_STR.toCharArray(MQTT_SUB_DATA_TOPIC_CH, MQTT_SUB_DATA_TOPIC_STR.length() + 1);
      MQTT_SUB_DATA_TOPIC = MQTT_SUB_DATA_TOPIC_CH;
      client_mqtt.subscribe(MQTT_SUB_DATA_TOPIC);
    } else {
      flag_wifi_mqtt = 1;
      return 0;
      /*
        Serial.print("failed, status code =");
        Serial.println(client_mqtt.state());
        i = i + 1;
        if (i >= 2)
        {
        flag_wifi_mqtt = 1;
        return 0;
        }*/
    }
  }
  flag_wifi_mqtt = 0;
  return 1;
}
void receivedCallback(char *topic, byte *payload, unsigned int length) {
  String mqtt_input = "";
  String topic_str = String(topic);
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    mqtt_input = mqtt_input + (char)payload[i];
  }
  Serial.println(mqtt_input);
  if (topic_str == MQTT_SUB_DATA_TOPIC_STR) {
    Serial.print("Data: ");
    Serial.println(mqtt_input);
    char mqtt_input_char[length];
    mqtt_input.toCharArray(mqtt_input_char, length + 1);
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, mqtt_input_char);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    if (doc.containsKey("out_01")) {
      byte data_out_01 = doc["out_01"];
      Serial.println(data_out_01);
      out_01_fn(data_out_01);
    }
    if (doc.containsKey("out_02")) {
      byte data_out_02 = doc["out_02"];
      Serial.println(data_out_02);
      out_02_fn(data_out_02);
    }
    if (doc.containsKey("auto_mode")) {
      flag_auto_mode = doc["auto_mode"];
      Serial.println(flag_auto_mode);
    }
  }
}
void Status_Online(void) {
  String json_pub = "online";
  int length_json = json_pub.length();
  char Buf[length_json];
  json_pub.toCharArray(Buf, length_json + 1);
  Serial.println(Buf);
  Serial.println("");

  char MQTT_LASTWILL_TOPIC_CH[MQTT_LASTWILL_TOPIC_STR.length()];
  MQTT_LASTWILL_TOPIC_STR.toCharArray(MQTT_LASTWILL_TOPIC_CH, MQTT_LASTWILL_TOPIC_STR.length() + 1);
  MQTT_LASTWILL_TOPIC = MQTT_LASTWILL_TOPIC_CH;
  client_mqtt.publish(MQTT_LASTWILL_TOPIC, Buf);
}
void led_debug_on(void) {
  return digitalWrite(led_debug, HIGH);
}
void led_debug_off(void) {
  return digitalWrite(led_debug, LOW);
}
void heart_beat_led(void) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(led_debug, !digitalRead(led_debug));
  }
}
void led_problem_disconnected_wifi(void) {
  unsigned long currentMillis_problem_wifi = millis();
  if (currentMillis_problem_wifi - previousMillis_problem_wifi >= interval_problem_wifi) {
    previousMillis_problem_wifi = currentMillis_problem_wifi;
    digitalWrite(led_debug, !digitalRead(led_debug));
  }
}
void out_01_fn(byte status) {
  digitalWrite(out_01, status);
}
void out_02_fn(byte status) {
  digitalWrite(out_02, status);
}
void funtion_button(void) {
  // đọc giá trị hiện tại của button
  buttonState = digitalRead(button_config_wifi);

  // so sánh với giá trị trước đó
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      // Nếu trạng thái bây giờ là button đang được nhấn
      // thì hiển nhiên trước đó là button chưa được nhấn (điều kiện trên)
      // chúng ta sẽ tăng số lần nhấn button lên 1
      buttonPushCounter++;
      Serial.println("Dang nhan");
      Serial.print("So lan nhan button la: ");
      Serial.println(buttonPushCounter);
      previousMillis_button = millis();
    } else {
      // Nếu trạng thái bây giờ là button đang được THẢ
      // thì hiển nhiên trước đó là button đang được nhấn (điều kiện trên)
      // Chúng ta sẽ thông báo là button đang được thả và không làm gì cả
      Serial.println("off");
    }
  }
  // lưu lại trạng thái button cho lần kiểm tra tiếp theo
  lastButtonState = buttonState;
  // nếu quá thời gian thì số lần nhấn nút sẻ reset lại về 0
  if (millis() - previousMillis_button >= button_expiration_time) {
    if (buttonPushCounter == 3) {
      // on/off auto mode
      Serial.println("on/off auto mode");
      state_auto_mode(!flag_auto_mode);
      flag_auto_mode = !flag_auto_mode;
    } else if (buttonPushCounter == 2) {
      // Triggle uotput 2
      Serial.println("Triggle uotput 2");
      if(!flag_auto_mode)out_02_fn(!digitalRead(out_02));
    } else if (buttonPushCounter == 1 && buttonState == HIGH) {
      // Triggle uotput 1
      Serial.println("Triggle uotput 1");
      if(!flag_auto_mode)out_01_fn(!digitalRead(out_01));
    }
    buttonPushCounter = 0;
  }
}
void state_auto_mode(byte state_auto_mode) {
  String json_pub = "{\"auto_mode_device\":" + String(state_auto_mode) + "}";
  int length_json = json_pub.length();
  char Buf[length_json];
  json_pub.toCharArray(Buf, length_json + 1);
  Serial.println(Buf);
  Serial.println("");

  char MQTT_PUB_DATA_TOPIC_CH[MQTT_PUB_DATA_TOPIC_STR.length()];
  MQTT_PUB_DATA_TOPIC_STR.toCharArray(MQTT_PUB_DATA_TOPIC_CH, MQTT_PUB_DATA_TOPIC_STR.length() + 1);
  MQTT_PUB_DATA_TOPIC = MQTT_PUB_DATA_TOPIC_CH;
  client_mqtt.publish(MQTT_PUB_DATA_TOPIC, Buf);
}
