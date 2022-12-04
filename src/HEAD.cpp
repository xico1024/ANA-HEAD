/*
  IOT: broker shiftr.io local/remoto
*/
#include <Arduino.h>
#include "Stream.h"
#include "Leds.h"
#include <Wire.h>
#include <WiFi.h>
#include <MQTT.h>
#include <WiFiUdp.h>
#include <AppleMIDI.h>
#include <Music.h>

IPAddress ip(192, 168, 0, 100); 
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0); 
Leds leds;
const char ssid[] = "Woodstick"; // your network SSID (name)
const char pass[] = "nuno2048";  // your network password (use for WPA, or use as key for WEP)

// COLORES
int i;
int red, green, blue;
int Rant, Gant, Bant;
int hue, sat, brightnes;
int modo, Mant;
long Pant, Fant, periodo, fade;
int val1, val2, val3;
bool cambio;

unsigned long lastMillisIOT = 0;
unsigned long lastMillisPulse = 0;
int8_t isConnected = 0;
byte midiPedal = 0;
int x;                  // para test de trasnmision
//bool custom_msg = true; // Global para enviar cutom msjes

/* 
PIN 1,2,3 R-G-B
PIN 0,12,13,14,15,16 Base leds
PIN 6,7,8,9,10,11 no usar en ESP 2866
PIN 4,5 I2C
*/
WiFiClient net;         //usado para MQTT
MQTTClient clienteMQTT;
Music musicDetector;
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "ANA-MIDI", 5004); 
//=====================================

// SERIAL -----------------------------
void serialPortON(int baud)
{                         // Enciende el serial port a @baudios y cambia funion pines
  Serial.begin(baud);
  delay(100);
}
void serialPortOFF()
{ // Cierra puerto y cambia funion pines para usarlos en los leds
  delay(100);
  Serial.end();
}


// I2C -----------------------------
void desSerialize(String CommandStr) // deja los 3 valores en val1 val2 val3
{
  String strAux;
  int c1=0, c2=0;
  strAux = CommandStr.substring(1, c1);
  val1 = strAux.toInt();

  c2 = CommandStr.indexOf(',', c1 + 1);
  strAux = CommandStr.substring(c1 + 1, c2);
  val2 = strAux.toInt();

  c1 = c2;
  c2 = CommandStr.indexOf(',', c1 + 1);
  strAux = CommandStr.substring(c1 + 1, c2);
  val3 = strAux.toInt();
}
String I2CRead()                     // Hace un request de 32 Bytes y devuelve el resultado String               
{
  String data;
  data = "";
  Wire.requestFrom(8, 32);        // Primer paquete de info I2C
  while (Wire.available())
  {                       // MEGA puede enviar menos de lo solicitado
    char c = Wire.read(); // Recibo los caracteres
    data += c;
  }
  return data;
}
void I2RequestAndPublish()           // Recibo todo los mensajes encolados del MEGA
{
  String data;
  String payload;
  String topicIOT;
  String valIOT;
  int id;
  int msgCue;

  do
  {
    data = I2CRead();                                  // leo  mensaje del MEGA
    id = data.substring(0, data.indexOf('@')).toInt(); //@ delimitador mensaje I2C
    payload = data.substring(data.indexOf('@') + 1, data.lastIndexOf('@'));
    msgCue = data.substring(data.lastIndexOf('@') + 1, data.lastIndexOf('@') + 2).toInt();
    /*
    serialPortON(115200);
    Serial.println(payload + " " + msgCue);
    serialPortOFF();
    */

    if (id == 0) // Posicion motores
    {
      topicIOT = "Status/Pos/Arm"; // info de M1 y M2
      String valIOT = payload;
      clienteMQTT.publish(topicIOT, valIOT);
    }

    if (id == 1) // Distancia Sensor
    {  
      topicIOT = "Status/Pos/Dist"; // info de M1 y M2
      String valIOT = payload;
      clienteMQTT.publish(topicIOT, valIOT);
    }

    if (id == 2) // COLOR CABEZA
    {                     
      desSerialize(payload); // deja la data en val1, val2,val3
      red = val1;
      green = val2;
      blue = val3;
      if ((Rant != red) or (Bant != blue) or (Gant != green))
      { // si hubo un cambio
        Rant = red;
        Gant = green;
        Bant = blue;
        cambio = true;
        leds.setRGB(red, green, blue);
      }
    }
    
    if (id == 3) // MODO CABEZA
    {
      desSerialize(payload); // deja la data en val1, val2,val3
      modo = val1;
      periodo = val2;
      fade = val3;
      if (fade == 0)
        fade = 1;
      if (periodo == 0)
        periodo = 1;

      if ((Mant != modo) or (Pant != periodo) or (Fant != fade))
      {
        Pant = periodo;
        Mant = modo;
        Fant = fade;
        leds.setMode(modo);
        leds.setFade(fade);
        leds.setPeriod(periodo);
        cambio = true;
      }
    }

    if (id == 4) // Comando IOT para retransmitir
        {
          topicIOT = payload.substring(0, data.indexOf('#'));
          valIOT = payload.substring(payload.indexOf('#') + 1, payload.lastIndexOf('#'));
          clienteMQTT.publish(topicIOT, valIOT);
        }

  } while (msgCue > 0);
}
void I2CTransmit(String datos)
{ // Transmite  el string data por  I2C canal 8
  char buffer[32];
  datos.toCharArray(buffer, 32);
  Wire.beginTransmission(8); // transmite al device #8
  Wire.write(buffer);
  Wire.endTransmission();    // stop transmitting
  /*
  serialPortON(115200);
  Serial.print("I2Ccmd: ");
  Serial.println(datos);
  serialPortOFF();
  */
}

