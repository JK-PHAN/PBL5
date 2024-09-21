#define BLYNK_TEMPLATE_ID "TMPL6AzTEYg2R"
#define BLYNK_TEMPLATE_NAME "parking"
#define BLYNK_AUTH_TOKEN "nMe8OJCSLdtMR9g0yI5mp8-1Ta5tYJve"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WebSocketsServer.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
char ssid[] = "P5";       
char pass[] = "908765432"; 

WebSocketsServer webSocket = WebSocketsServer(81);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Initialize the LCD

int ir_car3 = 14;  // Sensor for parking slot 3
int ir_car4 = 27;  // Sensor for parking slot 4

int available_slots_server = 2;
int occupied_slots_server = 0;

int available_slots_client = 0;
int occupied_slots_client = 0;

const int total_slots = 4;  // Total parking slots (2 for server, 2 for client)

bool slot_status[2] = {false, false}; // Track the status of server parking slots

// WebSocket event handling
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    // Parse message from client (format: "available_slots,occupied_slots")
    String message = String((char *)payload).substring(0, length);
    int commaIndex = message.indexOf(',');
    available_slots_client = message.substring(0, commaIndex).toInt();
    occupied_slots_client = message.substring(commaIndex + 1).toInt();

    Serial.print("Client Available Slots: ");
    Serial.println(available_slots_client);
    Serial.print("Client Occupied Slots: ");
    Serial.println(occupied_slots_client);

    updateLCD();
  }
}

// Update parking slot status
void updateSlotStatus() {
  for (int i = 0; i < 2; i++) {
    int sensor_pin = (i == 0) ? ir_car3 : ir_car4;
    bool isOccupied = digitalRead(sensor_pin) == HIGH;

    if (isOccupied && !slot_status[i]) {
      slot_status[i] = true;
      occupied_slots_server++;
      available_slots_server--;
      Blynk.virtualWrite(i + V3, 0);  // Slot occupied
    } else if (!isOccupied && slot_status[i]) {
      slot_status[i] = false;
      occupied_slots_server--;
      available_slots_server++;
      Blynk.virtualWrite(i + V3, 255);  // Slot available
    }
  }
}

// Update LCD with aggregated data
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Available: ");
  lcd.print(available_slots_client + available_slots_server);

  lcd.setCursor(0, 1);
  lcd.print("Occupied: ");
  lcd.print(occupied_slots_client + occupied_slots_server);
}

void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(ir_car3, INPUT);
  pinMode(ir_car4, INPUT);

  // LCD initialization
  lcd.init();
  lcd.backlight();

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  Serial.print("Server IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Blynk setup
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);


  // WebSocket server setup
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  Blynk.run();          // Blynk tasks
  webSocket.loop();     // WebSocket server tasks
  updateSlotStatus();   // Update parking slot status
  delay(1000);          // Update interval (adjust as needed)
}
