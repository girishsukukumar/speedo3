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
void CopyTemplateToRecordhtml()
{
  File inFile, outFile ;
  inFile =SPIFFS.open("/recordhead.html",  "r");
  outFile =SPIFFS.open("/records.html",  "w");
  while(inFile.available())
  {
    outFile.write(inFile.read());
  }
  inFile.close();
  outFile.close();
}
void SendFileToBrowser()
{
  File    filePtr ;
  String  selectedRecordFileNameStr, recordFile ;
  char    SelectedFileName[40], header[100];
  int     len, sent;
 
  if (webServer.hasArg("fileName"))
  {
     selectedRecordFileNameStr = webServer.arg("fileName") ;
     len = selectedRecordFileNameStr.length();
     selectedRecordFileNameStr.toCharArray(SelectedFileName,len+1); 
     
     if (SPIFFS.exists(SelectedFileName))
     { 
      
        filePtr = SPIFFS.open(SelectedFileName,  "r");
        
        recordFile = selectedRecordFileNameStr.substring(1); // Removing the / from the beginning of file name
        len = recordFile.length();
        recordFile.toCharArray(SelectedFileName,len+1); 
        sprintf(header,"filename=\"%s\"",SelectedFileName);
        webServer.sendHeader("Content-Disposition",header);
        sent = webServer.streamFile(filePtr, "application/text");  
        filePtr.close();
        webServer.send(200, "text/html", "<HTML> File sent </HTML>");   
     }
     else
     {
        char error[100];
        sprintf(error,"<HTML> <H1> File %s not found </H1> </HTML>",SelectedFileName);
        webServer.sendHeader("Connection", "close");
        webServer.send(200, "text/html", error);
     }
    
  }
  else
  {
        char error[100];
        sprintf(error,"<HTML> <H1> Argument received does not have fileName  </H1> </HTML>");
        webServer.sendHeader("Connection", "close");
        webServer.send(200, "text/html", error);
  }
}
void CopyTopToGraphHtml()
{
  File inFile, outFile ;
  inFile =SPIFFS.open("/graph_1.html",  "r");
  outFile =SPIFFS.open("/graph.html",  "w");
  while(inFile.available())
  {
    outFile.write(inFile.read());
  }
  inFile.close();
  outFile.close();  
}
void CopyBottomToGraphHtml()
{
  File inFile, outFile ;
  inFile =SPIFFS.open("/graph_2.html",  "r");
  outFile =SPIFFS.open("/graph.html",  FILE_APPEND);
  while(inFile.available())
  {
    outFile.write(inFile.read());
  }
  inFile.close();
  outFile.close();
  DEBUG_PRINTF("CopyBottomToGraphHtml\n");

}

