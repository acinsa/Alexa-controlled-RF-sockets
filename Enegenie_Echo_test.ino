#include <ESP8266WiFi.h>
#include "fauxmoESP.h"
#include <RCSwitch.h>

// RCSwitch configuration
RCSwitch mySwitch = RCSwitch();  // 433MHz Transmitter
RCSwitch s3Switch = RCSwitch();  // 315MHz Transmitter

// Create an instance of the server on Port 80 for webpage
WiFiServer server(80);
WiFiClient client;
unsigned long ulReqcount;        // how often has a valid page been requested


// Network configuration
fauxmoESP fauxmo;
#define WIFI_SSID "BTHub5-C7JX"
#define WIFI_PASS "834f4e89af"

// Declared for the webpage


// Allow access to ESP8266 SDK
extern "C" 
{
#include "user_interface.h"
}

// -----------------------------------------------------------------------------
// Switch the RF433/315 Sockets on/off
// -----------------------------------------------------------------------------
void SwItchRF( unsigned char deviceid, boolean Mystate ){
        if(deviceid == 0){          // Livingroom Lamp (433MHz)
        if(Mystate==true){
        mySwitch.switchOn(1,1);
        }else{
        mySwitch.switchOff(1,1);
        }
        }else if(deviceid==1){      // Hall White LED's (433MHz)
        if(Mystate==true){
        mySwitch.switchOn(1,2);
        }else{
        mySwitch.switchOff(1,2);
        }
        }else if(deviceid==2){      // Kitchen LED's (433MHz)
        if(Mystate==true){
        mySwitch.switchOn(1,3);
        }else{
        mySwitch.switchOff(1,3);
        }
        }else if(deviceid == 3){    // TV (433MHz) Turns one 433MHz socket on/off
        if(Mystate==true){
        s3Switch.send(7585927, 24);
        }else{
        s3Switch.send(7585926, 24);
        }
        }else if(deviceid==4){      // HiFi (433Mhz)Turns one 433MHz socket on/off
        if(Mystate==true){
        s3Switch.send(7585935, 24);
        }else{
        s3Switch.send(7585934, 24);
        }
        }else if(deviceid == 5){    // AV (433MHz)Turns both 433MHz sockets on/off
        if(Mystate==true){
        s3Switch.send(7585933, 24);
        }else{
        s3Switch.send(7585932, 24);
        }
        }else{return;}
}
        

void setup() {

    Serial.begin(115200);
    Serial.println();
    Serial.println();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.printf("\r\n\r\n[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    server.begin();
    mySwitch.enableTransmit( 4 );   // 433MHz Transmitter
    s3Switch.enableTransmit( 5 );   // 315MHz Transmitter
    
    // Fauxmo
    fauxmo.addDevice("lamp");
    fauxmo.addDevice("hall");
    fauxmo.addDevice("kitchen");
    fauxmo.addDevice("TV");
    fauxmo.addDevice("hifi");
    fauxmo.addDevice("AV");
    
    // fauxmoESP 2.0.0 has changed the callback signature to add the device_id
    // it's easier to match devices to action without having to compare strings.
    fauxmo.onMessage([](unsigned char device_id, const char * device_name, bool state) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
    SwItchRF( device_id, state );
    });

}




void loop() {

    // Since fauxmoESP 2.0 the library uses the "compatibility" mode by
    // default, this means that it uses WiFiUdp class instead of AsyncUDP.
    // The later requires the Arduino Core for ESP8266 staging version
    // whilst the former works fine with current stable 2.3.0 version.
    // But, since it's not "async" anymore we have to manually poll for UDP
    // packets
    fauxmo.handle();


  //----------------------------------------------------------------
  // Check if a client has connected
  //----------------------------------------------------------------
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  
  // Wait until the client sends some data
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    return; 
  }
  
  //----------------------------------------------------------------
  // Read the first line of the request
  //----------------------------------------------------------------
  String sRequest = client.readStringUntil('\r');
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
    client.stop();
    return;
  }
  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?show=1234 HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
 
  
  //----------------------------------------------------------------
  // format the html response
  //----------------------------------------------------------------


