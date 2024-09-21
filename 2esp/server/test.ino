#define BLYNK_TEMPLATE_ID "TMPL6AzTEYg2R"
#define BLYNK_TEMPLATE_NAME "parking"
#define BLYNK_AUTH_TOKEN "nMe8OJCSLdtMR9g0yI5mp8-1Ta5tYJve"


#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>  // Ensure this is installed correctly
#include <Wire.h>
#include <BlynkSimpleEsp32.h>  // Include Blynk library
#include <WebSocketsServer.h>

char auth[] = "nMe8OJCSLdtMR9g0yI5mp8-1Ta5tYJve";
const char* ssid = "B508_2point4Ghz";           // Thay bằng SSID WiFi của bạn
const char* password = "07012002";  // Thay bằng password WiFi của bạn

const int ir_enter = 5;
const int servoPin = 18;
// // Tạo đối tượng Servo
Servo gateServo;

#define sensorpin1 14   // Sensor for parking slot 1
#define sensorpin2 27   // Sensor for parking slot 2

WebSocketsServer webSocket = WebSocketsServer(81); // Sử dụng cổng 81 để WebSocket hoạt động

int ir_car1 = 0;
int ir_car2 = 0;
int ir_car3 = 0;
int ir_car4 = 0;

int slot1Status = 0;
int slot2Status = 0;
int slot3Status = 0;
int slot4Status = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2); 

int total_slot = 4;
int available_slots = 4;

int getSensorPin(int index) {
  switch (index) {
    case 0: return ir_car1;
    case 1: return ir_car2;
    case 2: return ir_car3;
    case 3: return ir_car4;
    default: return -1;
  }
}

bool gateOpen = false; // Biến trạng thái cổng (đóng hoặc mở)
int sensorState = 0;

bool slot_status[4] = {false, false, false, false}; // Track the status of each parking slot
const int virtual_pins[] = {V1, V2, V3, V4};

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[%u] Connected\n", num);
      break;
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected\n", num);
      break;
    case WStype_TEXT:

      Serial.printf("Received from client [%u]: %s\n", num, payload);

      // Xử lý dữ liệu nhận được
      String message = String((char*)payload); // Chuyển dữ liệu thành chuỗi

      int commaIndex = message.indexOf(',');
      String value3Str = message.substring(0, commaIndex);
      String value4Str = message.substring(commaIndex + 1);

      // Chuyển chuỗi thành giá trị số nguyên
      ir_car3 = value3Str.toInt();
      ir_car4 = value4Str.toInt();
      
      // In ra giá trị cảm biến nhận được
      Serial.print("Sensor 3 value received: ");
      Serial.println(ir_car3);
      Serial.print("Sensor 4 value received: ");
      Serial.println(ir_car4);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);

  pinMode(ir_enter, INPUT);
  pinMode(sensorpin1, INPUT);
  pinMode(sensorpin2, INPUT);

  gateServo.attach(servoPin);   // Gán chân điều khiển servo
  gateServo.write(0); // Đặt servo về vị trí đóng (0 độ)

  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Parking System");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Occupied: ");
  lcd.setCursor(0, 1);
  lcd.print("Available: ");



  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());  // In địa chỉ IP của ESP32 lên cổng Serial

  // Bắt đầu WebSocket Server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  Blynk.run();  // Run Blynk
  webSocket.loop();

  ir_car1 = digitalRead(sensorpin1);
  ir_car2 = digitalRead(sensorpin2);

  sensorState = digitalRead(ir_enter);
  //Serial.println(sensorState);

  if (!gateOpen && sensorState == LOW) {
    // Mở cổng (cho servo quay đến 90 độ)
    Serial.println("Object detected! Opening gate...");
    gateServo.write(90);
    gateOpen = true; // Cập nhật trạng thái cổng mở
  }

  // Kiểm tra nếu cổng đang mở và cảm biến không còn phát hiện vật thể nữa
  if (gateOpen && sensorState == HIGH) {
    // Đóng cổng lại (servo quay về 0 độ)
    Serial.println("No object detected! Closing gate...");
    gateServo.write(0);
    gateOpen = false; // Cập nhật trạng thái cổng đóng
  }

  updateSlotStatus();

  // Display the available slots on LCD
  lcd.setCursor(10, 0);
  lcd.print("    "); // Clear the previous value
  
  lcd.setCursor(10, 0);
  lcd.print(available_slots);

  //
  lcd.setCursor(10, 1);
  lcd.print("    "); // Clear the previous value
  
  lcd.setCursor(10, 1);
  lcd.print(4 - available_slots);

}

void updateSlotStatus() {
  for (int i = 0; i < 4; i++) {
    int sensor_pin = getSensorPin(i);
    bool isOccupied = sensor_pin == HIGH;
    
    if (isOccupied && !slot_status[i]) {
      slot_status[i] = true;
      if (available_slots >0) available_slots--;
      //available_slots_1 ++;
      Blynk.virtualWrite(virtual_pins[i], 0);  // Tắt LED (chỗ đỗ đã kín)
      sendWebSocketUpdate();
    } else if (!isOccupied && slot_status[i]) {
      slot_status[i] = false;
      if (available_slots < 4) available_slots++;
      //available_slots_1 --;
      Blynk.virtualWrite(virtual_pins[i], 255);  // Bật LED (chỗ đỗ còn trống)
      sendWebSocketUpdate();
    }
  }
}

void sendWebSocketUpdate() {
  String message = String(available_slots) + "," + String(4 - available_slots);
  webSocket.broadcastTXT(message);  // Broadcast update to all connected clients
}