void ShowGraph()
{
    File    recordFilePtr ,graphFilePtr ;
    String  selectedRecordFileNameStr, recordFile ;
    char    SelectedFileName[40], fileNameForScanning[40];
    int     len, sent;
    String  oneLineStr ;
    char    oneLine[35] ; 
    char    timeStamp[10];
    int     rpm ;
    float   Speed ;
    float   distance ;
    int     power ;
    char    lineToBeWritten[40];
    String  LineToBeWrittenStr ;
    //String  currentTime, currentDate,dateStamp;
    char    startTime[20],StartDate[20] ;
    int     splitT;
    int     i,count =0;
    bool    firstLineRead = false ;
    int     Day, Month, Year ;
  if (webServer.hasArg("fileName"))
  {
     selectedRecordFileNameStr = webServer.arg("fileName") ;
     len = selectedRecordFileNameStr.length();
     selectedRecordFileNameStr.toCharArray(SelectedFileName,len+1); 
     
     if (SPIFFS.exists(SelectedFileName))
     { 
        strcpy(fileNameForScanning,SelectedFileName);
        len = strlen(fileNameForScanning);
        for(i = 0 ; i < len ; i++)
        {
           if ((fileNameForScanning[i] == '-') || (fileNameForScanning[i] == ':'))
           {
               fileNameForScanning[i] = ' ' ;
           }
           
        }
        sscanf(fileNameForScanning,"/rec %d %d %d %d %d %d.csv",&Year, &Month,&Day, &len,&len,&len);
        
        len = 0 ;
        CopyTopToGraphHtml() ;
        graphFilePtr  = SPIFFS.open("/graph.html",  FILE_APPEND);
        if (graphFilePtr == NULL)
        {
          DEBUG_PRINTF("Unable to open graph.html is append mode\n");
        }
        recordFilePtr = SPIFFS.open(SelectedFileName,  "r");
        sprintf(lineToBeWritten,"['Time', 'RPM', 'Speed' , 'Power']");
        graphFilePtr.printf("%s",lineToBeWritten);
        
        while(recordFilePtr.available())
        {
          int i ;
          graphFilePtr.write(',');
          graphFilePtr.write('\n');
          oneLineStr = recordFilePtr.readStringUntil('\n'); 
          oneLineStr.replace(',',' '); // Replace comma with space so that sscanf() will work smoothly
          len = oneLineStr.length(); 
          oneLineStr.toCharArray(oneLine,len+1);
          
          i = sscanf(oneLine,"%s %d %f %f %d",timeStamp, &rpm, &Speed, &distance, &power);
          if (firstLineRead == false)
          {
            strcpy(startTime,timeStamp); //The time stamp of the first record show the work out start time
            firstLineRead = true ;
          }

         
          sprintf(lineToBeWritten, "['%s', %d,%f,%d]", timeStamp, rpm, Speed, power);
          
          LineToBeWrittenStr = lineToBeWritten ;
          
          graphFilePtr.printf("%s",lineToBeWritten);

          count++ ;
          
        }
        graphFilePtr.printf("]);\n");
        graphFilePtr.printf("var options = {\n");
        graphFilePtr.printf("title: 'Date:%d-%d-%d, Time of start: %s, Duration %d minutes, Distance %.2f Km',\n",Day,Month,Year,startTime, count,distance);
        
        recordFilePtr.close();
        graphFilePtr.close();
        CopyBottomToGraphHtml();
        
        graphFilePtr  = SPIFFS.open("/graph.html",  "r");
        sent = webServer.streamFile(graphFilePtr, "text/html");          
        graphFilePtr.close();
     }
     else
     {
        char error[100];
        sprintf(error,"<HTML> <H1> File %s not found </H1> </HTML>",SelectedFileName);
        webServer.sendHeader("Connection", "close");
        webServer.send(200, "text/html", error);
     }
  }
  else
  {
        char error[100];
        sprintf(error,"<HTML> <H1> Argument received does not have fileName  </H1> </HTML>");
        webServer.sendHeader("Connection", "close");
        webServer.send(200, "text/html", error);
  }
  
}
int CountNoOfRecords(String fileName)
{
  File csvFile ;
  int  count = 0 ;
  
  csvFile =SPIFFS.open(fileName,  FILE_READ);
  while(csvFile.available())
  {
     csvFile.readStringUntil('\n'); 
     count++ ;
  }
  csvFile.close() ;
  return count ;
}

