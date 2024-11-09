#if GYRO_ENABLED
#include <PID_v1.h>

//the 'k' values are the ones you need to fine tune before your program will work. Note that these are arbitrary values that you just need to experiment with one at a time.
//float Kp = 10;
//float Ki = 0.09;
//float Kd = 10;

double Kp=1, Ki=0.2, Kd=0.1;
PID pid(&actual_direction, &pid_output, &target_direction, Kp, Ki, Kd, P_ON_E, REVERSE);

void pid_setup() {
  pid.SetOutputLimits(-STEERING_MAX_ERPMS, STEERING_MAX_ERPMS);
  pid.SetMode(AUTOMATIC);
  /* 10ms refresh rate */
  pid.SetSampleTime(10);
}

void pid_handle() {
  if (!speed)
    return;

  if (fabs(actual_direction - target_direction) < STEERING_PRECISION) {
    target_direction = actual_direction;
    pid_output = 0;
  } else if (!straight_run_counter) {
    /* Steering */
    pid_output = mapf(actual_direction - target_direction, -STEERING_MAX_ANGLE, STEERING_MAX_ANGLE, -STEERING_MAX_ERPMS, STEERING_MAX_ERPMS);
  } else {
    /* Update pid_output */
    pid.Compute();
  }

}
#else
void pid_setup() {
}

void pid_handle() {
  pid_output = mapf(actual_direction - target_direction, -STEERING_MAX_ANGLE, STEERING_MAX_ANGLE, -STEERING_MAX_ERPMS, STEERING_MAX_ERPMS);
}

#endif /* GYRO_ENABLED */