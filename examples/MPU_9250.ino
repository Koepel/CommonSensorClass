#include <CommonSensorClass.h>

CommonSensorClass sensor;

// Select which serial port is used
// #define SERIAL_PORT SerialUSB
#define SERIAL_PORT Serial


void setup()
{
  SERIAL_PORT.begin( 9600);
  while( !SERIAL_PORT);   // wait for Leonardo and Zero using native USB port.

  SERIAL_PORT.println( "MPU-9250");

  sensor.begin( 0x68);            // Define the sensor. Only the address, the other parameters are default.
  sensor.writeByte( 0x6B, 0);     // wakeup the sensor
}

void loop()
{
  int16_t accel[3];
  int16_t gyro[3];

  sensor.get( 0x3B, accel, 2);      // get 6 bytes total, with 2 bytes per value.
  sensor.get( 0x43, gyro, 2);       // get 6 bytes total, with 2 bytes per value.


  SERIAL_PORT.print( "accel = ");
  for( int i=0; i<3; i++)
  {
    SERIAL_PORT.print( accel[i]);
    SERIAL_PORT.print( ", ");
  }
  SERIAL_PORT.println();

  SERIAL_PORT.print( "gyro = ");
  for( int i=0; i<3; i++)
  {
    SERIAL_PORT.print( gyro[i]);
    SERIAL_PORT.print( ", ");
  }
  SERIAL_PORT.println();

  delay( 1000);
}