void DisplayRecordsForAnalysis()
{
     File    htmlFile, rootDir,recordPtr ;
     size_t  sent;
     char    radioButtonTag[100];
     String  fileNameStr ;

     CopyTemplateToRecordhtml();
     rootDir = SPIFFS.open("/");
     htmlFile =SPIFFS.open("/records.html",  FILE_APPEND);
     htmlFile.print("<H1> <CENTER>Select a file for analysing </H1>\n");
     htmlFile.print("<form  action=\"/ShowGraph\" method=\"POST\">");
     htmlFile.print(" <TABLE border=\"5\"> ");

     recordPtr = rootDir.openNextFile();

     while(recordPtr)
     {
      recordPtr = rootDir.openNextFile();   
      fileNameStr = recordPtr.name();
      if (fileNameStr.endsWith(".csv") == true)
      {
        int noOfRecords;
        htmlFile.print("<TR>");
        htmlFile.print("<TD>");
        htmlFile.print(recordPtr.name());
        htmlFile.print("</TD>");
        noOfRecords = CountNoOfRecords(recordPtr.name());
        htmlFile.print("<TD>");
        htmlFile.print(noOfRecords);
        htmlFile.print("</TD>");
        htmlFile.print("<TD>");
        sprintf(radioButtonTag, "<input type=\"radio\" id=\"");
        htmlFile.print(radioButtonTag);
        htmlFile.print(recordPtr.name()); 
        htmlFile.print("\""); 
        htmlFile.print(" name=\"fileName\"");
        htmlFile.print(" value=\"");
        htmlFile.print(recordPtr.name());
        htmlFile.print("\">");        
        htmlFile.print("</TD>");
        htmlFile.print("</TR>\n");
      }
      
     }
  
     htmlFile.printf("</TABLE>\n");
     htmlFile.printf("<input type=\"submit\" value=\"Submit\">");
     htmlFile.printf("</form>");
     htmlFile.printf("</CENTER></HTML>\n");
     htmlFile.close();
     
     if (SPIFFS.exists("/records.html"))  
     { 
        htmlFile =SPIFFS.open("/records.html",  "r");
        sent =webServer.streamFile(htmlFile, "text/html");  
        htmlFile.close();
     }
     else
     {
         webServer.sendHeader("Connection", "close");
         webServer.send(200, "text/html", "<HTML> <H1> File records.htm; not found </H1> </HTML>");
     }
  
}
void ListRecords()
{
     File    htmlFile, rootDir,recordPtr ;
     size_t  sent;
     char    radioButtonTag[100];
     String  fileNameStr ;
     int     noOfRecords ;

     CopyTemplateToRecordhtml();
     rootDir = SPIFFS.open("/");
     htmlFile =SPIFFS.open("/records.html",  FILE_APPEND);
     htmlFile.print("<H1> <CENTER>Select  Files for Down load </H1>\n");
     htmlFile.print("<form  action=\"/ViewCSVFile\" method=\"POST\">");
     htmlFile.print(" <TABLE border=\"5\"> ");

     recordPtr = rootDir.openNextFile();

     while(recordPtr)
     {
      recordPtr = rootDir.openNextFile();   
      fileNameStr = recordPtr.name();
      if (fileNameStr.endsWith(".csv") == true)
      {
        htmlFile.print("<TR>");
        htmlFile.print("<TD>");
        htmlFile.print(recordPtr.name());
        htmlFile.print("</TD>");
        
        noOfRecords = CountNoOfRecords(recordPtr.name());
        htmlFile.print("<TD>");
        htmlFile.print(noOfRecords);
        htmlFile.print("</TD>");
                
        htmlFile.print("<TD>");
        sprintf(radioButtonTag, "<input type=\"radio\" id=\"");
        htmlFile.print(radioButtonTag);
        htmlFile.print(recordPtr.name()); 
        htmlFile.print("\""); 
        htmlFile.print(" name=\"fileName\"");
        htmlFile.print(" value=\"");
        htmlFile.print(recordPtr.name());
        htmlFile.print("\">");        
        htmlFile.print("</TD>");
        htmlFile.print("</TR>\n");
      }
      
     }
  

     htmlFile.printf("</TABLE>\n");
     htmlFile.printf("<input type=\"submit\" value=\"Submit\">");
     htmlFile.printf("</form>");
     htmlFile.printf("</CENTER> </HTML>\n");
     htmlFile.close();
     
     if (SPIFFS.exists("/records.html"))  
     { 
        htmlFile =SPIFFS.open("/records.html",  "r");
        sent =webServer.streamFile(htmlFile, "text/html");  
        htmlFile.close();
     }
     else
     {
         webServer.sendHeader("Connection", "close");
         webServer.send(200, "text/html", "<HTML> <H1> File records.htm; not found </H1> </HTML>");
     }

}
void DeleteRecordFile()
{
  String  selectedRecordFileNameStr ;
  if (webServer.hasArg("fileName"))
  {
     selectedRecordFileNameStr = webServer.arg("fileName") ;
     if (SPIFFS.exists(selectedRecordFileNameStr))
     { 
        SPIFFS.remove(selectedRecordFileNameStr);
        webServer.sendHeader("Connection", "close");
        webServer.send(200, "text/html", "<HTML> <H1> File Deleted </H1> </HTML>");
     }
  } 
  else
  {
        webServer.sendHeader("Connection", "close");
        webServer.send(200, "text/html", "<HTML> <H1> File not found  </H1> </HTML>");
    
  }
  
}
void DeleteRecords()
{
     File    htmlFile, rootDir,recordPtr ;
     size_t  sent;
     char    radioButtonTag[100];
     String  fileNameStr ;
     int     noOfRecords ;
     
     CopyTemplateToRecordhtml();
     rootDir = SPIFFS.open("/");
     htmlFile =SPIFFS.open("/records.html",  FILE_APPEND);
     htmlFile.print("<H1> <CENTER>Select  Files for Deletion </H1>\n");
     htmlFile.print("<form  action=\"/DeleteCSVFile\" method=\"POST\">");
     htmlFile.print(" <TABLE border=\"5\"> ");

     recordPtr = rootDir.openNextFile();
     while(recordPtr)
     {
      recordPtr = rootDir.openNextFile();
      
      fileNameStr = recordPtr.name();
      if (fileNameStr.endsWith(".csv") == true)
      {
        htmlFile.print("<TR>");
        htmlFile.print("<TD>");
        htmlFile.print(recordPtr.name());
        htmlFile.print("</TD>");

        noOfRecords = CountNoOfRecords(recordPtr.name());        
        htmlFile.print("<TD>");
        htmlFile.print(noOfRecords);
        htmlFile.print("</TD>");        
        
        htmlFile.print("<TD>");
        sprintf(radioButtonTag, "<input type=\"radio\" id=\"");
        htmlFile.print(radioButtonTag);
        htmlFile.print(recordPtr.name()); 
        htmlFile.print("\""); 
        htmlFile.print(" name=\"fileName\"");
        htmlFile.print(" value=\"");
        htmlFile.print(recordPtr.name());
        htmlFile.print("\">");        
        htmlFile.print("</TD>");
        htmlFile.print("</TR>\n");
      }
      
     }
     
     htmlFile.printf("</TABLE>\n");
     htmlFile.printf("<input type=\"submit\" value=\"Submit\">");
     htmlFile.print("</form>");
     htmlFile.printf("</CENTER> </HTML>\n");
     htmlFile.close();
     if (SPIFFS.exists("/records.html"))  
     { 
        htmlFile =SPIFFS.open("/records.html",  "r");
        sent =webServer.streamFile(htmlFile, "text/html");  
        htmlFile.close();
     }
     else
     {
         webServer.sendHeader("Connection", "close");
         webServer.send(200, "text/html", "<HTML> <H1> File Sensor.html not found </H1> </HTML>");
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

void ViewCSVFile()
{
  
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
  webServer.on("/ListRecords",      HTTP_POST,  ListRecords);
  webServer.on("/resettrip",        HTTP_POST,  ResetTrip);
  webServer.on("/DeleteRecords",    HTTP_POST,  DeleteRecords);
  webServer.on("/gaugeDisplay",     HTTP_POST,  DisplayGaugeDisplay);
  webServer.on("/ViewCSVFile",      HTTP_POST,  SendFileToBrowser);
  webServer.on("/DeleteCSVFile",    HTTP_POST,  DeleteRecordFile);
  webServer.on("/Analysis",         HTTP_POST,  DisplayRecordsForAnalysis);
  webServer.on("/ShowGraph",        HTTP_POST,  ShowGraph);
  
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
