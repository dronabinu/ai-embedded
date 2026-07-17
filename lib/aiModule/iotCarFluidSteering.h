#ifndef IOT_CAR_H // do not change name, we are overriding car type here
#define IOT_CAR_H

#include <HardwareSerial.h>
#include <CytronMotorDriver.h>
#include <iotCmd.h>


#include <boardState.h>

#define RGB_PIN 2  // Built-in LED is usually at GPIO2

// function headers

void configCar(int motorA1, int  motorA2, int motorB1, int  motorB2);


// Car motors
CytronMD* driveMotor = nullptr;
CytronMD* turnMotor = nullptr;


// when we started, we had a different UI command flow to trigger speed. Now speed and movement is via same IotCmd object
// will be deprecated
int g_speed = 125; 
int g_turn_strength = 125;
int g_max_pwm = 200;


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

    if (io.dev == DeviceCategory_led) {
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
  if (speed >= -100 && speed <= 100) {
    g_speed = (speed / 100.0) * g_max_pwm;
    Serial.printf("\n Setting speed %d\n", g_speed);
  } else {
    Serial.printf("\n Speed not in limit -100 -- 100 %d", speed);
  }
}

void setTurnStrength(int turnStrength) {
  if (turnStrength >= -100 && turnStrength <= 100) {
    g_turn_strength = (turnStrength / 100.0) * g_max_pwm;
    Serial.printf("\n Setting turnStrength %d\n", g_turn_strength);
  } else {
    Serial.printf("\n turnStrength not in limit -100 -- 100 %d", turnStrength);
  }
}

void move(int velocity, int turnStrength) {
  alterInBuiltLed(HIGH);
  setSpeed(velocity);
  setTurnStrength(turnStrength);
  driveMotor->setSpeed(g_speed);
  turnMotor->setSpeed(g_turn_strength); // negative for left motor, and positive for right motor
  Serial.println("********** Move with speed, turn value");
}

void moveForward() {
  alterInBuiltLed(HIGH);
  driveMotor->setSpeed(g_speed);
  Serial.println("********** Move Forward");
}

void moveBackward() {
  // alterInBuiltLed(HIGH);
  driveMotor->setSpeed(-g_speed);
  Serial.println("********** Move Backward");
}

void turnLeft() {
  alterInBuiltLed(HIGH);
  turnMotor->setSpeed(-g_turn_strength);
  Serial.println("********** Turn Left");
}

void turnRight() {
  alterInBuiltLed(HIGH);
  turnMotor->setSpeed(g_turn_strength);
  Serial.println("********** Turn Right");
}

void centerSteer() {
  setTurnStrength(0);
  turnMotor->setSpeed(LOW);
  Serial.println("********** Car Steering Stop");
}

void carStop() {
  setSpeed(0);
  setTurnStrength(0);
  alterInBuiltLed(LOW);
  driveMotor->setSpeed(LOW);
  turnMotor->setSpeed(LOW);
  Serial.println("********** Car Stop");
}

void controlpadWithSpeed(IotCommand* cmd) {
  int speedInt =  cmd->value1;
  int turnValue  =  cmd->value2;
  if (cmd->subcmd == SubCmdEnum_move) { move(speedInt, turnValue); }
  else if (cmd->subcmd == SubCmdEnum_move_forward) { setSpeed(speedInt);moveForward();}
  else if (cmd->subcmd == SubCmdEnum_move_backward) {setSpeed(speedInt);moveBackward();}
  else if (cmd->subcmd == SubCmdEnum_move_turn_left) {setTurnStrength(speedInt);turnLeft();}
  else if (cmd->subcmd == SubCmdEnum_move_turn_right) {setTurnStrength(speedInt);turnRight();}
  else if (cmd->subcmd == SubCmdEnum_move_center_steer) centerSteer();
  else if (cmd->subcmd == SubCmdEnum_move_stop) carStop();
}

//------------ CAR ---------------------
void configCar(int motorA1, int  motorA2, int motorB1, int  motorB2) {
  
  // Configure the motor driver.
  driveMotor = new CytronMD(PWM_PWM, motorA1, motorA2);  
  turnMotor = new CytronMD(PWM_PWM, motorB1, motorB2); 
  
  Serial.printf("Left motor pins %d, %d \n", motorA1, motorA2);
  Serial.printf("Right motor pins %d, %d \n", motorB1, motorB2);
  
}

void controlLed(int ledPin, int value) {
  Serial.printf("Setting Led Pin %d, value %d \n", ledPin, value);
  digitalWrite(ledPin, value);
}


#endif // IOT_CAR_H