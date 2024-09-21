#include <WiFi.h>
#include <WebSocketsClient.h>

const char* ssid = "B508_2point4Ghz";           // Thay bằng SSID WiFi của bạn
const char* password = "07012002";   // Thay bằng password WiFi của bạn

WebSocketsClient webSocket;
int sensorPin3 = 26; // Cổng analog đọc giá trị cảm biến
int sensorPin4 = 25;
int sensorValue3 = 0;
int sensorValue4 = 0;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("Connected to server");
      break;
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from server");
      break;
    case WStype_TEXT:
      Serial.printf("Message received: %s\n", payload);
      break;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin3, INPUT);
  pinMode(sensorPin4, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Kết nối tới WebSocket Server (ESP32 chính)
  webSocket.begin("192.168.1.8", 81, "/"); // Thay "IP_ESP32_server" bằng địa chỉ IP của ESP32 chính
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();

  // Đọc giá trị cảm biến
  sensorValue3 = digitalRead(sensorPin3);
  sensorValue4 = digitalRead(sensorPin4);

  // Gửi giá trị cảm biến 3 và 4 dưới dạng một chuỗi, phân cách bằng dấu phẩy
  String sensorData = String(sensorValue3) + "," + String(sensorValue4);
  webSocket.sendTXT(sensorData);

  delay(500); // Gửi mỗi giây một lần
}
