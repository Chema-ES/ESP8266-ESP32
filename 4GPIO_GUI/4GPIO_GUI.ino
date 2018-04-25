/*      4GPIO_GUI
 *       
 *       -GUI AMIGABLE
 *        Permite controlar 2 GPIOs de manera interactiva
 *        Sicronismo local vía servidor ntp
 *        Proporciona temp y humedad del dispositivo (sensor DH11)
 *        
 *        Otras funccionalidades menores:
 *          -Configuración del softAP y conexión al AP externo indicado
 *          -Escanea Wifis disponibles con nivel RSSI (solo salida serie).
 *          -Proporciona las IPs y MACs de ambas interfaces (AP externo y softAP) 
 *       
 *       
 *       V1.0
 *       
 *       Chema
 *       ELCA (Dpto. Electrónica)
 *       IES Francisco Asorey, Cambados (Galicia)
 */

// Para Node32-ESP32
#include <WiFi.h>
#define DEVICE "ESP32(WROOM32)"

// Para ESP8266 - NodeMCU
// (invertir lógica)
//#include <ESP8266WiFi.h>
//#define DEVICE "ESP8266"


#define FIRMWARE  "4GPIO_GUI_V1.0"

#include "time.h"
#include <DHT.h>

// GPIOs 
 

const int gpio[]={2,5,18,19,22,23};
int valorGPIO[]={LOW,LOW,LOW,LOW,LOW,LOW};
const int dhtPin = gpio[1];
const int ledPin=gpio[0];

// Datos conf. AP 

char* ssidAP="IoT-FD01";
char* passwAP="fogardixital";

// Datos AP externo

const char* ssid = "Wifi2.4G";
const char* password = "petrapetra";
const char* APsoft="IoT-FD01";

// Direcciones softAP y AP externo
  
  IPAddress apIP = IPAddress(192, 168, 10, 1);
  IPAddress apGW = IPAddress(192, 168, 10, 1);
  IPAddress apNM = IPAddress(255, 255, 255, 0);
  String apMAC;
  IPAddress miIP;
  IPAddress miSN;
  IPAddress miGW;
  String  miMAC2;
  String miSSID;

// Inicialización server y cliente

WiFiServer server(80); //Se puede configurar el puerto que se desee
WiFiClient cliente;
 
//Entorno de fecha y hora

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

struct tm timeinfo;

