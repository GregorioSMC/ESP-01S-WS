#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <DHT.h>
#include <Hash.h>
#include <FS.h>

#define D3 0
#define D4 2
#define DHTTYPE DHT11
#define WIFI_SSID "INFINITUM2000"
#define WIFI_PASSWORD "Cri24252000"

int16_t ultimoValor = 0; 
static unsigned long last;
float valTemperatura;
float valTemperaturaPrev = 0;
float temp, hum;
String myDateJS;
DHT dht (D3, DHTTYPE);
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);


void setup(){

  Serial.begin(9600);
  dht.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando: ");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println(WiFi.localIP());
  delay(2000);

  SPIFFS.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404 Archivo no encontrado");
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado");
  
}


void loop (){

  webSocket.loop();
  server.handleClient();

  if (abs(millis()-last)>100)
  {
    valTemperatura = temperatura();
    last = millis();
  }

  if (valTemperatura != valTemperaturaPrev)
  {
    myDateJS = String(valTemperatura);
    webSocket.broadcastTXT(myDateJS);
    valTemperaturaPrev = valTemperatura;
  }
  
}


float temperatura(){
  temp = dht.readTemperature();
  Serial.print("Temp: ");
  Serial.println(temp);
  return temp;
}

float humedad(){
  hum = dht.readHumidity();
  Serial.print("Humedad: ");
  Serial.println(hum);
  return hum;
}



void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch(type) {
    case WStype_DISCONNECTED: {
      Serial.printf("Usuario #%u - Desconectado\n", num);
      break;
    }
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("Nueva conexión: %d.%d.%d.%d Nombre: %s ID: %u\n", ip[0], ip[1], ip[2], ip[3], payload, num);
      String msj = String(ultimoValor);
      webSocket.broadcastTXT(msj);
      break;
    }
    case WStype_TEXT: {
      
      break;
    }    
  }
}


bool handleFileRead(String path){
  #ifdef DEBUG
    Serial.println("handleFileRead: " + path);
  #endif
  if(path.endsWith("/")) path += "index.html";
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }
  return false;
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
