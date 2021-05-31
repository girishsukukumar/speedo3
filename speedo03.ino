/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *  ESP8266 Arduino example
 */
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <SPI.h>
#include <WiFiMulti.h>
#include <WiFiMulti.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "SPIFFS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266FtpServer.h>
#include <NTPClient.h>
#include "time.h"
#define DEBOUNCETIME 30
#define WEBSERVER_PORT 80
#define JSON_CONFIG_FILE_NAME "/config.json" 
#define JSON_PERSISTANT_FILE_NAME "/persistant.json"
#define SSID_NAME_LEN 20 
#define SSID_PASSWD_LEN 20
#define NAME_LEN 40
#define FTP_USER_NAME "apollo11"
#define FTP_PASSWORD  "eagle"
#define MAX_LIMIT_OF_CONNETION_FAILURE 5


#define CORE_ONE 1
#define CORE_ZERO 0 

#define MQTT_FINGER_PRINT_LENGTH    150
#define JSON_STRING_LENGTH          350
#define PASSWORD_NAME_LENGTH         20
#define USERNAME_NAME_LENGTH         20
#define MQTT_MAX_CONNECTION_ATTEMPTS 10
#define GEN_NAME_LEN                 20
#define HOST_NAME_LENGTH             40
#define   DEBUG 1 
//#define  NETWORK_DEBUG 1

// *******************Enable and disable serial print **************
#ifdef DEBUG
#define DEBUG_PRINTLN(x)      Serial.println (x)
#define DEBUG_PRINT(x)        Serial.print (x)
#define DEBUG_PRINTF(f,...)   Serial.printf(f,##__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(f,...)
#endif


#ifdef NETWORK_DEBUG
    #define    DEBUG_PRINTF(f,...)  Debug.printf(f,##__VA_ARGS__)
#endif

// *******************Enable and disable serial print **************
typedef struct configData 
{
  char     ssid1[SSID_NAME_LEN]   ;
  char     password1[SSID_PASSWD_LEN]  ;
  char     ssid2[SSID_NAME_LEN] ;
  char     password2[SSID_PASSWD_LEN] ;
  char     ssid3[SSID_NAME_LEN] ;
  char     password3[SSID_PASSWD_LEN] ;
  float    wheelCirumference ;
  char     wifiDeviceName[NAME_LEN] ;  
  char     userName[NAME_LEN];
  bool     sendDataToinFlux ;
  char     mqttServer[HOST_NAME_LENGTH] ;
  int      mqttPort ;
  char     mqttUser[USERNAME_NAME_LENGTH] ;
  char     mqttPassword[PASSWORD_NAME_LENGTH] ;
  uint8_t  mqttCertFingerprint[MQTT_FINGER_PRINT_LENGTH] ;
  uint8_t  mqttJsonStatus ;
  char     dataRequestTopic[HOST_NAME_LENGTH+USERNAME_NAME_LENGTH];
  bool     TrainerMode ;
};

typedef struct speedDistance
{
  float Speed ;
  float distanceKM ;
} ;


int            gtripDuration =0.0;
String         gHotSpotIP ;
WiFiMulti      wifiMulti;          //  Create  an  instance  of  the ESP32WiFiMulti 
struct         configData ConfigData;
const byte     CADENCE_PIN = 18  ;
const byte     SPEED_PIN   = 19  ;
FtpServer      ftpSrv; 
WebServer      webServer(WEBSERVER_PORT);
WiFiUDP        ntpUDP;
NTPClient      timeClient(ntpUDP);
int            msg_queue_len = 10;
char           pulse = '1' ;

BaseType_t xHigherPriorityTaskWoken = pdFALSE;
static     QueueHandle_t CADENCE_MSG_QUEUE ;
static     QueueHandle_t SPEED_MSG_QUEUE ;
static     QueueHandle_t CADENCE_TO_DISPLAY_MODULE ;
static     QueueHandle_t SPEED_TO_DISPLAY_MODULE ;
static     QueueHandle_t SPEED_TO_WEBSERVER  ;
static     QueueHandle_t CADENCE_TO_WEBSERVER ;

portMUX_TYPE cadenceMux = portMUX_INITIALIZER_UNLOCKED;
volatile int      numberOfCadenceInterrupts = 0;
volatile bool     cadencePinLastState;
volatile uint32_t cadenceDebounceTimeout = 0;

