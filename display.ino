void DisplayDistance(float distance)
{  
    char buff[10];
    int  valueToBeDisplayed =0;
    display.clearDisplay();
    display.setCursor(10, 0);

 
    display.setTextSize(1);  
    sprintf(buff,"KM:");
    display.print(F(buff));       
    //valueToBeDisplayed = round(distance);
    sprintf(buff,"%.2f",distance);
    display.setTextSize(2); // Draw 2X-scale text
    display.println(F(buff));

    display.setTextSize(1); 
    sprintf(buff,"Usage:");
    display.print(F(buff));   
    valueToBeDisplayed = 0 ;    
    valueToBeDisplayed =  round(gBikeDistance) ;
    sprintf(buff,"%.2f",gBikeDistance);
    display.print(F(buff));
    sprintf(buff," Km");
    display.println(F(buff));       


    display.setTextSize(1);  
    sprintf(buff,"Mins:");
    display.print(F(buff));       
    valueToBeDisplayed = gtripDuration;
    sprintf(buff,"%d",valueToBeDisplayed);
    display.println(F(buff));

    display.display();

    
}


void DisplaySpeed(float gspeed)
{  
    char buff[10];
    byte Speed ;
    display.setCursor(10, 0);
    //Speed = round(gspeed);
    
    sprintf(buff,"KM/h:");
    display.setTextSize(1); // Draw 2X-scale text
    display.print(F(buff));
    //DEBUG_PRINTF("%s",buff);    
    
    sprintf(buff,"%.1f",gspeed);
    display.setTextSize(2); // Draw 2X-scale text
    display.println(F(buff));
    display.display();
}

void DisplayRPM(int grpm)
{
    byte rpm ;
    char buff[10];
      
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1); // Draw 2X-scale text
    sprintf(buff,"RPM:  "); 
    display.print(F(buff));
    //DEBUG_PRINTF(buff);
    sprintf(buff,"%d",grpm); 
    display.setTextSize(2); // Draw 2X-scale text
    display.println(F(buff));
    display.display();      // Show initial text
}

void SetupDisplay()
{
  char buff[10];
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { 
    // Address 0x3C for 128x32
    DEBUG_PRINTLN(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(500); 
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.println(F("Loading..."));
  display.setCursor(10, 0);
  display.display();      // Show initial text
}
