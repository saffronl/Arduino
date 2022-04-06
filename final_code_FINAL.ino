/* Water Safety Project
* April 4th, 2022 
* CPLN571 Sensing the City 
* University of Pennsylvania 
* Saffron Livaccari, Jie Wang, Jiali Yao, Yebei Yao

 code is adapted from https://circuitdigest.com/microcontroller-projects/arduino-ph-meter and ELEGOO Lessons 15 
 and from: https://lastminuteengineers.com/water-level-sensor-arduino-tutorial/ 

*/

#include <LiquidCrystal.h> // load the library for LCD display
#include <OneWire.h> // load the library for LCD display
#include <DallasTemperature.h> // load the library for Temperature

#define PH_PIN A0 // ph input
#define Turbidity_PIN A1 // turbidity input
#define ONE_WIRE_BUS 5 // for temperature, connect temperature yellow pin to Pin5
#define waterSensor 4 // for water level
#define sensorPin A2 // for water level

/// For LCD Screen:
//pins on LCD     BS  E  D4 D5  D6 D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); // identify each pin on LCD display connected to according pin on Arduino
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

/// For pH value:
float calibration_value = 21.34 + 2; // could "+/-" a number to adjust the calibration value based on the pH of distilled water
int phval = 0; // set the initial ph value as 0
unsigned long int avgval;
float volt_ph;
float ph_act;

/// For temperature:
int buffer_arr[10],temp; // define an array for later data smoothing
int delaytime = 3000;

/// For water level sensor:
int val = 0;
float heavyRain = 8.33; //0.0000000833 inches per milliseconds * 100000000 
// Since the float value only stores 6 decimals, and 0.0000000833 is longer than 6 decimals, 
// multiply this value by "100000000" to get a value that can be stored as a float 
float intensity;
int one_half_Inches = 600; // Change this value based on your sensor's calibration
extern volatile unsigned long timer0_millis; // reset the timer back to 0

/// For turbidity:
float ntu;
float volt_turbidity = 0;

void setup() {
  lcd.begin(16, 2); // set the LCD display as 16 columns and 2 rows
  pinMode(waterSensor, OUTPUT); // for water level sensor
  digitalWrite(waterSensor, LOW); // Set to LOW so no power flows through the sensor
  Serial.begin(9600); // the serial monitor setting
  sensors.begin();
}

void loop() { 
  
  //###### for pH ######
  //read 10 sample analog values and store them in an array to smooth the output value.
  for(int i=0;i<10;i++) { 
    buffer_arr[i]=analogRead(PH_PIN); // create an array of 10 and fill in the array with PH values
    delay(30);
    }
    //sort the Analog values received in ascending order, because we need to calculate the 
    //running average of samples later
   for(int i=0;i<9;i++) {
    for(int j=i+1;j<10;j++) {
      if(buffer_arr[i]>buffer_arr[j]) {
        temp=buffer_arr[i];
        buffer_arr[i]=buffer_arr[j];
        buffer_arr[j]=temp;
        }
     }
   }
   
   //calculate the average of a 6 centre sample Analog values
   avgval = 0;
   for(int i=2;i<8;i++)
   avgval += buffer_arr[i];
   
   //converted the average analog value into actual pH value
   volt_ph = (float)avgval*5.0/1024/6;
   ph_act = -5.70 * volt_ph + calibration_value;
   
   // #########for Turbidity########
   for(int i=0; i<800; i++) {
        volt_turbidity += ((float)analogRead(Turbidity_PIN)/1024)*5;
    }
    volt_turbidity = volt_turbidity/800;
    volt_turbidity = round_to_dp(volt_turbidity,1);
    if(volt_turbidity < 2.5){ // if the voltage is too low, read a high turbidity
      ntu = 3000;
    }
    else{ // convert voltage to NTU
      ntu = (-1120.4*(volt_turbidity*volt_turbidity)) + (5742.3*volt_turbidity) -4353.8; 
    }

    // Calcalatees a safety value based on pH and turbidity !
    int safety_value = safety(ph_act, ntu);

    // ######### Temperature ######## 
    int temp =  sensors.getTempFByIndex(0);

    // ######### Print on the LCD Screen ######## 
    PrintLCD(ph_act, ntu, temp, safety_value);
    
    // ######### Water Level ######## 
    int level = readSensor();
    // first if statement

    if (level == 0) {
     //// if there is no rainfall happening, just print out all the values on the LCD screen
       PrintLCD(ph_act, ntu, temp, safety_value);
      }
      
    
    if (level > 0) { // if there is water detected
       //if water is detected, start timer
      noInterrupts ();
      timer0_millis = 0; // reset the timer first
      interrupts ();
      timer0_millis = millis(); // start the timer
    
       // if water level hits >= fullySubmerged, get the time and calculate speed
       if (level >= one_half_Inches){ // fullySubmerged reading is roughly 1.5 inches

          intensity = intensity_converted(timer0_millis);
          Serial.print("Intensity (in/millis): ");
          Serial.println(intensity);
      
          if (intensity >= heavyRain){ // if the rainfall speed is greater than .3 in/hour
            Serial.print("The rainfall intensity is: ");
            Serial.println(intensity);
            Serial.println("The rainfall is very heavy! Flood Warning!");
            
            // If there is heavy rainfall, just print out Flood Warning on the LCD Screen
            lcd.setCursor(0, 0);
            lcd.print("HEAVY RAINFALL  ");
            lcd.setCursor(0, 1);
            lcd.print("FLOOD WARNING   ");

            
            delay(86400000);  // delay and hold this message for one hour
            }
           
          else { // If it isnt heavy rainfall
            noInterrupts ();
            timer0_millis = 0; // reset timer back to 0
            interrupts (); 
            
            // If there is no heavy rainfall, just print out normal readings on the LCD Screen
            PrintLCD(ph_act, ntu, sensors.getTempFByIndex(0), safety_value);
            }
        }
    }

    // ######### Print values on the Serial Monitor ######## 
    // print the pH value in serial monitor:
    Serial.print("pH Val: ");
    Serial.println(ph_act);  // ph value
    Serial.print("Turbidity Val: ");
    Serial.println(ntu); // turbidity value
    // Serial.print(volt2);
    Serial.print("Celsius temperature: "); // Temperature
    Serial.println(sensors.getTempCByIndex(0)); 
    Serial.print("Fahrenheit temperature: ");
    Serial.println(temp);
    Serial.print("Water Level: "); // Water level
    Serial.println(level);
    Serial.print("Contact safe?   ");
    safety_SerialMonitor(safety_value); // prints contact safe on serial monitor
    Serial.println("");

    delay(delaytime);
}


