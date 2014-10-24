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

void setup() {
     Serial.begin(9600); // requried for the http lib
     delay(1000); // per the doc, delay before we do anything with the sensor
     dht.begin();
}

void loop() {
// Wait a few seconds between measurements.
   delay(DELAY);

// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a 
// very slow sensor)
   float h = dht.getHumidity();
// Read temperature as Celsius
   float t = dht.getTempCelcius();
// Read temperature as Farenheit
   float f = dht.getTempFarenheit();
   
   int tm = Time.now();
   
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
    request.path += "time=" + String(tm);
    
    http.get(request, response, headers);

    if (debug) {
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
       Serial.println("*C");
       Serial.println(tm);
    }
}
