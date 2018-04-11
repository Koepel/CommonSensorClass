#ifndef COMMONSENSORCLASS_h
#define COMMONSENSORCLASS_h

// CommonSensorClass
// -----------------
// A wrapper around the Wire library for easier interfacing sensors.
//
// Version 1.00   2018 april 9    by Koepel
// Testing an idea to avoid direct use of the Wire library for 95% of the sensors.
// Consider version 1.00 as Quick and Dirty.
// Licence: Public Domain / At your own risk
//
//
//
// Other existing libraries for a common class.
// ---------------------------------------------------------------------------------
//    I tried really hard, but I could not find an existing library like this one.
//    Other common sensor libraries are more specific.
//    This library is not specific, but still supports auto MSB-LSB order for variables.
//
//
// This library could provide a fast start to build upon.
// ------------------------------------------------------
//    Some sensors require extensive calculations, for example the BME280.
//    That requires a dedicated library.
//    This library can still be useful for such a dedicated library to provide
//    a faster start to interface with the sensor.
//
//
// List of reachable and unreachable goals:
// ----------------------------------------
//    Be able to select an other (software) Wire library.
//
//    The put() function has a baseSize parameter. Is it possible
//    to do that automatically ?
//
//    Allow more than 32 bytes for the AVR microcontrollers 
//    by using multiple I2C transactions.
//
//    Add callback functions. This is usefull when someday there will be
//    a better non-blocking I2C library for faster processors and multitasking.
//
//    Add the possibility of a delay or timeout or poll the sensor for sensors 
//    that require some time for a new sample.
//
//    Make Slave code that works well with this class.
//
//    Support other busses (SPI, 1-Wire, Serial).
//
//    Make it more advanced by writing a struct/class for a sensor that
//    automatically updates the data with the sensor when something
//    is read or written to/from the struct/class. This is possible in C++,
//    but is it useful ?
//
//    


#include <inttypes.h>
#include <Arduino.h>
#include <Wire.h>


#define COMMONSENSORCLASS_VERSION 100

class CommonSensorClass
{
public:

  // Declaring the object. It does not change anything to the pins for I2C.
  CommonSensorClass()
  {
    _data_initialized = false;
  }
  
  ~CommonSensorClass()
  {
  }

  // The begin() function starts the I2C with Wire.begin().
  // From now on the pins are claimed for the I2C bus.
  void begin(
    int deviceAddress,             // The 7-bit I2C address of the sensor.
    int registerAddressSize = 1,   // The size of the register pointer, 0, 1, or 2.
    bool repeatedStart = false)    // Is a repeated start supported/required ? true or false.
  {
    Wire.begin();                  // Assuming it is allowed to call Wire.begin() multiple times.
  
    _device_address = (uint8_t) deviceAddress;
    _register_address_size = (uint8_t) registerAddressSize;
    _repeated_start = repeatedStart;

    _data_initialized = true;
  }


  // The pins for the I2C bus are released.
  void end()
  {
    Wire.end();
    _data_initialized = false;
  }


  // The function put() returns a bool, because the only thing that is needed to know,
  // is whether the put() was successful or not.
  // return value: true = success, false = fail or bus error.
  //
  // The function put() can be used in two ways:
  //    Either with a variable, then the size of the variable itself is used.
  //    Or when the variable is a single byte then the parameter 'size' is used for the bytes to transfer.
  template <typename T> bool put( uint16_t registerAddress, const T &t, int size = 1, bool I2Cstop = true)
  {
    if( !_data_initialized)                 // safety check if .begin() was not called.
    {
      return( false);
    }
    
    const uint8_t *ptr = (const uint8_t*) &t;
  
    Wire.beginTransmission( _device_address);
    
    if( _register_address_size == 1)
    {
      Wire.write( (uint8_t) registerAddress);
    }
    else if( _register_address_size == 2)
    {
      // MSB is written first.
      Wire.write( (uint8_t) (registerAddress >> 8));
      Wire.write( (uint8_t) registerAddress);
    }

    if( size > 0)
    {
      if( size > sizeof( T))
      {
        Wire.write( ptr, (size_t) size);
      }
      else
      {
        Wire.write( ptr, (size_t) sizeof( T));
      }
    }

    uint8_t error = Wire.endTransmission( I2Cstop);     // send true for a stop, false for repeated start.
    
    return( error == 0);              // return true if success, that means true of no error.
  }


