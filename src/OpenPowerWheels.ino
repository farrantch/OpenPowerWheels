#include <ESP32Servo.h>

// PARAMETERS
#define ENABLE_DEBUGGING 1
#define THRESHOLD_HALL_MIN 1320
#define THRESHOLD_HALL_MAX 3340
#define THRESHOLD_POT_MIN 200
#define THRESHOLD_POT_MAX 4000
#define MAX_ACCEL = 5
// INPUTS
#define PIN_INPUT_THROTTLE 36
#define PIN_INPUT_BRAKE 39
#define PIN_INPUT_GOVERNOR 35
#define PIN_INPUT_TRANS_DIRECTION 23
#define PIN_INPUT_TRANS_HIGHLOW 19
#define PIN_INPUT_LIGHT_BLINKLEFT 3
#define PIN_INPUT_LIGHT_BLINKRIGHT 19
// OUTPUTS
#define PIN_OUTPUT_GOVERNOR_PWM 26
#define PIN_OUTPUT_THROTTLE_PWM 32
#define PIN_OUTPUT_THROTTLE_POT 33
#define PIN_OUTPUT_LIGHT_NIGHT 18
#define PIN_OUTPUT_LIGHT_BRAKE 23
#define PIN_OUTPUT_LIGHT_BLINKLEFT 24
#define PIN_OUTPUT_LIGHT_BLINKRIGHT 25

Servo ESC_THROTTLE;

long timeIntervalBlink = 300;

// Debugging
long debugInterval = 1000;
long debugPreviousTime = 0;
int printDebug = LOW;
// Throttle
int throttle = 0;
// Governor
int governor = 4096;
// Left Blinker
int ledLeftState = LOW;
long ledLeftPreviousTime = 0;
// Right Blinker
int ledRightState = LOW;
long ledRightPreviousTime = 0;
// Transmission
char transDirection = 'F';
int transHighLow = LOW;


// int val = 0;

void setup() {
  // Serial.begin(9600);
  Serial.begin(115200);
  // INPUTS
  pinMode(PIN_INPUT_BRAKE, INPUT);
  pinMode(PIN_INPUT_THROTTLE, INPUT);
  pinMode(PIN_INPUT_GOVERNOR, INPUT_PULLUP);
  pinMode(PIN_INPUT_TRANS_DIRECTION, INPUT_PULLUP);
  pinMode(PIN_INPUT_TRANS_HIGHLOW, INPUT_PULLUP);
  pinMode(PIN_INPUT_LIGHT_BLINKLEFT, INPUT_PULLUP);
  pinMode(PIN_INPUT_LIGHT_BLINKRIGHT, INPUT_PULLUP);
  // // OUTPUTS
  // pinMode(PIN_OUTPUT_LIGHT_NIGHT, OUTPUT);
  pinMode(PIN_OUTPUT_LIGHT_BRAKE, OUTPUT);
  pinMode(PIN_OUTPUT_LIGHT_BLINKLEFT, OUTPUT);
  pinMode(PIN_OUTPUT_LIGHT_BLINKRIGHT, OUTPUT);
  pinMode(PIN_OUTPUT_GOVERNOR_PWM, OUTPUT);
  ESC_THROTTLE.attach(PIN_OUTPUT_THROTTLE_PWM,1000,2000);
}

void read_input_trans_direction() {
  int value = digitalRead(PIN_INPUT_TRANS_DIRECTION);
  if (value == LOW) {
    transDirection = 'R';
  } else {
    transDirection = 'F';
  }
}

void read_input_trans_highlow() {
  int input_value = digitalRead(PIN_INPUT_TRANS_HIGHLOW);
  if (input_value == LOW) {
    transHighLow = HIGH;
  } else {
    transHighLow = LOW;
  }
  if (printDebug == HIGH) {
    Serial.print("Trans HighLow Input: ");
    Serial.println(input_value);
    Serial.print("Trans HighLow Output: ");
    Serial.println(transHighLow);
  }
}

void read_input_brake() {
  int input_value = analogRead(PIN_INPUT_BRAKE);
  if (input_value < THRESHOLD_HALL_MIN) {
    digitalWrite(PIN_OUTPUT_LIGHT_BRAKE, LOW);
  } else {
    digitalWrite(PIN_OUTPUT_LIGHT_BRAKE, HIGH);
  }
  if (printDebug == HIGH) {
    Serial.print("Brake Input: ");
    Serial.println(input_value);
  }
}

