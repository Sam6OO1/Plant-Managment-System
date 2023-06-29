#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SimpleDHT.h>
#include "RTClib.h"

// Define the maximum number of strings and the maximum length of each string for char array
#define MAX_STRINGS 7
#define MAX_STRING_LENGTH 17
#define PIN_RELAY_1  9 //Define arduino pin for relay 1 (Relay one is for a humidifier)
#define PIN_RELAY_2  10 //Define ardunio pin for relay 2 (Relay two is for a sprinkler system)
#define PIN_RELAY_3  11 //Define ardunio pin for relay 3 (Relay three is for a fan)
#define PIN_RELAY_4  12 //Define ardunio pin for relay 4 (Relay four is for a heater)
LiquidCrystal_I2C lcd(0x27, 16, 2); //Creating LCD object with i2c address, number of columns and rows.
RTC_DS3231 rtc; //Creating RTC object
// Wiring: Connect SDA to a4, Connect SDL to a5

unsigned long pTime; //Time since program started
unsigned long tTime = 0; //LCD display message toggle time
bool showLogs = false;
bool Toggle = false; //Toggle for alert display every five seconds.
bool currentAlertMD = false; //Variable for if dry soil moisture alert is currently taking place
bool currentAlertMW = false; //Variable for if wet soil moisture alert is currently taking place
bool currentAlertHL = false; //Variable for if low humidity alert is currently taking place
bool currentAlertHH = false; //Variable for if high humidity alert is currently taking place
bool currentAlertTL = false; //Variable for if low temperature alert is currently taking place
bool currentAlertTH = false; //Variable for if high temperature alert is currently taking place
bool timeRegistered = false;

int counter = 1; 
int valueCount1 = 0;
int valueCount2 = 0;
int currentStringIndex = 0; 
int currentTimeIndex = 0;  


const int pinDHT11 = 5; // Temperature and Humidity Sensor
const int pinSEN0114 = 17; // Moisture Sensor
const int pinButtonR = 3; // Right Menu Button
const int pinButtonL = 4; // Left Menu Button
const int pinButtonE = 2; // Enable Menu
const int MAX_VALUES = 10; // Maximum number of time values to store

// Range of temperature and humidity that is acceptable and doesn't require any relay activation change to prefrences 
const int minimumRTemperature = 18; // Minimum Recommended Temperature
const int maximumRTemperature = 24; // Maximum Recommended Temperature
const int minimumRHumidity = 75; // Minimum Recommended Humidity
const int maximumRHumidity = 85; // Maximum Recommnded Humidity

char strings[MAX_STRINGS][MAX_STRING_LENGTH]; //array to store alert strings
char timeStrings[MAX_STRINGS][MAX_STRING_LENGTH]; //array to store alert times

SimpleDHT11 dht11(pinDHT11);
DateTime timeValues1[MAX_VALUES];
DateTime timeValues2[MAX_VALUES];


void alertLog() { //Show logs to user
    int btnState = digitalRead(pinButtonE);
    showLogs = !showLogs;

	}
