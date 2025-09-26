#include <ESP32Servo.h>
#include <HardwareSerial.h>
#include <CytronMotorDriver.h>
#include <iotCmd.h>
#include <Stepper.h>
#include <AccelStepper.h>

#define LEFT_FORWARD_PIN    19  // Built-in LED is usually at GPIO2
#define LEFT_BACKWARD_PIN   18  // Built-in LED is usually at GPIO2
#define RIGHT_FORWARD_PIN    5  // Built-in LED is usually at GPIO2
#define RIGHT_BACKWARD_PIN   17  // Built-in LED is usually at GPIO2

// Define stepper motor control pins
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17

// NEMA STEPPER WITH TB6600 (3PIN, 3V Enable)

#define TB6600_STEP_PIN 33
#define TB6600_DIR_PIN 32
// #define TB6600_ENABLE_PIN 16

bool NEMA_ACTIVE = false;

// Define a stepper motor 1 for arduino 
// TB6600_STEP_PIN Digital 5 (CLK), direction Digital TB6600_DIR_PIN 17 (CW), 
AccelStepper nema_stepper(1, TB6600_STEP_PIN, TB6600_DIR_PIN);

const int nema_stepsPerRevolution = 1600; // Full steps per revolution of your motor (e.g., 200 for 1.8 degree stepper)
long nema_currentStep = 0;

#define RGB_PIN 2  // Built-in LED is usually at GPIO2

// Configure the motor driver.
CytronMD leftMotor(PWM_PWM, LEFT_FORWARD_PIN, LEFT_BACKWARD_PIN);   // PWM 1A = Pin 3, PWM 1B = Pin 9.
CytronMD rightMotor(PWM_PWM, RIGHT_FORWARD_PIN, RIGHT_BACKWARD_PIN); // PWM 2A = Pin 10, PWM 2B = Pin 11.


const int stepsPerRevolution = 2048; // for 28BYJ-48
// need better way to resolve pin conflicts #TODO binu
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4); // 28byj stepper
int currentStep = 0; // track current step position


// when we started, we had a different UI command flow to trigger speed. Now speed and movement is via same IotCmd object
// will be deprecated
int g_speed = 125; 
int g_max_pwm = 250;

// Number of servos
const int servoCount = 3;

// Define servo pins
const int servoPins[servoCount] = {27, 26, 27}; // servo pins

// Create Servo objects
Servo servos[servoCount];



// Alter the state of the onboard led
// To be used when there is a command sent on bluetooth
void alterInBuiltLed(int state) {
#ifdef RGB_PIN
  digitalWrite(RGB_PIN, state);
#endif
}

void configPins() {
    
  pinMode(RGB_PIN, OUTPUT);  // Initialize the LED pin as output

  pinMode(LEFT_FORWARD_PIN, OUTPUT); 
  pinMode(LEFT_BACKWARD_PIN, OUTPUT); 
  pinMode(RIGHT_FORWARD_PIN, OUTPUT); 
  pinMode(RIGHT_BACKWARD_PIN, OUTPUT); 

  // Attach each servo to its pin
  for (int i = 0; i < servoCount; i++) {
    if (!servos[i].attach(servoPins[i])) {
      Serial.printf("Failed to attach servo %d at pin %d \n", i, servoPins[i]);
    } else {
      Serial.printf("Servo %d up on pin %d \n", i, servoPins[i]);
    }
  }

  // TODO BINU, make this dynamic, later
  // nema setup
  nema_stepper.setMaxSpeed(1000);//1100
  nema_stepper.setAcceleration(1100);

}

void setSpeed(int speed) {
  if (speed >= 0 && speed <= 100) {
    g_speed = (speed / 100.0) * g_max_pwm;
    Serial.printf("\n Setting speed %d\n", g_speed);
  } else {
    Serial.printf("\n Speed not in limit 0 -- 100 %d", speed);
  }
}

void moveForward() {
  alterInBuiltLed(HIGH);
  leftMotor.setSpeed(g_speed);
  rightMotor.setSpeed(g_speed);
  Serial.println("********** Move Forward");
}

void moveBackward() {
  // alterInBuiltLed(HIGH);
  leftMotor.setSpeed(-g_speed);
  rightMotor.setSpeed(-g_speed);
  Serial.println("********** Move Backward");
}

void turnLeft() {
  alterInBuiltLed(HIGH);
  leftMotor.setSpeed(-g_speed);
  rightMotor.setSpeed(g_speed);
  Serial.println("********** Turn Left");
}

void turnRight() {
  alterInBuiltLed(HIGH);
  leftMotor.setSpeed(g_speed);
  rightMotor.setSpeed(-g_speed);
  Serial.println("********** Turn Right");
}

void carStop() {
  setSpeed(0);
  alterInBuiltLed(LOW);
  leftMotor.setSpeed(LOW);
  rightMotor.setSpeed(LOW);
  Serial.println("********** Car Stop");
}

void controlpadWithSpeed(IotCommand* cmd) {
  int speedInt =  cmd->value1;
  setSpeed(speedInt);
  if (cmd->subcmd == SubCmdEnum_move_forward) moveForward();
  else if (cmd->subcmd == SubCmdEnum_move_backward) moveBackward();
  else if (cmd->subcmd == SubCmdEnum_move_turn_left) turnLeft();
  else if (cmd->subcmd == SubCmdEnum_move_turn_right) turnRight();
  else if (cmd->subcmd == SubCmdEnum_move_stop) carStop();
}


/// @brief Changes angle on the chosen servo
/// @param servoNumber 
/// @param angle 
void controlServo(int servoNumber, int angle) {
  // servo number index starts in arduino at 0, client index starts at 1
  servoNumber--;
  Serial.printf("Setting Servo %d, Angle %d \n", servoNumber, angle);
  servos[servoNumber].write(angle);
}

void controlLed(int ledPin, int value) {

  Serial.printf("Setting Led Pin %d, value %d \n", ledPin, value);
  digitalWrite(ledPin, value);
}

void controlStepper(int servoNumber, int angle) {

  int targetStep = (angle * stepsPerRevolution) / 360;
  int stepToMove = targetStep - currentStep;
  myStepper.step(stepToMove); // move motor by calculated steps
  currentStep = targetStep;
  
  
  Serial.print("Moved stepper to angle: ");
  Serial.println(angle);

}

void controlNemaStepper(int servoNumber, int angle) {



  // Clamp angle between 0 and 360
  if (angle > 360) angle = 360;
  if (angle < 0) angle = 0;

  int targetStep = (angle * nema_stepsPerRevolution) / 360;

  int currentStep = nema_stepper.currentPosition();

  if (NEMA_ACTIVE) {
    Serial.printf("Old run active, interrupt, move new %d \n", angle);
    // return;
  }

  int stepToMove = targetStep - currentStep;
  nema_stepper.move(stepToMove); // move motor by calculated steps
  nema_currentStep = targetStep;
  
  Serial.printf("Moved nema step: %d, angle %d \n", currentStep, angle);

}

// run any functions of motors, servo, stepper, which has to be looped here
void runActuatorLoop() {
  // Run the stepper until the target position is reached
  if (nema_stepper.distanceToGo() != 0) {
    NEMA_ACTIVE = true;
    nema_stepper.run();
    
  } else {
    NEMA_ACTIVE = false;
  }
}