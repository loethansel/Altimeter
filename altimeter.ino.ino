// Eriks Telemetrieprogramm
/*
 * Requirements
 * ------------
 * - FrskySP library - https://github.com/jcheger/frsky-arduino
 * - Recent version of Arduino's IDE (ex. 1.6.1), else SoftwareSerial will fail at 57600 bds.
 * 
 * origin: https://github.com/jcheger/frsky-arduino
 */
#include <FrskySP.h>
#include "seeed_bme680.h"
#include <SoftwareSerial.h>

// make #define DEBUG for serial outputs
#undef DEBUG
// defines for i2c pins (bme680)
#define BME_SCK  13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS   10

// define for I2C ADDRESS BME Sensor
#define IIC_ADDR  uint8_t(0x76)

// BME CLASS Declaration
Seeed_BME680 bme680(IIC_ADDR); /* IIC PROTOCOL */
// FRSKY CLASS Declaration
FrskySP FrskySP(3);

// global variables
float hoehe       = 0.0;
float temperatur  = 0.0;
float luftfeuchte = 0.0;
unsigned char cnt = 100;
unsigned char i   = 0;
unsigned char taranisread;
static unsigned long sensor_millis; 

// SETUP procedure is called only 1 time at the beginning
void setup() 
{
  FrskySP.ledSet(13);
  Serial.begin(9600);
  Serial.println("Serial start!!!");
  delay(100);
  while (!bme680.init()) { Serial.println("bme680 init failed ! can't find device!"); }
  // catch milliseconds on program start
  sensor_millis = millis();
}

// loop => loops forever
void loop() 
{
  //--- bme680 reading each 100ms
  if ((millis() - sensor_millis) >= 100) {
     if (bme680.read_sensor_data());
     hoehe       = ((1013 - (bme680.sensor_result_value.pressure/100)) * 8.0);
     temperatur  = bme680.sensor_result_value.temperature;
     luftfeuchte = bme680.sensor_result_value.humidity;
     #ifdef DEBUG
     Serial.println("reading bme680!");
     #endif
     // catch millis for the next compare
     sensor_millis = millis ();
  }
  //--- bme680 reading end   
  //--- TARANIS SBUS telemetrie handling
  while(FrskySP.available()) {
     // get header 0x7E => start of telemetrie
     if(FrskySP.read() == 0x7E) {
        // read ID-Byte following after getting 0x7E
        taranisread = FrskySP.read();
        #ifdef DEBUG
        Serial.print(taranisread,HEX);
        #endif
        // look wich id is asked for
        switch (taranisread) {
           // Physical ID 1 - Vario2 (altimeter high precision)
           case 0x00: 
              FrskySP.sendData(FRSKY_SP_ALT, hoehe * 100);
              #ifdef DEBUG
              Serial.print("Altitude: "); Serial.print(hoehe); Serial.println(" m");
              #endif
           break;
           // Physical ID 27 - Temperature1 / temperature2   
           case 0xBA:  // Physical ID 27 - 
              if(i % 2 == 0) FrskySP.sendData (FRSKY_SP_T1, temperatur);
              if(i % 2 == 1) FrskySP.sendData (FRSKY_SP_T2, luftfeuchte);
              i++;
              #ifdef DEBUG
              Serial.print("Temperatur:  "); Serial.print(temperatur);  Serial.println(" gradC");
              Serial.print("Luftfeuchte: "); Serial.print(luftfeuchte); Serial.println(" %");
              #endif
           break;      
 
        }
     }
  }
  //--- TARANIS SBUS ENDS
}
