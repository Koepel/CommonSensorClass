# CommonSensorClass
A wrapper around the Wire library for easier interfacing sensors.

### 2023 : Not a good idea
It turns out that it is not possible to make a "common" class for every sensor. I might remove most of it and keep only the basic interface with .put() and .get(), similar to the EEPROM.put() and EEPROM.get().

<hr>

The library consists of just one file: CommonSensorClass.h  
At this moment it is just an idea. There might be bugs in it and the parameters for .put() and .get() could change.

Reading the x,y,z values from a sensor can be like this:
```
int16_t accel[3];
sensor.get( 0x3B, accel);
```
This will read 6 bytes of data from register address 0x3B and up. The MSB and LSB of each variable is automatically stored in the right location.  
The variable type should match the sensor. If the sensor has 16-bits signed integers, then 16-bits signed integers should be read.

This library is only for the sensor. When a Wire or Wire-compatible library has to be set to certain pins or at a certain speed, that has to be done before the CommonSensorClass is used.

In the future the SPI bus might be added. The CommonSensorClass can use other ways to communicate. The first step for this is a simulated external I2C EEPROM, which is rerouted to the internal EEPROM. See the SimulateEEPROM example.

To do: I might add this check: https://forum.arduino.cc/index.php?topic=670763.msg4514930#msg4514930 but only when SDA and SCL are defined.
