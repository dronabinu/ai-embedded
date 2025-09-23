#include <AccelStepper.h>

#define stepPin 5
#define dirPin 17
#define enablePin 18

const int stepsPerRevolution = 3200;
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);
int currentStep = 0;

void setup() {
  Serial.begin(115200);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, LOW); // Enable the driver
  
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
  stepper.setCurrentPosition(0); // home position

  Serial.println("Stepper Arm Initialized. Send target angle 0-360:");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    int angle = input.toInt();

    // Clamp angle 0 to 360
    if (angle < -360) angle = -360;
    if (angle > 360) angle = 360;

    Serial.printf("Moving to angle: %d\n", angle);

    // Calculate target step position for the given angle
    int targetStep = (angle * stepsPerRevolution) / 360;

    int stepToMove = targetStep - currentStep;
    stepper.move(stepToMove); // move motor by calculated steps
    currentStep = targetStep;
    // Command stepper to move to target position
    // Run stepper until target is reached
    while (stepper.distanceToGo() != 0) {
      stepper.run();
    }

    Serial.println("Position reached. Awaiting next angle.");
  }
}
