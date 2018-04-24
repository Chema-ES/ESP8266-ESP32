/*      soft AP  
 *       V1.1
 *       
 *       Configura el AP del módulo
 *       Con separación funcional
 *       */

// Para Node32-ESP32
#include <WiFi.h>

// Para ESP8266 - NodeMCU
//#include <ESP8266WiFi.h>


int ledPin=2;

// Datos conf. AP 

char* ssid="IoT-FD01";
char* passw="fogardixital";

IPAddress ip = IPAddress(192, 168, 10, 1);
IPAddress pasarela = IPAddress(192, 168, 10, 1);
IPAddress mascara = IPAddress(255, 255, 255, 0);
IPAddress apIP;
String apMAC;

WiFiServer server(80); //Se puede configurar el puerto que se desee
WiFiClient cliente;

void html(){
               
        cliente.println("<!DOCTYPE HTML>");
        cliente.println("<HTML>");
        cliente.println("<HEAD>");
        cliente.print("<TITLE>");
        cliente.print(ssid);
        cliente.println("</TITLE>");
        cliente.println("</HEAD>");
        cliente.println("<BODY>");
        cliente.print("<H1>Servidor web ");
        cliente.print(ssid);
        cliente.println("</H1><br>");
        cliente.println("IP AP : ");
        cliente.println(apIP);
        cliente.println("<br>MAC AP : ");
        cliente.println(apMAC);
        cliente.println("</BODY>");
        cliente.println("</HTML>");
}

void httpOK(){
   cliente.println("HTTP/1.1 200 OK");
   cliente.println("Content-Type: text/html");
   cliente.println(""); 
}

//Main
void setup() {
  
  Serial.begin(115200);
  pinMode(ledPin,OUTPUT);

  // Configura soft AP

  WiFi.softAP(ssid,passw); 
  WiFi.softAPConfig(ip, pasarela, mascara);
  apIP = WiFi.softAPIP();
  apMAC=WiFi.softAPmacAddress();
  delay(100);
  // Activa radio 
  WiFi.begin();

 // Activa servidor   
  server.begin();
  digitalWrite(ledPin, HIGH);
  
 Serial.println("Servidor operativo");
 Serial.print("Usa ");
 Serial.print("http://");
 Serial.print(WiFi.softAPIP());
 Serial.print("/ desde host asociado a ");
 Serial.println(ssid);
}
 
void loop() {  
  cliente = server.available();
  if (cliente) {
    Serial.println("Nuevo cliente!");
    while (cliente.connected()) {
      if (cliente.available()){ 
        Serial.println("..iniciando html");
        //Respuesta conexión OK para cliente
        httpOK();
        //Envío doc html
        html();
        slider();
        //cerrando conexión
        Serial.println("cliente STOP");
        cliente.flush();
        cliente.stop();   
      }
    }
  }
}


void slider(){
  cliente.println(".vranger {");
  cliente.println("margin-top: 50px;");
  cliente.println("transform: rotate(270deg);");
  cliente.println("-moz-transform: rotate(270deg); /*do same for other browsers if required*/");
  cliente.println("}");
  cliente.println("<input type='range' class='vranger'/>");
}