// IOT -----------------------------
void IOTpublish()
{ // PUBLICACION IOT

  clienteMQTT.publish("behaviour/Color/Hue", String(leds.hue));
  clienteMQTT.publish("behaviour/Color/Sat", String(leds.saturation));
  clienteMQTT.publish("behaviour/Color/Bright", String(leds.brightnes));

  clienteMQTT.publish("behaviour/Mode/Mode", String(leds.modo));
  clienteMQTT.publish("behaviour/Mode/Fade", String(leds.fade));
  clienteMQTT.publish("behaviour/Mode/Period", String(leds.period));

  clienteMQTT.publish("emotions/Hormones/Dopamine", "66");
  clienteMQTT.publish("emotions/Hormones/Endorfine", "20");
  clienteMQTT.publish("emotions/Hormones/Oxitocine", "40");
  clienteMQTT.publish("emotions/Hormones/Serotonine", "20");

  clienteMQTT.publish("stimulus/Music/connected", String(isConnected));
  clienteMQTT.publish("stimulus/Music/pedal", String(musicDetector.pedal));
  clienteMQTT.publish("stimulus/Music/bassPwr", String(musicDetector.gravesPotencia));
  clienteMQTT.publish("stimulus/Music/midPwr", String(musicDetector.mediosPotencia));
  clienteMQTT.publish("stimulus/Music/treblePwr", String(musicDetector.agudosPotencia));
  clienteMQTT.publish("stimulus/Music/lowerNotes", String(musicDetector.gravesNotas));
  clienteMQTT.publish("stimulus/Music/HighNotes", String(musicDetector.agudosNotas));

}
void IOTCustomPublish(String topic, String payload)
{ // Publicacion IOT de un string
  clienteMQTT.publish(topic, payload);
}
void IOTmessageReceived(String &topic, String &payload)
{ // Mensage recibido por MQTT
  String comando;
  serialPortON(115200);
  Serial.println("Comando IOT");
  Serial.println(topic + ": " + payload);
  serialPortOFF();

  if (String(topic) == "Command")
  { // envio un comando a la base
    comando = payload;  
    I2CTransmit(payload);
    
    
  }
  if (String(topic) == "behaviour/setMode/Mode")
  {
    modo = payload.toInt();
    leds.setMode(modo);
  }
  if (String(topic) == "behaviour/setMode/Period")
  {
    periodo = payload.toInt();
    leds.setPeriod(periodo);
  }
  if (String(topic) == "behaviour/setMode/Fade")
  {
    fade = payload.toInt();
    leds.setFade(fade);
  }
  if (String(topic) == "behaviour/setColor/Hue")
  {
    hue = payload.toInt();
    leds.setHue(hue);
  }
  if (String(topic) == "behaviour/setColor/Sat")
  {
    sat = payload.toInt();
    leds.setSat(sat);
  }
  if (String(topic) == "behaviour/setColor/Bright")
  {
    brightnes = payload.toInt();
    leds.setBri(brightnes);
  }
}
void IOT_loop()
{                     // Loop de IOT verifica que no se deconecte
  clienteMQTT.loop(); // Loop de IOT
  delay(10);  // <- fixes some issues with WiFi stability

  if (!clienteMQTT.connected()) {
    clienteMQTT.connect("ANA", "public", "public");
  }
 
}

