#include <VescUart.h>

/*
 * 1600 RPM max is 15mph
 * ERPM = Npoles * RPM / 2;
 * Npoles = 30?
 * Max idle ERPMS observed 9200
 */
//#define RPM_INPUT
#define ZERO_THRESHOLD 40

#ifdef RPM_INPUT /* RPM input */
#define FORWARD_MAX_ERPMS 6000
#define BACKWARD_MAX_ERPMS 4000
#define STEERING_MAX_ERPMS 500
#else
#define FORWARD_MAX_ERPMS 80
#define BACKWARD_MAX_ERPMS 40
#define STEERING_MAX_ERPMS 20
#endif


#define sign(x) ((x > 0) - (x < 0))
unsigned long time_on_fall[2] = {0};
unsigned long time_on_raise[2] = {0};
unsigned int pwm_period[2] = {0};
unsigned int pwm_durty[2] = {0};

VescUart UART_LEFT;
VescUart UART_RIGHT;

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
  {20, 1000, 1485, 2000, 0, 0, 0},
  {21, 1000, 1490, 2000, 0, 0, 0},
};

void isr(int ch) {
  unsigned long current_time = micros();
  struct channel *c = &channels[ch];

  if(digitalRead(c->pin) == HIGH){
    c->htime = current_time;
  } else {
    c->raw = current_time - c->htime;
    c->timestamp = millis();
  }
}

void isr_thro() {
  isr(THRO);
}

void isr_str() {
  isr(STR);
}

void setup() {
  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(20), isr_thro, CHANGE);
  attachInterrupt(digitalPinToInterrupt(21), isr_str, CHANGE);

  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);
  while (!Serial1) {;}
  while (!Serial2) {;}

  UART_LEFT.setSerialPort(&Serial1);
  UART_RIGHT.setSerialPort(&Serial2);
  delay(500);
}

void loop() {

  if (millis() - channels[0].timestamp < 100 && millis() - channels[1].timestamp < 100) {
    int thr = channels[0].raw;
    int str = channels[1].raw;
    Serial.print(thr);
    Serial.print(" ");  
    Serial.println(str);  


 

    if (thr > channels[0].mid + ZERO_THRESHOLD)
      thr = map(thr, channels[0].mid + ZERO_THRESHOLD, channels[0].max, 0, FORWARD_MAX_ERPMS);
    else if (thr >= channels[0].mid - ZERO_THRESHOLD)
      thr = 0;
    else
      thr = map(thr, channels[0].min, channels[0].mid - ZERO_THRESHOLD, -BACKWARD_MAX_ERPMS, 0);

    if (str > channels[1].mid + ZERO_THRESHOLD)
      str = map(str, channels[1].mid + ZERO_THRESHOLD, channels[1].max, 0, STEERING_MAX_ERPMS);
    else if (str >= channels[1].mid - ZERO_THRESHOLD)
      str = 0;
    else
      str = map(str, channels[1].min, channels[1].mid - ZERO_THRESHOLD, -STEERING_MAX_ERPMS, 0);


    float left = thr + str;
    float right = thr - str;
    Serial.print(left);
    Serial.print(" ");  
    Serial.println(right);  
#ifdef RPM_INPUT
    UART_LEFT.setRPM(left);
    UART_RIGHT.setRPM(right);
#else
//    UART_LEFT.setCurrent(left);
//    UART_RIGHT.setCurrent(right);
    UART_LEFT.setDuty((float)left/100);
    UART_RIGHT.setDuty((float)right/100);
#endif
  } else {
    /* No PPM data. transmitter lost ?*/
#ifdef RPM_INPUT
    UART_LEFT.setRPM(0);
    UART_RIGHT.setRPM(0);
#else
//    UART_LEFT.setCurrent(0);
//    UART_RIGHT.setCurrent(0);
    UART_LEFT.setDuty(0);
    UART_RIGHT.setDuty(0);
    UART_LEFT.setBrakeCurrent(50);
    UART_RIGHT.setBrakeCurrent(50);
#endif
  }

/*
  if ( UART_LEFT.getVescValues() && UART_RIGHT.getVescValues()) {
    Serial.print("RPMS: "); 
    Serial.print(UART_LEFT.data.rpm);
    Serial.print(" ");  
    Serial.println(UART_RIGHT.data.rpm); 
  }
*/
  delay(50);
}