void combineTimeValuesToString() { //Combine stored time values and convert to string
  if (currentTimeIndex < MAX_STRINGS) {
    char tempTimeString[MAX_STRING_LENGTH];

    if (valueCount1 > 0 && valueCount2 > 0)  { //Check if there are current time values stored for both start and end times
      DateTime startTime = timeValues1[0];
      DateTime endTime = timeValues2[valueCount2 - 1];

      sprintf(tempTimeString, "%02d %02d:%02d-%02d:%02d", //Convert time value in string
              startTime.day(), startTime.hour(), startTime.minute(),
               endTime.hour(), endTime.minute());

      strncpy(timeStrings[currentTimeIndex], tempTimeString, MAX_STRING_LENGTH);

      currentTimeIndex++;  // Increment the currentIndex for the next combined string
      valueCount1 = 0; //Deleted previous stored time values after combining
      valueCount2 = 0;
    
    }
  else {
    currentTimeIndex = 0;
  }
  }
}
void addString(const char* newString) {// Add strings to character array
  if (currentStringIndex < MAX_STRINGS) {
    strncpy(strings[currentStringIndex], newString, MAX_STRING_LENGTH);
    currentStringIndex++;
    Serial.println("String added successfully");
  } else {
    currentStringIndex = 1; //Reset position to array to overwrite old alerts with new ones

  }
}
void setup() {


#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. 
#endif

  if (! rtc.begin()) { //Check if RTC is working and detected.
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC suffered power loss setting time");
    // Set time after power loss
    // Sets time and date to system time at time of compiling 
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time.
  }

  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(pinButtonE),alertLog,FALLING); //Attach Interrupt to push button
  lcd.init(); // Initialize LCD
  lcd.backlight(); // Turns on backlight
  lcd.setCursor(1, 0); // Sets where the text should be displayed, The first parameter shows column and second parameter shows row
  pinMode(PIN_RELAY_1, OUTPUT); //Set relay pins as outputs
  pinMode(PIN_RELAY_2, OUTPUT);
  pinMode(PIN_RELAY_3, OUTPUT);
  pinMode(PIN_RELAY_4, OUTPUT);
  pinMode(pinButtonE, INPUT_PULLUP); //Sets button pins as input pullups
  pinMode(pinButtonL, INPUT_PULLUP);
  pinMode(pinButtonR, INPUT_PULLUP);
  addString("Test String"); //Add test string to logs
 



}
void storeTimeValue(DateTime time, int arrayIndex) { //Store current time value into an array, two arrays to allow for begin and end time values
  if (arrayIndex == 1) {
    if (valueCount1 > 0) {
      // Shift the existing values in array to make space for the new value
      for (int i = 0; i < valueCount1 - 1; i++) {
        timeValues1[i] = timeValues1[i + 1];
      }
      valueCount1--;
    }

    if (valueCount1 < MAX_VALUES) {
      timeValues1[valueCount1] = time;
      valueCount1++;
    }
  }
  else if (arrayIndex == 2) {
    if (valueCount2 > 0) {
      // Shift the existing values in array to make space for the new value
      for (int i = 0; i < valueCount2 - 1; i++) {
        timeValues2[i] = timeValues2[i + 1];
      }
      valueCount2--;
    }

    if (valueCount2 < MAX_VALUES) {
      timeValues2[valueCount2] = time;
      valueCount2++;
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  pTime = millis();

  if (showLogs == true) { 
    if (digitalRead(pinButtonR) == LOW){  
      counter --;
       if (counter <= 0){ //Make sure counter is never equal to 0 the test string
        counter = 1;
       if (counter == 7){ //If counter is equal to 7 reset to 1 as max array size is 6
         counter = 1;
       } 
      }
  
    }
    if (digitalRead(pinButtonL) == LOW){
      counter ++;
      if (counter <= 0){ //Make sure counter is never equal to 0 the test string
        counter = 1;
      }
      if (counter == 7){ //If counter is equal to 7 reset to 1 as max array size is 6
        counter = 1;
      }
         
    }
    //Display logs to LCD display
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(strings[counter]);
    Serial.print(strings[counter]);
    lcd.setCursor(0,1);
    lcd.print(timeStrings[counter-1]);
    lcd.setCursor(15, 0);
    lcd.print(counter);
    delay(100);
    
  } 

  if (showLogs == false){
    lcd.clear();
    displayData(); 
  }
  
  Serial.println(); 
}

void displayData(){
  DateTime now = rtc.now();

   // read without sample
  int soilMositure = analogRead(pinSEN0114);
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
    // check if dht11 is working
    return;
  }
  //print temp, moisture and humidity to serial and lcd
  Serial.print("Temperature: ");
  Serial.print((int)temperature); Serial.print(" C "); 
  Serial.print("Humidity: ");
  Serial.print((int)humidity); Serial.print(" % ");
  Serial.print("Moisture: ");
  Serial.print(soilMositure);
  lcd.clear();
  lcd.print("T: ");
  lcd.print((int)temperature); lcd.print(" C "); 
  lcd.setCursor(0, 1);
  lcd.print("H: ");
  lcd.print((int)humidity); lcd.print(" %");
  lcd.setCursor(9,0);
  lcd.print("M: ");
  lcd.print(soilMositure);

  if (soilMositure < 300) { //Print soil moisture alerts to LCD and serial display
    Serial.print(" Dry");
    lcd.setCursor(9, 1);
    lcd.print("Dry");
    digitalWrite(PIN_RELAY_2, HIGH); 
    currentAlertMD = true;
    if (timeRegistered == false){ //Check If time hasn't already been registered (If removed will record the exact time value of state change) 
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 1);
      timeRegistered = true;
    }
  }
   if (700 < soilMositure) {
    Serial.print(" Wet");
    lcd.setCursor(9, 1);
    lcd.print("Wet");
    currentAlertMW = true;
    if (timeRegistered == false){
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 1);
      timeRegistered = true;
    }

  }
  if (300 < soilMositure && soilMositure < 700) {  
    Serial.print(" Moist");
    lcd.setCursor(9, 1);
    lcd.print("Moist");
    digitalWrite(PIN_RELAY_2, LOW);
    if (currentAlertMD == true){
      currentAlertMD = false;
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 2);
      addString("Soil Dry"); 
      combineTimeValuesToString();
      timeRegistered = false;
    }
    if (currentAlertMW == true){
      currentAlertMW = false;
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 2);
      addString("Soil Wet"); 
      combineTimeValuesToString();
      timeRegistered = false;
    }

  }
  if ((pTime - tTime) > 5000) //Check if 5 seconds has passed and set toggle time variable equal to how long the program has been running
  {
    tTime = pTime;  
    Toggle = true;

  }

  if (minimumRTemperature > temperature) { //Check if temperature is below minimumRTemperature if so print to LCD and serial console
    Serial.print(" Below minimum set temperature");
    digitalWrite(PIN_RELAY_4, HIGH);
    currentAlertTL = true;
    if (timeRegistered == false){
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 1);
      timeRegistered = true;
    }
  
    if (Toggle == true){ //If five seconds has passed display alert to lcd display
      lcd.clear();
      lcd.print("Below minimum");
      lcd.setCursor(0,1);
      lcd.print("set temperature");
      Toggle = false;
    }
  }
  if (maximumRTemperature < temperature) { //Check if temperature is over maximumRTemperature if so print to LCD and serial console
    Serial.print(" Over maximum set temperature");
    digitalWrite(PIN_RELAY_3, HIGH);
    currentAlertTH = true;
    if (timeRegistered == false){
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 1);
      timeRegistered = true;
    }


    if (Toggle == true){ //If five seconds has passed display alert to lcd display
      lcd.clear();
      lcd.print("Over maximum");
      lcd.setCursor(0,1);
      lcd.print("set temperature");
      Toggle = false;
    }
  }
  if (minimumRHumidity > humidity) { //Check if humidity is below minimumRHumidity if so print to LCD and serial console
    Serial.print(" Below minimum set humidity");
    digitalWrite(PIN_RELAY_1, HIGH);
    currentAlertHL = true;
    if (timeRegistered == false){
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 1);
      timeRegistered = true;
    }

    if (Toggle == true){ //If five seconds has passed display alert to lcd display
      lcd.clear();
      lcd.print("Below minimum");
      lcd.setCursor(0,1);
      lcd.print("set humidity");
      Toggle = false;
    }
  }
  if (maximumRHumidity < humidity) { //Check if humidity is over maximumRHumidity if so print to LCD and serial console
    Serial.print(" Over maximum set humidity");
    digitalWrite(PIN_RELAY_3, HIGH);
    currentAlertHH = true;
    if (timeRegistered == false){
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 1);
      timeRegistered = true;
    }

    if (Toggle == true){ //If five seconds has passed display alert to lcd display
      lcd.clear();
      lcd.print("Over maximum");
      lcd.setCursor(0,1);
      lcd.print("set humidity");
      Toggle = false;
    }
  }
  if (minimumRHumidity < humidity){
    digitalWrite(PIN_RELAY_1, LOW);
    if (currentAlertHL == true){
      currentAlertHL = false;
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 2);
      addString("Low Humidity"); 
      combineTimeValuesToString();
      timeRegistered = false;
    }
    
  }
  if (humidity < maximumRHumidity){
    digitalWrite(PIN_RELAY_3, LOW);
    if (currentAlertHH == true){
      currentAlertHH = false;
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 2);
      addString("High Humidity"); 
      combineTimeValuesToString();
      timeRegistered = false;
    }
   
  }
  if (temperature < maximumRTemperature){
    digitalWrite(PIN_RELAY_3, LOW);
    if (currentAlertTL == true){
      currentAlertTL = false;
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 2);
      addString("Low Temp"); 
      combineTimeValuesToString();
      timeRegistered = false;
    }
  }
  if (minimumRTemperature < temperature){
    digitalWrite(PIN_RELAY_4, LOW);
    if (currentAlertTH == true){
      currentAlertTH = false;
      now = rtc.now(); // Get the updated time 
      storeTimeValue(now, 2);
      addString("High Temp"); 
      combineTimeValuesToString();
      timeRegistered = false;
    }
  }
  delay(1500);
 
}