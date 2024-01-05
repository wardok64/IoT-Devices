/*
 * Gateway_Xbee_ESP12_MQTT
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <XBee.h>

const char* ssid = "SSID";
const char* password = "1234";

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();

/* Local ip/DNS Raspberry pi( Broker MQTT ) */
const char* mqtt_server = "nodered.local"; 

WiFiClient espClient2;
PubSubClient client(espClient2);

void setup_wifi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println("");
  //Serial.print("WiFi connected - ESP IP address: ");
  //Serial.println(WiFi.localIP());
}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    //Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client2")) 
    {
     // Serial.println("connected");  
    } 
    else 
    {
     //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void setup() 
{
  //Serial.begin(9600);
  xbee.begin(Serial);
  setup_wifi();

  // Config status LED 
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);

  // Server and port MQTT
  client.setServer(mqtt_server, 1883);
  //client.publish("/gateway/estado", "online");
}


void loop() 
{

  String payload, elemento;
  String SH;
  String SL;

  if (!client.connected()) 
  {
    reconnect();
  }
  if(!client.loop())
  {
    client.connect("ESP8266Client2");
  }

  xbee.readPacket(); 

  if (xbee.getResponse().isAvailable()) 
  {
    // if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) { 
    xbee.getResponse().getZBRxResponse(rx);
    for (int i = 9; i < rx.getDataLength(); i++) 
    {
      payload += (char)rx.getData(i);
    }

    SH = String(rx.getRemoteAddress64().getMsb(),HEX);
    SL = String(rx.getRemoteAddress64().getLsb(),HEX);

    // Serial.println(SH);
    // Serial.println(SL);
    // Serial.println(payload);
    
    String topic = "/linea/area/equipo/";
    String topic_;
    char variable = (char)rx.getData(8);
    elemento += char(rx.getData(6));
    elemento += char(rx.getData(7));
    String slash = "/";
    
    topic_ = topic + elemento + slash + variable + slash + SL;
      
    char _SL[39];// [sizeof(topic_)];//topic_
    topic_.toCharArray(_SL,39);// sizeof(_SL));
        
    char _payload[sizeof(payload)];
    payload.toCharArray(_payload, sizeof(_payload));
        
    client.publish(_SL, _payload);
    for(int i=0; i < 2; i++)
    { 
      // Blink state LED
      digitalWrite(2, 0);
      delay(30);
      digitalWrite(2, 1);
      delay(30);
    }
    // }
  }
  else if (xbee.getResponse().isError()) 
  {
    // Serial.println("Error reading packet.  Error code: ");  
    // Serial.println(xbee.getResponse().getErrorCode());
  } 
  delay(100);
}
