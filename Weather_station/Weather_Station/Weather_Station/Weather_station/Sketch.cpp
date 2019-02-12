﻿#include <Arduino.h>

// control lcd with I2C IO bus expansion board with PCF8574*I2C IO expander
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTPIN 6        // digital pin we're connected to
#define SWITCH 2
#define DHTTYPE DHT11   // DHT 11 sensor
// find i2c bus address (replace the first parameter in the following line)
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void changeTempScale();

// dht sensor object instance
DHT dht(DHTPIN, DHTTYPE);

// delay time settings
int stepDelay   = 100;
int shortDelay  = 200;
int mediumDelay = 500;
int longDelay   = 3000;
volatile int nCounter = 3;

// variables used for measurements
float humidity = 0;
float tempFahr = 0;
float tempCels = 0;  
float tempHeatC= 0;
float tempHeatF= 0;
float tempCMIN = 0;
float tempCMAX = 0;
float tempFMIN= 0;
float tempFMAX= 0;

bool  isFirstExec   = true;
char celciusOrFahren = 'c';
bool reset = false;

String txtTemp      = "";   // Temperature text
String txtUmidita   = "";   // Humidity text
String txtHeatTemp  = "";   // Heat Temperature text

//
// ******************************************************* [LOGIC] **************************************************
//

// debug
void debugDataCelsius(float h, float t, float hit) {
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" \t");

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");

  Serial.print("Heat Temp.: ");
  Serial.print(hit, 1);
  Serial.print(" *C ");

  Serial.println("\t");
}

String getTextTemparatureCentigrade() {
  String txt = "TEMP: ";
  txt.concat(String(tempCels, 0));
  txt.concat((char)223);          // ASCII degree ° char http://www.rapidtables.com/code/text/ascii-table.htm
  txt.concat("C");
  return txt;
}

String getTextTemperatureFahrenheit(){
  String txt = "TEMP: ";
  txt.concat(String(tempFahr, 0));
  txt.concat((char)223);          
  txt.concat("F");
  return txt;
}

String getTextHumidity() {
  return "HUMIDITY: " + String(humidity, 0) + "%";
}

String getTextHeatTempCel() {
  String txt  = "H.TEMP: ";
  txt.concat(String(tempHeatC, 1));
  txt.concat((char)223);          
  txt.concat("C");
  return txt;
}

String getTextHeatTempFah() {
	String txt  = "H.TEMP: ";
	txt.concat(String(tempHeatF, 1));
	txt.concat((char)223);
	txt.concat("F");
	return txt;
}

String getTextMinMaxTempCelcius() {
  String txt = "MAX:";
  txt.concat(String(tempCMAX, 0));
  txt.concat((char)223);          
  txt.concat(" ");
  txt.concat("MIN:");
  txt.concat(String(tempCMIN, 0));
  txt.concat((char)223);
  return txt;
}

String getTextMinMaxTempFah() {
	String txt = "MAX:";
	txt.concat(String(tempFMAX, 0));
	txt.concat((char)223);
	txt.concat(" ");
	txt.concat("MIN:");
	txt.concat(String(tempFMIN, 0));
	txt.concat((char)223);
	return txt;
}

void readSensorData() {
  //Serial.println("DHT11 sensor data request...");

  humidity = dht.readHumidity();            // Read humidity data

  tempCels = dht.readTemperature();         // Read temperature as Celsius

  tempFahr = dht.readTemperature(true);     // Read temperature as Fahrenheit (shown on interrupt)

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(tempCels) || isnan(tempFahr)) {
    //Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  tempHeatF = dht.computeHeatIndex(tempFahr, humidity);

  // Compute heat index in Celsius (isFahreheit = false)
  tempHeatC = dht.computeHeatIndex(tempCels, humidity, false);

  if(isFirstExec){
    tempCMAX = tempCMIN = tempCels;
		tempFMAX = tempFMIN = tempFahr;
    isFirstExec = false;
  }
  // set max,min for celcius.
  if (tempCels > tempCMAX) {
    tempCMAX = tempCels;
  }
  
  if (tempCels < tempCMIN) {
    tempCMIN = tempCels;
  }
	
	//set max,min for fahrenheit
	if (tempFahr > tempFMAX) {
		tempFMAX = tempFahr;
	}
	  
	if (tempFahr < tempFMIN) {
		tempFMIN = tempFahr;
	}
}
// print temperature on the first row
void printLcdTemperatureRow() {
  lcd.setCursor(0, 0);
	if(celciusOrFahren == 'c'){      
		lcd.print(getTextTemparatureCentigrade());  
	}
	else{
		lcd.print(getTextTemperatureFahrenheit());
	}
}

void printGreeting(){
  lcd.setCursor(0,0);
  lcd.print("Arduino Mini");
  lcd.setCursor(0,1);
  lcd.print("Weather Station");
}

void printLcdDataRow() {
  
  lcd.setCursor(0, 1);                // moving to second row
  
  // it checks what to print
	if(celciusOrFahren == 'c'){
		if (nCounter == 0) {
			lcd.print(getTextHumidity());     // if loop #1 it prints humidity in second row
		} else if (nCounter == 1) {
			lcd.print(getTextHeatTempCel());     // if loop #2 it prints heat temp.
		} else if (nCounter == 2) {
			lcd.print(getTextMinMaxTempCelcius());   // if loop #3 it prints min/max temp.
		}
	}
	else{
		if (nCounter == 0) {
			lcd.print(getTextHumidity());     // if loop #1 it prints humidity in second row
			} else if (nCounter == 1) {
			lcd.print(getTextHeatTempFah());     // if loop #2 it prints heat temp.
			} else if (nCounter == 2) {
			lcd.print(getTextMinMaxTempFah());   // if loop #3 it prints min/max temp.
		}
	}
}

void scrollText(){
  // scroll text to the right
  for (int positionCounter = 0; positionCounter < 16; positionCounter++) {
    lcd.scrollDisplayRight();
    delay(stepDelay);
  }
}

/**** [SETUP] ****/


void setup() {
  
  Serial.begin(9600);                // debug
  
  lcd.begin(16, 2);                     // set up the LCD's number of columns and rows:

  dht.begin();

  // show Greeting on startup
  for(byte i = 0; i < 3; i++){
    printGreeting();
    delay(mediumDelay);
    lcd.clear();
    delay(mediumDelay);
  }
	pinMode(SWITCH, INPUT);
  //Serial.println("DHT11 Started..."); // debug
	attachInterrupt(digitalPinToInterrupt(SWITCH), changeTempScale, RISING);
}


/**** [LOOP] ****/

void loop() {

  lcd.clear();
  if (nCounter > 2) {
    nCounter = 0;       // reset counter
    readSensorData();   // every 3 loops it reads data from sensor
  }

// debugDataCelsius(humidity, tempCels, tempHeatC);

  printLcdTemperatureRow();
  printLcdDataRow();
  delay(longDelay);

  lcd.clear();

  delay(mediumDelay);

  printLcdTemperatureRow();
  printLcdDataRow();
  delay(longDelay);

  scrollText();

  nCounter++;

  delay(mediumDelay);
}

void changeTempScale(){
	celciusOrFahren = (celciusOrFahren == 'c') ? 'f' : 'c';
	Serial.println("Interrupt captured!");
}