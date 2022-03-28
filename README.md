# Arduino

Code written for my arduino class!



Explanation of RainfallGage code:

How to Create a Simple Rain Gauge

For this tutorial, we will need:

A water level sensor
Wires
Arduino
Short cup

The wiring of this sensor is straightforward with only three pins to connect. On the left is the signal pin which will connect to an analog input in the Arduino. The middle pin is for power, recommended between 3.3V and 5V. The right pin is the ground.

 

The top of the sensor is not designed to be submerged in water, only the bottom of it. Due to this constraint, the cup which will hold this sensor will be as tall as the bottom of the sensor, only 2 inches. If water fills to that point, the rainwater will spill over the sides of the cup and not submerge the top of the sensor. Additionally, to drain the rainwater slowly, there will a needle size pin hole in the side of the cup. This is to allow water to slowly drain away, but also allow water to fill the cup quickly enough to get a decent estimate of the rainfall intensity.

The water level sensor uses water as a kind of resistor.  The more water the sensor is immersed in, the better the conductivity, and the lower the resistance. The less water the sensor is immersed in, the poorer the conductivity, and the higher the resistance. The sensor produces an output voltage according to the resistance, which can determine the water level. If we place this sensor in a cup on its own, we can determine if it is raining or not. If it is raining or if it just recently rained, there will be water in the cup that the sensor will detect. A reading of 0 means no rain, a reading of 420 means the cup is filled half-way, and a reading of 530 means the sensor is fully submerged.

You will be required to calibrate your sensor to determine what its output reading mean. To do this, use a ruler and a cup. Submerge your sensor at known heights (using the ruler) and read the outputs on the serial monitor.

Once the sensor is calibrated, the code for reading the rainfall can be ran. Make sure to change the value for what your sensor states is the value when the water is two inches high. 

This code reads the water level every 15 minutes, and if there is water detected, the code will determine if the water is rising quick enough to send a flood warning. The flood warning is based on the criteria for ‘heavy rain’, rainfall intensity is greater than 0.3 inches per hour
Heavy rain is one factor that may cause flooding, but it should be noted that it isn’t the only factor. Heavy rain is the one factor that this sensor can detect.

Within the loop, the code reads the level from the sensor. If the sensor is reading 0, then there is no water detected and the code will only print “Water Level: Empty”. If the sensor reads anything above 0, the code will first start a timer (millis()) and will print the current water level. The code will continue doing this in 15-minute increments until the water level reaches two inches. This value for two inches is what we calibrated earlier and stored in the variable twoInches. Once this value is reached, the code will calculate the intensity of how quickly the water rose to two inches using the time stored in the timer. If the rain was faster than .3 inches per hour, then the code will send a flood warning message and reset the timer back to 0. If the intensity was not reached, the code will reset the timer back to zero.

There is a third if statement included within the loop in case there is any other reading the sensor produced. Each if statement includes a 15-minute delay as well.

How the sensor works:
https://lastminuteengineers.com/water-level-sensor-arduino-tutorial/

Sensor with LEDs:
https://create.arduino.cc/projecthub/123lYT/simple-water-level-sensor-with-led-ebf3f7

Longer Sensor:
https://www.sparkfun.com/products/10221

Tutorial:
https://arduinogetstarted.com/tutorials/arduino-water-sensor

Water level with LCD screen:
https://www.thegeekpub.com/236571/arduino-water-level-sensor-tutorial/

Intensity of Rainfall:
https://www.baranidesign.com/faq-articles/2020/1/19/rain-rate-intensity-classification