// GENERALES -----------------------------
void procesamientoPeriodico(unsigned long periodo)
{ // Procesamiento cada n cantidad de tiempo
  if (millis() - lastMillisIOT > periodo)
  {
    lastMillisIOT = millis();
    // procesarNoteArray();                                 // proceso array de Midi
    I2RequestAndPublish();                                  // I2C con la base, solicita 1 paquete y lo procesa
    IOTpublish();                                           // Publicacion IOT
                            // I2CTransmit("Alive");                                // envio status al MEGA
    //musicDetector.procesarNoteArray();                     //decremento la fuerza de las notas 
  }
}
void connectAll()
{ // Conecta al wifi y a broker
  serialPortON(115200);
  Serial.println("A.N.A HEAD");
  Serial.print("Connections");
  
  WiFi.config(ip, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  { 
    Serial.print(".");
    delay(200);
  }
  Serial.println();
  Serial.print("A.N.A host info:");
  Serial.println(WiFi.localIP());
  Serial.print("Network: ");
  Serial.print(ssid);
  Serial.println();

  // Conexion al broker iot local y callback
  clienteMQTT.begin("192.168.0.26", net);
  clienteMQTT.onMessage(IOTmessageReceived);

  Serial.print("Connecting to broker: ");
  while (!clienteMQTT.connect("ANA", "public", "public"))
  {
    Serial.print(".");
    delay(200);
  }
  Serial.println("OK");

  //Conexion MIDI
  Serial.print("MIDI Port: ");
  Serial.println(AppleMIDI.getPort());
  Serial.print("MIDI Name: ");
  Serial.println(AppleMIDI.getName());
  MIDI.begin();

  serialPortOFF();

  clienteMQTT.subscribe("behaviour/setColor/#");
  clienteMQTT.subscribe("behaviour/setMode/#");
  clienteMQTT.subscribe("Command/#");
  clienteMQTT.subscribe("/hello");
}
void clearScreen()
{
  for (int a = 0; a < 100; a++) Serial.println();  

}


//========================================
void setup()
{
  delay(1000);
  Serial.begin(115200);
  clearScreen();
  Wire.begin(); // I2C ADDRESS EN MASTER OPCIONAL
  connectAll();  // conexion a Wifi/BrokerIOT/MIDI
  leds.init();
  
  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc, const char *name)
    {
    isConnected++;
   
    Serial.println (); 
    Serial.print ("MIDI session: ");
    Serial.print(ssrc);
    Serial.print (" @ ");
    Serial.println(name);
  
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc)
    {
    isConnected--;
  
    Serial.println("Disconnected");
    Serial.println(ssrc);
   
  });
  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity)
  {  
    musicDetector.escribirNoteArray(note, true, velocity);
    if (musicDetector.nuevaNota) I2CTransmit(musicDetector.procesarNoteArray());
    leds.setRGB(red, green, blue); 
  });
  MIDI.setHandleControlChange([](byte channel, byte number, byte value)
  {
   if (number == 64) musicDetector.escribirPedal(value);
   if (number == 77) leds.setHue(value*6); 
   if (number == 78) leds.setSat(value*2); 
   if (number == 79) leds.setBri(value*2);
   if (number == 80) 
   {
     if (value <= 5) {leds.setMode(0);}// Mant=0;}
     if (value > 5 and value <= 40 ) {leds.setMode(1); } //Mant=1;}
     if (value > 40 and value <= 80 ) {leds.setMode(2); } //Mant=2;}
     if (value > 80 and value < 127 ) {leds.setMode(3); } //Mant=3;}
    }
   if (number == 81) {
      int aux = value;
      if (aux == 0) aux = 1;
      leds.setPeriod(aux);  
    }
   if (number == 82) {
      int aux = value ;
      if (aux == 0) aux = 1;
      leds.setFade(aux);  
    } 
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity)
  {musicDetector.escribirNoteArray(note, false, velocity); 
  });
 
}

void loop()
{
  leds.Loop(); // Loop luces
  IOT_loop();  // Loop de IOT verifica que no se deconecte 
  MIDI.read();

  procesamientoPeriodico(1000); // pubicacion de mensaj0e I2c & IOT  por segundo
}
