#ifndef IOT_ACTUATOR_H
#define IOT_ACTUATOR_H

#include <ESP32Servo.h>
#include <HardwareSerial.h>
#include <CytronMotorDriver.h>
#include <iotCmd.h>

#include <AccelStepper.h>
#include <boardState.h>

#define RGB_PIN 2  // Built-in LED is usually at GPIO2

// function headers
void configServo(int servoNum, int  servoPin);
void configNemaStepper(int stepPin, int dirPin); 
void configCar(int motorA1, int  motorA2, int motorB1, int  motorB2);

bool NEMA_ACTIVE = false;

// Define a stepper motor 1 for arduino 
// TB6600_STEP_PIN Digital 5 (CLK), direction Digital TB6600_DIR_PIN 17 (CW), 
//AccelStepper nema_stepper(1, TB6600_STEP_PIN, TB6600_DIR_PIN);
AccelStepper* nema_stepper = nullptr;

// Car motors
CytronMD* leftMotor = nullptr;
CytronMD* rightMotor = nullptr;

const int nema_stepsPerRevolution = 1600; // Full steps per revolution of your motor (e.g., 200 for 1.8 degree stepper)
long nema_currentStep = 0;

const int stepsPerRevolution = 2048; // for 28BYJ-48
// need better way to resolve pin conflicts #TODO binu
// Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4); // 28byj stepper
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

void initializeIODevices(ConnectedIO devices[]) {

  int servoCount = 1;
  // iterate through all the devices
  for(size_t i=0; i<MAX_ITEMS; i++) {
    ConnectedIO io = devices[i];
    if (io.dev == DeviceCategory_stepper) {
      // configure stepper
      configNemaStepper(io.pin[0], io.pin[1]);
    } else if (io.dev == DeviceCategory_servo) {
      // configure stepper
      configServo(servoCount, io.pin[0]);
      servoCount++;
    } else if (io.dev == DeviceCategory_led) {
      pinMode(io.pin[0], OUTPUT);  // Initialize the LED pin as output
    } else if (io.dev == DeviceCategory_car_2_wheel_module) {
      configCar(io.pin[0], io.pin[1], io.pin[2], io.pin[3]);
    }
  }
    
  pinMode(RGB_PIN, OUTPUT);  // Initialize the LED pin as output



  // TODO BINU, make this dynamic, later
  // nema setup

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
  leftMotor->setSpeed(g_speed);
  rightMotor->setSpeed(g_speed);
  Serial.println("********** Move Forward");
}

void moveBackward() {
  // alterInBuiltLed(HIGH);
  leftMotor->setSpeed(-g_speed);
  rightMotor->setSpeed(-g_speed);
  Serial.println("********** Move Backward");
}

void turnLeft() {
  alterInBuiltLed(HIGH);
  leftMotor->setSpeed(-g_speed);
  rightMotor->setSpeed(g_speed);
  Serial.println("********** Turn Left");
}

void turnRight() {
  alterInBuiltLed(HIGH);
  leftMotor->setSpeed(g_speed);
  rightMotor->setSpeed(-g_speed);
  Serial.println("********** Turn Right");
}

void carStop() {
  setSpeed(0);
  alterInBuiltLed(LOW);
  leftMotor->setSpeed(LOW);
  rightMotor->setSpeed(LOW);
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

//------------ CAR ---------------------
void configCar(int motorA1, int  motorA2, int motorB1, int  motorB2) {
  // pinMode(motorA1, OUTPUT); 
  // pinMode(motorA2, OUTPUT); 
  // pinMode(motorB1, OUTPUT); 
  // pinMode(motorB2, OUTPUT);
  
  // Configure the motor driver.
  leftMotor = new CytronMD(PWM_PWM, motorA1, motorA2);  
  rightMotor = new CytronMD(PWM_PWM, motorB1, motorB2); 
  
  Serial.printf("Left motor pins %d, %d \n", motorA1, motorA2);
  Serial.printf("Right motor pins %d, %d \n", motorB1, motorB2);
  
}

//------------ SERVO -------------------

void configServo(int servoNum, int  servoPin) {
  servoNum--;
  if (!servos[servoNum].attach(servoPin)) {
    Serial.printf("Failed to attach servo %d at pin %d \n", servoNum, servoPin);
  } else {
    Serial.printf("Servo %d up on pin %d \n", servoNum, servoPin);
  }
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

//------------ STEPPER -------------------

void configNemaStepper(int stepPin, int dirPin) {
  nema_stepper = new AccelStepper(1, stepPin, dirPin);
  Serial.printf("Initialized Stepper with step pin %d, dir pin %d\n", stepPin, dirPin);
  nema_stepper->setMaxSpeed(1000);//1100
  nema_stepper->setAcceleration(1100);
}

// void controlStepper(int servoNumber, int angle) {

//   int targetStep = (angle * stepsPerRevolution) / 360;
//   int stepToMove = targetStep - currentStep;
//   myStepper.step(stepToMove); // move motor by calculated steps
//   currentStep = targetStep;
    
//   Serial.print("Moved stepper to angle: ");
//   Serial.println(angle);

// }

void controlNemaStepper(int stepperNum, int angle) {

  if (nema_stepper == nullptr) {
    Serial.printf("Nema Stepper not initialized");
  }

  // Clamp angle between 0 and 360
  if (angle > 360) angle = 360;
  if (angle < 0) angle = 0;

  int targetStep = (angle * nema_stepsPerRevolution) / 360;

  int currentStep = nema_stepper->currentPosition();

  if (NEMA_ACTIVE) {
    Serial.printf("Old run active, interrupt, move new %d \n", angle);
    // return;
  }

  int stepToMove = targetStep - currentStep;
  nema_stepper->move(stepToMove); // move motor by calculated steps
  nema_currentStep = targetStep;
  
  Serial.printf("Moving Stepper %d with angle %d\n", stepperNum, angle);
  
}

// run any functions of motors, servo, stepper, which has to be looped here
void loopActuator() {
  if (nema_stepper == nullptr) {
    return;
  }
  // Run the stepper until the target position is reached
  if (nema_stepper->distanceToGo() != 0) {
    NEMA_ACTIVE = true;
    nema_stepper->run();
    
  } else {
    NEMA_ACTIVE = false;
  }
}

#endif // IOT_ACTUATOR_H