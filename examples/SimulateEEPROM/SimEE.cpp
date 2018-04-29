// This is the absolute and bare minimum to simulate an external I2C EEPROM
// by using the internal EEPROM of the AVR microcontrollers with the CommonSensorClass.
//
// public domain
//

#include "SimEE.h"

SIMEE::SIMEE()
{
}

void SIMEE::begin( void)
{
}

void SIMEE::end( void)
{
}

void SIMEE::beginTransmission( uint8_t address)
{
  _index = 0;
}

uint8_t SIMEE::endTransmission( void)
{
  return( 0);
}

uint8_t SIMEE::endTransmission( uint8_t stop)
{
  return( 0);
}

uint8_t SIMEE::requestFrom( uint8_t address, uint8_t length)
{
  _length = length;
  return( length);
}

size_t SIMEE::write( uint8_t data)
{
  write( &data, 1);
  return( 1);
}

size_t SIMEE::write( const uint8_t *pData, size_t length)
{
  for( int i=0; i<(int)length; i++)
  {
    if( _index == 0)
    {
      // MSB byte of register address
      _registerAddress &= 0x00FF;   // clear MSB
      uint16_t data = *pData++;
      data <<= 8;
      _registerAddress |= data;     // add MSB
    }
    else if( _index == 1)
    {
      // LSB byte of register address
      _registerAddress &= 0xFF00;   // clear LSB
      uint16_t data = *pData++;
      _registerAddress |= data;     // add LSB
    }
    else
    {
      EEPROM.write( _registerAddress, *pData++);
      _registerAddress++;
    }
    _index++;
  }
  return( length);
}

int SIMEE::available( void)
{
  return( _length);
}

int SIMEE::read( void)
{
  int data = EEPROM.read( _registerAddress);
  _registerAddress++;
  _length--;
  return( data);
}
