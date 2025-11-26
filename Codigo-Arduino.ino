/*
Fixing the value of Vf = Vf0 at ambient temperature Ta, the temperature value Tx is calculated  with the
following formula:

Tx = Ta - [Vf(Tx)- Vf0]*K

Where:

K = -2,2 °C/mV. 
Vf(tx) = dTx - Vf0 is obtained by acquiring as many samples of Vf.
*/

// Directives
// OLED I2C display libraries and definitions:
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <gfxfont.h>
#include <Wire.h>

#define OLED_RESET 4
#define OLED_ADDR 0x3C

Adafruit_SSD1306 oled (128, 64, &Wire, OLED_RESET);

// Constant values:
const float Vf0 = 730; // Forward voltage reference at 690mV at 25°C with a test current 4mA
const float K = -2; // Temperature coefficient (-2 mV/°C for Si diodes)
const float Ta = 25; // Diode junction temperature reference

const int PSensor = A0; // Used to the sensor diode input (PA0 for STM32F103C8 board)
const float adcSample = 5000; // Voltage in milivolts for AD Converter
const float error = 16;

const float weight = 0.1;  // Smoothing factor, in range [0,1]
  // Higher the value, less smoothing and higher the latest reading impact

// Variable values:
double Tx; // Room temperature calculated 
double Vf; // Forward voltage measured at room temperature with a test current at 4mA
double dTx, dTxAvg; // For junction average temperature calculations

float smoothValue;
float prevSmoothValue;
int lightValue;

void setup() {
  
  // OLED I2C display setup:
  Wire.begin();
  oled.begin (SSD1306_SWITCHCAPVCC, OLED_ADDR);
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setCursor(17, 9);
  oled.setTextSize(2);
  oled.print ("AUDIOLEO");
  oled.setCursor(6, 28);
  oled.setTextSize(1);
  oled.print ("The HiFi Experience");
  oled.display();

  // Measurements and other initial settings:
  prevSmoothValue = analogRead(PSensor) * (adcSample/1023) + error; // divide by 4095 if case of a 12-bit ADC
  delay (2000);

}

void loop() {

  // Measurement and filtering:
  lightValue = analogRead(PSensor) * (adcSample/1023) + error; // divide by 4095 if case of a 12-bit ADC
  smoothValue = filter(lightValue, weight, prevSmoothValue);
  prevSmoothValue = smoothValue;

  // Temperature calculations:
  Vf = smoothValue; 
  dTx = (Vf - Vf0)/K;
  Tx = dTx - Ta;

  // Data display outputs:
  oled.clearDisplay();
  oled.setCursor (0,0);
  oled.setTextSize (2);
  oled.print ("TEMP:");
  oled.setTextSize (2);
  oled.setCursor (0,20);
  oled.print (Tx); oled.print (" C");
  oled.setTextSize (1);
  oled.setCursor (0,42);
  oled.print ("Vf: "); oled.print (Vf); oled.print (" mV");
  oled.display ();
  
  delay (1000);

}

// Exponential moving average filter:
float filter (float sensorValue, float w, float prevValue) {

  float result = w * sensorValue + (1.0 - w) * prevValue;
  return result;

}