if(sPath=="/")
  {   
   ulReqcount++;
   String diagdat="";
   String duration1 = " ";
   int hr,mn,st;
   st = millis() / 1000;
   mn = st / 60;
   hr = st / 3600;
   st = st - mn * 60;
   mn = mn - hr * 60;
   if (hr<10) {duration1 += ("0");}
   duration1 += (hr);
   duration1 += (":");
   if (mn<10) {duration1 += ("0");}
   duration1 += (mn);
   duration1 += (":");
   if (st<10) {duration1 += ("0");}
   duration1 += (st);     
   client.println("HTTP/1.1 200 OK"); 
   client.println("Content-Type: text/html");
   client.println("Connection: close");
   client.println();
   client.println("<!DOCTYPE HTML>");
   diagdat+="<html><head><title>Home Monitor</title></head><body>";
   diagdat+="<font color=\"#000000\"><body bgcolor=\"#a0dFfe\">";
   diagdat+="<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
   diagdat+="<h1>Alexa Home Voice / Webpage Control<BR>ESP8266 FauxMo Device</h1>";
   diagdat+="<BR>  Web Page Requests = ";
   diagdat+=ulReqcount;                            
   diagdat+="<BR>  Free RAM = ";
   client.println(diagdat);
   client.print((uint32_t)system_get_free_heap_size()/1024);
   diagdat=" KBytes";            
   diagdat+="<BR>  System Uptime =";
   diagdat+=duration1;                                                             
   client.print(diagdat);
   diagdat="<BR><hr><BR><table><tr><td>";//  Webpage buttons for RF 433/315 in HTML Table
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=1-on\"><INPUT TYPE=\"submit\" VALUE=\"Lamp On\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=2-on\"><INPUT TYPE=\"submit\" VALUE=\"Hall LED On\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=3-on\"><INPUT TYPE=\"submit\" VALUE=\"Kitchen On\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=4-on\"><INPUT TYPE=\"submit\" VALUE=\"TV On\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=5-on\"><INPUT TYPE=\"submit\" VALUE=\"hifi On\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=6-on\"><INPUT TYPE=\"submit\" VALUE=\"AV On\"></FORM><br></td></tr><tr><td>";                                
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=1-off\"><INPUT TYPE=\"submit\" VALUE=\"Lamp Off\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=2-off\"><INPUT TYPE=\"submit\" VALUE=\"Hall LED Off\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=3-off\"><INPUT TYPE=\"submit\" VALUE=\"Kitchen Off\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=4-off\"><INPUT TYPE=\"submit\" VALUE=\"TV Off\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=5-off\"><INPUT TYPE=\"submit\" VALUE=\"hifi Off\"></FORM><br></td><td>";
   diagdat+="<FORM METHOD=\"LINK\" ACTION=\"/&socKet=6-off\"><INPUT TYPE=\"submit\" VALUE=\"AV Off\"></FORM><br></td></tr></table>";
   client.print(diagdat);
   diagdat="<hr><BR>";
   diagdat+="<BR><FONT SIZE=-1>environmental.monitor.log@gmail.com<BR><FONT SIZE=-1>ESP8266 With RF 433MHz + 315MHz <BR> FauxMO(WEMO Clone) to Amazon ECHO Dot Gateway<BR>";
   diagdat+="<FONT SIZE=-2>Compiled Using ESP ver. 2.2.3(Arduino 1.6.13), built March, 2017<BR></body></html>";
   client.println(diagdat);
   diagdat = "";
   duration1 = "";
   // and stop the client
   delay(1);
   client.stop();
  }
  
if (sPath.startsWith("/&socKet=")){          // Request from webpage buttons
   client.println("HTTP/1.1 204 OK");        // No Data response to buttons request
   client.println("Connection: close");      // We are done Close the connection
   delay(1);                                 // Give time for connection to close
   client.stop();                            // Stop the client
   ulReqcount++;                             // Tracking counter for page requests
   unsigned char device_id;
   boolean state;
   if (sPath.startsWith("/&socKet=1-on")) {
    device_id=0;
    state=true;
    SwItchRF( device_id, state );            // Button request to SwItchRF()
  } else if (sPath.startsWith("/&socKet=1-off")) {
    device_id=0;
    state=false;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=2-on")) {
    device_id=1;
    state=true;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=2-off")) {
    device_id=1;
    state=false;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=3-on")) {
    device_id=2;
    state=true;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=3-off")) {
    device_id=2;
    state=false;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=4-on")) {
    device_id=3;
    state=true;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=4-off")) {
    device_id=3;
    state=false;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=5-on")) {
    device_id=4;
    state=true;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=5-off")) {
    device_id=4;
    state=false;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=6-on")) {
    device_id=5;
    state=true;
    SwItchRF( device_id, state );
  } else if (sPath.startsWith("/&socKet=6-off")) {
    device_id=5;
    state=false;
    SwItchRF( device_id, state );
  }else{return;}
    
}
} 
