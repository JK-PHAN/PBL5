#define BLYNK_TEMPLATE_ID "TMPL6AzTEYg2R"
#define BLYNK_TEMPLATE_NAME "parking"
#define BLYNK_AUTH_TOKEN "nMe8OJCSLdtMR9g0yI5mp8-1Ta5tYJve"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WebSocketsClient.h>

// WiFi credentials
char ssid[] = "P5";       
char pass[] = "908765432"; 

WebSocketsClient webSocket;

int ir_car1 = 14;   // Sensor for parking slot 1
int ir_car2 = 27;   // Sensor for parking slot 2

int available_slots = 2;  // Initially all slots are available
int occupied_slots = 0;

// Blynk virtual pins for slot status
const int virtual_pins[] = {V1, V2};

bool slot_status[2] = {false, false}; // Track the status of each parking slot

// WebSocket event handling
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  // WebSocket event handling, if needed for client
}

// Update parking slot status
void updateSlotStatus() {
  for (int i = 0; i < 2; i++) {
    int sensor_pin = (i == 0) ? ir_car1 : ir_car2;
    bool isOccupied = digitalRead(sensor_pin) == HIGH;

    if (isOccupied && !slot_status[i]) {
      slot_status[i] = true;
      occupied_slots++;
      available_slots--;
      Blynk.virtualWrite(virtual_pins[i], 0);  // Slot occupied
    } else if (!isOccupied && slot_status[i]) {
      slot_status[i] = false;
      occupied_slots--;
      available_slots++;
      Blynk.virtualWrite(virtual_pins[i], 255);  // Slot available
    }
  }
  
  // Send status to server
  String message = String(available_slots) + "," + String(occupied_slots);
  webSocket.sendTXT(message);
}

void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(ir_car1, INPUT);
  pinMode(ir_car2, INPUT);

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Blynk setup
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // WebSocket client setup
  webSocket.begin("192.168.1.100", 81, "/");  // Replace with the IP of your server ESP32
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  Blynk.run();           // Blynk tasks
  webSocket.loop();      // WebSocket client tasks
  updateSlotStatus();    // Update parking slot status
  delay(1000);           // Update interval (adjust as needed)
}
