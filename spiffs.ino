void ReadPersistantDataFromSPIFFS()
{
    File jsonFile ;
  
/*
   {
      "TotalDistance": 100000.1,
      "TripDuration"  : 4000
   }
*/

   const size_t capacity = JSON_OBJECT_SIZE(1) + 20;

   DynamicJsonDocument doc(capacity);
   DEBUG_PRINTF("Reading Persistant Data File\n");
   jsonFile = SPIFFS.open(JSON_PERSISTANT_FILE_NAME, FILE_READ);
  
   if (jsonFile == NULL)
   {     
     gBikeDistance = 0  ;
     gtripDuration  = 0 ;
     DEBUG_PRINTF("Unable to open %s, default values assumed",JSON_PERSISTANT_FILE_NAME);
     return ;
   }

   deserializeJson(doc, jsonFile);

   float TotalDistance = doc["TotalDistance"]; 
   int TripDuration    = doc["TripDuration"]; // 4000

   // Convert Km to meters as our distance calculation is in meters.
   gBikeDistance= TotalDistance ;
   
   gBikeDistanceInMeters = TotalDistance *1000.0 ;
   
   gtripDuration  = TripDuration ;
   DEBUG_PRINTF("Reading Persistant Data File Done\n");
   jsonFile.close();
   return ;
}

void WritePersistantDataToSPIFFS()
{
    File jsonFile ; 
    StaticJsonDocument<256> doc;
    char JsonStr[250];
    DEBUG_PRINTF("WritePersistantDataToSPIFFS done \n");

    jsonFile = SPIFFS.open(JSON_PERSISTANT_FILE_NAME, FILE_WRITE);
    
    if (jsonFile == NULL)
    {
      DEBUG_PRINTF("Unable to open %s",JSON_PERSISTANT_FILE_NAME);
      return ;
    }
    
    doc["TotalDistance"] = gBikeDistance ;
    doc["TripDuration"] = gtripDuration ;

    serializeJson(doc, JsonStr);
    jsonFile.printf("%s\n", JsonStr);
    DEBUG_PRINTF("WritePersistantDataToSPIFFS done \n");
    jsonFile.close();
    return ;
}



