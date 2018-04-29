#ifndef SIMEE_h
#define SIMEE_h

#include <inttypes.h>
#include <Arduino.h>
#include <EEPROM.h>

#define SIMEE_I2C_ADDRESS 0x10

class SIMEE
{
public:
  SIMEE();
  void begin( void);
  void end( void);
  void beginTransmission( uint8_t);
  uint8_t endTransmission( void);
  uint8_t endTransmission( uint8_t);
  uint8_t requestFrom( uint8_t, uint8_t);
  size_t write( uint8_t);
  size_t write( const uint8_t *, size_t);
  int available( void);
  int read( void);

private:
  uint16_t _index;
  uint16_t _registerAddress;
  int _length;
};

#endif
