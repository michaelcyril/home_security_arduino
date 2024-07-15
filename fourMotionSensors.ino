#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Include the I2C LCD library

// WiFi credentials
const char* ssid = "Mike";
const char* password = "mike7187";

// Pin definitions
const int pirPin1 = D0;       // Digital pin for PIR sensor 1
const int pirPin2 = D4;       // Digital pin for PIR sensor 2
const int pirPin3 = D7;       // Digital pin for PIR sensor 3
const int pirPin4 = D8;       // Digital pin for PIR sensor 4
const int buzzerPin = A0;     // Digital pin for Buzzer

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Replace with your LCD I2C address if different

// WiFi server details
const char* serverName = "192.168.206.193";  // Replace with your server IP address
const int serverPort = 8000;                // HTTP port

// Initialize WiFi client
WiFiClient client;

// Motion status array
int motionStatus[4] = {0, 0, 0, 0};

void setup() {
  Serial.begin(115200);
  lcd.init();                      // Initialize the LCD
  lcd.backlight();                 // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");    // Print initializing message
  delay(2000);                     // Wait 2 seconds
  lcd.clear();                     // Clear screen
  
  pinMode(pirPin1, INPUT);
  pinMode(pirPin2, INPUT);
  pinMode(pirPin3, INPUT);
  pinMode(pirPin4, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  connectWiFi();  // Connect to WiFi
}

void loop() {
  // Read sensor values
  motionStatus[0] = digitalRead(pirPin1);
  motionStatus[1] = digitalRead(pirPin2);
  motionStatus[2] = digitalRead(pirPin3);
  motionStatus[3] = digitalRead(pirPin4);

  // Check if any sensor detects motion
  bool anyMotion = false;
  for (int i = 0; i < 4; i++) {
    if (motionStatus[i] == HIGH) {
      anyMotion = true;
      sendNotification("Motion detected at Door " + String(i + 1));
    }
  }

  if (anyMotion) {
    digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
  } else {
    digitalWrite(buzzerPin, LOW);   // Turn off the buzzer
  }

  // Update the LCD display with the status of all doors
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("D1:");
  lcd.print(motionStatus[0]);
  lcd.print("    D2:");
  lcd.print(motionStatus[1]);
  lcd.setCursor(0, 1);
  lcd.print("D3:");
  lcd.print(motionStatus[2]);
  lcd.print("    D4:");
  lcd.print(motionStatus[3]);

  delay(1000);  // Delay for stability
}

void connectWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi...");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Connected to WiFi");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected to ");
  lcd.print(WiFi.SSID());
  lcd.setCursor(0, 1);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());
  delay(2000);  // Display WiFi status for 2 seconds
}

void sendNotification(String message) {
  if (client.connect(serverName, serverPort)) {
    Serial.println("Connected to server");
    
    // Construct POST data
    String postData = "message=" + message;  // Example data to send
    
    client.println("POST / HTTP/1.1");
    client.println("Host: " + String(serverName));
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);
    
    Serial.println("Notification sent: " + message);
    delay(500);  // Allow server to process
    
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();
  } else {
    Serial.println("Failed to connect to server");
  }
}
