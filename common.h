extern double Kp, Ki, Kd;
extern int straight_run_counter;
void stop_all();
void resume_all();

bool next_event(unsigned long *var, unsigned long period) {
  unsigned long current = millis();
  if (*var + period < current) {
    *var = current;
    return true;
  }
  return false;
}