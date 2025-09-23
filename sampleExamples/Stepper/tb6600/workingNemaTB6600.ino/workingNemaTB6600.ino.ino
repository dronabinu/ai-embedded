#define stepPin 5
#define dirPin 17
#define enablePin 18

const int stepsPerRevolution = 1600; // Change this according to your microstep setting
int count = 0;

void setup() {
  Serial.begin(115200);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, LOW); 
  Serial.println("Stepper Initialized. Send angle in degrees:");
}

void moveToAngle(int angle) {
  // Clamp angle between 0 and 360 for convenience (optional)
  if(angle < 0) angle = 0;
  else if(angle > 360) angle = 360;

  Serial.printf("Received angle: %.2f degrees\n", angle);

  // Calculate target step position relative to 0 degrees position
  long targetStepPosition = (long)((angle / 360.0) * stepsPerRevolution);

  // Calculate steps to move and direction
  long stepsToMove = targetStepPosition - currentStepPosition;

  if (stepsToMove != 0) {
    if (stepsToMove > 0) {
      digitalWrite(dirPin, HIGH); // Clockwise
    } else {
      digitalWrite(dirPin, LOW); // Counter-clockwise
      stepsToMove = -stepsToMove; // Make positive for looping
    }

    // Pulse step pin stepsToMove times
    for (long i = 0; i < stepsToMove; i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(500); // Speed control
      digitalWrite(stepPin, LOW);
      delayMicroseconds(500);
    }
    currentStepPosition = targetStepPosition;
  } else {
    Serial.println("Already at the requested angle.");
  }
}

void loop() {
  if (Serial.available() > 0) {
    // Read angle as float from serial
    float angle = Serial.parseFloat();
    moveToAngle(angle);
    Serial.println("Ready for next angle input.");
  }
  delay(1000); 
  
}