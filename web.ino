void DisplayserverIndex()
{
     File  file ;
     size_t  sent;
 
     if (SPIFFS.exists("/serveridx.html"))  
     { 
        file =SPIFFS.open("/serveridx.html",  "r");
        sent =webServer.streamFile(file, "text/html");  
        file.close();
     }
     else
     {
         webServer.sendHeader("Connection", "close");
         webServer.send(200, "text/html", "<HTML> <H1> File Sensor.html not found </H1> </HTML>");
     }


}
void ChangeDetails()
{
      File  file ;
     size_t  sent;
 
     if (SPIFFS.exists("/config.html"))  
     { 
        file =SPIFFS.open("/config.html",  "r");
        sent =webServer.streamFile(file, "text/html");  
        file.close();
     }
     else
     {
         webServer.sendHeader("Connection", "close");
         webServer.send(200, "text/html", "<HTML> <H1> File loginIndex.html not found </H1> </HTML>");
     }
 
}
void FileUpload()
{
     File  file ;
     size_t  sent;
 
     if (SPIFFS.exists("/upload.html"))  
     { 
        file =SPIFFS.open("/upload.html",  "r");
        sent =webServer.streamFile(file, "text/html");  
        file.close();
     }
     else
     {
         webServer.sendHeader("Connection", "close");
         webServer.send(200, "text/html", "<HTML> <H1> File loginIndex.html not found </H1> </HTML>");
     }
  
}

void RebootDevice()
{
    ESP.restart();
}
void DisplayLoginIndex()
{
//    webServer.sendHeader("Connection", "close");
//    webServer.send(200, "text/html", loginIndex);
     File  file ;
     size_t  sent;
 
     if (SPIFFS.exists("/loginIndex.html"))  
     { 
        file =SPIFFS.open("/loginIndex.html",  "r");
        sent =webServer.streamFile(file, "text/html");  
        file.close();
     }
     else
     {
         webServer.sendHeader("Connection", "close");
         webServer.send(200, "text/html", "<HTML> <H1> File loginIndex.html not found </H1> </HTML>");
     }

}
void DisplayDashboard()
{
     File  file ;
     size_t  sent;
 
     if (SPIFFS.exists("/sensor.html"))  
     { 
        file =SPIFFS.open("/sensor.html",  "r");
        sent =webServer.streamFile(file, "text/html");  
        file.close();
     }
     else
     {
         webServer.sendHeader("Connection", "close");
         webServer.send(200, "text/html", "<HTML> <H1> File Sensor.html not found </H1> </HTML>");
     }

}
void DisplayGaugeDisplay()
{
     File  file ;
     size_t  sent;
 
     if (SPIFFS.exists("/gauge.html"))  
     { 
        file =SPIFFS.open("/gauge.html",  "r");
        sent =webServer.streamFile(file, "text/html");  
        file.close();
     }
     else
     {
         webServer.sendHeader("Connection", "close");
         webServer.send(200, "text/html", "<HTML> <H1> File gauge.html not found </H1> </HTML>");
     }

}
void PostDetails()
{
  int   hours   = 0 ;
  int   seconds = 0 ;
  int   minutes = 0 ;
  char  durationStr[15]; 
  const size_t capacity = JSON_OBJECT_SIZE(18);
  DynamicJsonDocument doc(capacity);
  static float SpeedData ;
  static int   RPMData ;
  float  gTripDistance  = 0 ;
  float  gDistanceKM    = 0 ;
  int    gTotalDistance = 0 ;

//  minutes = gtripDuration / 60;
  //seconds = gtripDuration % 60;
//  hours   = minutes / 60;
//  minutes = minutes % 60;
  
  
  char jsonString[250];
  int gSpeed ;
  float  gRoomTemp      = 0.0 ;
  
  int    gPulseRate     = 0 ;
  float  gBodyTempInCelius = 0.0 ;
  float  gRoomHumidity  = 0.0 ;
  int    gSatellites    = 0 ;
  struct speedDistance speedAndDistance ;
  sprintf(durationStr,"%d:%d:%d",hours, minutes,seconds); 
  
  Serial.println("Post Details");
  
  if (xQueueReceive(SPEED_TO_WEBSERVER, (void *)&speedAndDistance, 0) == pdTRUE) 
  {
     gSpeed = round(speedAndDistance.Speed);
  }

  if (xQueueReceive(CADENCE_TO_WEBSERVER, (void *)&RPMData, 0) != pdTRUE) 
  {
    
  }
  
 
  
  doc["Speed"]         = round(gSpeed);
  doc["Cadence"]       = round(RPMData);
  doc["Distance"]      = round(speedAndDistance.distanceKM);
  doc["TotalDistance"] = round(gTotalDistance);
  doc["Pulse"]         = round(gPulseRate);
 
  
  if (gSpeed > 0.0)
  {
      doc["Status"] = "Moving";
  }
  else 
  {
    doc["Status"] = "Stopped";
  }
  gCounter++;    
  doc["Duration"]          = durationStr;
  doc["RoomTemperature"]   = gRoomTemp ;
  doc["BodyTemperature"] = gBodyTempInCelius ;
  
  doc["RoomHumidity"]      = round(gRoomHumidity) ;
  doc["AverageSpeed"]      = 0.0;
  doc["RestDuration"]      = 0.0;
  doc["Satellites"]        = gSatellites ;
  doc["GPSTimeSet"]        = false;
  doc["KeepAlive"]          = gCounter ;
  serializeJson(doc, jsonString);
  webServer.sendHeader("Connection", "close");
  webServer.send(200, "json", jsonString);
  Serial.println(jsonString);

}
void UpdateConfigJson()
{
  // To update config.json
}

void ResetTrip()
{
 //  gtripDuration =0 ;
 //  CreateNewRecordFile();
}

void setupWebHandler()
{
   /*return index page which is stored in serverIndex */
  
  webServer.on("/",                 HTTP_GET,   DisplayLoginIndex);
  webServer.on("/serverIndex",      HTTP_GET,   DisplayserverIndex);
  webServer.on("/dashboard",        HTTP_POST,  DisplayDashboard);
  webServer.on("/speed",            HTTP_POST,  PostDetails);
  webServer.on("/ConfigPage",       HTTP_POST,  ChangeDetails);
  webServer.on("/Fileupload",       HTTP_POST,  FileUpload);
  webServer.on("/rebootDevice",     HTTP_POST,  RebootDevice);
  webServer.on("/updateConfigJson", HTTP_POST,  UpdateConfigJson);
  webServer.on("/resettrip",        HTTP_POST,  ResetTrip);
//webServer.on("/ListRecords",      HTTP_GET,   ListRecords);
//webServer.on("/DeleteRecords",    HTTP_POST,  DeleteRecords);
  webServer.on("/gaugeDisplay",     HTTP_POST,  DisplayGaugeDisplay);



  /*handling uploading firmware file */
  webServer.on("/update", HTTP_POST, []() {
  webServer.sendHeader("Connection", "close");
  webServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = webServer.upload();
    if (upload.status == UPLOAD_FILE_START) {
      //DEBUG_PRINTF("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        //DEBUG_PRINTF("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  webServer.begin();
  Serial.println("WebServer Setup Completed");
}
