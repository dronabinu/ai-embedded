 #include <Stepper.h>

const int stepsPerRevolution = 2048; // for 28BYJ-48

// Define motor control pins
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
int currentStep = 0; // track current step position


void setup() {
  myStepper.setSpeed(15); // speed in RPM
  Serial.begin(115200);
    
  Serial.println("Send angle (0-360):");
}

void loop() {

    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        int angle = input.toInt();
        if (angle >= 0 && angle <= 360) {
        int targetStep = (angle * stepsPerRevolution) / 360;
        int stepToMove = targetStep - currentStep;
        myStepper.step(stepToMove);
        currentStep = targetStep;
        Serial.print("Moved to angle: ");
        Serial.println(angle);
        } else {
        Serial.println("Invalid angle. Please enter 0-360.");
        }
    }
}

void rotateStepper(int angle) {
    int stepsToMove = (stepsPerRevolution * angle) / 360;
    myStepper.step(stepsToMove); // move motor by calculated steps
}

