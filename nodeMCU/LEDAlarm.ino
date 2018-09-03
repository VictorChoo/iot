
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include "ArduinoJson.h"

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
String url = "/" STAGE_NAME "/" RESOURCE_NAME "?group=" GROUP_NAME "&type=nodemcu" ; // Resource path

String response_body = "";

WiFiClientSecure client;

/* defines pins numbers */
int LED_Red_Pin = D8;
int LED_Blue_Pin = D1;


/* defines variables */
// Common Variable
unsigned long lastMsg = 0;

/* initial settings */
void setup() {
    Serial.println("Setup Start");
    
    /* Led Setting */
    pinMode(LED_Red_Pin, OUTPUT); 
    pinMode(LED_Blue_Pin, OUTPUT); 
    Serial.begin(115200); // Starts the serial communication
   
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
    
    delay(1500);

    Serial.println("Setup End");
}

/* exectuing main */
void loop() {
    // Now time
    unsigned long time_now = millis();

    // publish data and debug mqtt connection every 15 seconds
    if (time_now - lastMsg > 5000) 
    {
        //Setting program running time from beginning on lastMsg
        lastMsg = time_now;
    
        Serial.println("Loop Start");

        /* ---------------- */
        /* Get data to aws */
        /* ---------------- */
        // get message
        getDataToAWS_wifi();

        /* ---------------- */
        /* Setting light    */
        /* ---------------- */
        // Setting LED color
        changeLED();
        
        Serial.println("Loop End");
    }
}

/* ---------------- */
/* Define functions */
/* ---------------- */

//Get data to aws
void getDataToAWS_wifi()
{
    WiFiClientSecure client;

    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    Serial.print("url : ");
    Serial.println(url);

    // This will send the request to the server
    String requestBody = String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "x-api-key: " API_KEY "\r\n" +
               "User-Agent: nodeMCU/2.0\r\n" +
               "Content-Type: application/json\r\n" +
               "Cache-Control: no-cache\r\n" +
//               "Content-length: " + String(jsonbody.length()) + "\r\n" +
                "\r\n"
//               jsonbody
               ;
   
    client.print(requestBody);
    client.println("");
    client.println("");

    Serial.println(requestBody);
//    Serial.println(client.available());

    while (!client.available()) {
      Serial.print(".");
      delay(300);
    }
    Serial.println("");

    String line = "";
    while(client.available()){
//      Serial.println("Available!!");
      line = client.readStringUntil('\n');      

      if (line.substring(1) == "{") 
      {
          break;
      }
    }
    response_body = line;
    Serial.print("response_body : ");
    Serial.println(response_body);
    
    
}

// Change LED color according to server response
void changeLED()
{
    Serial.println("change LED");

    int group_status = 0;
    String st_status = "";

    StaticJsonBuffer<400> jsonBuffer;    
    JsonObject& Obj = jsonBuffer.parseObject(response_body);

    st_status = Obj["ResponseStatus"]["status"].asString();    
    group_status = st_status.toInt();
    Serial.print("status : ");
    Serial.println(group_status);

    if(group_status > 0)
    {
        Serial.println("full");
        digitalWrite(LED_Blue_Pin, LOW); 
        delay(300);
        digitalWrite(LED_Red_Pin, HIGH); 
    }
    else
    {
        Serial.println("empty");
        digitalWrite(LED_Red_Pin, LOW); 
        delay(300);  
        digitalWrite(LED_Blue_Pin, HIGH);
    }
    
    return;
}

