/*
 WiFiEsp example: WebServer

 A simple web server that shows the value of the analog input 
 pins via a web page using an ESP8266 module.
 This sketch will print the IP address of your ESP8266 module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to display the web page.
 The web page will be automatically refreshed each 20 seconds.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

#include "WiFiEsp.h"
#include "Pixy2.h"

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

char ssid[] = "SK_WiFiGIGAAFD7";            // your network SSID (name)
char pass[] = "1707023859";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int reqCount = 0;                // number of requests received
//char client_server[] = "arduino.cc";
char client_server[] = "115.85.181.24";
String content = "{ \"status\": \"this is electic car\"}";
int lampOn = 0;
Pixy2 pixy;

WiFiEspServer server(80);


void setup()
{
  pixy.init();
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  //printWifiStatus();
  
  // start the web server on port 80
  server.begin();
}


void loop()
{
//  http_server();
  pixy_loop();
//  http_client();
}

void pixy_loop(){
  static int c;
  int i;
  // lamp on & off
  if (lampOn == 1)
    lampOn_loop();
  else
    lampOff_loop();

  // grab blocks!
  pixy.ccc.getBlocks();
  // If there are detect blocks, print them!
  if (pixy.ccc.numBlocks)
  {
    int object = pixy.ccc.blocks[i].m_signature;
    for (i=0; i<pixy.ccc.numBlocks; i++) {
      //Serial.print(object); 
      //Serial.print("\n");
      //electric car
      if (object == 13523 || object == 8842 || object == 650 || object == 211) {
        c++;
        Serial.println(c);
        if (c == 30) {
          Serial.print(" electric car \n");
          pixy.setLED(0,0,255);
          c = 0 ;
          http_client();
        }
      }
    }
  }
}

void lampOn_loop() 
{
  pixy.setLamp(1, 1);
  pixy.setCameraBrightness(23);
}

void lampOff_loop()
{
  pixy.setLamp(0, 0);
  pixy.setCameraBrightness(50);
}

void http_client()
{
  WiFiEspClient client;
  Serial.println("[client] Starting connection to server...");
  // if you get a connection, report back via serial
  if (client.connect(client_server,8084)) {
    Serial.println("[client] Connected to server");
    // Make a HTTP request
    client.println("POST /alarm/1 HTTP/1.1");
    client.println("Host: arduino.cc");
    client.println("Accept: */*");
    client.println("Content-Length: " + String(content.length()));
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(content);
  } else{
    Serial.println("fail");
  }
//  while(true){
//    bool flag = false;
//    while (client.available()) {
//      char c = client.read();
//      Serial.write(c);
//      flag = true;
//    }
//    if(flag){
//      break; 
//    }
//  }

  // if the server's disconnected, stop the client
  if (!client.connected()) {
    Serial.println();
    Serial.println("[client] Disconnecting from server...");
    client.stop();
  }
}


void http_server()
{
    // listen for incoming clients
  WiFiEspClient client = server.available();
  if (client) {
    Serial.println("New client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          Serial.println("[server] Sending response");
          
          // send a standard http response header
          // use \r\n instead of many println statements to speedup data send
          client.print(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"  // the connection will be closed after completion of the response
            "Refresh: 20\r\n"        // refresh the page automatically every 20 sec
            "\r\n");
          client.print("<!DOCTYPE HTML>\r\n");
          client.print("<html>\r\n");
          client.print("<h1>Hello World!</h1>\r\n");
          client.print("Requests received: ");
          client.print(++reqCount);
          client.print("<br>\r\n");
          client.print("Analog input A0: ");
          client.print(analogRead(0));
          client.print("<br>\r\n");
          client.print("</html>\r\n");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);

    // close the connection:
    client.stop();
    Serial.println("[server] Client disconnected");
  }
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}
