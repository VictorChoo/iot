
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <TimeLib.h>
#include "ArduinoJson.h"

//----- Time Settings
int timezone = 9; // GMT+9
String now_time = ""; // YYYYMMDDHHMMSS

//----- Cloud Settings
#define GROUP_NAME "XXXXXXX" // where your thing
#define THING_NAME "XXXXXXX"  // Your thing name in dynamoDB

#define STAGE_NAME "XXXXXXX" // Your stage name in API Gateway
#define RESOURCE_NAME "XXXXXXX" // Your resource name in API Gateway
#define API_KEY "XXXXXXXXXXXXXXXXXXXXXXX" // Your Api Key using in API Gateway

char ssid[] = "XXXXXX"; // Wifi ssid
char password[] = "XXXXXXXXXX"; //Wifi password

const char* host = "XXXXXXXXXX.execute-api.{Region}.amazonaws.com";//api endpoint
const int httpPort = 443; //for https
String url = "/" STAGE_NAME "/" RESOURCE_NAME; // Resource path

WiFiClientSecure client;

/* defines pins numbers */
int led_pin = 2; //NodeMCU's LED

// Ultrasonic pin numbers
const int trigPin = D1;
const int echoPin = D2;

// PIR pin numbers
int pirPin = D7; // Input for HC-S501

/* defines variables */
// Common Variable
unsigned long lastMsg = 0;

// Ultrasonic variable
long duration, distance;

//PIR variable
int pirValue; // Place to store read PIR Value

/* initial settings */
void setup() {
    Serial.println("Setup Start");
    
    /* Led Setting */
    pinMode(led_pin, OUTPUT);
    
    // Blink led to check nodeMCU is alive
//    digitalWrite(led_pin, HIGH);
//    delay(500);
//    digitalWrite(led_pin, LOW);

    /* Ultrasonic I/O Setting */
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  
    /* PIR I/O Setting */
    //pinMode(ledPin, OUTPUT);
    pinMode(pirPin, INPUT);
    //digitalWrite(ledPin, LOW);
    Serial.begin(115200); // Starts the serial communication
    Serial.setDebugOutput(true);
    Serial.println();

    /* Wifi connect */
    for(uint8_t t = 3; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFi.begin(ssid, password);
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    Serial.print("Connecting to AP");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());

    // Get time data when initiating
    getTimeData();

    delay(1500);

    Serial.println("Setup End");
}

/* exectuing main */
void loop() {
    // Now time
    unsigned long time_now = millis();

    // publish data and debug mqtt connection every 15 seconds
    if (time_now - lastMsg > 15000) 
    {
        //Setting program running time from beginning on lastMsg
        lastMsg = time_now;
    
        Serial.println("Loop Start");

        /* ---------------- */
        /* Get time data    */
        /* ---------------- */
        getTimeData();
        
        /* ---------------- */
        /* Get data to send */
        /* ---------------- */
        getSensorData();

        /* ---------------- */
        /* Send data to aws */
        /* ---------------- */
        // Send message
        sendSensorDataToAWS_wifi(pirValue, distance);
        
        Serial.println("Loop End");
    }
}

/* ---------------- */
/* Define functions */
/* ---------------- */

// Recieve time data
void getTimeData(){
  
    configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("\nWaiting for time");
    while (!time(nullptr)){
        Serial.print(".");
        delay(1000);
    }
    Serial.println("");
//     hour(); // The hour now (0-23)
//     minute(); // The minute now (0-59)
//     second(); // The second now (0-59)
//     millis(); //The millisecond now (0-1000)
//     day(); // The day now (1-31)
//     weekday(); // Day of the week (1-7), Sunday is day 1
//     month(); // The month now (1-12)
//     year(); // The full four digit year: (2009, 2010 etc)

    
    time_t now = time(nullptr);
    
    Serial.println(ctime(&now));

    //Make data string YYYYMMDDHHMMSS
    String st_year = "";
    String st_month = "";
    String st_day = "";
    String st_hour = "";
    String st_minute = "";
    String st_second = "";

    st_year = String(year(now));
    
    if( month(now) < 10)  st_month = "0" + String(month(now));
    else  st_month = String(month(now));

    if( day(now) < 10)  st_day = "0" + String(day(now));
    else  st_day = String(day(now));

    if( hour(now) < 10)  st_hour = "0" + String(hour(now));
    else  st_hour = String(hour(now));

    if( minute(now) < 10)  st_minute = "0" + String(minute(now));
    else  st_minute = String(minute(now));

    if( second(now) < 10)  st_second = "0" + String(second(now));
    else  st_second = String(second(now));

    now_time = st_year + st_month + st_day + st_hour + st_minute + st_second;
    
    Serial.println(now_time);
}

// Getting Sensor Data to send
void getSensorData() {
    pirValue, duration, distance = 0;
    
    /* PIR Sensor */
    pirValue = digitalRead(pirPin);
    Serial.printf("[PIR value] : %d\n", pirValue);
    
    /* Ultrasonic Sensor */
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);

    // Calculating the distance
    distance= duration*0.034/2;
    //distance=(duration/2)/29.1;
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
}

//Send data to aws
void sendSensorDataToAWS_wifi(int pirValue, int distance)
{
    WiFiClientSecure client;

    String timestamp = now_time;

    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    // Make json data to send
    String jsonbody = String("{\r\n \"timestamp\":\"" + timestamp + "\",\r\n \"mac\":\"" + WiFi.macAddress() + "\",\r\n \"group\":\"" + GROUP_NAME + "\",\r\n \"thing_name\":\"" + THING_NAME + "\",\r\n \"pir\":" + pirValue + ",\r\n \"ultra\":" + distance + "\r\n}");

    // This will send the request to the server
    String requestBody = String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "x-api-key: " API_KEY "\r\n" +
               "Content-Type: application/json\r\n" +
               "Cache-Control: no-cache\r\n" +
               "Content-length: " + String(jsonbody.length()) + "\r\n\r\n" +
               jsonbody;
   
    client.print(requestBody);
    client.println("");
    client.println("");

    Serial.println(requestBody);
    Serial.println(client.available());
    
    while(client.available()){
      Serial.println("Available!!");
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    
}
