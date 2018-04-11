// an I2C scanner, using the CommonSensorClass
// public domain

#include <CommonSensorClass.h>

CommonSensorClass sensor;

// Select which serial port is used
// #define SERIAL_PORT SerialUSB
#define SERIAL_PORT Serial

void setup()
{
  SERIAL_PORT.begin( 9600);

  while( !SERIAL_PORT);   // wait for Leonardo and Zero using native USB port.
  
  SERIAL_PORT.println( "I2C Scanner");
}

void loop()
{
  bool found = false;
  for( uint8_t i=1; i<=127; i++)    // I2C address 0 is the broadcast message
  {
    sensor.begin( i);               // Start with sensor at this I2C address
    if( sensor.exists())
    {
      found = true;
      
      SERIAL_PORT.print( "Sensor at 0x");
      if( i < 0x10)
      {
        SERIAL_PORT.print( "0");
      }  
      SERIAL_PORT.println( i, HEX);
    }
    sensor.end();                   // Stop with this sensor
  }

  if( !found)
  {
    SERIAL_PORT.println( "No sensor found");
  }
  
  delay( 1500);
}