// Function for Turbidity
float round_to_dp(float in_value, int decimal_place) {
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}


// Function for reading Water Level
int readSensor() {
  digitalWrite(waterSensor, HIGH);
  delay(10); // wait 10 milliseconds
  val = analogRead(sensorPin);
  digitalWrite(waterSensor, LOW);
  return val;
  delay(1000); // only read sensor every 15 minutes
}


// Function for calculated rainfall intensity
float intensity_converted(float milliseconds) {
  return (1.5/(milliseconds)* 100000000);
  // Rainfall intensity is .3 inches per hour for Heavy Rainfall
  // This equation has '1.5' becuase we set our trigger (variable one_half_Inches)
  // to be equivalent to 1.5 inches, and rainfall intensity is measured as inches per hour
  // currentMillis/3600000 converts milliseconds to hour
}


// Function that determines safety of water contact
int safety(float ph_funct, float ntu_funct) {
  int safety_num;
  if (ph_funct<6.5 || ph_funct>8.5) { // if pH is outside of 6.5 to 8.5 range
    if (ntu_funct > 400) { // if turbidity is higher than 50
      safety_num = 2; // 2 for high turb AND out of range
    }
    else { // if turbidity < 50
      safety_num = 1; // 1 for out of range pH only, but turbidity is in range
    }
  }
  else { // if turbidity is inside the range of 6.5 to 8.5
    if (ntu_funct > 400) {
      safety_num = 1; // 1 for high turb only, pH is safe
    }
    else {// pH normal and turbidity is <50
      safety_num = 0;
    }
  }
  return safety_num;

  // Safe Values are
  // ph between 6.5 and 8.5
  // turbidity below 50
}


// Function that prints out the safety onto serial monitor
void safety_SerialMonitor(int value) {
  if (value == 2) {
    Serial.println("Not safe for contact");
  }
  else if (value == 1) {
    Serial.println("Safe for fishing or boating ONLY");
  }
  else if (value == 0) {
    Serial.println("Safe for swimming");
  }
  else {
    Serial.println("Out of Range");
  }
}


// Function that prints out the safety onto LCD Screen
void safety_LCD(int value) {
  if (value == 2) {
    lcd.setCursor(0, 0);
    lcd.print("NOT SAFE      ");
    lcd.setCursor(0, 1);
    lcd.print("FOR CONTACT     ");
  }
  else if (value == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Safe for fishing");//16 char
    lcd.setCursor(0, 1);
    lcd.print("Safe for boating");//16 char
  }
  else if (value == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Safe for        ");//16 char
    lcd.setCursor(0, 1);
    lcd.print("swimming        ");//16 char
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("Out of Range    ");//16
    lcd.setCursor(0, 1);
    lcd.print("                ");//16 char
  }
}


// Function for printing on LCD Screen
// https://www.tweaking4all.com/hardware/arduino/arduino-programming-course/arduino-programming-part-6/
void PrintLCD(float ph_funct, float ntu_funct, int temp_funct, int safety_funct) {
  lcd.setCursor(0, 0);  // clear the screen
  lcd.print("                ");//16 char
  lcd.setCursor(0, 1);
  lcd.print("                ");//16 char
  
  lcd.setCursor(0, 0); // ph
  lcd.print("pH Value:       ");//16 char
  lcd.setCursor(0, 1);
  lcd.print(ph_funct); //ph value has  
  delay(delaytime - 1000); // Change features
  
  lcd.setCursor(0, 0); // turbidity
  lcd.print("Turbidity (NTU):");
  lcd.setCursor(0, 1); // second row
  lcd.print(ntu_funct); // ntu value
  delay(delaytime - 1000); // Change features
  
  lcd.setCursor(0, 0); // temperature
  lcd.print("Temperature (F):"); //16char
  lcd.setCursor(0, 1); // second row
  lcd.print(temp_funct); // temp value
  lcd.setCursor(2, 1);
  lcd.print("        ");
  delay(delaytime - 1000); // Change features
  
  safety_LCD(safety_funct); // contact safe ?
  delay(delaytime); // Change features 
}