portMUX_TYPE speedMux = portMUX_INITIALIZER_UNLOCKED;
volatile int      numberOfSpeedInterrupts = 0;
volatile bool     speedPinLastState;
volatile uint32_t speedDebounceTimeout = 0;
char              recordFileName[NAME_LEN]; 
//                     1    2    3    4    5   6    7    8   9    10
//-------------------------------------------------------------------
int PowerTable[60] = { 0,   0,   0,   1,   3,  7,  10,  15,  21,  25, //0
                      30,   35, 40,  45,  50, 55,  65,  70,  75,  80, //1
                      90,  100,105, 115, 125,135, 145, 155, 165, 180, //2
                     190,  210,220, 240, 250,265, 280, 300, 320, 330, //3
                     345,  360,380, 410, 430,450,480,  500, 530, 550, //4
                     575,  610,640, 675, 720,750,790,  825, 860, 900};//5 

int    gCounter       = 0 ;
float  gTripDistance  = 0 ;
float  gBikeDistance  = 0.0 ;
float  gBikeDistanceInMeters = 0.0;
bool   gWifiConnection = false ;
int    timeCounter = 0 ; // To be used to track time, in case wifi is not avalable

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//Adafruit_SSD1306 display(0x3c, 21, 22); 
//Adafruit_SSD1306 display(0x3c, 21, 22); 



void ICACHE_RAM_ATTR SPEED_PIN_Handler()
{
  portENTER_CRITICAL_ISR(&speedMux);
  numberOfSpeedInterrupts++;
//  DEBUG_PRINTLN(numberOfSpeedInterrupts);
  speedPinLastState = digitalRead(SPEED_PIN);
  speedDebounceTimeout = xTaskGetTickCount(); //version of millis() that works from interrupt
  portEXIT_CRITICAL_ISR(&speedMux);
}



void ICACHE_RAM_ATTR CADENCE_PIN_Handler()
{
  portENTER_CRITICAL_ISR(&cadenceMux);
  numberOfCadenceInterrupts++;
  cadencePinLastState = digitalRead(CADENCE_PIN);
  cadenceDebounceTimeout = xTaskGetTickCount(); //version of millis() that works from interrupt
  portEXIT_CRITICAL_ISR(&cadenceMux);
}

bool ConnectToWifi()
{
  int count ;
  wifiMulti.addAP(ConfigData.ssid1, ConfigData.password1);   
  wifiMulti.addAP(ConfigData.ssid2, ConfigData.password2);    
  wifiMulti.addAP(ConfigData.ssid3, ConfigData.password3);    
  count  = 0 ;
  while  (wifiMulti.run()  !=  WL_CONNECTED) 
  { 
    //  Wait  for the Wi-Fi to  connect:  scan  for Wi-Fi networks, and connect to  the strongest of  the networks  above       
    delay(1000);        
    DEBUG_PRINTF("*");    
    count++ ;
    if (count > 40)
    {
       return false ;  
    }
  }   
  delay(5000);
  WiFi.setHostname(ConfigData.wifiDeviceName);
  DEBUG_PRINTF("\n");   
  DEBUG_PRINTF("Connected to  ");   
  DEBUG_PRINTF("%s\n",WiFi.SSID().c_str());         
  DEBUG_PRINTF("IP  address: ");   
  DEBUG_PRINTLN(WiFi.localIP().toString()); 
  WiFi.softAPdisconnect (true);   //Disable the Access point mode.
  return true ;
}


int ComputePower(float s)
{
  /* 
   *  PowerTable is specific to Jet Black Pro fluid trainer
   *  This can be extract from the specification of any trainer
   *  Provided the manufacture publishes it
   *  The power table for Jet Black trainer can be found at 
   *  https://www.jetblackcycling.com/indoor-cycle-trainers-and-accessories/z1-pro-fluid-trainer/
   *  https://www.jetblackcycling.com/wp-content/uploads/2020/03/jetblack-cycling-Fluid-Resistance-1536x697.png
   *  Alternatively an equation for Kurt Kinetic Trainer can be found at 
   *  https://www.cycling-inform.com/how-to-use-a-cheap-25-bike-computer-to-measure-power-with-a-kurt-kinetic-trainer
   *  This formula can be tuned  for other trainer too.
   *  
  */
  
  int idx, power ;
  idx = round(s);
  if (idx >= 60)
  {
    power = PowerTable[59];
  }
  else if (idx > 0)
  {
     power = PowerTable[idx-1] ;
  }
  else if (idx == 0)
  {
    power = 0 ;
  }
  else
     power = -1 ; // Error will be reported in record file
  
  DEBUG_PRINTF("Computer Power %f, %d, %d \n", s, idx, power);
      
  return power ;
}