void printLocalTime()
{
  Serial.println("Servidor ntp");
  Serial.println("------------");
  if(!getLocalTime(&timeinfo)){
    
    Serial.println("Error ntp");
    return;
 }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Entorno  del sensor de temp-humedad

#define DHTTYPE DHT11   // DHT 11
// Inicializado del sensor

DHT dht(dhtPin, DHTTYPE);

// Variables temporales (temp en Celsius)

char temp[7];
char humedad[7];

void leer_dht(){
  // humedad en %
      float h = dht.readHumidity();
      // temperatura en C
      float t = dht.readTemperature();
      // comprobar ACK sensor
      if (isnan(h) || isnan(t)) {
          strcpy(temp,"err");
          strcpy(humedad, "err");  
          valorGPIO[1]=LOW;       
       }
        else{
          // calcula temp y humedad y formatea salida sobre las variables
          float hic = dht.computeHeatIndex(t, h, false);       
          dtostrf(hic, 6, 1, temp);                   
          dtostrf(h, 6, 1, humedad);
          valorGPIO[1]=HIGH;
      }
}

void print_dht(void){
  Serial.println('\n');
  Serial.println("Higrométría");
  Serial.println("---------------------------------");
  Serial.print("Temp (C):");
  Serial.println(temp);
  Serial.print("Humedad (%):");
  Serial.println(humedad);
  Serial.println('\n');
}

// Gestor HTTP Y GUI

void http_ok(void){
   cliente.println("HTTP/1.1 200 OK");
   cliente.println("Content-Type: text/html");
   cliente.println(""); 
}

void html_start(void){   
 
        cliente.println("<!DOCTYPE HTML>");
        cliente.println("<HTML>");
        cliente.println("<HEAD>");
        cliente.println("<meta http-equiv='Content-type' content='text/html;charset=UTF-8'>");
        cliente.print("<TITLE>");
        cliente.print(APsoft);
        cliente.println("</TITLE>");
        cliente.println("</HEAD>");
        cliente.println("<BODY>");
}

void html_end(void){
        
        cliente.println("</BODY>");
        cliente.println("</HTML>");
}

// ************ GUI general ESP32  **************

void gui_fecha(void)
{
  cliente.println("<!-- #######  GUI ESP8266/ESP32 #########-->");
  cliente.println("<table style='width: 342px; height: 41px;' border='5'>");
  cliente.println("<tbody>");
  cliente.println("<tr>");
  cliente.println("<td style='width: 112px;'>");
  cliente.println("<p style='text-align: center;'><strong>E L C A</strong></p>");
  cliente.println("</td>");
  cliente.print("<td style='width: 109px;'>");
  cliente.print(&timeinfo,"%d %B %Y");
  cliente.println("</td>");
  cliente.print("<td style='width: 111px;'>");
  cliente.print(&timeinfo,"%H:%M:%S");
  cliente.println("</td>");
  cliente.println("</tr>");
  cliente.println("</tbody>");
  cliente.println("</table>");
}

void gui_dispo(void){
  cliente.println("<h2 style='color: #2e6c80; text-align: left;'>*** Iot-FD01 ***</h2>");
  cliente.println("<table style='width:300px;'>");
  cliente.println("<tbody>");
  cliente.println("<tr>");
  cliente.print("<td style='width: 50px;'><b>apIP :</b></td>");
  cliente.println("</td>");
  cliente.print("<td style='width: 200px;'><span style='color: #0000ff; background-color: #ffff00;'>");
  cliente.print(apIP);
  cliente.print(" (");
  cliente.println(apMAC);
  cliente.println(")</span></td>");
  cliente.println("</tr>");
  cliente.println("</tbody>");
  cliente.println("</table>");
}

void gui_temp(void){
  cliente.println("<table style='height: 65px; width: 321px;'>");
  cliente.println("<tbody>");
  cliente.println("<tr>");
  cliente.println("<td style='width: 60px;'><em><b>Temp (<sup>o</sup>C):</b></em></td>");
  cliente.println("<td style='width: 40px;'>");
  cliente.print(temp);
  cliente.println("</td>");
  cliente.println("<td style='width: 70px;'><em><b>Humedad (%):</b></em></td>");
  cliente.println("<td style='width: 40px;'>");
  cliente.print(humedad);
  cliente.println("</td>");
  cliente.println("</tr>");
  cliente.println("</tbody>");
  cliente.println("</table>");
}

void gui_net(void){
  cliente.println("<br>");
  cliente.println("<table style='height: 65px; width: 221px;'>");
  cliente.println("<tbody>");
  cliente.println("<tr>");
  cliente.println("<td style='width: 58px;'><b>SSID:</b></td>");
  cliente.print("<td style='width: 149px;'>");
  cliente.print(miSSID);
  cliente.println("</td>");
  cliente.println("</tr>");
  cliente.println("<tr>");
  cliente.println("<td style='width: 58px;'><b>devIP:</b></td>");
  cliente.print("<td style='width: 149px;'>");
  cliente.print(miIP);
  cliente.print(" (");
  cliente.print(miMAC2);
  cliente.print(")");
  cliente.println("</td>");
  cliente.println("</tr>");
  cliente.println("<tr>");
  cliente.println("<td style='width: 58px;'><b>Máscara:</b></td>");
  cliente.print("<td style='width: 149px;'>");
  cliente.print(miSN);
  cliente.println("</td>");
  cliente.println("</tr>");
  cliente.println("<tr>");
  cliente.println("<td style='width: 58px;'><b>Pasarela:</b></td>");
  cliente.print("<td style='width: 149px;'>");
  cliente.print(miGW);
  cliente.println("</td>");
  cliente.println("</tr>");
  cliente.println("</tbody>");
  cliente.println("</table>");
}
  
void gui_GPIO(void){

  cliente.println("<table style='width: 356px;'>");
  cliente.println("<thead>");
  cliente.println("<tr style='height: 16px;'><span style='color: #2e6c80;'>");
  cliente.println("<td style='width: 74px; text-align: center; height: 16px;'><span style='color: #ffffff; background-color: #008080;'><b>GPIO</b></span></td>");
  cliente.println("<td style='width: 72px; text-align: center; height: 16px;'><span style='color: #ffffff; background-color: #008080;'><b>Estado</b></span></td>");
  cliente.println("<td style='width: 87px; text-align: center; height: 16px;'><span style='color: #ffffff; background-color: #008080;'><b>OFF</b></span></td>");
  cliente.println("<td style='width: 89px; text-align: center; height: 16px;'><span style='color: #ffffff; background-color: #008080;'><b>ON</b></span></td>");
  cliente.println("</tr>");
  cliente.println("</thead>");
  cliente.println("<tbody>");
  cliente.println("<tr style='height: 18px;'>");
  cliente.println("<td style='width: 74px; height: 18px; text-align: center;'><span style='color: #008000;'>D2(led)</span></td>");
  cliente.println("<td style='width: 72px; height: 18px; text-align: center;'><span style='color: #ffffff; background-color: #ff0000;'>[");
  cliente.print (valorGPIO[0]);
  cliente.println("]</span></td>");
  cliente.println("<td style='width: 87px; height: 18px; text-align: center;'><strong>x</strong></td>");
  cliente.println("<td style='width: 89px; height: 18px; text-align: center;'><strong>x</strong></td>");
  cliente.println("</tr>");
  cliente.println("<tr style='height: 18px;'>");
  cliente.println("<td style='width: 74px; height: 18px; text-align: center;'><span style='color: #008000;'>D5(dht)</span></td>");
  cliente.println("<td style='width: 72px; height: 18px; text-align: center;'><span style='color: #ffffff; background-color: #ff0000;'>[");
  cliente.print (valorGPIO[1]);
  cliente.println("]</span></td>");
  cliente.println("<td style='width: 87px; height: 18px; text-align: center;'><strong>x</strong></td>");
  cliente.println("<td style='width: 89px; height: 18px; text-align: center;'><strong>x</strong></td>");
  cliente.println("</tr>");
  cliente.println("<tr style='height: 18px;'>");
  cliente.println("<td style='width: 74px; height: 18px; text-align: center;'><span style='color: #008000;'>D18</span></td>");
  cliente.println("<td style='width: 72px; height: 18px; text-align: center;'><span style='color: #ffffff; background-color: #ff0000;'>[");
  cliente.print(valorGPIO[2]);
  cliente.println("]</span></td>");
  cliente.print("<td style='width: 87px; height: 18px; text-align: center;'>");
  cliente.print("<a href='GPIO18=OFF'><button> Off </button></a>");
  cliente.println("</td>");
  cliente.print("<td style='width: 87px; height: 18px; text-align: center;'>");
  cliente.print("<a href='GPIO18=ON'><button> On </button></a>");
  cliente.println("</td>");
  cliente.println("<tr style='height: 18px;'>");
  cliente.println("<td style='width: 74px; height: 18px; text-align: center;'><span style='color: #008000;'>D19</span></td>");
  cliente.println("<td style='width: 72px; height: 18px; text-align: center;'><span style='color: #ffffff; background-color: #ff0000;'>[");
  cliente.print(valorGPIO[3]);
  cliente.println("]</span></td>");
  cliente.print("<td style='width: 87px; height: 18px; text-align: center;'>");
  cliente.print("<a href='GPIO19=OFF'><button> Off </button></a>");
  cliente.println("</td>");
  cliente.print("<td style='width: 87px; height: 18px; text-align: center;'>");
  cliente.print("<a href='GPIO19=ON'><button> On </button></a>");
  cliente.println(" </span></td>");
  cliente.println("</tr>");
    cliente.println("<tr style='height: 18px;'>");
  cliente.println("<td style='width: 74px; height: 18px; text-align: center;'><span style='color: #008000;'>D22</span></td>");
  cliente.println("<td style='width: 72px; height: 18px; text-align: center;'><span style='color: #ffffff; background-color: #ff0000;'>[");
  cliente.print(valorGPIO[4]);
  cliente.println("]</span></td>");
  cliente.print("<td style='width: 87px; height: 18px; text-align: center;'>");
  cliente.print("<a href='GPIO22=OFF'><button> Off </button></a>");
  cliente.println("</td>");
  cliente.print("<td style='width: 87px; height: 18px; text-align: center;'>");
  cliente.print("<a href='GPIO22=ON'><button> On </button></a>");
  cliente.println("</td>");
  cliente.println("<tr style='height: 18px;'>");
  cliente.println("<td style='width: 74px; height: 18px; text-align: center;'><span style='color: #008000;'>D23</span></td>");
  cliente.println("<td style='width: 72px; height: 18px; text-align: center;'><span style='color: #ffffff; background-color: #ff0000;'>[");
  cliente.print(valorGPIO[5]);
  cliente.println("]</span></td>");
  cliente.print("<td style='width: 87px; height: 18px; text-align: center;'>");
  cliente.print("<a href='GPIO23=OFF'><button> Off </button></a>");
  cliente.println("</td>");
  cliente.print("<td style='width: 87px; height: 18px; text-align: center;'>");
  cliente.print("<a href='GPIO23=ON'><button> On </button></a>");
  cliente.println(" </span></td>");
  cliente.println("</tr>");
  cliente.println("</tbody>");
  cliente.println("</table>");
}

void gui_pie(void){
  cliente.println("<br>");
  cliente.println("<table style='height: 10px; width: 300px;' border='2'>");
  cliente.println("<tbody>");
  cliente.println("<tr>");
  cliente.println("<td style='width: 203px; text-align: center;'>DevTyp:</td>");
  cliente.println("<td style='width: 302px; text-align: center;'>");
  cliente.print(DEVICE);
  cliente.println("</td>");
  cliente.println("</tr>");
  cliente.println("<tr>");
  cliente.println("<td style='width: 126px; text-align: center;'>Firmw. :</td>");
  cliente.println("<td style='width: 245.px; text-align: center;'>");
  cliente.print(FIRMWARE);
  cliente.println("</td>");
  cliente.println("</tr>");
  cliente.println("</tbody>");
  cliente.println("</table>");
}

void github(void){
  cliente.println("<table width='369' height='30'>");
  cliente.println("<tbody>");
  cliente.println("<tr>");
  cliente.println("<td style='width: 50.px;'></td>");
  cliente.print("<td style='width: 200px;'>");
  cliente.println("<p><a title='ESP8266-ESP32 @ Github' href='http://github.com/Chema-ES/ESP8266-ESP32' target='_blank' rel='noopener'>★ ESP32/8266 @ GitHub ★</a></p>");
  cliente.println("</td>");
  cliente.println("<td style='width: 100px;'></td>");
  cliente.println("</tr>");
  cliente.println("</tbody>");
  cliente.println("</table>");
}
  
// FINAL GUI
 

void setup() {
  
  Serial.begin(115200);

  for(int i=2;i<=5;i++){
    pinMode(gpio[i],OUTPUT);
  }
  pinMode(dhtPin,INPUT);
  pinMode(ledPin, OUTPUT);
  
  // Configura soft AP
  WiFi.softAP(ssidAP,passwAP); 
  WiFi.softAPConfig(apIP, apGW, apNM);
  apIP = WiFi.softAPIP();
  apMAC=WiFi.softAPmacAddress();
  delay(100);
  // Activa radio 
  WiFi.begin();
  digitalWrite(ledPin, LOW);
  // Scan SSIDs
  Serial.println("...escaneo de redes\n");
  int numeroSSIDs=WiFi.scanNetworks();

  Serial.println("SSID\t\t(RSSI)");
  Serial.println("-----------------------------------");
  
  for(int i=0;i<numeroSSIDs;i++){
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print("\t(");
    Serial.print(WiFi.RSSI(i));
    Serial.println(")");
  }
  
  Serial.println("\n\n");

  Serial.print("Conectando a ");
  Serial.println(ssid);
 
  
  // Se asacia a AP externo
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  valorGPIO[0]=HIGH;
  digitalWrite(ledPin,HIGH);
  Serial.println("");
  Serial.println("WiFi conectada");
 
 // Check entorno

  apIP = WiFi.softAPIP();
  apMAC =WiFi.softAPmacAddress();
  miIP = WiFi.localIP();
  miSN = WiFi.subnetMask();
  miGW = WiFi.gatewayIP();
  miMAC2 = WiFi.macAddress(); 
  miSSID = WiFi.SSID();

  Serial.print("Wifi @ softAP: ");
  Serial.print(apIP);
  Serial.print(" /(");
  Serial.print(apMAC);
  Serial.println(")");
  Serial.println("\n----- cliente local ----");
  Serial.print(miIP);
  Serial.print(" /(");
  Serial.print(miMAC2);
  Serial.println(")");
  Serial.println(miSN);
  Serial.println(miGW);
  Serial.println("\n");
  
  server.begin();
  
  Serial.print("Server ");
  Serial.print(APsoft);
  Serial.println(" operativo!");
  Serial.print("Usa: \n");
  Serial.print("1.http://");
  Serial.print(WiFi.localIP());
  Serial.print("/ desde host asociado a ");
  Serial.println(ssid);
  Serial.print("2.http://");
  Serial.print(WiFi.softAPIP());
  Serial.print("/ desde host asociado a ");
  Serial.println(APsoft);

  leer_dht();
  // salida ntp
  printLocalTime();
  // salida higrometría
  print_dht();
}
 
void loop() {
  
  // Gestión del server web 
  
  cliente = server.available();
  
  if (cliente) {
 
    leer_dht();
    // salida ntp
    printLocalTime();
    // salida higrometría
    print_dht();
    
    Serial.println("Nuevo cliente");

    while (cliente.connected()) {
      if (cliente.available()) { 
        String request = cliente.readStringUntil('\r');
        Serial.println(request);

        // Parser de request
        if (request.indexOf("GPIO18=ON") != -1){
        digitalWrite(gpio[2], HIGH);
        valorGPIO[2] = HIGH;
        }
        if (request.indexOf("GPIO18=OFF") != -1){
        digitalWrite(gpio[2], LOW);
        valorGPIO[2] = LOW;
        }
    
        if (request.indexOf("GPIO19=ON") != -1){
        digitalWrite(gpio[3], HIGH);
        valorGPIO[3] = HIGH;
        }
        if (request.indexOf("GPIO19=OFF") != -1){
        digitalWrite(gpio[3],LOW);
        valorGPIO[3] = LOW;
        }
        
        if (request.indexOf("GPIO22=ON") != -1){
        digitalWrite(gpio[4], HIGH);
        valorGPIO[4] = HIGH;
        }
        if (request.indexOf("GPIO22=OFF") != -1){
        digitalWrite(gpio[4],LOW);
        valorGPIO[4] = LOW;
        }
        
        if (request.indexOf("GPIO23=ON") != -1){
        digitalWrite(gpio[5], HIGH);
        valorGPIO[5] = HIGH;
        }
        if (request.indexOf("GPIO23=OFF") != -1){
        digitalWrite(gpio[5],LOW);
        valorGPIO[5] = LOW;
        }
        for(int i=2;i<=5;i++){
          digitalWrite(gpio[i],valorGPIO[i]);
        }
        Serial.println("...iniciando GUI");
        //Confirmación conexión cliente
        http_ok();
        // Inicio html
        html_start();
        // Cabeza GUI con fecha y hora
        gui_fecha();
        // Identificación dispo
        gui_dispo();
        //Temp y humedad
        gui_temp();
        // GUI GPIOs
        gui_GPIO();
        // INFO softAP
        gui_net();
        // Info de firmware
        gui_pie();
        // Github ESP
        github();
        //Final HTML
        html_end();
        Serial.println("GUI Ok!");
      // Cerrando conexión
      delay(100);
      cliente.stop();
     Serial.println("Cliente desconectado de ");
     Serial.println(APsoft);
    Serial.println("");
   }
  }
 }
}



