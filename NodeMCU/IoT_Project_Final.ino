
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>  // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Connect and publish to the MQTT broker
#include <Wire.h>
#include <DHT.h>
#include "MAX30105.h"
#include "heartRate.h"

// WiFi
const char* wifi_ssid = "TP-LINK_FEC8";     // Your personal network SSID
const char* wifi_password = "44682378";     // Your personal network password

// MQTT
const char* mqtt_server = "192.168.1.184";  // IP of the MQTT broker
const char* humidity_topic = "nodemcu/sensor/humidity";
const char* temperature_topic = "nodemcu/sensor/temperature";
const char* heartrate_topic = "nodemcu/sensor/heartrate";
const char* oxygen_topic = "nodemcu/sensor/oxygen";
const char* bodytemp_topic = "nodemcu/sensor/bodytemp";
const char* mqtt_username = "";      // MQTT username
const char* mqtt_password = "";      // MQTT password
const char* clientID = "nodemcu";    // MQTT client ID

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
// 1883 is the listener port for the Broker
PubSubClient client(mqtt_server, 1883, wifiClient); 


//MAX30102
MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
 
float beatsPerMinute = 0.0;
int beatAvg = 0;
int oxygen = 0;
float body_temp = 0.0;
unsigned long irValue = 0;

unsigned long previousTimeMonitor = 0;

const int sending_interval = 3000;

const float K1 = -45.06;
const float K2 = 30.354;
const float K3 = 94.845;

//DHT11
#define DHTPIN D3        // what digital pin the DHT11 is conected to
#define DHTTYPE DHT11    // there are multiple kinds of DHT sensors
DHT dht(DHTPIN, DHTTYPE);
float t = 0.0;
float h = 0.0;

void setup() {
  
  Serial.begin(115200);
  dht.begin();
  setup_wifi();

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");
   
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

}


// Custom function to connet to the MQTT broker via WiFi
void connect_MQTT(){
  // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  // If the connection is failing, make sure you are using the correct MQTT Username and Password (Setup Earlier in the Instructable)
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {    
    Serial.println("Connection to MQTT Broker failed...");
    Serial.println("NodeMCU is restarting now!");
    ESP.restart();
  }

}

void setup_wifi(){
  delay(10);
  WiFi.begin(wifi_ssid, wifi_password);
  while(WiFi.status() != WL_CONNECTED){
  
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  
    unsigned long currentTime = millis();
      
    irValue = particleSensor.getIR();
    unsigned long redValue = particleSensor.getRed();
    float Z = (float)redValue/(float)irValue;  

    if (checkForBeat(irValue) == true)
    {
      body_temp = particleSensor.readTemperature();
         
      oxygen = 0;
      if(beatAvg != 0) 
        oxygen = (int)(K1 * Z + K2) * Z + K3;

      //We sensed a beat!
      unsigned long delta = millis() - lastBeat;
      lastBeat = millis();
       
      beatsPerMinute = 60 / (delta / 1000.0);
       
      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable
         
        //Take average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }

    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);
    Serial.print(", Oxygen=");
    Serial.print(oxygen);
    if(irValue < 50000){
      Serial.println(" No finger?");
    }
    else Serial.println();
    
    Serial.print("Body Temperature="); 
    Serial.print(body_temp);
    Serial.println(" *C");

    h = dht.readHumidity();
    t = dht.readTemperature();    
    Serial.print("Humidity=");
    Serial.print(h);
    Serial.print(" %, Temperature=");
    Serial.print(t);
    Serial.println(" *C");  

    if(currentTime - previousTimeMonitor > sending_interval){

      connect_MQTT();
      
      // PUBLISH to the MQTT Broker (topic = Temperature, defined at the beginning)
      if (client.publish(temperature_topic, String(t).c_str())) {
        Serial.println("Temperature sent!");
      }
    
      // PUBLISH to the MQTT Broker (topic = Humidity, defined at the beginning)
      if (client.publish(humidity_topic, String(h).c_str())) {
        Serial.println("Humidity sent!");
      }
      
      if(irValue >= 50000){
           
        // PUBLISH to the MQTT Broker(Heartrate)
        if (client.publish(heartrate_topic, String(beatAvg).c_str())) {
          Serial.println("Heartrate sent!");
        }

        // PUBLISH to the MQTT Broker(Oxygen)
        if (client.publish(oxygen_topic, String(oxygen).c_str())) {
          Serial.println("Oxygen sent!");
        }
        
        // PUBLISH to the MQTT Broker(Body Temperature)
        if (client.publish(bodytemp_topic, String(body_temp).c_str())) {
          Serial.println("Body Temp sent!");
        }     
      }
      previousTimeMonitor = currentTime;
    }

    Serial.println();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
