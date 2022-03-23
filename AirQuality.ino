/* 
Code written by Saffron Livaccari and Jiali Yao.
Lab07
March 12nd, 2022 

This code uses the Adafruit GPS and Grove Dust Sensor together to collect PM data in the city.
This code writes a file onto the SD card that contains the date, time in EST, latitude, longitude, and PM concentration.

 https://www.seeedstudio.com/Grove-Dust-Sensor-PPD42N-p-1050.html
  https://www.shinyei.co.jp/stc/eng/optical/main_ppd42.html
 
 JST Pin 1 (Black Wire)  => Arduino GND
 JST Pin 3 (Red wire)    => Arduino 5VDC
 JST Pin 4 (Yellow wire) => Arduino Digital Pin 8
*/

unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sample 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

#include <SD.h> //Load SD card library
#include<SPI.h> //Load SPI Library

#include <Adafruit_GPS.h>    //Install the adafruit GPS library
#include <SoftwareSerial.h> //Load the Software Serial library
SoftwareSerial mySerial(8,7); //Initialize the Software Serial port, make sure the arduino is set to Soft Serial
Adafruit_GPS GPS(&mySerial); //Create the GPS Object

// Define the variables first:
String NMEA1; //Variable for first NMEA sentence
String NMEA2; //Variable for second NMEA sentence
char c; //to read characters coming from the GPS
char filename[15]; // defines filename as a string of 15 characters
int chipSelect = 10; //chipSelect pin for the SD card Reader
File mySensorData; //Data object you will write your sesnor data to

void setup() 
{
  Serial.begin(115200); //Turn on serial monitor - baud rate is 
  pinMode(4,INPUT);
  starttime = millis();//get the current time;
  
  GPS.begin(9600); //Turn on GPS at 9600 baud
  GPS.sendCommand("$PGCMD,33,0*6D");  //Turn off antenna update nuisance data
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //Request RMC and GGA Sentences only
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); //Set update rate to 1 hz
  delay(1000); 
  
  pinMode(10, OUTPUT); //Must declare 10 an output and reserve it to keep SD card happy
  SD.begin(chipSelect); //Initialize the SD card reader
  
  if (SD.exists("NMEA.txt")) { //Delete old data files to start fresh
    SD.remove("NMEA.txt");
  }

  // Write new GPS logs to a new textfile each time to not overwrite previous files
  strcpy(filename, "GPSLOG00.TXT"); // make filename equal to GPSLOG00.TXT
  for (uint8_t i = 0; i < 100; i++) { // loops through the value i until it reached 99
    filename[6] = '0' + i/10; //this is for the first number (6th character) in GPSLOG00.TXT
                              //the "/" means to divide
                              //Since most of the numbers divided by 10 are decimals, this number will just be 0
                              //when i = 10, 10/10 will make filename[6]=1
                              //when i = 20, 20/10 will make filename[6]=2, etc.
    filename[7] = '0' + i%10; //this is for the second number (7th character) in GPSLOG00.TXT
                              //the "%" means remainder
                              //2%10 = 2, 33%10 = 3, etc.
                              //As i increases, this will value remain between 0-9
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) { // if the filename does not exist, break this loop and use that value for the filename
      break;
    }
  }
  //Serial.print("File Name: ");
  //Serial.println(filename);
}

void loop()  {
    readGPS();
    if(GPS.fix==1) {
      duration = pulseIn(4, LOW);
      lowpulseoccupancy = lowpulseoccupancy+duration;
      if ((millis()-starttime) > 30000)//if the sample time == 30s
        {
          mySensorData = SD.open(filename, FILE_WRITE); // this creates a new file on the SD card
          if (GPS.hour >= 5) {  // if the hour is greater than 5 then just subract 5 hours to get the time into EST
            if ((GPS.hour-5) < 10) { mySensorData.print('0'); }// If the hour is only one digit in length (for example, 1 or 8) 
            mySensorData.print(GPS.hour-5, DEC);                // then make 0 the first digit instead
            mySensorData.print(':'); }
          else { mySensorData.print(GPS.hour+19, DEC); // if the hour is less than 5, add 19 hours to get the time into EST
            mySensorData.print(':'); } //print the hour followed by :
          if (GPS.minute < 10) { mySensorData.print('0'); }// If the minute is only one digit in length then make 0 the first digit instead
          mySensorData.print(GPS.minute, DEC); mySensorData.print(':');//print the minute followed by :
          if (GPS.seconds < 10) { mySensorData.print('0'); } // If the second is only one digit in length, then make 0 the first digit instead
          mySensorData.print(GPS.seconds, DEC); mySensorData.print(", "); // print the seconds followed by , and a space
          //Serial.print("Seconds: ");
          //Serial.println(GPS.seconds, DEC);
          mySensorData.print(GPS.day, DEC); mySensorData.print('/'); // print the day and /
          mySensorData.print(GPS.month, DEC); mySensorData.print("/20"); // print the month and /20 (for the start of the year in the next line)
          mySensorData.print(GPS.year, DEC);mySensorData.print(", "); // pring the year and , 
          mySensorData.print(GPS.latitudeDegrees, 4); //prints the latitude in degrees
          Serial.println(GPS.latitudeDegrees, 4);
          mySensorData.print(", ");
          mySensorData.print(GPS.longitudeDegrees, 4); // prints the longitude in degrees
           // closes the file on the SD card
          ratio = lowpulseoccupancy/(300000.0);  // Integer percentage 0=>100
          concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
          mySensorData.print(", ");
          mySensorData.println(concentration);
          lowpulseoccupancy = 0;
          starttime = millis();
          mySensorData.close();
    }
  }
}

void readGPS() {
  clearGPS(); // This function is defined below
  while(!GPS.newNMEAreceived()) { //Loop until you have a good NMEA sentence
    c=GPS.read(); // The variable c was defined as character string in the beginning of the code
    // GPS.read() reads the NMEA sentence from the GPS
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence
  NMEA1=GPS.lastNMEA(); //make the NMEA sentence equal to the variable NMEA1
  
   while(!GPS.newNMEAreceived()) { //Loop until you have a good NMEA sentence
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence
  NMEA2=GPS.lastNMEA(); //make the NMEA sentence equal to the variable NMEA2
  
  //Serial.println(NMEA1); //Prints NMEA1 into the serial monitor
  //Serial.println(NMEA2); //Prints NMEA2 into the serial monitor
  Serial.println(""); //Prints an enter
  Serial.print("Fix: ");
  Serial.print((int)GPS.fix); // Prints if the GPS has a fix or not as an integer, we use this line to check if GPS identifies any valid coordinates in the current environment
}

void clearGPS() {  //Clear old and corrupt data from serial port 
  while(!GPS.newNMEAreceived()) { //Loop until you have a good NMEA sentence
    c=GPS.read(); //
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence
  
  while(!GPS.newNMEAreceived()) { //Loop until you have a good NMEA sentence
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence
   while(!GPS.newNMEAreceived()) { //Loop until you have a good NMEA sentence
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA()); //Parse that last good NMEA sentence
  
}
