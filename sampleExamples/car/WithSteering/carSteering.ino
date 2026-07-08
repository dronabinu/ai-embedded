#include "BluetoothSerial.h"

// Initialize Bluetooth
BluetoothSerial SerialBT;

// --- Back Motor Pins (Drive) ---
const int ENA = 25; // Speed Control (PWM)
const int IN1 = 26; // Direction 1
const int IN2 = 27; // Direction 2

// --- Front Motor Pins (Steering) ---
const int ENB = 14; // Steering Speed Control (PWM)
const int IN3 = 12; // Steering Direction 1
const int IN4 = 13; // Steering Direction 2

// --- Settings ---
int backSpeed = 200;  // 0 - 255 for back wheel speed
int turnSpeed = 150;  // 0 - 255 for turning speed (determines turn radius)

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT_Car"); // Bluetooth device name
  
  // Set motor pins as output
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Stop all motors initially
  stopCar();
  Serial.println("Bluetooth Car Ready!");
}

void loop() {
  // Check for Bluetooth data
  if (SerialBT.available()) {
    String incomingString = SerialBT.readStringUntil('\n');
    incomingString.trim(); // Remove any trailing spaces or newlines

    char command = incomingString.charAt(0);
    
    // Parse numeric value if present (e.g., "V255" or "T180")
    int value = incomingString.substring(1).toInt();

    switch (command) {
      // --- DRIVE COMMANDS ---
      case 'F': // Forward
        setBackMotors(backSpeed, HIGH, LOW);
        break;
      case 'B': // Backward
        setBackMotors(backSpeed, LOW, HIGH);
        break;
      case 'L': // Turn Left
        setTurnMotor(turnSpeed, HIGH, LOW);
        break;
      case 'R': // Turn Right
        setTurnMotor(turnSpeed, LOW, HIGH);
        break;
      case 'S': // Stop both
        stopCar();
        break;
        
      // --- SPEED & TURN RADIUS COMMANDS ---
      case 'V': // Set Velocity / Back Motor Speed
        backSpeed = constrain(value, 0, 255);
        Serial.print("Back Speed set to: ");
        Serial.println(backSpeed);
        break;
      case 'T': // Set Turn Speed (Controls Turn Radius)
        turnSpeed = constrain(value, 0, 255);
        Serial.print("Turn Speed set to: ");
        Serial.println(turnSpeed);
        break;
      case 'C': // Center Steering
        stopSteering();
        break;
      default:
        // Ignore unknown characters
        break;
    }
  }
}

// Function to control back wheels
void setBackMotors(int speedVal, int dir1, int dir2) {
  analogWrite(ENA, speedVal);
  digitalWrite(IN1, dir1);
  digitalWrite(IN2, dir2);
}

// Function to control front steering motor
void setTurnMotor(int speedVal, int dir1, int dir2) {
  analogWrite(ENB, speedVal);
  digitalWrite(IN3, dir1);
  digitalWrite(IN4, dir2);
}

void stopSteering() {
  analogWrite(ENB, 0);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void stopCar() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  stopSteering();
}