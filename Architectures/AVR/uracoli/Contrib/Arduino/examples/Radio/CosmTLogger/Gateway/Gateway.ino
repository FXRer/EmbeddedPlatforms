/*
  Cosm temperature logger
 
  This sketch is derived from PachubeClientString Example bundled with
  Arduino. It receives a character line from temperature sensor and feeds
  it to cosm.
  
  Hardware:
   - Radiofaro + DFRobot Ethernet Shield attached
  
 */

#include <SPI.h>
#include <Ethernet.h>
#include "HardwareRadio.h"


#define APIKEY         "The quick brown fox" // replace your Pachube api key here
#define FEEDID         123456 // replace your feed ID
#define USERAGENT      "Radiofaro" // user agent is the project name


// assign a MAC address for the ethernet controller.
// fill in your address here:
  byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// fill in an available IP address on your network here,
// for manual configuration:
IPAddress ip(10,0,1,20);

// initialize the library instance:
EthernetClient client;

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(216,52,233,121);      // numeric IP for api.pachube.com
//char server[] = "api.pachube.com";   // name address for pachube API

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 20*1000;  //delay between updates to pachube.com

char ln[80]; /* max line length */
byte idx = 0;
byte lncnt = 0;
boolean hasData=0;

String dataString = "";

String addr_whitelist[] = {"0xFECA", "0xAFFE"};

void setup() {
  Radio.begin();
  Serial.begin(9600);

  // give the ethernet module time to boot up:
  delay(1000);
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // DHCP failed, so use a fixed IP address:
    Ethernet.begin(mac, ip);
  }
}

boolean parse(char *ln)
{
    byte i=0;
    char *sarr[8];
    char *s = ln;
    boolean valid=0;
    
    Serial.print("Line In: ");
    Serial.print(ln); // line break already in line
    
    /* tokenize to all space separated */
    do{
      if((*s==',') || (*s == '=')) *s=' ';
    }while(*s++);

    s=sarr[i++]=strtok(ln, " ");
    dataString="";
    do{
      s=sarr[i++]=strtok(NULL, " ");
    }while( (NULL != s) && (i<sizeof(sarr)/sizeof(sarr[0])) );
 
    if(sarr[2][0]=='T' && sarr[4][0]=='V'){
        
        s=sarr[3];
        while(*s && (*s>='0' && *s<='9' || *s=='.' || *s=='-') ){
          s++;
        }; 
      
        if(*s){ /* did not reach NULL terminator */
          /* N.a.N. */ 
        } else {
          dataString += "temp,";
          dataString += sarr[3];      
          dataString += "\nvolt,";
          dataString += sarr[5];
          valid = 1;
        }
    }
    
    if(!valid){
       Serial.println("Parsing error");
    } else {
      Serial.print("Parsed: ");
      Serial.println(dataString);
    }
    return valid;
}

void loop() {

  if (Radio.available() > 0)
  {
    char c = Radio.read();

    if(c == '\0'){ /* unexpected input, restart */
      idx = 0;
    } else {
      ln[idx] = c;
      idx++;
    }
    ln[idx] = '\0';
    
    if( ('\n' == c) || ('\r' == c) ) { /* line completed */
      hasData = parse(ln);
      
      lncnt++;
      idx = 0;
    }
  }

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data: 
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval) && hasData>0) {
    Serial.print("Pachube: ");
    Serial.println(dataString);

    sendData(dataString);
    hasData=0;
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void sendData(String thisData) {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.print("PUT /v2/feeds/");
    client.print(FEEDID);
    client.println(".csv HTTP/1.1");
    client.println("Host: api.pachube.com");
    client.print("X-pachubeApiKey: ");
    client.println(APIKEY);
    client.print("User-Agent: ");
    client.println(USERAGENT);
    client.print("Content-Length: ");
    client.println(thisData.length());

    // last pieces of the HTTP PUT request:
    client.println("Content-Type: text/csv");
    client.println("Connection: close");
    client.println();

    // here's the actual content of the PUT request:
    client.println(thisData);
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  // note the time that the connection was made or attempted:
  lastConnectionTime = millis();
}

