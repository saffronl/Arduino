
//Start by defining
#define sensorPower 7
#define sensorPin A0

int val = 0;
float heavyRain = .3; //.3 inches per hour
float intensity;
float intensity_converted;
int twoInches = 500;
int level = readSensor();
unsigned long currentMillis = 0; // reset the timer if it started

void setup() {
  // put your setup code here, to run once:
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);
  Serial.begin(9600);
}

void loop() {
  // first if statement
  if (level == 0) { // if no water is detected
    // if there is no water detected, do nothing
    Serial.println("Water Level: Empty");
    delay(900000); // only read sensor every 15 minutes
    }
    // second if statement
  else if (level > 0) { // if water is detected
    //if water is detected, start timer
    currentMillis = millis();
    Serial.print("Water Level: ");
    level = readSensor();
    Serial.println(level);
    
    // if water level hits >= fullySubmerged, get the time and calculate speed
    if (level >= twoInches){ // fullySubmerged reading is roughly 2 inches
      level = readSensor();
      Serial.print("Water Level: ");
      Serial.println(level);
      
      intensity = level/currentMillis;
      // Rainfall intensity is .3 inches per hour for Heavy Rainfall
      // This equation has '2' becuase we set our trigger (variable twoInches)
      // to be equivalent to two inches, and rainfall intensity is measured as
      // inches per hour
      // currentMillis/3600000 converts milliseconds to hour
      intensity_converted = (2/(currentMillis/3600000));
      Serial.print("Intensity (in/hour): ");
      Serial.println(intensity_converted);
      
      if (intensity_converted >= violentRain){ // equivalent to 2in/hour
        Serial.print("The rainfall intensity is: ");
        Serial.println(intensity_converted);
        Serial.println("The rainfall is very heavy! Flood Warning!");
        currentMillis = 0; // reset the timer back to 0
        }
      else {
        currentMillis = 0; // reset the timer back to 0
      }
      }
     delay(900000); // only read sensor every 15 minutes
    }
    // third if statement
    else { // catch all for any other readings.
      Serial.print("Water Level: ");
      level = readSensor();
      Serial.println(level);
      delay(900000); // only read sensor every 15 minutes
      }
 }


int readSensor() {
  digitalWrite(sensorPower, HIGH);
  val = analogRead(sensorPin);
  digitalWrite(sensorPower, LOW);
  return val;
}
