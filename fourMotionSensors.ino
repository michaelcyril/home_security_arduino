#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Include the I2C LCD library

// WiFi credentials
const char* ssid = "Mike";
const char* password = "mike7187";

// Pin definitions
const int pirPin1 = D0;  // Digital pin for PIR sensor 1
const int pirPin2 = D4;  // Digital pin for PIR sensor 2
const int pirPin3 = D7;  // Digital pin for PIR sensor 3
const int pirPin4 = D8;  // Digital pin for PIR sensor 4
// const int buzzerPin = A0;  // Digital pin for Buzzer
const int buzzerPin = D3;  // Digital pin for Buzzer

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Replace with your LCD I2C address if different

// WiFi server details
const char* serverName = "192.168.206.193";  // Replace with your server IP address
const int serverPort = 8000;                 // HTTP port

// Initialize WiFi client
WiFiClient client;

// Motion status array
int motionStatus[4] = { 0, 0, 0, 0 };

void setup() {
  Serial.begin(115200);
  lcd.init();       // Initialize the LCD
  lcd.backlight();  // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");  // Print initializing message
  delay(2000);                   // Wait 2 seconds
  lcd.clear();                   // Clear screen

  pinMode(pirPin1, INPUT);
  pinMode(pirPin2, INPUT);
  pinMode(pirPin3, INPUT);
  pinMode(pirPin4, INPUT);
  pinMode(buzzerPin, OUTPUT);


  // Allow PIR sensors to calibrate
  lcd.setCursor(0, 0);
  lcd.print("Calibrating PIR...");
  delay(1000);  // 30 seconds warm-up time for PIR sensors

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
      sendNotification(String(i + 1), 1);
    }
    if (anyMotion) {
      sendNormalAlarmStatus(1);
    } else {
      sendNormalAlarmStatus(0);
    }
  }

  // Check if the alarm is on or off
  if (client.connect(serverName, serverPort)) {
    Serial.println("Checking Alarm Status");

    // Send HTTP GET request
    client.println(String("GET /create-get-alarm HTTP/1.1"));
    client.println("Host: " + String(serverName));
    client.println("Connection: close");
    client.println();

    // Wait for server response
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\r');
        response += line;
      }
    }
    client.stop();
    // Serial.println("GET request sent");
    int splitIndex_status = response.indexOf('*');
    String statusStr_status = response.substring(splitIndex_status + 1);
    int status = statusStr_status.toInt();

    // Serial.println("GET request sent");
    int splitIndex_normal_status = response.indexOf('=');
    String statusStr_normal_status = response.substring(splitIndex_normal_status + 1);
    int normal_status = statusStr_normal_status.toInt();

    // Process the response
    Serial.println("Response ===============:");
    Serial.println("status_force: " + String(status));
    Serial.println("status_normal: " + String(normal_status));

    if (status == 0) {
      // If status is 1, trigger the alarm regardless of normal_status
      anyMotion = false;
    } else if (status == 1) {
      // If status is 0, check the condition of normal_status
      if (normal_status == 1) {
        anyMotion = true;
      } else {
        anyMotion = false;
      }
    }

  } else {
    // Serial.println("Failed to connect to server for GET request");
  }


  if (anyMotion) {
    digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
  } else {
    digitalWrite(buzzerPin, LOW);  // Turn off the buzzer
  }

  // Update the LCD display with the status of all doors
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("D1:");
  lcd.print(getDoorReport(motionStatus[0]));
  lcd.print(" D2:");
  lcd.print(getDoorReport(motionStatus[1]));
  lcd.setCursor(0, 1);
  lcd.print("D3:");
  lcd.print(getDoorReport(motionStatus[2]));
  lcd.print(" D4:");
  lcd.print(getDoorReport(motionStatus[3]));

  delay(3000);  // Delay for stability
}

String getDoorReport(int value) {
  if (value == 1) {
    return "Atmp";
  } else {
    return "NAtmp";
  }
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

void sendNotification(String pir, int status) {
  String jsonData = pir + "_" + String(status);
  httpPost(serverName, serverPort, "/create-get-intruder-attempt", jsonData);
}

void sendNormalAlarmStatus(int status) {
  String jsonData = "normal_" + String(status);
  httpPost(serverName, serverPort, "/create-get-intruder-attempt", jsonData);
}



void httpGet(const char* server, int port, String path) {
  if (client.connect(server, port)) {
    Serial.println("Connected to server for GET request");

    // Send HTTP GET request
    client.println("GET " + path + " HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Connection: close");
    client.println();

    // Wait for server response
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }
    }
    client.stop();
    Serial.println("GET request sent");
  } else {
    Serial.println("Failed to connect to server for GET request");
  }
}

void httpPost(const char* server, int port, String path, String postData) {
  if (client.connect(server, port)) {
    // Serial.println("Connected to server for POST request");

    // Send HTTP POST request
    client.println("POST " + path + " HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);

    // Wait for server response
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\r');
        // Serial.print(line);
      }
    }
    client.stop();
    // Serial.println("POST request sent");
  } else {
    // Serial.println("Failed to connect to server for POST request");
  }
}