void read_input_governor() {
  int input_value = analogRead(PIN_INPUT_GOVERNOR);
  // Filter out low noise
  if (input_value < THRESHOLD_POT_MIN) {
    governor = 0;
  }
  // Filter out high noise
  else if (input_value > THRESHOLD_POT_MAX){
    governor = 4095;
  }
  else {
    governor = input_value;
  }
  int duty_cycle = map(governor, 0, 4095, 0, 255);
  analogWrite(PIN_OUTPUT_GOVERNOR_PWM, duty_cycle);
  if (printDebug == HIGH) {
    Serial.print("Governor Input: ");
    Serial.println(input_value);
    Serial.print("Governor Final: ");
    Serial.println(governor);
    Serial.print("Governor PWM: ");
    Serial.println(duty_cycle);
  }
}

void read_input_throttle() {
  int input_value = analogRead(PIN_INPUT_THROTTLE);
  int filter_value = 0;
  int range = THRESHOLD_HALL_MAX - THRESHOLD_HALL_MIN;
  int midrange = range/2;
  // Filter out low noise
  if (input_value < THRESHOLD_HALL_MIN) {
    filter_value = 0;
  }
  // Filter out high noise
  else if (input_value > THRESHOLD_HALL_MAX){
    filter_value = range;
  }
  // Set adjusted value
  else {
    adjusted_value = filter_value - THRESHOLD_HALL_MIN;
  }


  
  // Halve the input value since each direction is half of the output throttle range
  int adjusted_value = adjusted_value / 2;
  // Adjust speed based on High/Low gear
  if (transHighLow == LOW) {
    adjusted_value = adjusted_value / 2;
  }
  // Apply directional offset
  // Forward Direction
  if (transDirection == 'F') {
    adjusted_value = adjusted_value + midrange;
  }
  // Reverse Direction
  else if (transDirection == 'R') {
    adjusted_value = midrange - adjusted_value;
  }
  int speed = map(adjusted_value, 0, range, 0, 180);
  if (printDebug == HIGH) {
    Serial.print("Throttle Input: ");
    Serial.println(input_value);
    Serial.print("Throttle Filtered: ");
    Serial.println(filter_value);
    Serial.print("Throttle Adjusted: ");
    Serial.println(adjusted_value);
    Serial.print("Throttle Servo: ");
    Serial.println(speed);
  }
  ESC_THROTTLE.write(speed);
}

void read_input_light_blinkleft(unsigned long currentTime) {
  // On
  if (digitalRead(PIN_INPUT_LIGHT_BLINKLEFT) == LOW) {
    // Change state if time interval exceeded
    if (currentTime - ledLeftPreviousTime > timeIntervalBlink) {
      ledLeftPreviousTime = currentTime;
      if (ledLeftState == HIGH) {
        ledLeftState = LOW;
      } else {
        ledLeftState = HIGH;
      }
      digitalWrite(PIN_OUTPUT_LIGHT_BLINKLEFT, ledLeftState);
    }
  }
  // Off
  else {
    digitalWrite(PIN_OUTPUT_LIGHT_BLINKLEFT, LOW);
    ledLeftState = LOW;
    ledLeftPreviousTime = 0;
  }
}

void read_input_light_blinkright(unsigned long currentTime) {
  // On
  if (digitalRead(PIN_INPUT_LIGHT_BLINKRIGHT) == LOW) {
    // Change state if time interval exceeded
    if (currentTime - ledRightPreviousTime > timeIntervalBlink) {
      ledRightPreviousTime = currentTime;
      if (ledRightState == HIGH) {
        ledRightState = LOW;
      } else {
        ledRightState = HIGH;
      }
      digitalWrite(PIN_OUTPUT_LIGHT_BLINKRIGHT, ledRightState);
    }
  }
  // Off
  else {
    digitalWrite(PIN_OUTPUT_LIGHT_BLINKRIGHT, LOW);
    ledRightState = LOW;
    ledRightPreviousTime = 0;
  }
}

void write_output_throttle() {

}

// the loop function runs over and over again forever
void loop() {
  unsigned long currentTime = millis();
  if (ENABLE_DEBUGGING == HIGH) {
    if (currentTime - debugPreviousTime > debugInterval) {
      printDebug = HIGH;
      Serial.println("");
    }
  }
  read_input_brake();
  read_input_governor();
  read_input_throttle();
  read_input_trans_direction();
  read_input_trans_highlow();
  read_input_light_blinkleft(currentTime);
  read_input_light_blinkright(currentTime);
  write_output_throttle();
  if (ENABLE_DEBUGGING == HIGH) {
    if (printDebug == HIGH) {
      printDebug = LOW;
      debugPreviousTime = currentTime;
    }
  }
}