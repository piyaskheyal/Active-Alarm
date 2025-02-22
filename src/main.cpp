#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

int alarmHr = 19;
int alarmMin = 0;

// Wi-Fi credentials
const char* ssid     = "Robo Sapiens ";
const char* password = "2Bornot2B";

// NTP settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 6*3600;  // Change according to your time zone
const int   daylightOffset_sec = 0;  // Adjust for daylight saving time

#define EN1 25
#define IN1 27
#define IN2 26

#define PWM_CHANNEL 0 // Use channel 0
#define PWM_FREQ 5000 // 5 kHz frequency
#define PWM_RES 8     // 8-bit resolution (0-255)

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    
    // Wait for Wi-Fi to connect
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print("#");
    }
    Serial.println("\nConnected to WiFi");

    // Synchronize time with NTP server
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES); // Configure PWM
    ledcAttachPin(EN1, PWM_CHANNEL);       // Attach pin to PWM channel
    
    Serial.print("Current time: ");
    Serial.println(asctime(&timeinfo));
}

void loop() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        Serial.printf("Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        if(timeinfo.tm_hour == alarmHr && timeinfo.tm_min == alarmMin){
            while(timeinfo.tm_min <= alarmMin + 2){
                Serial.println("Alarm!!!");
                digitalWrite(IN1, LOW);
                digitalWrite(IN2, HIGH);
                ledcWrite(PWM_CHANNEL, 255);
            }
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, LOW);
        }
    }
    delay(1000); // Update every second
}
