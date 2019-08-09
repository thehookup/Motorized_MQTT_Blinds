#define USE_WIFIMANAGER
#define AP_NAME "ST Blinds Config"
#define ST_HUB_IP 192.168.86.70
/******************  ALEXA INTEGRATION LIBRARIES *************************************/
#include <SmartThingsESP8266WiFi.h>
#include <ESP8266mDNS.h>          //if you get an error here you need to install the ESP8266 board manager
/******************  WIFI MANAGER LIBRARIES *************************************/
#include <FS.h>  
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson//this needs to be first, or it all crashes and burns...
/******************  SKETCH SPECIFIC LIBRARIES *************************************/
#include <AH_EasyDriver.h>        //http://www.alhin.de/arduino/downloads/AH_EasyDriver_20120512.zip

/******************  BASIC PACKAGE LIBRARIES *************************************/
#include <SimpleTimer.h>          //https://github.com/thehookup/Simple-Timer-Library
#include <PubSubClient.h>         //https://github.com/knolleary/pubsubclient
#include <ESP8266WiFi.h>          //if you get an error here you need to install the ESP8266 board manager 
#include <ESP8266mDNS.h>          //if you get an error here you need to install the ESP8266 board manager 
#include <ArduinoOTA.h>           //ArduinoOTA is now included with the ArduinoIDE

/*****************  START USER CONFIG SECTION *********************************/
/*****************  START USER CONFIG SECTION *********************************/
/*****************  START USER CONFIG SECTION *********************************/
/*****************  START USER CONFIG SECTION *********************************/

//define your default values here, if there are different values in config.json, they are overwritten.
SmartThingsCallout_t messageCallout;    // call out function forward decalaration
IPAddress smartthings_hub_ip;
char char_hub_ip[20];
char rotations[6];
char current_position[6] = "0";
const unsigned int serverPort = 8090; // port to run the http server on
const unsigned int hubPort = 39500;
String string_hub_ip;
unsigned long send_rotations;

#define STEPPER_SPEED             35                  //Defines the speed in RPM for your stepper motor
#define STEPPER_STEPS_PER_REV     1028                //Defines the number of pulses that is required for the stepper to rotate 360 degrees
#define STEPPER_MICROSTEPPING     0                   //Defines microstepping 0 = no microstepping, 1 = 1/2 stepping, 2 = 1/4 stepping 
#define DRIVER_INVERTED_SLEEP     1                   //Defines sleep while pin high.  If your motor will not rotate freely when on boot, comment this line out.


#define STEPPER_DIR_PIN           D6
#define STEPPER_STEP_PIN          D7
#define STEPPER_SLEEP_PIN         D5
#define STEPPER_MICROSTEP_1_PIN   14
#define STEPPER_MICROSTEP_2_PIN   12
 
/*****************  END USER CONFIG SECTION *********************************/
/*****************  END USER CONFIG SECTION *********************************/
/*****************  END USER CONFIG SECTION *********************************/
/*****************  END USER CONFIG SECTION *********************************/
/*****************  END USER CONFIG SECTION *********************************/

WiFiClient espClient;
PubSubClient client(espClient);
SimpleTimer timer;
AH_EasyDriver shadeStepper(STEPPER_STEPS_PER_REV, STEPPER_DIR_PIN ,STEPPER_STEP_PIN,STEPPER_MICROSTEP_1_PIN,STEPPER_MICROSTEP_2_PIN,STEPPER_SLEEP_PIN);
st::SmartThingsESP8266WiFi smartthing(serverPort, ST_HUB_IP, hubPort, messageCallout);

//Global Variables
bool shouldSaveConfig = false;
bool boot = true;
int currentPosition = 0;
int newPosition = 0;
bool moving = false;
char positionPublish[50];

void messageCallout(String message)
{
  if (message.equals("on"))
  {
    smartthing.send("on");        // send message to cloud
    newPosition = send_rotations;
  }
  else if (message.equals("off"))
  {
    smartthing.send("off");       // send message to cloud
    newPosition = 0;
  }
}

