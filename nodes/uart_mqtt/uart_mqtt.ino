/*
The code for the ethernet enabled arduino. 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <Dhcp.h>
#include <PubSubClient.h>

#define SUB_TOPIC_SUFIX "/requests"
#define CLIENT_NAME     "Gateway"

String topic_prefix("/RFM69/");

/* Network config */
// setup the ip of the server
const IPAddress mqtt_server_ip(10, 1, 10, 97);
const uint16_t mqtt_port = 1883;
// This needs to be changed
const byte mac[]    = {  0xDE, 0xED, 0xBA1, 0xFE, 0xFE, 0xED };
 
// setup the networking objects
EthernetClient eth_client;
PubSubClient mqtt_client(eth_client);

String sub_topic;

uint8_t getNetworkID();
void reconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);

void setup()
{
  Serial.begin(115200);
  //Serial.setTimeout(1000);
  
  mqtt_client.setServer(mqtt_server_ip, mqtt_port);
  mqtt_client.setCallback(mqttCallback);

  Ethernet.begin(mac);

  //get the netid
  uint8_t net_id = getNetworkID();
  //add it to the prefix
  topic_prefix += String(net_id);
  
  //Serial.print(F("Network ID: "));
  //Serial.println(net_id);

  // set the sub topic based on the net id
  sub_topic = String(topic_prefix + SUB_TOPIC_SUFIX).c_str();
  
  //Serial.println(sub_topic);

  topic_prefix += '/';
  
  delay(1500);
}

bool publish = false;

void loop()
{
  
  if (!mqtt_client.connected()) {
    reconnect();
  }
  
  mqtt_client.loop();
}

// new data is available
void serialEvent()
{
  
  //Serial.println(F("Serial Event"));
  
  //keep reading bytes until the starting char is found
  if(Serial.read() != '/') {
    //Serial.println(F("ERROR:FORMAT"));
    return;
  }
  
  String pub_topic = topic_prefix + Serial.readStringUntil(':');

  //Serial.println(pub_topic);

  // parse the data to publish
  String payload = Serial.readStringUntil('\n');
  payload.trim();

  //Serial.println(message);

  /*
  // check to make sure the last to bytes are as suspected
  if (Serial.read() != '\n') {
    Serial.println(F("ERROR:FORMAT"));
    return;
  }
  */
  
  // publish the message
  mqtt_client.publish(pub_topic.c_str(), payload.c_str());

  /*
  if(mqtt_client.publish(pub_topic.c_str(), payload.c_str())) {
    Serial.println(F("Message Pubed"));
  } else {
    Serial.println(F("Pub Failed"));
  }
  */
}

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  
  //Serial.print(topic);
  //Serial.print(F(":"));

  // send the payload over serial
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print('\n');
  
}

// function to connect to server if connection is lost
void reconnect()
{
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    if (mqtt_client.connect(CLIENT_NAME)) {
      mqtt_client.subscribe(sub_topic.c_str());
    } 
    else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// request the network id from uart
uint8_t getNetworkID()
{
  
  while(Serial.available() < 1) {
    Serial.print(F("NETID"));
    Serial.print('\n');
    delay(500);
  }
  
  return Serial.parseInt();
}
