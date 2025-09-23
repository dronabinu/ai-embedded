#include <AccelStepper.h>

// IMPORTANT BINU
// NEMA has 1600 steps per revolution with tb6600 ON DIP PINS as ON,OFF.ON.OFF (1,2,3,4)
// the Voltage at signal pin has to be given to 3V on esp32, else reverse does not work.

const int stepsPerRevolution = 1600; // Full steps per revolution of your motor (e.g., 200 for 1.8 degree stepper)
long currentStep = 0;

// Define a stepper motor 1 for arduino 
// direction Digital 9 (CW), pulses Digital 8 (CLK)
AccelStepper stepper(1, 5, 17);
void setup()
{  
  // Change these to suit your stepper if you want
  stepper.setMaxSpeed(1000);//1100
  stepper.setAcceleration(1100);
  Serial.begin(115200);
  Serial.printf("Enter an angle between 0 and 360\n");
}

void loop()
{
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      int angle = input.toInt();
      // Clamp angle between 0 and 360
      if (angle > 360) angle = 360;
      if (angle < 0) angle = 0;

      Serial.printf("Received angle: %d\n", angle);

      // // Calculate target step position
      long targetStep = (long)((angle / 360.0) * stepsPerRevolution);

      // // Calculate step difference to move
      long stepsToMove = targetStep - currentStep;
      stepper.move( stepsToMove );
      // Update current step
      currentStep = targetStep;
  }

  // Run the stepper until the target position is reached
  if (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  
}