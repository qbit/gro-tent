// This #include statement was automatically added by the Spark IDE.
#include "WebServer/WebServer.h"

// This #include statement was automatically added by the Spark IDE.
#include "Adafruit_TSL2561_U/Adafruit_TSL2561_U.h"

// This #include statement was automatically added by the Spark IDE.
#include "HttpClient/HttpClient.h"

// This #include statement was automatically added by the Spark IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"

#define DHTPIN D2            // what pin we're connected to
#define DELAY 60000         // time to wait between measurements
#define DHTTYPE DHT22		// DHT 22 (AM2302)

#define RELAY1 D4 // humidifier
#define RELAY2 D5 // fan
#define RELAY3 D6 // light
#define RELAY4 D3 // empty

#define PREFIX "/monitor"
WebServer webserver(PREFIX, 80);

int humidity = 80;
int oldMinute = Time.minute();

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

void monitorValues(WebServer &server, WebServer::ConnectionType type, char *, bool) {
    if (type == WebServer::POST) {
		bool repeat;
		char name[16], value[16];
		
		do {
			repeat = server.readPOSTparam(name, 16, value, 16);
			if (strcmp(name, "humidity") == 0) {
				humidity = strtoul(value, NULL, 10);
			}
			if (strcmp(name, "lights") == 0) {
			    // Something to toggle the lights.
			    // Need to keep state so we can toggle / update the webpage with correct state.
			}
		} while (repeat);
					   
		server.httpSeeOther(PREFIX);
		return;
	}

	server.httpSuccess();

	if (type == WebServer::GET) {
		P(message) = "<!DOCTYPE html><html><head>"
			"<title>Monitor</title>"
			"<link href='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.16/themes/base/jquery-ui.css' rel=stylesheet />"
			//"<meta http-equiv='Content-Script-Type' content='text/javascript'>"
			"<script src='http://ajax.googleapis.com/ajax/libs/jquery/1.6.4/jquery.min.js'></script>"
			"<script src='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.16/jquery-ui.min.js'></script>"
			"<style> #slider { margin: 10px; } </style>"
			"<script>"
			"var lval = false;"
			"function changeH(event, ui) { $('#indicator').text(ui.value); $.post('/monitor', { humidity: ui.value } ); } "
			"function changeL(event, ui) { lval = !lval; $.post('/monitor', {lights: lval}); }"
			"$(document).ready(function(){ $('#slider').slider({min: 0, max:100, change:changeH}); $('#lights').click(changeL); });";
		P(mid) =
			"</script>"
			"</head>"
			"<body style='font-size:62.5%;'>"
			"<h1>Humidity</h1>"
			"<div id=slider></div>"
			"<p id=indicator>"
			"<input type='checkbox' id='lights'><label for='lights'>Lights</label>";
			
		P(foot) = "</p>"
			"</body>"
			"</html>";
			
	   server.printP(message);
	   server.print("$(document).ready(function(){ $('#slider').slider('value', " + String(humidity) + "); });");
	   server.printP(mid);
	   server.print("<p>Desired humidity:" + String(humidity) + "</p>");
	   server.print("<p>Current humidity:" + String(dht.getHumidity()) + "</p>");
	   server.printP(foot);
	}
}

void setup() {
	Serial.begin(9600); // requried for the http lib
	delay(1000); // per the doc, delay before we do anything with the DHT sensor 
	
	Time.zone(-7);
	
	if (debug) {
	    displaySensorDetails();
	}
	
    if (!tsl.begin()) {
        Serial.println("Can't talk to lux sensor");
        while(1);
    } 

    tsl.enableAutoRange(true);

    /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
    
    webserver.setDefaultCommand(&monitorValues);
    webserver.begin();
	
	dht.begin();
	
	//delay(5000);
	
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);
    // Initialize all relays to an OFF state
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, LOW);
    digitalWrite(RELAY4, LOW);

}

void sendData(String reqstr) {
    // http://data.sparkfun.com/input/[publicKey]?private_key=[privateKey]&dew_p=[value]&heat_idx_c=[value]&humidity=[value]&temp_c=[value]&temp_f=[value]&temp_k=[value]&time=[value]

    request.hostname = "block.bold.daemon";
    request.port = 8080;
    request.path = "/data";
    
    request.path += reqstr;
    /*
    request.path += "?";
    request.path += "dew_p=" + String(dp) + "&";
    request.path += "heat_idx_c=" + String(hi) + "&";
    request.path += "humidity=" + String(h) +"&";
    request.path += "temp_c=" + String(t) + "&";
    request.path += "lux=" + String(l) + "&";
    request.path += "itime=" + String(tm);
    */
    
    if (!debug) {
        http.get(request, response, headers);
    } else {
        Serial.println(reqstr);
        /*
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
    	*/
    }
}

void updateState() {
    
}

void recordData() {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a 
    // very slow sensor)
	float h = dht.getHumidity();
    // Read temperature as Celsius
	float t = dht.getTempCelcius();
    // Read temperature as Farenheit
	float f = dht.getTempFarenheit();
	
	int tm = Time.now();
	int hour = Time.hour(tm);
	
	float l = 0;
	
	if (h > humidity) {
	    digitalWrite(RELAY1, LOW); // humidifier
	    digitalWrite(RELAY2, LOW);
	} else {
	    digitalWrite(RELAY1, HIGH);
	    digitalWrite(RELAY2, HIGH);
	}
	
	if (hour >= 20 || hour <= 8) {
	    digitalWrite(RELAY3, LOW);
	} else {
	    digitalWrite(RELAY3, HIGH);
	}
	
	sensors_event_t event;
	
	tsl.getEvent(&event);
	
	if (event.light) {
	    l = event.light;
	} else {
	    Serial.println("Failed to read from Light sensor!");
	}
	
// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t) || isnan(f)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

// Compute heat index
// Must send in temp in Fahrenheit!
	float hi = dht.getHeatIndex();
	float dp = dht.getDewPoint();
	float k = dht.getTempKelvin();
	
	if (isnan(dp)) {
	    dp = 0;
	}
	
	String reqstr = "";
    
    reqstr += "?";
    reqstr += "dew_p=" + String(dp) + "&";
    reqstr += "heat_idx_c=" + String(hi) + "&";
    reqstr += "humidity=" + String(h) +"&";
    reqstr += "temp_c=" + String(t) + "&";
    reqstr += "lux=" + String(l) + "&";
    reqstr += "itime=" + String(tm);
    
    sendData(reqstr);
}

void loop() {
    
    char buff[64];
    int len = 64;

    /* process incoming connections one at a time forever */
    webserver.processConnection(buff, &len);
    
    int newMinute = Time.minute();
    
    if (oldMinute < newMinute) {
        recordData();
        oldMinute = newMinute;
    }
    
    //delay(10);
}