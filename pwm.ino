#include <VescUart.h>
#include <SoftwareSerial.h>

unsigned long time_on_fall[2] = {0};
unsigned long time_on_raise[2] = {0};
unsigned int pwm_period[2] = {0};
unsigned int pwm_durty[2] = {0};

/** Initiate VescUart class */
VescUart vesc;

/** Initiate SoftwareSerial class */
SoftwareSerial vescSerial(16, 5);

enum {
  THRO = 0,
  STR = 1,
};

struct channel {
  int pin;
  unsigned long min, mid, max;
  unsigned long timestamp;
  unsigned long raw;
  unsigned long smooth;
  unsigned long out;
  /* internal */
  unsigned long htime;

};

struct channel channels[] = {
  {D5, 1107, 1486, 1904, 0, 0, 0},
  {D6, 1001, 1486, 2001, 0, 0, 0},
};

void ICACHE_RAM_ATTR isr(int ch) {
  unsigned long current_time = micros();
  struct channel *c = &channels[ch];

  if(digitalRead(c->pin) == HIGH){
    c->htime = current_time;
  } else {
//    pwm_period[ch] = current_time - time_on_fall[ch];
    c->raw = current_time - c->htime;
    // Simple 1:4 low pass filtering
    c->smooth = (c->smooth - (c->smooth >> 2)) + (c->raw >> 2);
    if (c->smooth < c->min)
      c->out = 0;
    else if (c->smooth > c->max)
      c->out = c->max - c->min;
    else
      c->out = c->smooth - c->min;
    c->timestamp = millis();
  }
}

void ICACHE_RAM_ATTR isr_thro() {
  isr(THRO);
}

void ICACHE_RAM_ATTR isr_str() {
  isr(STR);
}

void setup() {
  Serial.begin(115200);
//  Serial.swap();
//  pinMode(D5, INPUT_PULLUP);
//  pinMode(D6, INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(D5), isr_thro, CHANGE);
//  attachInterrupt(digitalPinToInterrupt(D6), isr_str, CHANGE);

  /** Setup SoftwareSerial port */
  vescSerial.begin(9600);
  vescSerial.setTimeout(1000);
  /** Setup UART port (Serial1 on Atmega32u4) */
//  Serial1.begin(115200);
  delay(1000);
//  vesc.setDebugPort(&Serial);
  /** Define which ports to use as UART */
//  vesc.setSerialPort(&Serial1);
  vesc.setSerialPort(&vescSerial);

}

// the loop function runs over and over again forever
void loop() {
  delay(1000);
//  if (millis() - channels[0].timestamp < 100 && millis() - channels[1].timestamp < 100)
//    Serial.printf("%d %d\n", channels[0].out, channels[1].out);

  /** Call the function getVescValues() to acquire data from VESC */
  if ( vesc.getVescValues() ) {

    Serial.println(vesc.data.rpm);
    Serial.println(vesc.data.inpVoltage);
    Serial.println(vesc.data.ampHours);
    Serial.println(vesc.data.tachometerAbs);

  }
  else
  {
    Serial.println("Failed to get data!");
  }
//    Serial.printf("%d %d\n", channels[0].raw, channels[1].raw);
}
