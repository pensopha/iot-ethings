/* Here ESP32 will keep 1 roles: 
1/ read data from DHT22 sensor
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ssd1306syp.h>   
#define SDA_PIN 4              // uses GPIO pins 4(SDA) and 5(SCL) of the external Adafruit Feather
#define SCL_PIN 5 

Adafruit_ssd1306syp display(SDA_PIN,SCL_PIN);     


/* change it with your ssid-password */
const char* ssid = "YOUR SSID";
const char* password = "YOUR WIFIPASSWORD";
/* this is the IP of PC/raspberry where you installed MQTT Server 
on Wins use "ipconfig" 
on Linux use "ifconfig" to get its IP address */
const char* mqtt_server = "ethings.io";

/* define DHT pins */
#define DHTPIN 13
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float temperature = 0;
float humidity = 0;
/* create an instance of PubSubClient client */


WiFiClient espClient;
PubSubClient client(espClient);

/*LED GPIO pin*/
const char led = 21;

/* topics */
#define TEMP_TOPIC    "demo/temparature/putyournamehere"


long lastMsg = 0;
char msg[20];

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  /* we got '1' -> on */
  if ((char)payload[0] == '1') {
    digitalWrite(led, HIGH); 
  } else {
    /* we got '0' -> on */
    digitalWrite(led, LOW);
  }

}

void mqttconnect() {
  /* Loop until reconnected */
    int count=0;
  while (!client.connected()) {
    Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "putyournamehere";
    /* connect now */
    if (client.connect("putyournamehere", "testethings", "passethings" )) {
      Serial.println("connected");
      /* subscribe topic with default QoS 0*/
      
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
     
         delay(5000);
       if(count > 1){
        setup();
        }
        count++;
    }
  }
}

void setup() {
   int count1=0;
  Serial.begin(115200);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
       
    delay(500);
    Serial.print(".");
    if(count1 > 1){
        setup();
        }
        count1++;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  /* configure the MQTT server with IPaddress and port */
  client.setServer(mqtt_server, 1883);
  /* this receivedCallback function will be invoked 
  when client received subscribed topic */
  client.setCallback(receivedCallback);
  /*start DHT sensor */
  dht.begin();
   display.begin(SSD1306_SWITCHCAPVCC, 0x78 >> 1);
      display.display();
      delay(2000);
      display.clearDisplay();
 
}
void loop() {
  /* if client was disconnected then try to reconnect again */
  if (!client.connected()) {
    mqttconnect();
  }
  /* this function will listen for incomming 
  subscribed topic-process-invoke receivedCallback */
  client.loop();
  /* we measure temperature every 3 secs
  we count until 3 secs reached to avoid blocking program if using delay()*/
  long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;
    /* read DHT11/DHT22 sensor and convert to string */
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
                 char thbuffer[50];                         // Combine temp, humidity into a buffer of size X
            char *p = thbuffer; 
            
    if (!isnan(temperature)) {
            dtostrf(temperature, 4, 2, p);
            p += strlen(p);
            p[0] = ','; p++;                                                     
            dtostrf(humidity, 4, 2, p);
            p += strlen(p);
   
      /* publish the message */
      client.publish(TEMP_TOPIC, thbuffer);
      Serial.println(thbuffer);
       display.drawString(0, 0, "www.ethings.io");
  // text display tests

  display.setCursor(20, 50);
  display.print(temperature);
  display.println("C");
  display.setCursor(20, 52);
  display.print(humidity);
  display.println("%");
  display.clearDisplay();
      
    }
  }
}