void DISPLAY_TASK(void *parameters)
{
   int cadence  = 0 ;
   struct speedDistance speedAndDistance ;
   bool flag = false ;
   int zeroCadenceWaterMark =0  ;
  display.clearDisplay();
  DisplaySpeed(speedAndDistance.Speed);
  DisplayRPM(cadence);

   while(1)
   {    
    if (xQueueReceive(CADENCE_TO_DISPLAY_MODULE, &cadence, 0) == pdTRUE) 
    {
       //DEBUG_PRINT("RPM: "); 
       //DEBUG_PRINTLN(cadence);
       flag = true ;
       //WritePersistantDataToSPIFFS();
       if (cadence == 0)
       {
          // Count the number of time Cadence is zero
          zeroCadenceWaterMark++ ;
       }
       else
       {
          zeroCadenceWaterMark = 0 ;
       }

       if (zeroCadenceWaterMark <= 2)
       {
          // Do not write to record file, in case cadence remains at zero 
          // for more than 2 minutes.
          
          WriteToRecordFile(speedAndDistance.Speed,
                            cadence,
                            speedAndDistance.distanceKM);
       }
    }
    if (xQueueReceive(SPEED_TO_DISPLAY_MODULE, &speedAndDistance, 0) == pdTRUE) 
    {
        //DEBUG_PRINT("Speed: "); 
        //DEBUG_PRINTLN(speedAndDistance.Speed) ;    
        flag = true ;
    }
    if (flag)
    {
      display.clearDisplay();
      DisplaySpeed(speedAndDistance.Speed);
      DisplayRPM(cadence);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      display.clearDisplay();
      DisplayDistance(speedAndDistance.distanceKM);      
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      flag = false ;      
    }    
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
}
void COUNT_CADENCE(void *parameters) 
{
  int cadenceTicks =0;
  char item = 'c';
  int currentTime  ;
  int LastCadenceComputedTime =0;
  int RPM ;
  int timerVal;
  int diff ;
  bool persistantDataWritten = false ;
  while(true)
  {
    if (xQueueReceive(CADENCE_MSG_QUEUE, (void *)&item, 0) == pdTRUE) 
    {
      cadenceTicks++ ;     
    }
    currentTime = millis(); 
    diff = currentTime - LastCadenceComputedTime ;
    if (diff >= 60000)
    {
       RPM = cadenceTicks ;
       xQueueSend(CADENCE_TO_DISPLAY_MODULE, (void*) &cadenceTicks,0);
       xQueueSend(CADENCE_TO_WEBSERVER, (void*) &cadenceTicks,  0);
       if (cadenceTicks == 0)
       {
         DEBUG_PRINTF("cadenceTick = %d %d \n", cadenceTicks,diff);
         if (diff >= 60000)
         {
            DEBUG_PRINTF("cadenceTick = %d \n", cadenceTicks);
            if (persistantDataWritten == false)
            {
               DEBUG_PRINTF("Calling WritePersistantDataToSPIFFS\n");
               WritePersistantDataToSPIFFS();
               persistantDataWritten = true ;
               // Persistant data need to written only once if the 
               // pedal stops for more that 2 mins 
            }
         }
       }       
       if (cadenceTicks > 0)
       {
           gtripDuration= gtripDuration + 1 ;
           persistantDataWritten = false ;
       }
       cadenceTicks = 0 ;                     
       LastCadenceComputedTime = currentTime ;   
    }
  }  
}

void COUNT_SPEED(void *parameters) 
{
  int    speedTicks =0;
  float  distanceTravelled = 0 ;
  char   item  = 'c';
  int    currentTime = 0 ;
  float  timeFrame ;
  //float  Speed  ;
  int    LastSpeedComputedTime=0 ;

  float  DistanceKM     = 0 ;
  int    gTotalDistance = 0 ;
  struct speedDistance speedAndDistance ;
  //ConfigData.wheelCirumference = 2.19  ; //meter had coded for testing to beremoved
  while (true)
  {
     if (xQueueReceive(SPEED_MSG_QUEUE, (void *)&item, 0) == pdTRUE) 
     {
       speedTicks++ ;
     }  
     
     currentTime = millis();
     timeFrame = (currentTime - LastSpeedComputedTime)/1000 ;
     if (timeFrame >= 5.0)
     {
         distanceTravelled =  (speedTicks * ConfigData.wheelCirumference);
         LastSpeedComputedTime = currentTime ;
         speedAndDistance.Speed  = distanceTravelled/timeFrame ;

         speedAndDistance.Speed  = distanceTravelled / timeFrame ; // This is meters/per
         speedAndDistance.Speed  = speedAndDistance.Speed * 3.6 ; // convert m/sec into Km/hr 
 
         gTripDistance =gTripDistance+distanceTravelled ;
         speedAndDistance.distanceKM= gTripDistance/1000 ;
         //DEBUG_PRINTF("Distance = %f", speedAndDistance.distanceKM);
         
         if (gTripDistance > 1000.0)
         {
            if (speedAndDistance.Speed > 0)
            {
               gBikeDistanceInMeters = gBikeDistanceInMeters + distanceTravelled;

               // Convert the distance in KM
               gBikeDistance =  gBikeDistanceInMeters/1000 ;            
            }
         }
         speedTicks = 0 ;
         // Make structure and send it across ro WEBSERVER
         xQueueSend(SPEED_TO_WEBSERVER, (void*) &speedAndDistance,  0);
         xQueueSend(SPEED_TO_DISPLAY_MODULE, (void*) &speedAndDistance , 0);         
     }
    
  }
}
void ConfigureAsAccessPoint()
{
   IPAddress local_IP(192,168,4,4);
   IPAddress gateway(192,168,5,5);
   IPAddress subnet(255,255,255,0);

   WiFi.softAP(ConfigData.wifiDeviceName, "12345689");  //Start HOTspot removing password will disable security
   DEBUG_PRINTF("Wifi.softAP completed ");
   WiFi.softAPConfig(local_IP, gateway, subnet);
   IPAddress myIP = WiFi.softAPIP(); //Get IP address
   gHotSpotIP = myIP.toString() ; 
   display.clearDisplay();
   display.println(F("WiFi Hotspot"));
   display.println(gHotSpotIP.c_str());  //extract the string for display
   display.display();
   vTaskDelay(5000);

}

void setup() 
{
   int GMTOffset = 19800; //sec

   gBikeDistance = 0.0;
   Serial.begin(115200);
   delay(10);
   SPIFFS.begin(true) ;
   SetupDisplay();
   ReadConfigValuesFromSPIFFS();
   
   DisplayConfigValues();
   CADENCE_MSG_QUEUE = xQueueCreate(msg_queue_len, sizeof(char));   
   SPEED_MSG_QUEUE = xQueueCreate(msg_queue_len, sizeof(char));

   SPEED_TO_DISPLAY_MODULE  = xQueueCreate(msg_queue_len, sizeof(struct speedDistance));
   CADENCE_TO_DISPLAY_MODULE  = xQueueCreate(msg_queue_len, sizeof(int));

   SPEED_TO_WEBSERVER  = xQueueCreate(msg_queue_len, sizeof(struct speedDistance));
   CADENCE_TO_WEBSERVER  = xQueueCreate(msg_queue_len, sizeof(int));
   
   //DEBUG_PRINTF(" gBikeDistance= %f\n",gBikeDistance);
   pinMode(CADENCE_PIN, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(CADENCE_PIN), CADENCE_PIN_Handler, RISING);

   pinMode(SPEED_PIN, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(SPEED_PIN), SPEED_PIN_Handler, RISING);
  // We start by connecting to a WiFi network
  ConfigData.TrainerMode = true ;
  display.clearDisplay();
  display.println(F("Connecting to Wifi..."));
  display.setCursor(10, 0);
  display.display();      // Show initial text

  if (ConnectToWifi() == true)
  {
     configTime(GMTOffset, 0, "pool.ntp.org");
     gWifiConnection = true ;
     timeClient.begin();
     timeClient.setTimeOffset(GMTOffset); /* GMT + 5:30 hours */
     timeClient.update(); // Keep the device time up to date
     delay(5000);
  }
  if (ConnectToWifi() == false)
  {
       DEBUG_PRINTF("WifiSetup: failed \n");
      ConfigureAsAccessPoint(); 
  }
  CreateRecordFile();
  WriteToRecordFile(0.0, 0 , 0.0);

  display.clearDisplay();
  display.setCursor(10, 0);
  display.setTextSize(1);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.println(F("Sensors:Ready"));
  display.display();
  setupWebHandler();
  ftpSrv.begin(FTP_USER_NAME,FTP_PASSWORD);
  ReadPersistantDataFromSPIFFS();
  xTaskCreatePinnedToCore(COUNT_CADENCE,
                          "Track Cadence",
                          4096,
                          NULL,
                          1,
                          NULL,
                          1);

   xTaskCreatePinnedToCore(COUNT_SPEED,
                          "Track Speed",
                          2048,
                          NULL,
                          1,
                          NULL,
                          1);
                          
  xTaskCreatePinnedToCore(DISPLAY_TASK,
                          "Display Task",
                          2048,
                          NULL,
                          1,
                          NULL,
                          1);
   xTaskCreatePinnedToCore(MANAGE_DEBOUNCE_CADENCE_PIN,
                          "Manage Debouce Cadence Task",
                          2048,
                          NULL,
                          1,
                          NULL,
                          1);
                      
 xTaskCreatePinnedToCore(MANAGE_DEBOUNCE_SPEED_PIN,
                          "Manage Debouce Speed Task",
                          2048,
                          NULL,
                          1,
                          NULL,
                          1);
}

void MANAGE_DEBOUNCE_CADENCE_PIN(void *parameters)
{
  uint32_t saveDebounceTimeout;
  bool     saveLastState;
  int      save;

  while (1) 
  {

    portENTER_CRITICAL_ISR(&cadenceMux); // so that value of numberOfButtonInterrupts,l astState are atomic - Critical Section
       save  = numberOfCadenceInterrupts;
       saveDebounceTimeout = cadenceDebounceTimeout;
       saveLastState  = cadencePinLastState;
    portEXIT_CRITICAL_ISR(&cadenceMux); // end of Critical Section

    bool currentState = digitalRead(CADENCE_PIN);

    // This is the critical IF statement
    // if Interrupt Has triggered AND Button Pin is in same state AND the debounce time has expired THEN you have the button push!
    //
    if ((save != 0) //interrupt has triggered 
         && (currentState == saveLastState) // pin is still in the same state as when intr triggered
         && (millis() - saveDebounceTimeout > DEBOUNCETIME ))
    { 
      // and it has been low for at least DEBOUNCETIME, then valid keypress
      
      if (currentState == HIGH)
      {
        //DEBUG_PRINTF("Cadence tick high\n");        
        xQueueSend(CADENCE_MSG_QUEUE, &pulse, 0 );
      }
      else
      {
        //Serial.printf("Cadence tick low %d\n",save);        
      }
      
      //Serial.printf("Button Interrupt Triggered %d times, current State=%u, time since last trigger %dms\n", save, currentState, millis() - saveDebounceTimeout);
      
      portENTER_CRITICAL_ISR(&cadenceMux); // can't change it unless, atomic - Critical section
      numberOfCadenceInterrupts = 0; // acknowledge keypress and reset interrupt counter
      portEXIT_CRITICAL_ISR(&cadenceMux);
      //Serial.printf("Out of critical section");

      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);

  }
}

void MANAGE_DEBOUNCE_SPEED_PIN(void *parameters)
{
  uint32_t saveDebounceTimeout;
  bool     saveLastState;
  int      save;

  while (1) 
  {
    portENTER_CRITICAL_ISR(&speedMux); // so that value of numberOfButtonInterrupts,l astState are atomic - Critical Section
       save  = numberOfSpeedInterrupts;
       saveDebounceTimeout = speedDebounceTimeout;
       saveLastState  = speedPinLastState;
    portEXIT_CRITICAL_ISR(&speedMux); // end of Critical Section

    bool currentState = digitalRead(SPEED_PIN);

    // This is the critical IF statement
    // if Interrupt Has triggered AND Button Pin is in same state AND the debounce time has expired THEN you have the button push!
    //
    if ((save != 0) //interrupt has triggered 
         && (currentState == saveLastState) // pin is still in the same state as when intr triggered
         && (millis() - saveDebounceTimeout > DEBOUNCETIME ))
    { 
      // and it has been low for at least DEBOUNCETIME, then valid keypress
      
      if (currentState == HIGH)
      {
        //DEBUG_PRINTLN(" Speed tick high");        
        xQueueSend( SPEED_MSG_QUEUE, &pulse, 0 );
      }
      else
      {
       
      }
      
      //Serial.printf("Button Interrupt Triggered %d times, current State=%u, time since last trigger %dms\n", save, currentState, millis() - saveDebounceTimeout);
      
      portENTER_CRITICAL_ISR(&speedMux); // can't change it unless, atomic - Critical section
      numberOfSpeedInterrupts = 0; // acknowledge keypress and reset interrupt counter
      portEXIT_CRITICAL_ISR(&speedMux);
      //Serial.printf("Out of critical section");

      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);

  }
  
}

void loop() 
{
  webServer.handleClient();
  ftpSrv.handleFTP(); 
  yield();
}
