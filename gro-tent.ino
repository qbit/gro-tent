// This #include statement was automatically added by the Spark IDE.
#include "Adafruit_TSL2561_U/Adafruit_TSL2561_U.h"

// This #include statement was automatically added by the Spark IDE.
#include "HttpClient/HttpClient.h"

// This #include statement was automatically added by the Spark IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"

#define DHTPIN 2            // what pin we're connected to
#define DELAY 30000         // time to wait between measurements
#define DHTTYPE DHT22	       // DHT 22 (AM2302)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

bool debug = false;

HttpClient http;

http_header_t headers[] = {
    {"Accept", "*/*"},
    {NULL, NULL}
};

http_request_t request;
http_response_t response;

DHT dht(DHTPIN, DHTTYPE);

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void setup() {
     Serial.begin(9600); // requried for the http lib
     delay(1000); // per the doc, delay before we do anything with the DHT sensor 
     
     if (debug) {
         displaySensorDetails();
	 }
	 
    if (!tsl.begin()) {
        Serial.println("Can't talk to lux sensor");
        while(1);
    } 

    tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
    
    dht.begin();
}

void loop() {
// Wait a few seconds between measurements.
   if (!debug) {
       delay(DELAY);
   } else {
       delay(5000);
   }
// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a 
// very slow sensor)
   float h = dht.getHumidity();
// Read temperature as Celsius
   float t = dht.getTempCelcius();
// Read temperature as Farenheit
   float f = dht.getTempFarenheit();
   
   int tm = Time.now();
   
   float l = 0;
   
   sensors_event_t event;
   
   tsl.getEvent(&event);
   
   if (event.light) {
       l = event.light;
   }
       
// Check if any reads failed and exit early (to try again).
   if (isnan(h) || isnan(t) || isnan(f)) {
      if (debug) {
         Serial.println("Failed to read from DHT sensor!");
      }
      return;
   }

// Compute heat index
// Must send in temp in Fahrenheit!
   float hi = dht.getHeatIndex();
   float dp = dht.getDewPoint();
   float k = dht.getTempKelvin();

// http://data.sparkfun.com/input/[publicKey]?private_key=[privateKey]&dew_p=[value]&heat_idx_c=[value]&humidity=[value]&temp_c=[value]&temp_f=[value]&temp_k=[value]&time=[value]

    request.hostname = "data.sparkfun.com";
    request.port = 80;
    request.path = "/input/xRl4Mb5XbMcOwQA5dr2R";
    
    request.path += "?";
    request.path += "private_key=omgnoyoudidnt&";
    request.path += "dew_p=" + String(dp) + "&";
    request.path += "heat_idx_c=" + String(hi) + "&";
    request.path += "humidity=" + String(h) +"&";
    request.path += "temp_f=" + String(f) + "&";
    request.path += "temp_c=" + String(t) + "&";
    request.path += "temp_k=" + String(k) + "&";
    request.path += "lux=" + String(l) + "&";
    request.path += "time=" + String(tm);
    
    if (!debug) {
        http.get(request, response, headers);
    } else {
        Serial.println(request.path);
    	Serial.print("Humid: "); 
    	Serial.print(h);
    	Serial.print("% - ");
    	Serial.print("Temp: "); 
    	Serial.print(t);
    	Serial.print("*C ");
    	Serial.print(f);
    	Serial.print("*F ");
    	Serial.print(k);
    	Serial.print("*K - ");
    	Serial.print("DewP: ");
    	Serial.print(dp);
    	Serial.print("*C - ");
    	Serial.print("HeatI: ");
    	Serial.print(hi);
    	Serial.print("*C");
    	Serial.print(" - Lux: ");
    	Serial.print(l);
        Serial.print(" - time: ");
    	Serial.print(tm);
    	Serial.println("");
    }
}