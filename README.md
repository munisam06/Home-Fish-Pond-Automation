# IoT Based Home Fish Pond Automation

 # Overview
 The Smart Fish Pond Automation system is an IoT-based solution designed to automate the management of a fish pond. It uses sensors, actuators, and machine learning   
 to monitor water levels, control a water pump, and manage fish feeding schedules. The system is powered by an ESP32 microcontroller and integrates with the V-One 
 AIoT Cloud platform for real-time monitoring and data analysis.


# Components
 ## Hardware
ESP32 Microcontroller
Rain Sensor
SG90 Servo Motor
Relay Module
Micro-submersible Water Pump
Push Button
LED

 ## Software
Programming Language: C++ (Arduino Framework)
Cloud Platform: V-One AIoT Cloud for telemetry and analytics

 ### Libraries Used:
VOneMqttClient.h (for V-One cloud communication)
ESP32Servo.h (for servo motor control)


# Installation
 1. Open the project in the Arduino IDE
 2. Install required libraries
 3. Configure the following in the VOneMqttClient.h file

  #define WIFI_SSID "Your_WiFi_Name"
  #define WIFI_PASSWORD "Your_WiFi_Password"

  V-One Cloud Device IDs: Replace the placeholders with your actual device IDs from the V-One 
 Cloud portal.

4. Upload the code to your ESP32.




