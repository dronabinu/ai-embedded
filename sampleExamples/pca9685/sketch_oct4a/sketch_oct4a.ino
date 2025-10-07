#include <Arduino.h>
#include <PCA9685.h>

const int stepChannel = 2;
const int dirChannel = 3;
bool dir = false;
int freq = 1500;

namespace constants
{
const PCA9685::DeviceAddress device_address = 0x40;
const PCA9685::Pin output_enable_pin = 2;

const size_t loop_delay = 100;

const PCA9685::Channel channel = 0;

// DC Motor driver
// const PCA9685::DurationMicroseconds servo_pulse_duration_min = 15000;
// const PCA9685::DurationMicroseconds servo_pulse_duration_max = 20000;
// const PCA9685::DurationMicroseconds servo_pulse_duration_increment = 100;

// Servo
const PCA9685::DurationMicroseconds servo_pulse_duration_min = 900;
const PCA9685::DurationMicroseconds servo_pulse_duration_max = 2100;
const PCA9685::DurationMicroseconds servo_pulse_duration_increment = 100;
}

PCA9685 pca9685;

PCA9685::DurationMicroseconds servo_pulse_duration;

void setDirection(bool clockwise) {
  if (clockwise) {
    pca9685.setChannelServoPulseDuration(dirChannel, 4095); // 100% duty cycle = HIGH
  } else {
    pca9685.setChannelServoPulseDuration(dirChannel, 0);    // 0% duty cycle = LOW
  }
}

void setStepFrequency(int frequency) {
  // pwm.setPWMFreq(frequency);
  // Setup stepChannel for 50% duty cycle square wave
  pca9685.setChannelServoPulseDuration(stepChannel, frequency); // ON at 0, OFF at 2048 (half of 4096)
}



void setup()
{
  pca9685.setupSingleDevice(Wire,constants::device_address);

  pca9685.setupOutputEnablePin(constants::output_enable_pin);
  pca9685.enableOutputs(constants::output_enable_pin);

  pca9685.setToServoFrequency();

  servo_pulse_duration = constants::servo_pulse_duration_min;

  pca9685.setupOutputEnablePin(stepChannel);
  pca9685.enableOutputs(stepChannel);

  pca9685.setupOutputEnablePin(dirChannel);
  pca9685.enableOutputs(dirChannel);

  setDirection(true);
  setStepFrequency(2000); // 2kHz stepping speed
}

void loop()
{
  if (servo_pulse_duration > constants::servo_pulse_duration_max)
  {
    servo_pulse_duration = constants::servo_pulse_duration_min;
  }

  if (freq > 2000) {
    freq = 0;
    if (dir) {
      dir = false;
    } else {
      dir = true;
    }
    setDirection(dir);
    freq = 0;
  }

  setStepFrequency(freq); // 2kHz stepping speed

  freq = freq + 100;
  pca9685.setChannelServoPulseDuration(constants::channel,servo_pulse_duration);
  servo_pulse_duration += constants::servo_pulse_duration_increment;
  delay(constants::loop_delay);


}