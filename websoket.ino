#define BLYNK_TEMPLATE_ID "TMPL6AzTEYg2R"
#define BLYNK_TEMPLATE_NAME "parking"
#define BLYNK_AUTH_TOKEN "nMe8OJCSLdtMR9g0yI5mp8-1Ta5tYJve"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>  // Ensure this is installed correctly
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>  // Include Blynk library
#include <WebSocketsServer.h>

// Blynk Auth Token, WiFi credentials
char auth[] = "nMe8OJCSLdtMR9g0yI5mp8-1Ta5tYJve";  // Replace with your Blynk auth token
char ssid[] = "P5";       // Replace with your Wi-Fi SSID
char pass[] = "908765432";   // Replace with your Wi-Fi password

// Define pins for ESP32
#define ir_enter 12  // Sensor for car entry detection
// 1 ir_enter
#define ir_car1 14   // Sensor for parking slot 1
#define ir_car2 27   // Sensor for parking slot 2
#define ir_car3 26   // Sensor for parking slot 3
#define ir_car4 25   // Sensor for parking slot 4
// 4 ir_car 
#define servo_pin 13 // Servo control pin
//them 1 servo

int slot1Status = 0;
int slot2Status = 0;
int slot3Status = 0;
int slot4Status = 0; 
// 4 slots
//
int total_slot = 4;
int available_slots = 4; 
// 8

int getSensorPin(int index) {
  switch (index) {
    case 0: return ir_car1;
    case 1: return ir_car2;
    case 2: return ir_car3;
    case 3: return ir_car4;
    default: return -1;
  }
}
//THÊM THÀNH 8

bool carEntered = false; // Biến cờ để theo dõi trạng thái của cảm biến ir_enter
bool slot_status[4] = {false, false, false, false}; // Track the status of each parking slot 
//THÀNH 8
bool slotStatus[4]; // Adjust size based on your needs  
//THÀNH 8

const int virtual_pins[] = {V1, V2, V3, V4}; 
//THÊM 4 V

Servo myservo; // Use ESP32Servo library
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  // Xử lý sự kiện WebSocket tại đây nếu cần
}
void setup() {
  myservo.attach(servo_pin);
  lcd.init();
  lcd.begin(16,2);
  lcd.backlight();
  lcd.clear();
  pinMode(ir_enter, INPUT);
  //pinMode(ir_enter_2, INPUT);
  pinMode(ir_car1, INPUT);
  pinMode(ir_car2, INPUT);
  pinMode(ir_car3, INPUT);
  pinMode(ir_car4, INPUT);
  //8

  lcd.setCursor(0, 0);
  lcd.print("Parking System");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Occupied: ");
  lcd.setCursor(0, 1);
  lcd.print("Available: ");
  // Blynk Setup
  Blynk.begin(auth, ssid, pass);
  Serial.begin(115200);
    WiFi.begin(ssid, pass);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
  Serial.println("Connected to WiFi!");
  
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());  // In địa chỉ IP của ESP32 lên cổng Serial
  //
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

}


void loop() {
  Blynk.run();  // Run Blynk
  webSocket.loop();  // Xử lý các sự kiện WebSocket
  
// Kiểm tra tín hiệu từ cảm biến ir_enter_1
  if (digitalRead(ir_enter) == HIGH && !carEntered) {  
    // Nếu có tín hiệu và xe chưa được ghi nhận là đã vào
    carEntered = true;  // Cập nhật trạng thái xe đã vào
    myservo.write(90);  // Mở servo
    delay(1000);  // Đợi một thời gian để xe đi qua
    myservo.write(0);   // Đóng servo sau khi mở
  } 
  else if (digitalRead(ir_enter) == LOW && carEntered) {
    // Nếu cảm biến không có tín hiệu và trước đó xe đã vào
    carEntered = false;  // Reset trạng thái
  }

  // Kiểm tra tín hiệu từ cảm biến ir_enter_2
  /*if (digitalRead(ir_enter_2) == HIGH && !carEntered) {  
    // Nếu có tín hiệu và xe chưa được ghi nhận là đã vào
    carEntered = true;  // Cập nhật trạng thái xe đã vào
    myservo.write(90);  // Mở servo
    delay(1000);  // Đợi một thời gian để xe đi qua
    myservo.write(0);   // Đóng servo sau khi mở
  } 
  else if (digitalRead(ir_enter_2) == LOW && carEntered) {
    // Nếu cảm biến không có tín hiệu và trước đó xe đã vào
    carEntered = false;  // Reset trạng thái
  }
  */

  //checkParkingStatus();
  // Update parking slot status
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
    bool isOccupied = digitalRead(sensor_pin) == HIGH;
    
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

