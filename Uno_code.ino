#include <AccelStepper.h>

// Define stepper motor connections for Motor 1
#define IN1_1 8
#define IN2_1 9
#define IN3_1 10
#define IN4_1 11

// Define stepper motor connections for Motor 2
#define IN1_2 4
#define IN2_2 5
#define IN3_2 6
#define IN4_2 7

// Steps per revolution for 28BYJ-48 with a nominal 64:1 gear ratio
#define STEPS_PER_REV 4096  // Full rotation requires 2048 steps

// Create two stepper motor instances
AccelStepper stepper1(AccelStepper::HALF4WIRE, IN1_1, IN3_1, IN2_1, IN4_1);
AccelStepper stepper2(AccelStepper::HALF4WIRE, IN1_2, IN3_2, IN2_2, IN4_2);

// Variables to store motor positions
long targetSteps1 = 0;
long targetSteps2 = 0;

void setup() {
  // Start Serial communication with ESP8266
  Serial.begin(9600);

  // Set max speed and acceleration for both motors
  stepper1.setMaxSpeed(500);
  stepper1.setAcceleration(100);

  stepper2.setMaxSpeed(500);
  stepper2.setAcceleration(100);
}

void loop() {
  // Check if there's any serial data coming from the ESP8266
  if (Serial.available() > 0) {
    // Read the entire input line
    String input = Serial.readStringUntil('\n');

    // Print received input for debugging
    Serial.print("Received: ");
    Serial.println(input);

    // Process motor 1 angle
    int index1 = input.indexOf("motor1=");
    if (index1 != -1) {
      int end1 = input.indexOf(";", index1);
      int motor1Angle = input.substring(index1 + 7, end1).toInt();
      targetSteps1 = map(motor1Angle, -360, 360, -STEPS_PER_REV, STEPS_PER_REV);
      stepper1.moveTo(targetSteps1);
    }

    // Process motor 2 angle
    int index2 = input.indexOf("motor2=");
    if (index2 != -1) {
      int end2 = input.indexOf(";", index2);
      int motor2Angle = input.substring(index2 + 7, end2).toInt();
      targetSteps2 = map(motor2Angle, -360, 360, -STEPS_PER_REV, STEPS_PER_REV);
      stepper2.moveTo(targetSteps2);
    }
  }

  // Run both motors
  stepper1.run();
  stepper2.run();
}
