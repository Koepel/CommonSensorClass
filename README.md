# CommonSensorClass
A wrapper around the Wire library for easier interfacing sensors.


The library consists of just one file: CommonSensorClass.h  
At this moment it is just an idea. There might be bugs in it and the parameters for .put() and .get() could change.

Reading the x,y,z values from a sensor can be like this:
```
int16_t accel[3];
sensor.get( 0x3B, accel);
```
This will read 6 bytes of data from register address 0x3B and up. The MSB and LSB of each variable is automatically stored in the right location.  
The variable type should match the sensor. If the sensor has 16-bits signed integers, then 16-bits signed integers should be read.
