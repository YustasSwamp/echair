/*
 * 1600 RPM max is 15mph
 * ERPM = Npoles * RPM / 2;
 * Npoles = 30?
 * Max idle ERPMS observed 9200
 */

#define ZERO_THRESHOLD 40

#define FORWARD_MAX_ERPMS 80
#define BACKWARD_MAX_ERPMS 40
#ifdef ECHAIR_4x4
#define STEERING_MAX_ERPMS 40
#else
#define STEERING_MAX_ERPMS 15
#endif
#define STEERING_MAX_ANGLE 30.0
/* Minimum steergin precision to avoid wobbling around 0 degree */
#define STEERING_PRECISION 0.0
#define PPM_NOISE_FILTERING_COUNT 3

int straight_run_counter; /* samples while STR in neutral position */

struct channel {
  int pin;
  unsigned long min, mid, max;
  unsigned long timestamp;
  unsigned long raw;
  int ready;
  /* internal */
  unsigned long htime;

};


enum {
  THR = 0,
  STR = 1,
  AUX = 2,
};

struct channel channels[] = {
  {34, 1000, 1485, 2000, 0, 0, 0, 0},
  {35, 1000, 1490, 2000, 0, 0, 0, 0},
  {32, 1000, 1490, 2000, 0, 0, 0, 0},
};

void isr(int ch) {
  unsigned long current_time = micros();
  struct channel *c = &channels[ch];

  if(digitalRead(c->pin) == HIGH){
    c->htime = current_time;
  } else {
    c->raw = current_time - c->htime;
    c->timestamp = millis();
    c->ready = 1;
  }
}

void isr_thr() {
  isr(THR);
}

void isr_str() {
  isr(STR);
}

void isr_aux() {
  isr(AUX);
}

void remote_setup() {
  pinMode(channels[THR].pin, INPUT_PULLUP);
  pinMode(channels[STR].pin, INPUT_PULLUP);
  pinMode(channels[AUX].pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(channels[THR].pin), isr_thr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(channels[STR].pin), isr_str, CHANGE);
  attachInterrupt(digitalPinToInterrupt(channels[AUX].pin), isr_aux, CHANGE);
}

double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void remote_handle() {
  static int aux_counter; /* noise filtering */
  if (millis() - channels[THR].timestamp > 100 && millis() - channels[STR].timestamp > 100) {
    /* No PPM data. transmitter lost ? */
    stop_all();
    return;
  }
#if 0
  if (channels[THR].ready && channels[STR].ready && channels[AUX].ready) {
    Serial.print(channels[THR].raw);
    Serial.print(" ");
    Serial.print(millis() - channels[THR].timestamp);
    Serial.print(" ");  
    Serial.print(channels[STR].raw);  
    Serial.print(" ");  
    Serial.print(millis() - channels[STR].timestamp);
    Serial.print(" ");  
    Serial.print(channels[AUX].raw);
    Serial.print(" ");  
    Serial.print(millis() - channels[AUX].timestamp);
    Serial.print(" stop all: ");  
    Serial.println(stop_all);
  }
#endif

  if (channels[THR].ready) {
    int thr = channels[THR].raw;
    channels[THR].ready = 0;
    if (thr > channels[THR].mid + ZERO_THRESHOLD)
      speed = mapf(thr, channels[THR].mid + ZERO_THRESHOLD, channels[THR].max, 0, FORWARD_MAX_ERPMS);
    else if (thr >= channels[THR].mid - ZERO_THRESHOLD)
      speed = 0;
    else
      speed = mapf(thr, channels[THR].min, channels[THR].mid - ZERO_THRESHOLD, -BACKWARD_MAX_ERPMS, 0);
  }


  if (channels[STR].ready) {
    int str = channels[STR].raw;
    channels[STR].ready = 0;
    if (str > channels[STR].mid + ZERO_THRESHOLD) {
      target_direction = actual_direction + mapf(str, channels[STR].mid + ZERO_THRESHOLD, channels[STR].max, 0, STEERING_MAX_ANGLE);
      straight_run_counter = 0;
    } else if (str < channels[STR].mid - ZERO_THRESHOLD) {
      target_direction = actual_direction + mapf(str, channels[1].min, channels[1].mid - ZERO_THRESHOLD, -STEERING_MAX_ANGLE, 0);
      straight_run_counter = 0;
    } else {
      /* Steering is in neutral: noise?, end of steering motion, or straight run */
      if (straight_run_counter < PPM_NOISE_FILTERING_COUNT) {
        /* Noise filtering, keep counting */
        straight_run_counter++;
      } else if (straight_run_counter < PPM_NOISE_FILTERING_COUNT * 2) {
        /* End of steering motion, target direction achieved, no PID adjustments */
        straight_run_counter++;
        target_direction = actual_direction;
      } else {
        /* Going straight. Do nothing. Rely on PID to correct actual_direction */
      }
    }
  }

  if (channels[AUX].ready) {
    int aux = channels[AUX].raw;
    if (aux < 1200) {
      if (aux_counter == -PPM_NOISE_FILTERING_COUNT)
        stop_all();
      else
        aux_counter--;
    }
    else if (aux > 1800) {
      if (aux_counter == PPM_NOISE_FILTERING_COUNT)
        resume_all();
      else
        aux_counter++;
    }
    else {
      aux_counter = 0;
    }
    channels[AUX].ready = 0;
  }
}