#ifdef USE_WIFIMANAGER
void saveConfigCallback() 
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void configPortal()
{
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) 
  {
    Serial.println("mounted file system");
    
    //RECALL PROGRAM SPECIFIC SAVED VARIABLES
    
    if (SPIFFS.exists("/storedPosition.json"))
    {
      File storedFile = SPIFFS.open("/storedPosition.json", "r");
      if (storedFile) 
      {
        Serial.println("opened config file");
        size_t size = storedFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        storedFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
        {
          Serial.println("\nparsed json");
          strcpy(current_position, json["current_position"]);
          String tempPos = String((char *)current_position);
          currentPosition = tempPos.toInt();
          newPosition = tempPos.toInt();
          boot = false;
         } 
        else 
        {
          Serial.println("failed to load json config");
        }
      }
    }

    //RECALL WIFIMANAGER SPECIFIC VARIABLES
    
    if (SPIFFS.exists("/config.json")) 
    {
      Serial.println("opened wificonfig file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) 
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
        {
          Serial.println("\nparsed json");
          strcpy(char_hub_ip, json["char_hub_ip"]);
          strcpy(rotations, json["rotations"]);
        } 
        else 
        {
          Serial.println("failed to load json config");
        }
      }
    }
  } 
  else 
  {
    Serial.println("failed to mount FS");
  }

  //SPECIFIC PARAMETERS TO BE CONFIGURED IN WIFI MANAGER GO HERE
  //ORDER IS (ID, PLACEHOLDER TEXT, VARIABLE TO FILL, DEFAULT LENGTH

  WiFiManagerParameter custom_smartthings_hub_ip("ST_Hub_IP", "IP of your ST Hub", char_hub_ip, 20);
  WiFiManagerParameter custom_rotations("Rotations", "Number of rotations (12 recommended)", rotations, 6);

  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //ADD A SPECIFIC PARAMETER FOR EACH PROMPT ABOVE
  wifiManager.addParameter(&custom_smartthings_hub_ip);
  wifiManager.addParameter(&custom_rotations);

  //RESET STORED VARIABLES IF PIN D1 IS PULLED LOW
  if(digitalRead(D1) == LOW)
  {
    wifiManager.resetSettings();
  }
  
  //THIS STARTS THE AP
  if (!wifiManager.autoConnect(AP_NAME)) 
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //THIS HAPPENS IF THERE IS A SUCCESSFUL WIFI CONNECTION
  Serial.println("connected...");

  //GRAB THE VALUES FROM THE INPUTS AND SAVE TO VARIABLES
  strcpy(char_hub_ip, custom_smartthings_hub_ip.getValue());
  string_hub_ip = String(char_hub_ip);
  smartthings_hub_ip.fromString(string_hub_ip);
  strcpy(rotations, custom_rotations.getValue());
  send_rotations = strtoul(rotations, NULL, 10);
  
  //PUT THE VALUES FROM THE INPUTS INTO SPIFFS
  if (shouldSaveConfig) 
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["char_hub_ip"] = char_hub_ip;
    json["rotations"] = rotations;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) 
    {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
}
#endif

void processStepper()
{
  if (newPosition > currentPosition)
  {
    #if DRIVER_INVERTED_SLEEP == 1
    shadeStepper.sleepON();
    #endif
    #if DRIVER_INVERTED_SLEEP == 0
    shadeStepper.sleepOFF();
    #endif
    shadeStepper.move(80, FORWARD);
    currentPosition++;
    moving = true;
  }
  if (newPosition < currentPosition)
  {
    #if DRIVER_INVERTED_SLEEP == 1
    shadeStepper.sleepON();
    #endif
    #if DRIVER_INVERTED_SLEEP == 0
    shadeStepper.sleepOFF();
    #endif
    shadeStepper.move(80, BACKWARD);
    currentPosition--;
    moving = true;
  }
  if (newPosition == currentPosition && moving == true)
  {
    #if DRIVER_INVERTED_SLEEP == 1
    shadeStepper.sleepOFF();
    #endif
    #if DRIVER_INVERTED_SLEEP == 0
    shadeStepper.sleepON();
    #endif
    String temp_str = String(currentPosition);
    temp_str.toCharArray(positionPublish, temp_str.length() + 1);
    Serial.println("saving position");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["current_position"] = positionPublish;

    File storedFile = SPIFFS.open("/storedPosition.json", "w");
    if (!storedFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(storedFile);
    storedFile.close();
    
    moving = false;
  }
  Serial.println(currentPosition);
  Serial.println(newPosition);
}

//Run once setup
void setup() {
  pinMode(D1, INPUT_PULLUP);
  Serial.begin(115200);
  configPortal();
  string_hub_ip = String(char_hub_ip);
  smartthings_hub_ip.fromString(string_hub_ip);
  Serial.print("SmartThings Hub IP Set From WiFi Manager: ");
  Serial.println(smartthings_hub_ip);
  shadeStepper.setMicrostepping(STEPPER_MICROSTEPPING);            // 0 -> Full Step                                
  shadeStepper.setSpeedRPM(STEPPER_SPEED);     // set speed in RPM, rotations per minute
  #if DRIVER_INVERTED_SLEEP == 1
  shadeStepper.sleepOFF();
  #endif
  #if DRIVER_INVERTED_SLEEP == 0
  shadeStepper.sleepON();
  #endif
  WiFi.mode(WIFI_STA);
//  ArduinoOTA.setHostname();
//  ArduinoOTA.begin(); 
  delay(10);
  smartthing.init();
  if(currentPosition == 0)
  {
    smartthing.send("off");
  }
  else if(currentPosition == send_rotations)
  {
    smartthing.send("on");
  }
  timer.setInterval(((1 << STEPPER_MICROSTEPPING)*5800)/STEPPER_SPEED, processStepper);   

}

void loop() 
{
  smartthing.run();
//  ArduinoOTA.handle();
  timer.run();
}

