#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SimpleDHT.h>
#define PIN_RELAY_1  9 //Define arduino pin for relay 1 (Relay one is for a humidifier)
#define PIN_RELAY_2  10 //Define ardunio pin for relay 2 (Relay two is for a sprinkler system)
#define PIN_RELAY_3  11 //Define ardunio pin for relay 3 (Relay three is for a fan)
#define PIN_RELAY_4  12 //Define ardunio pin for relay 4 (Relay four is for a heater)
LiquidCrystal_I2C lcd(0x27, 16, 2); //Creating LCD object with i2c address, number of columns and rows.
// Wiring: Connect SDA to a4, Connect SDL to a5

unsigned long pTime; //Time since program started
unsigned long tTime = 0; //LCD display message toggle time
bool Toggle = false; //

const int pinDHT11 = 5; // Temperature and Humidity Sensor
const int pinSEN0114 = 17; // Moisture Sensor
const int pinButton = 2; 
const int pinLed = LED_BUILTIN;

// Change based on preferences 
const int minimumRTemperature = 18; // Minimum Recommended Temperature
const int maximumRTemperature = 24; // Maximum Recommended Temperature
const int minimumRHumidity = 75; // Minimum Recommended Humidity
const int maximumRHumidity = 85; // Maximum Recommnded Humidity

SimpleDHT11 dht11(pinDHT11);
volatile int buttonState = 0; //Variable for button status


void changeThing() {      
  buttonState = digitalRead(pinButton);
  digitalWrite(pinLed, !buttonState);

}


void setup() {
  Serial.begin(9600);
  pinMode(pinButton, INPUT_PULLUP);
  pinMode(pinLed, OUTPUT);
  lcd.init(); // Initialize LCD
  lcd.backlight(); // Turns on backlight
  lcd.setCursor(1, 0); // Sets where the text should be displayed, The first parameter shows column and second parameter shows row
  attachInterrupt(0, changeThing, CHANGE); // Attach interrupt to ISR vector
  pinMode(PIN_RELAY_1, OUTPUT); //Set relay pins as outputs
  pinMode(PIN_RELAY_2, OUTPUT);
  pinMode(PIN_RELAY_3, OUTPUT);
  pinMode(PIN_RELAY_4, OUTPUT);


}


void loop() {
  // put your main code here, to run repeatedly:
  pTime = millis();


  displayData(); 
  Serial.println(pTime); // prints time since program started
 
  delay(1500);
  Serial.println();
}

void displayData(){
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


  }
  if (300 < soilMositure && soilMositure < 700) {  
    Serial.print(" Humid");
    lcd.setCursor(9, 1);
    lcd.print("Humid");
    digitalWrite(PIN_RELAY_2, LOW);


  }
   if (700 < soilMositure) {
    Serial.print(" Wet");
    lcd.setCursor(9, 1);
    lcd.print("Wet");

  }
  if ((pTime - tTime) > 5000) //Check if 5 seconds has passed and set toggle time variable equal to how long the program has been running
  {
    tTime = pTime;  
    Serial.print(tTime);
    Toggle = true;

  }



  if (minimumRTemperature > temperature) { //Check if temperature is below minimumRTemperature if so print to LCD and serial console
    Serial.print(" Below minimum set temperature");
    digitalWrite(PIN_RELAY_4, HIGH);

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
  }
   if (humidity < maximumRHumidity){
    digitalWrite(PIN_RELAY_3, LOW);
  }
 





}