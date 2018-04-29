// Example sketch for the CommonSensorClass
// public domain.
//
// The CommonSensorClass is used together with the SimEE object to simulate
// an external I2C EEPROM, but using the internal AVR EEPROM instead.
// The simulated EEPROM has a 16-bit register address.


// Define the used I2C bus. 
// This is not a I2C bus but a simulation of external I2C EEPROM.
#include "SimEE.h"
#include <CommonSensorClass.h>
SIMEE SimEE;
CommonSensorClass <SIMEE> simmy( SimEE);


// Select which serial port is used
// #define SERIAL_PORT SerialUSB
#define SERIAL_PORT Serial


void setup()
{
  SERIAL_PORT.begin( 9600);
  while( !SERIAL_PORT);           // wait for Leonardo and Zero using native USB port.

  SERIAL_PORT.println();
  SERIAL_PORT.println( "CommonSensorClass + SimEE");
  SERIAL_PORT.println( "-------------------------");

  simmy.begin( 0x10, CSC_REGISTER_ADDRESS_SIZE_2); // Define the sensor.

  if( !simmy.exists())           // Check if sensor exists on the I2C bus.
  {
    SERIAL_PORT.println( "Error, sensor not found");
  }

  const int dataOut[10] = { 10, 20, 30, 40, 100, 1000, 10000, -1, -200, -4000};
  simmy.put( 0x10, dataOut);


  SERIAL_PORT.print( "Read data via CommonSensorClass = ");
  int dataIn[10];
  simmy.get( 0x10, dataIn);


  for( int i=0; i<10; i++)
  {
    SERIAL_PORT.print( dataIn[i]);
    SERIAL_PORT.print( ", ");
  }
  SERIAL_PORT.println();
}


void loop()
{
}