void CreateRecordFile()
{
  File    recordFile ;
  int len;
  String currentTime, currentDate,dateStamp;
  char    deviceTime[20],deviceDate[20] ;
  int     splitT;
  
  if (gWifiConnection == true)
  {
      currentDate = timeClient.getFormattedDate(); 

      splitT = currentDate.indexOf("T");
      dateStamp = currentDate.substring(0, splitT);
      len = dateStamp.length();   
      currentDate.toCharArray(deviceDate,len+1);
   
      currentTime = timeClient.getFormattedTime(); 
      len = currentTime.length();
      currentTime.toCharArray(deviceTime,len+1);
             
      sprintf(recordFileName, "/rec-%s-%s.csv",deviceDate,deviceTime);
  }
  else
  {
    //Wifi is not available so date function do not work 
    // Create record name using random number generator
    
     sprintf(recordFileName, "/outdoor-%d-%d.csv",random(1000, 2000),random(3000, 4000));
  }
  recordFile =  SPIFFS.open(recordFileName, FILE_WRITE);
  DEBUG_PRINTF("Created record file,%s\n",recordFileName);
  recordFile.close(); 
}
void ReadConfigValuesFromSPIFFS()
{
  File jsonFile ;
  const size_t capacity = JSON_OBJECT_SIZE(8) + 240;
  DynamicJsonDocument doc(capacity);

  //const char* json = "{\"ssid1\":\"xxxxxxxxxxxxxxxxxxxx\",\"password1\":\"xxxxxxxxxxxxxxxxxxxx\",\"ssid2\":\"xxxxxxxxxxxxxxxxxxxx\",\"password2\":\"xxxxxxxxxxxxxxxxxxxx\",\"ssid3\":\"xxxxxxxxxxxxxxxxxxxx\",\"password3\":\"xxxxxxxxxxxxxxx\",\"wheelDiameter\":85.99,\"devicename\":\"xxxxxxxxxxxxxx\"}";
  
  jsonFile = SPIFFS.open(JSON_CONFIG_FILE_NAME, FILE_READ);
  
  if (jsonFile == NULL)
  {
     DEBUG_PRINTF("Unable to open %s",JSON_CONFIG_FILE_NAME);
     return ;
  }
  
  deserializeJson(doc, jsonFile);

  const char* ssid1 = doc["ssid1"]; // "xxxxxxxxxxxxxxxxxxxx"
  const char* password1 = doc["password1"]; // "xxxxxxxxxxxxxxxxxxxx"
  const char* ssid2 = doc["ssid2"]; // "xxxxxxxxxxxxxxxxxxxx"
  const char* password2 = doc["password2"]; // "xxxxxxxxxxxxxxxxxxxx"
  const char* ssid3 = doc["ssid3"]; // "xxxxxxxxxxxxxxxxxxxx"
  const char* password3 = doc["password3"]; // "xxxxxxxxxxxxxxx"
  float wheelDiameter = doc["wheelDiameter"]; // 85.99
  const char* devicename = doc["devicename"]; // "xxxxxxxxxxxxxx"
  jsonFile.close();
  
  strcpy(ConfigData.ssid1,ssid1);
  strcpy(ConfigData.password1,password1);
  
  strcpy(ConfigData.ssid2,ssid2);
  strcpy(ConfigData.password2,password2);

  strcpy(ConfigData.ssid3,ssid3);
  strcpy(ConfigData.password3,password3);
  DEBUG_PRINTF("%s \n", ssid3);
  ConfigData.wheelCirumference = (3.14 * wheelDiameter)/100 ;// in meters
  DEBUG_PRINTF("%s \n", devicename);
  strcpy(ConfigData.wifiDeviceName,devicename);

//TODO Items for config file 
  strcpy(ConfigData.mqttServer, "platform.i2otlabs.com");
                                
  ConfigData.mqttPort = 1883 ;
  
  strcpy(ConfigData.userName,"girish_kumar"); // TODO Read from JSON file
  sprintf(ConfigData.dataRequestTopic,"MASTER\/WORKOUTDATA\/%s",ConfigData.userName);
  strcpy(ConfigData.mqttUser,"dataone");
  strcpy(ConfigData.mqttPassword,"onedata");
  ConfigData.TrainerMode = true ;
}

void WriteToRecordFile(float Speed,int cadence,float DistanceKM)
{
  int     Power ;
  File    recordFile ;
  
  String  currentTime,formattedDate ;
  char    deviceTime[20] ;
  int     len ;
  
  recordFile =  SPIFFS.open(recordFileName, FILE_APPEND);
  
  if (recordFile == NULL)
  {
    DEBUG_PRINTF("Unable to open record file,%s\n",recordFileName);
    return ;
  }
  
  Power = ComputePower(Speed);
  
  DEBUG_PRINTF("WriteToRecordFile gWifiConnection = %d \n",gWifiConnection);
  
  if (gWifiConnection == true)
  {
     currentTime = timeClient.getFormattedTime(); 
     len = currentTime.length();
     currentTime.toCharArray(deviceTime,len+1);
      DEBUG_PRINTF("Writting to record file %s\n", deviceTime);
     recordFile.printf("%s, %d,%0.1f,%0.1f,%d\n",
                             deviceTime,cadence,Speed,DistanceKM,Power);
  }  
  else
  {
  
     recordFile.printf("%d,%d,%0.1f,%0.1f,%d\n",
                        timeCounter,cadence,Speed,DistanceKM,Power);
     timeCounter++ ;
  }
  //WritePersistantDataToSPIFFS();                
  recordFile.flush();
  recordFile.close();
}

void DisplayConfigValues()
{
   DEBUG_PRINTF("ssid1 %s \n",ConfigData.ssid1);
   DEBUG_PRINTF("Password %s \n", ConfigData.password1);

   DEBUG_PRINTF("ssid2 %s \n",ConfigData.ssid2);
   DEBUG_PRINTF("Password2 %s \n", ConfigData.password2);

   DEBUG_PRINTF("ssid3 %s \n",ConfigData.ssid3);
   DEBUG_PRINTF("Password3 %s \n", ConfigData.password3);

   DEBUG_PRINTF("Wheel Circumference = %f\n", ConfigData.wheelCirumference);
   DEBUG_PRINTF("Device name = %s ", ConfigData.wifiDeviceName);
}
