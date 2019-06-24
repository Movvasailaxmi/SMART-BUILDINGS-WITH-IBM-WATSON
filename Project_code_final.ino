#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"  
#define DHTPIN D2   
#define DHTTYPE DHT11   

DHT dht (DHTPIN, DHTTYPE);

float temperature;
int humidity;
int ldr_data;
int pir_data;
String command;
String data="";

void callback(char* topic, byte* payload, unsigned int payloadLength);

//WIFI CREDENTIALS
const char* ssid = "milee";
const char* password = "mahi485ka";

//DEVICE CREDENTIALS 
#define ORG "4y51d2"
#define DEVICE_TYPE "smartBuild"
#define DEVICE_ID "141414"
#define TOKEN "hzfkbw60xr66R&yj!7" 

//PIN DECLARATIONS
#define LED1 D0
#define LED2 D1
#define PIR D3
#define ldr A0


const char publishTopic[] = "iot-2/evt/sensorsdata/fmt/json";
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/cmd/home/fmt/String";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;



WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 5000; 
long lastPublishMillis;
void publishData();

void setup() 
{  
   Serial.begin(115200);//to begin serial communication
   Serial.println();
   pinMode(ldr,INPUT);//to declare whether the pins are used as I/O
   pinMode(PIR,INPUT);
   pinMode(LED1,OUTPUT);
   pinMode(LED2,OUTPUT);
   dht.begin(); 
   wifiConnect();
   mqttConnect();
}
 

void loop() 
{
 if (millis() - lastPublishMillis > publishInterval)
  {
    publishData();
    lastPublishMillis = millis();
  } 
  if (!client.loop()) 
     mqttConnect();
}


void wifiConnect() 
{
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected, IP address: "); 
  Serial.println(WiFi.localIP());
}


void mqttConnect() 
{
  if (!client.connected()) 
  {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
    Serial.print(".");
    delay(500);
  }
  initManagedDevice();
  Serial.println();
  }
}


void initManagedDevice() 
{
  if (client.subscribe(topic)) 
    Serial.println("subscribe to cmd OK");
  else 
    Serial.println("subscribe to cmd FAILED");
}


void callback(char* topic, byte* payload, unsigned int payloadLength) 
{  
  Serial.print("callback invoked for topic: ");
  Serial.println(topic);
  for (int i = 0; i < payloadLength; i++) 
  {  
    command+= (char)payload[i];
  }
  Serial.print("data: "+ command);
  control_func();
  command= "";
}


void control_func()
{
  if(command=="echo")
  {
    if(pir_data)
    {
      if(ldr_data>500) //based on light intensity the leds are operated
      {
        analogWrite(LED1,255);
        analogWrite(LED2,255);
      }
       else
       {
        analogWrite(LED1,20);
        analogWrite(LED2,20);
       }
    }  
  else
    {
      digitalWrite(LED1,LOW);
      digitalWrite(LED2,LOW);
      Serial.println("Nobody is present");
      Serial.println("Lights are OFF");
    }
  }
  
  else if(command=="away")
  {
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    Serial.println("Lights are OFF");
  }
  
  else if(command== "lightoff")
  {
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    Serial.println("Lights are OFF");    
  }
  
  else if(command== "lighton")
  {
    digitalWrite(LED1,HIGH);
    digitalWrite(LED2,HIGH);
    Serial.println("Lights are ON");
  }
   
  else
  {
    Serial.println("No commands have been subscribed");
  }
}

void publishData() 
{
  humidity= dht.readHumidity();
  temperature= dht.readTemperature();
  ldr_data=analogRead(A0);
  pir_data=digitalRead(D3);
   
  if (isnan(humidity) || isnan(temperature)||isnan(ldr_data)||isnan(pir_data))
  {
    Serial.println("Failed to read from the sensors!");
    return;
  }
  
  String payload = "{\"d\":{\"Temperature\":";
  payload += temperature;
  payload += ",""\"Humidity\":";
  payload +=  humidity;
  payload += ",""\"LDR\":";
  payload +=  ldr_data;
  payload += ",""\"PIR\":";
  payload +=  pir_data;
  payload += "}}";

  Serial.print("\n");
  Serial.print("Sending payload: "); 
  Serial.println(payload);

  if (client.publish(publishTopic, (char*) payload.c_str())) 
  {
    Serial.println("Publish OK");
  }
  else 
  {
    Serial.println("Publish FAILED");
  }
}
