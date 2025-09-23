#include <AccelStepper.h>

// Define DRV8825 pins
const int dirPin = 5;     // Direction pin (e.g., GPIO 4 on ESP32)
const int stepPin = 17;   // Step pin (e.g., GPIO 16 on ESP32)

// Define the number of steps per revolution for your NEMA 17 motor (e.g., 200)
const int stepsPerRevolution = 200;

// Create an AccelStepper object
// AccelStepper(driverInterface, stepPin, dirPin)
// For DRV8825, we use the DRIVER type (1) with step/direction pins
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 DRV8825 NEMA 17 Stepper Control");

  // Set the maximum speed and acceleration for the stepper motor
  stepper.setMaxSpeed(1000); // Steps per second
  stepper.setAcceleration(500); // Steps per second^2

  // Set the desired number of steps for one revolution
  stepper.setCurrentPosition(0); // Start at position 0
}

void loop() {
  // Move the motor 1 revolution clockwise
  stepper.moveTo(stepsPerRevolution);
  stepper.run(); // Run the motor to the target position

  // Wait for 1 second
  delay(1000);

  // Move the motor 1 revolution counter-clockwise
  stepper.moveTo(-stepsPerRevolution);
  stepper.run(); // Run the motor to the target position

  // Wait for 1 second
  delay(1000);
}
