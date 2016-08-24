#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "website.h" //location of html file flash array

const int digitalPin[] = {12,13,14,15,16}; //digital pins to use
const char *ssid = "ESPNETWORK"; //network name
const char *password = "reconsucks1"; //network password, if applicable
String serverResponse;
String serialData;
String path;

ESP8266WebServer server(80);

void setup()
{
  Serial.begin (115200);
  Serial.println("\nESP8266 starting...");
  for(int pin : digitalPin){ //initializing all the pins defined in digitalPin[] as outputs and LOW
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  
//  WiFi.softAP(ssid, password);
  WiFi.softAP(ssid); //configuring server to have no security (lack of password parameter)
  
  server.onNotFound(handleRequest); //way of handling all requests
  
  server.begin();
  delay(500);
  Serial.println("Ready");
}

void loop(){
  server.handleClient(); //waits for client and handles and passes all requests to onNotFound -> handleRequest
}

void handleRequest(){
  path = server.uri(); //defines the global 'path' string to the path of the client's request
  if(path.endsWith("/")){ //client opening the main webpage '/'
    handleRoot();
  }
  else if(path.indexOf("digital")>-1){ //client is making a '/digital/PIN/STATE' request
    handleDigital();
  }
  else if(path.indexOf("serial")>-1){ //clioent making a '/server/DATA' request
    handleSerial();
  }
  else{
    handleNotFound();
  }
}

void handleRoot(){
  loadFromFlash(path);
}

void handleDigital(){
  int pin = path.substring(9,11).toInt(); //parses the PIN parameter from the request
  int inVal = path.substring(12,13).toInt(); //parses the STATE paramter from the request
  digitalWrite(pin, inVal);
  buildDigitalResponse(pin); //builds the global 'serverResponse' string
  server.send(200, "text/plain", serverResponse); //'200 OK' and 'serverResponse' sent to the client
}

void buildDigitalResponse(int pin){
  serverResponse="digital,"+String(pin)+","+String(digitalRead(pin))+"\n";
}

void handleSerial(){
    Serial.println(path); //sends the data received to the serial port
    buildSerialResponse(); //builds the global 'serverResponse' string
    server.send(200, "text/plain", serverResponse); //'200 OK' and 'serverResponse' sent to the client
}

void buildSerialResponse(){
  serverResponse = "Data received at: "; //builds the data confirmation response
  serverResponse += String(millis());
  serverResponse += "\n";
}

bool loadFromFlash(String path) { //loads the root webpage from FLASH; flash array located in 'website.h'
  if(path.endsWith("/")) path += "index.html";
  
  int NumFiles = sizeof(files)/sizeof(struct t_websitefiles);
  
  for(int i=0; i<NumFiles; i++) {
    if(path.endsWith(String(files[i].filename))) {      
      _FLASH_ARRAY<uint8_t>* filecontent;
      String dataType = "text/plain";
      unsigned int len = 0;
      
      dataType = files[i].mime;
      len = files[i].len;
      
      server.send(200, dataType, "", len);
      filecontent = (_FLASH_ARRAY<uint8_t>*)files[0].content;
      filecontent->open();
      
      WiFiClient client = server.client();     
//      server.sendContent_P(reinterpret_cast<const char*>(filecontent));
      client.write(*filecontent, 100);
//      server.send(200, files[i].mime, *&filecontent);
      return true;
    }
  }
  return false;
}
void handleNotFound() { //in the case that the request is actually not valid

  if(loadFromFlash(server.uri())){
    return;
  }
  
  String message = "File Not Found\n\n";
  message += "URI..........: ";
  message += server.uri();
  message += "\nMethod.....: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments..: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message += "\n";
  message += "FreeHeap.....: " + String(ESP.getFreeHeap()) + "\n";
  message += "ChipID.......: " + String(ESP.getChipId()) + "\n";
  message += "FlashChipId..: " + String(ESP.getFlashChipId()) + "\n";
  message += "FlashChipSize: " + String(ESP.getFlashChipSize()) + " bytes\n";
  message += "getCycleCount: " + String(ESP.getCycleCount()) + " Cycles\n";
  message += "Milliseconds.: " + String(millis()) + " Milliseconds\n";
  server.send(404, "text/plain", message);
}

