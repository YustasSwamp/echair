#include <Arduino.h>


#define ECHAIR_4x4

#ifdef ECHAIR_4x4
#define GYRO_ENABLED 0
#else
#define GYRO_ENABLED 0
#endif


double actual_direction, target_direction, pid_output, speed;

bool stopped = false;

// CAN IDs
#ifdef ECHAIR_4x4
#define LF 31
#define LR 106
#define RF 110
#define RR 85
#else
#define LF 51
#define RF 49
#endif

#include "common.h"
#include "led.h"
#include "vesc.h"
#include "gyro.h"
#include "wlan.h"
#include "ota.h"
#include "remote.h"
#include "pid.h"
#include "web.h"


void serial_setup() {
  // debug
  Serial.begin(115200);
  Serial.println("Booting");
}

void set_duty(float l, float r) {
  static float last_l, last_r;
  if (stopped) {
    l = r = 0;
  }

  if (l == last_l && r == last_r)
    return;

  last_l = l;
  last_r = r;

#ifdef ECHAIR_4x4
  vesc_set_duty(l, LF);
  vesc_set_duty(l, LR);
  vesc_set_duty(r, RF);
  vesc_set_duty(r, RR);
#else
  vesc_set_duty(l, LF);
  vesc_set_duty(r, RF);
#endif

#if 0
  Serial.print("l:"); Serial.print(l);
  Serial.print(" r:"); Serial.println(r);
#endif
}

void set_current_brake(float c) {
#ifdef ECHAIR_4x4
  vesc_set_current_brake(c, LF);
  vesc_set_current_brake(c, LR);
  vesc_set_current_brake(c, RF);
  vesc_set_current_brake(c, RR);
#else
  vesc_set_current_brake(c, LF);
  vesc_set_current_brake(c, RF);
#endif
}

void setup() {
  serial_setup();
  vesc_setup();
  wifi_setup();
  webserial_setup();
  ota_setup();
  gyro_setup();
  pid_setup();
  led_setup();
  remote_setup();
}

void stop_all() {
  if (stopped)
    return;
  stopped = true;
  set_duty(0, 0);
  set_current_brake(-10);
  led_off();
}

void resume_all() {
  if (!stopped)
    return;
  target_direction = actual_direction;
  pid_output = 0;
  stopped = false;
  led_on();  
}

void loop() {
  static double t, d, o, s, p;
  ota_handle();
  webserial_handle();
  gyro_get_direction(&actual_direction);
  remote_handle();
  pid_handle();

  if (s != speed || p != pid_output) {
    s = speed;
    p = pid_output;
    set_duty((s - p)/100, (s + p)/100);
  }

  static unsigned long debug_event = 0; 

  if (next_event(&debug_event, 100)) {
//    WebSerial.printf("Target %f, Actual %f, Speed %f, PID %f\n", target_direction, actual_direction, speed, pid_output);
    Serial.printf("Target %f, Actual %f, Speed %f, PID %f, stopped %d\n", target_direction, actual_direction, speed, pid_output, stopped);
  }

#if 0
  if (t != target_direction || d != actual_direction || o != pid_output) {
    t = target_direction;
    d = actual_direction;
    o = pid_output;
  #if 1
    Serial.print("s:"); Serial.print(speed);
    Serial.print("t:"); Serial.print(t);
    Serial.print(",d:"); Serial.print(d);
    Serial.print(",o:"); Serial.print(o);
    Serial.println();
  #endif
  }
#endif
}
