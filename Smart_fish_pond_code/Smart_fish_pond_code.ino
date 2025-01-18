#include "VOneMqttClient.h"
#include <ESP32Servo.h>

// V-One Cloud Device IDs (Ensure Correct IDs from V-One Cloud Portal)
const char* ServoMotor = "9e5a08f2-a9d4-4594-b15a-a84f36fe52d6"; 
const char* WaterLevelSensor = "9d816adf-acbb-4e45-b071-f57c1ed0a5c3"; 
const char* PumpRelay = "ab71ab10-3ecb-4b6b-8d75-3520cb9d42c1";  
const char* ButtonSensor = "49a2e6c6-e4d9-463a-b9c3-d184e40fb1e3";  
const char* LEDIndicator = "a52187fe-78bd-414c-8737-1571e3dcbefc";     

// Pin Assignments
const int servoPin = 5;  
const int relayPin = 38;     
const int buttonPin = 39;    
const int ledGreen = 7;      
const int rainSensorPin = 4;  

unsigned long lastMsgTime = 0;


// Water Threshold and Timing
const int waterThreshold = 3000; 
const int pumpDuration = 5000;   
const unsigned long feedingInterval = 120000; 

// State Variables
bool pumpActive = false;
bool greenLEDState = false;
bool relayPumpState = false;
int servoPosition = 0;

// Servo Motor Instance
Servo feederServo;


bool buttonPressed = false;
bool isMoving = false;
unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 200;     
unsigned long servoMoveStartTime = 0;
const unsigned long feedingInterval = 120000; // Automatic feeding interval (2 minit = 12 Hours(IRL))
unsigned long lastServoActionTime = 0;



// V-One Client Instance
VOneMqttClient voneClient;

void setup_wifi() {
    Serial.println("Connecting to WiFi...");
    Serial.println(WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    Serial.println("IP address: ");
     Serial.println(WiFi.localIP());
}




void triggerActuator_callback(const char* actuatorDeviceId, const char* actuatorCommand) {
    Serial.print("Main received callback: ");
    Serial.print(actuatorDeviceId);
    Serial.print(" : ");
    Serial.println(actuatorCommand);

    String errorMsg = "";

    JSONVar commandObject = JSON.parse(actuatorCommand);
    JSONVar keys = commandObject.keys();

    String key = "";
    for (int i = 0; i < keys.length(); i++) {
        key = (const char*)keys[i];
    }

    if (String(actuatorDeviceId) == LEDIndicator) {
        greenLEDState = (bool)commandObject[key];
        digitalWrite(ledGreen, greenLEDState);
        Serial.println(greenLEDState ? "Green LED ON" : "Green LED OFF");
        voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, true);
    } 
    

    else if (String(actuatorDeviceId) == PumpRelay) {
        relayPumpState = (bool)commandObject[key];
        digitalWrite(relayPin, relayPumpState);
        Serial.println(relayPumpState ? "Relay Water Pump ON" : "Relay Water Pump OFF");
        voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, true);
        delay(1000);
    } 
    else if (String(actuatorDeviceId) == ServoMotor) {
        int servoAngle = (int)commandObject[key];
        feederServo.write(servoAngle);    
        voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, true);//publish actuator status


    }
    else
    {
        Serial.print(" No actuator found : ");
        Serial.println(actuatorDeviceId);
        errorMsg = "No actuator found";
        voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, false);//publish actuator status
    } 
}


void setup() {
    Serial.begin(9600);
    setup_wifi();
    voneClient.setup();
    voneClient.registerActuatorCallback(triggerActuator_callback);

    // Pin Setup
    feederServo.attach(servoPin);
    feederServo.write(0); 
    pinMode(buttonPin, INPUT_PULLUP); 

    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);

    pinMode(ledGreen, OUTPUT);
    digitalWrite(ledGreen, LOW);

    pinMode(rainSensorPin, INPUT);
}



void loop() {


    voneClient.loop();  

     if (!voneClient.connected()) {
    voneClient.reconnect();
    voneClient.publishDeviceStatusEvent(ButtonSensor, true);
    voneClient.publishDeviceStatusEvent(WaterLevelSensor, true);
    }

int buttonState = digitalRead(buttonPin);
int waterLevel = analogRead(rainSensorPin);  // Read water level sensor
unsigned long currentMillis = millis();

    Serial.print("Button State: ");
    Serial.println(buttonState);
    Serial.print("Water Level: ");
    Serial.println(waterLevel);


  
if (currentMillis - lastMsgTime > INTERVAL) {
        lastMsgTime = currentMillis;

        // Create JSON objects for telemetry
        JSONVar payloadObject;
        payloadObject["Raining"] = waterLevel;

        JSONVar payloadObjectBtn;
        payloadObjectBtn["Button1"] = buttonState;

        // Publish telemetry
        voneClient.publishTelemetryData(WaterLevelSensor, "Raining", waterLevel);
        voneClient.publishTelemetryData(ButtonSensor, "Button1", buttonState);
        Serial.println("Telemetry data sent to V-One Cloud.");


      
    
}



    // Handle button press with debounce
    if (buttonState == LOW && !buttonPressed && (currentMillis - lastButtonPressTime > debounceDelay)) {
        buttonPressed = true;
        lastButtonPressTime = currentMillis;

        // Start servo movement
        isMoving = true;
        servoMoveStartTime = currentMillis;  // Record start time
        digitalWrite(ledGreen, HIGH);
        feederServo.write(180);  // Move servo to 180 degrees
        Serial.println("Button Pressed: Servo moving to 180 degrees.");

        // Publish button press telemetry and status
        voneClient.publishTelemetryData(ButtonSensor, "Button1", 1);
        voneClient.publishDeviceStatusEvent(ButtonSensor, true, "Button Pressed");
    }

    // Handle button release
    if (buttonState == HIGH && buttonPressed) {
        buttonPressed = false;
        Serial.println("Button Released.");

        // Publish button release telemetry and status
        voneClient.publishTelemetryData(ButtonSensor, "Button1", 0);
        voneClient.publishDeviceStatusEvent(ButtonSensor, false, "Button Released");
    }


    // Automatic Feeding
  if (currentMillis - lastServoActionTime >= feedingInterval && !isMoving) {
    lastServoActionTime = currentMillis;
   // Trigger automatic feeding

     isMoving = true;
     servoMoveStartTime = currentMillis;  // Record start time
    digitalWrite(ledGreen, HIGH);
    feederServo.write(180);  // Move servo to 180 degrees
    Serial.println("Automatic feeding: Servo moving to 180 degrees.");
   }

    // Handle servo movement logic
    if (isMoving) {
        if (currentMillis - servoMoveStartTime >= 10000) {  
            feederServo.write(0);  // Move back to 0 degrees
            isMoving = false;  // Reset movement state
            digitalWrite(ledGreen, LOW);
            Serial.println("Servo returned to 0 degrees.: Feeding Complete :D");
        }
    }

   

    // Activate Pump Control Based on Water Level
    if (waterLevel < waterThreshold && !pumpActive) {
      
      Serial.println("Water Pump ON");
      digitalWrite(relayPin, HIGH);
      pumpActive = true;
      voneClient.publishDeviceStatusEvent(PumpRelay, true, "Pump Activated");  // 
      delay(pumpDuration);

      digitalWrite(relayPin, LOW);
      pumpActive = false;
      voneClient.publishDeviceStatusEvent(PumpRelay, false, "Pump Deactivated");  // 
      Serial.println("Water Pump OFF - Pond Full");
    }

  }



