#if GYRO_ENABLED
#include <Wire.h>

const int MPU = 0x68; // MPU6050 I2C address

float gyroAngleZ, gyroZ, yaw;
long gyroZ0 = 0;  //Gyro offset = mean value

int16_t getRotationZ()
{
//  uint8_t buffer[14];
//  I2Cdev::readBytes(MPU6050_ADDR, MPU6050_RA_GYRO_ZOUT_H, 2, buffer);
//  return (((int16_t)buffer[0]) << 8) | buffer[1];
  Wire.beginTransmission(MPU);
  Wire.write(0x47); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 2, true); // Read 4 registers total, each axis value is stored in 2 registers
  return ((int16_t)Wire.read()) << 8 | Wire.read(); 
}

void calibration()
{
  //Here we do 100 readings for gyro sensitiv Z-axis output -gyroZ, we sum them and divide them by 100.
  // gyrZ0 mean value of gyroZ raw register values in 100 ms
  
  unsigned short times = 100; //Sampling times
  for (int i = 0; i < times; i++)
  {
    gyroZ = getRotationZ();     // gyroZ - Raw register values gyroscope Z axis
    gyroZ0 += gyroZ; //sum all measured values gyroZ
  }
  gyroZ0 /= times; 
}

void gyro_setup() {
  Wire.begin(21, 22);              // Initialize I2C comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission
  delay(1000);
  // Call this function if you need to get the IMU error values for your module
  calibration();
}

float calcYaw(){
  float dt;
  static unsigned long lastTime;
  unsigned long currentTime = millis();   //current time(ms)
  //get the current timestamp in Milliseconds since epoch time which is
  dt = (currentTime - lastTime) / 1000.0; //Differential time(s)
  lastTime = currentTime;                 //Last sampling time(ms)

  gyroZ = getRotationZ();
  
  float angularZ = (gyroZ - gyroZ0) / 131.0 * dt; //angular z: =t
  if (fabs(angularZ) < 0.05)
    angularZ = 0.00;

  gyroAngleZ += angularZ; //returns the absolute value of the z-axis rotazion integral 
  yaw = - gyroAngleZ;

//  Serial.print("GyZo = "); Serial.print(gyroZ0);  
//  Serial.print(" | gyroZ = "); Serial.print(gyroZ);
//  Serial.print(" | angularZ = "); Serial.print(angularZ);
//  Serial.print(" | yaw = "); Serial.print (yaw);
//  Serial.println();
  return yaw;
}

void gyro_get_direction(double *dir) {
  static unsigned long gyro_event = 0; 

  if (next_event(&gyro_event, 10)) {
    // === Read gyroscope data === //
    *dir = (double)calcYaw();
  }
}
#else
void gyro_setup() {
}

void gyro_get_direction(double *dir) {
  *dir = actual_direction;
}

#endif /* GYRO_ENABLED */