  // The function get() returns a bool, because the only thing that is needed to know,
  // is whether the get() was successful or not.
  // return value: true = success, false = fail or bus error.
  //
  // Note: There is a undocumented Wire.requestFrom() in the Arduino Wire library 
  //       that already writes the register address and reads the data from that register.
  //
  // The optional baseSize is when for example 3 bytes (24-bits) are read and
  // put into a 4 byte variable.
  // When for example an array of 3 set of 4 bytes each is used for 24-bit sensor values,
  // then the baseSize should be 3.
  // Converting them to a signed int should be done by the user.
  //
  // Getting the MSB and LSB in the right order should work for every processor.
  // This is achieved by reading the bytes from the sensor assuming MSB first and
  // then shifting them in the right place in a variable. The compiler will take care
  // that the MSB is stored in the MSB and the LSB in the LSB.
  //
  // The function get() can be used in two ways:
  //    Either with a variable, then the size of the variable is used.
  //    The 'baseSize' is the number of bytes in the sensor that belong together.
  //    Or when the variable is a single byte then the parameter 'baseSize' is used 
  //    for the amount of bytes to transfer.
  template <typename T> bool get( uint16_t registerAddress, T &t, int baseSize = 1)
  {
    if( !_data_initialized)         // safety check if .begin() was not called.
    {
      return( false);
    }
    
    uint8_t *ptr = (uint8_t *) &t;
    bool success = true;           // default true, make it false if something fails later on.

    int stepSize = baseSize;
    uint16_t totalSize = sizeof( T);

    // Test if the sensor has 3-byte values that needs to be stored in 4-bytes variables.
    if( stepSize == 3 && totalSize > 3)
    {
      stepSize = 4;
    }

    if( totalSize == 1)
    {
      // The 'baseSize' was used as the number of bytes and data is a pointer
      totalSize = baseSize;
      stepSize = 1;
      baseSize = 1;
    }

    // Test if the sensor uses a register address.
    // Some sensors (like the BH1750) don't have a register address in the sensor.
    if( _register_address_size > 0)
    {
      success = put( registerAddress, NULL, 0);
    }

    if( success)
    {
      uint16_t n = (uint16_t) Wire.requestFrom( _device_address, totalSize);
      if( n == totalSize)
      {
        // The right amount of bytes have been received, 
        // That means that valid received bytes are in the buffer.
        // Therefor it is no need to test every Wire.read for -1.
        
        // MSB is often the first byte in a sensor.
        // However, the Arduino AVR family has the LSB at the lowest memory location.
        // It is therefor not possible to copy the data directly into the variable.
        // By shifting the bytes into their place, the compiler takes automatically
        // care of the right MSB-LSB order.
        //
        // The baseSize is needed, because if 12 bytes would be requested it could
        // be 3 sets of 4 bytes or 4 sets of 3 bytes or 6 sets of 2 bytes, and so on.

        for( unsigned int i=0; i<(totalSize / stepSize); i++)
        {
          switch( baseSize)
          {
          case 0:
            break;
          case 1:
            *ptr++ = (uint8_t) Wire.read();
            break;
          case 2:
            {                     // allow local variables with extra brackets
              uint16_t *p = (uint16_t *) ptr;
              *p = Wire.read();
              *p <<= 8;
              *p |= Wire.read();
              ptr += 2;
            }
            break;
          case 3:
            {                     // allow local variables with extra brackets
              uint32_t *p = (uint32_t *) ptr;
              *p = Wire.read();
              *p <<= 8;
              *p |= Wire.read();
              *p <<= 8;
              *p |= Wire.read();
              ptr += 4;
            }
            break;
          case 4:
            {                     // allow local variables with extra brackets
              uint32_t *p = (uint32_t *) ptr;
              *p = Wire.read();
              *p <<= 8;
              *p |= Wire.read();
              *p <<= 8;
              *p |= Wire.read();
              *p <<= 8;
              *p |= Wire.read();
              ptr += 4;
            }
            break;
          }
        }
      }
      else
      {
        success = false;
      }
    }
    
    return( success);
  }

  // This function checks if the sensor responds, rather than really checking if it exists.
  // Some sensors are not resonding to the I2C bus when busy.
  bool exists()
  {
    Wire.beginTransmission( _device_address);
    uint8_t error = Wire.endTransmission();
    return( error == 0);
  }

  // This function is called readByte(), to avoid confusion with Wire.read().
  // A complete I2C transaction is used to read a single byte.
  // The return value is the data or -1 when it failed.
  int readByte( uint16_t registerAddress)
  {
    uint8_t data;
    if( get( registerAddress, data))
    {
      return( (int) data);
    }
    else
    {
      return( -1);
    }
  }

  void readBytes( uint16_t registerAddress, uint8_t *pData, int size)
  {
    get( registerAddress, *pData, size);
  }


  // This function is called writeByte(), to avoid confusion with Wire.write().
  // A complete I2C transaction is used to write a single byte.
  // No error is returned yet.
  void writeByte( uint16_t registerAddress, uint8_t data)
  {
    put( registerAddress, data);
  }

  void writeBytes( uint16_t registerAddress, uint8_t *pData, int size)
  {
    put( registerAddress, *pData, size);
  }

private:
  // This data describes the sensor.
  uint8_t _device_address;            // The 7-bit I2C address of the sensor.
  uint8_t _register_address_size;     // The size of the register pointer, 0, 1, or 2.
  bool _repeated_start;               // True if the repeated start is supported/required.
  bool _data_initialized;             // Tells if these variables are initialized.
};

#endif
