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
// Version 1.01   2018 april 15   by Koepel
// Still an experimental version.
// A sensor with 24 bit data might not work.
// Instead of (too) many parameters with .begin(), a descriptor with 32 bits is used.
//
// Version 1.02   2018 april 18   by Koepel
// Added overloading for an array as parameter.
// Needs more testing.
//
// Version 1.03   2018 april 22   by Koepel
// Added support for every Wire compatible I2C library.
// Using a template with the class of the used Wire library is the best option,
// but the creator in the sketch is more complex.
// Added support for up to 255 bytes transfer.
// Probably added a number of bugs as well. Not ready to be used yet.
//
//
//
// Other existing libraries for a common class.
// ---------------------------------------------------------------------------------
//    I could not find an existing library like this one.
//    Other common sensor libraries are more specific.
//    This library is not specific and has auto MSB-LSB order for variables.
//
//
// This library could provide a quick start to build upon.
// -------------------------------------------------------
//    Some sensors require extensive calculations, for example the BME280.
//    That requires a dedicated library.
//    This library can still be useful for such a dedicated library to provide
//    a quick start to interface with the sensor.
//
//
// List of reachable and unreachable goals:
// ----------------------------------------
//    Detect if the standard Arduino Wire library is used, and then
//    create the CommonSensorClass for that automatically ?
//
//    Add callback functions for non-blocking I2C libraries.
//
//    Add the possibility of a delay or timeout or poll the sensor for sensors 
//    that require some time for a new sample.
//
//    Make Slave code that works well with this class.
//
//    Support other busses (SPI, 1-Wire, Serial).
//
//    Allow the use of multiple sensors located at different I2C busses,
//    and those I2C busses use different kinds of Wire libraries.
//    For example a sensor at the hardware I2C bus and another sensor at 
//    a software I2C bus and both are caught in an array of the CommonSensorClass object.
//
//    Simulate an external I2C EEPROM for the AVR internal EEPROM.
//
//


#include <inttypes.h>
#include <Arduino.h>


#define COMMONSENSORCLASS_VERSION 103


// The buffer size of the used Wire library.
// The size is 32 for the Arduino AVR microcontrollers.
// It is less for the TinyWire and more for the Arduino SAMD processors.
// This should be set automatically to the right value, 
// but there is no way to get that number from the used Wire library.
#define COMMONSENSORCLASS_WIRE_BUFFER_SIZE 32


// CSC is short for COMMONSENSORCLASS
// A value of zero is defined as not being initialized yet.
#define CSC_NO_REGISTER_ADDRESS       0x00000001  // The sensor has no register address.
#define CSC_REGISTER_ADDRESS_SIZE_1   0x00000002  // The sensor register address is one byte.
#define CSC_REGISTER_ADDRESS_SIZE_2   0x00000004  // The sensor register address is two bytes.
#define CSC_REPEATED_START            0x00000010  // The sensor requires/supports a repeated start.
#define CSC_24BIT_SIGNED              0x00000020  // The sensor has 24-bit signed data.
#define CSC_24BIT_UNSIGNED            0x00000040  // The sensor has 24-bit unsigned data;
#define CSC_SENSOR_LSB_FIRST          0x00000080  // The sensor has the register address and data as LSB first.


