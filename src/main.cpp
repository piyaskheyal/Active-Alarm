#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <WebServer.h>

// Global alarm time variables
int alarmHr = 19;
int alarmMin = 0;

// Wi-Fi credentials
const char* ssid     = "Robo Sapiens ";
const char* password = "2Bornot2B";

// NTP settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 6 * 3600;  // For UTC+6
const int   daylightOffset_sec = 0;    // No DST

// Motor and PWM definitions
#define EN1 25
#define IN1 27
#define IN2 26

#define PWM_CHANNEL 0 // Use channel 0
#define PWM_FREQ 5000 // 5 kHz frequency
#define PWM_RES 8     // 8-bit resolution (0-255)

// Create a web server on port 80
WebServer server(80);

// Serve the homepage with the alarm-setting form
void handleRoot() {
    String html = "<html><head><title>Set Alarm</title></head><body>";
    html += "<h1>Set Alarm</h1>";
    html += "<form action='/set' method='get'>";
    html += "Alarm Hour (0-23): <input type='number' name='hr' min='0' max='23'><br>";
    html += "Alarm Minute (0-59): <input type='number' name='min' min='0' max='59'><br>";
    html += "<input type='submit' value='Set Alarm'>";
    html += "</form>";
    html += "<p>Current Alarm: " + String(alarmHr) + ":" + String(alarmMin) + "</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

// Handle the form submission and update the alarm time
void handleSet() {
  if (server.hasArg("hr") && server.hasArg("min")) {
    alarmHr = server.arg("hr").toInt();
    alarmMin = server.arg("min").toInt();
    String response = "<html><body>";
    response += "Alarm set to: " + String(alarmHr) + ":" + String(alarmMin) + "<br>";
    response += "<a href='/'>Go Back</a>";
    response += "</body></html>";
    server.send(200, "text/html", response);
  } else {
    server.send(400, "text/html", "Bad Request");
  }
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    // Connect to Wi-Fi
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("#");
    }
    Serial.println("\nConnected to WiFi");

    // Start the web server and set up the URL handlers
    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.begin();
    Serial.println("Web server started");

    // Synchronize time with NTP server
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
    }

    // Initialize motor control pins
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    // Setup PWM for motor control
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(EN1, PWM_CHANNEL);

    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());


    Serial.print("Current time: ");
    Serial.println(asctime(&timeinfo));
}

void loop() {
    // Handle incoming HTTP requests
    server.handleClient();
    
    // Get the current time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        Serial.printf("Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        
        // Use static variables to manage the alarm state without blocking
        static bool alarmActive = false;
        static unsigned long alarmStartMillis = 0;
        
        // Check if current time matches the alarm time
        if (!alarmActive && timeinfo.tm_hour == alarmHr && timeinfo.tm_min == alarmMin) {
        alarmActive = true;
        alarmStartMillis = millis();
        Serial.println("Alarm starting!");
        }
        
        // If the alarm is active, run it for 2 minutes
        if (alarmActive) {
        if (millis() - alarmStartMillis < 2 * 60 * 1000) {
            Serial.println("Alarm!!!");
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, HIGH);
            ledcWrite(PWM_CHANNEL, 255);
        } else {
            // Turn off the alarm after 2 minutes
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, LOW);
            ledcWrite(PWM_CHANNEL, 0);
            alarmActive = false;
            Serial.println("Alarm ended.");
        }
        }
    }
    
    delay(1000); // Update every second
}