template <class T_WIRE_LIBRARY> class CommonSensorClass
{
public:

  // Declaring the object. 
  // It does not change anything to the pins for I2C yet.
  // The constructure uses the object of the used Wire library.
  // The "_WireLib" is the object stored in this template.
  CommonSensorClass( T_WIRE_LIBRARY& WireLibrary): _WireLib( WireLibrary)
  {
    _descriptor = 0;                 // reset the descriptor of the sensor
  }
  
  ~CommonSensorClass()
  {
  }

  // The begin() function starts the I2C with Wire.begin().
  // From now on the pins are claimed for the I2C bus.
  void begin(
    int deviceAddress,               // The 7-bit I2C address of the sensor.
    uint32_t sensorDescriptor = CSC_REGISTER_ADDRESS_SIZE_1)  // A bitwise combination of the CSC defines.
  {
    _WireLib.begin();                   // Assuming it is allowed to call Wire.begin() multiple times.
  
    _device_address = (uint8_t) deviceAddress;
    _descriptor = sensorDescriptor;  // Store the desciptor, no error checking yet.
  }


  // The pins for the I2C bus are released.
  void end()
  {
    _WireLib.end();
    _descriptor = 0;
  }


  // The function put() returns a bool, because the only thing that is needed to know,
  // is whether the put() was successful or not.
  // return value: true = success, false = fail or bus error.
  //
  // The function put() can be used in two ways:
  //    Either with a variable, then the size of the variable itself is used.
  //    Or when the variable is a single byte then the parameter 'size' is used for the bytes to transfer.
  template <typename T> bool put( uint16_t registerAddress, const T (&t), size_t size = 1, bool I2Cstop = true)
  {
    if( _descriptor == 0)                         // safety check if .begin() was called.
    {
      return( false);
    }
    
    const uint8_t *ptr = (const uint8_t*) &t;
    bool success = true;           // default true, make it false if something fails later on.

    unsigned int totalSize = (unsigned int) sizeof( T);

    // Test if the I2C action has to be done without data.
    if( size == 0)
    {
      totalSize = 0;
    }
    else
    {
      // Test if the size was used as the number of bytes and data is a pointer
      if( totalSize == 1)
      {
        totalSize = size;
      }
    }

    // If more data needs to be transmitted, then split it into seperate parts.
    // Increase the registerAddress for each part.
    // It is allowed to do one I2C bus transaction without data.
    
    do
    {
      unsigned int bytesToWrite = min( COMMONSENSORCLASS_WIRE_BUFFER_SIZE, totalSize);
  
      _WireLib.beginTransmission( _device_address);

      if( (_descriptor & CSC_NO_REGISTER_ADDRESS) != 0)
      {
        // the sensor has no register address
      }
      else if( (_descriptor & CSC_REGISTER_ADDRESS_SIZE_1) != 0)
      {
        _WireLib.write( (uint8_t) registerAddress);
      }
      else if( (_descriptor & CSC_REGISTER_ADDRESS_SIZE_2) != 0)
      {
        if( (_descriptor & CSC_SENSOR_LSB_FIRST) != 0)
        {
          // Is there a sensor with the register address LSB first ?
          _WireLib.write( (uint8_t) registerAddress);
          _WireLib.write( (uint8_t) (registerAddress >> 8));
        }
        else
        {
          // MSB is written first for most sensors.
          _WireLib.write( (uint8_t) (registerAddress >> 8));
          _WireLib.write( (uint8_t) registerAddress);
        }
      }

      if( bytesToWrite > 0)
      {
        _WireLib.write( ptr, (size_t) bytesToWrite);
      }

      uint8_t error = _WireLib.endTransmission( I2Cstop);     // send true for a stop, false for repeated start.
      if( error != 0)
      {
        success = false;                      // Some kind of I2C bus error, stop sending data.
      }

      totalSize -= bytesToWrite;
      registerAddress += bytesToWrite;
    }
    while( totalSize > 0 && success);
    
    return( success);              // return true if success, that means true if no error.
  }

  template <typename T, size_t N> bool put( uint16_t registerAddress, const T (&t)[N])
  {
    size_t elementSize = sizeof( T) / N;
    return( put( registerAddress, t, elementSize));
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
  template <typename T> bool get( uint16_t registerAddress, T (&t), size_t size = 1)
  {
    if( _descriptor == 0)                  // safety check if .begin() was not called.
    {
      return( false);
    }
    
    uint8_t *ptr = (uint8_t *) &t;
    bool success = true;           // default true, make it false if something fails later on.

    unsigned int totalSize = (unsigned int) sizeof( T);
    unsigned int bytesPerElement = (unsigned int) size;

    // Test if the 'size' was used as the number of bytes and data is a pointer
    if( totalSize == 1)
    {
      totalSize = size;
      bytesPerElement = 1;
    }

    // Test if the sensor uses a register address.
    // Some sensors (like the BH1750) don't have a register address in the sensor.
    if( (_descriptor & CSC_NO_REGISTER_ADDRESS) == 0)           // the bit for no register address is not active ?
    {
      success = put( registerAddress, NULL, 0);
    }

    if( success)
    {
      // If more data is requested than the buffer size of the used Wire library,
      // then split the request into seperate parts.
      // This is no need to write the register address, 
      // the Wire.requestFrom() is called multiple times.
      do
      {
        unsigned int bytesToRead = min( COMMONSENSORCLASS_WIRE_BUFFER_SIZE, totalSize);

        uint16_t n = (uint16_t) _WireLib.requestFrom( _device_address, bytesToRead);
        if( n == bytesToRead)
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

          for( unsigned int i=0; i<(totalSize / bytesPerElement); i++)
          {
            switch( bytesPerElement)
            {
            case 0:
              break;
            case 1:
              *ptr++ = (uint8_t) _WireLib.read();
              break;
            case 2:
              {                     // allow local variables with extra brackets
                uint16_t data16 = 0;
                if( (_descriptor & CSC_SENSOR_LSB_FIRST) == 0)
                {
                  // MSB first, this is normal
                  data16 = uint16_t( _WireLib.read()) << 8;
                  data16 |= _WireLib.read();
                }
                else
                {
                  // LSB first, this is not normal, but some sensors have LSB first.
                  data16 = _WireLib.read();
                  data16 |= uint16_t( _WireLib.read()) << 8;
                }
                uint16_t *p = (uint16_t *) ptr;
                *p = data16;
                ptr += 2;
              }
              break;
            case 4:
              // The data needs to be stored into 4 bytes.
              // Test if the sensor has 3-byte values that needs to be stored in 4-bytes variables.
              if( (_descriptor & CSC_24BIT_SIGNED) != 0 && totalSize > 3)
              {
                uint32_t data3x8 = 0;
                if( (_descriptor & CSC_SENSOR_LSB_FIRST) == 0)
                {
                  // MSB first, this is normal
                  data3x8 = uint32_t( _WireLib.read()) << 16;
                  data3x8 |= uint32_t( _WireLib.read()) << 8;
                  data3x8 |= _WireLib.read();
                }
                else
                {
                  // LSB first, this is not normal, but some sensors have LSB first.
                  data3x8 = _WireLib.read();
                  data3x8 |= uint32_t( _WireLib.read()) << 8;
                  data3x8 |= uint32_t( _WireLib.read()) << 16;
                }

                // extend sign to the MSB byte.
                if( (data3x8 & 0x00800000) != 0)
                {
                  data3x8 |= 0xFF000000;
                }

                uint32_t *p = (uint32_t *) ptr;
                *p = data3x8;
                ptr += 4;
              }
              else if( (_descriptor & CSC_24BIT_UNSIGNED) != 0 && totalSize > 3)
              {
                uint32_t data3x8 = 0;
                if( (_descriptor & CSC_SENSOR_LSB_FIRST) == 0)
                {
                  // MSB first, this is normal
                  data3x8 = uint32_t( _WireLib.read()) << 16;
                  data3x8 |= uint32_t( _WireLib.read()) << 8;
                  data3x8 |= _WireLib.read();
                }
                else
                {
                  // LSB first, this is not normal, but some sensors have LSB first.
                  data3x8 = _WireLib.read();
                  data3x8 |= uint32_t( _WireLib.read()) << 8;
                  data3x8 |= uint32_t( _WireLib.read()) << 16;
                }
                uint32_t *p = (uint32_t *) ptr;
                *p = data3x8;
                ptr += 4;
              }
              else
              {
                uint32_t data32 = 0;
                if( (_descriptor & CSC_SENSOR_LSB_FIRST) == 0)
                {
                  // MSB first, this is normal
                  data32 = uint32_t( _WireLib.read()) << 24;
                  data32 |= uint32_t( _WireLib.read()) << 16;
                  data32 |= uint32_t( _WireLib.read()) << 8;
                  data32 |= _WireLib.read();
                }
                else
                {
                  // LSB first, this is not normal, but some sensors have LSB first.
                  data32 = _WireLib.read();
                  data32 |= uint32_t( _WireLib.read()) << 8;
                  data32 |= uint32_t( _WireLib.read()) << 16;
                  data32 |= uint32_t( _WireLib.read()) << 24;
                }
                uint32_t *p = (uint32_t *) ptr;
                *p = data32;
                ptr += 4;
              }
              break;
            }
          }
  
          totalSize -= bytesToRead;
        }
        else
        {
          // The Wire.requestFrom() failed.
          success = false;
        }
      } while( totalSize > 0 && success);
    }
    
    return( success);
  }

  template <typename T, size_t N> bool get( uint16_t registerAddress, const T (&t)[N])
  {
    size_t elementSize = sizeof( T) / N;
    return( get( registerAddress, t, elementSize));
  }
  
  
  
  // This function checks if the sensor responds, rather than really checking if it exists.
  // Some sensors are not resonding to the I2C bus when busy.
  bool exists()
  {
    _WireLib.beginTransmission( _device_address);
    uint8_t error = _WireLib.endTransmission();
    return( error == 0);              // if error is 0, then the sensor exists and this returns true.
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
  T_WIRE_LIBRARY & _WireLib;          // The object (by template) of the used Wire library

  // This data describes the sensor.
  uint8_t _device_address;            // The 7-bit I2C address of the sensor. Zero is allowed.
  uint32_t _descriptor;               // Describes the sensor. Zero means not initialized yet.
};

#endif
