/*
  Leds.h - Library for ANA HEAD
  Created by Julian Petrina 2020.
  Maneja 3 colores mediante leds

*/
#ifndef Leds_h
#define Leds_h

#define LEDPIN 19
#define LEDQTY 36

#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
class Leds 
{
  public:      
    int red, green, blue;
    int hue, saturation, brightnes;
    int bright;
    int modo = 1;
    unsigned long fade = 1;
    unsigned long period = 1;
    
    Leds(void); //constructor
    void init();
    void setMode(int m);                                         // modo 0 Off, 1 ON, 2 PULSE, 3 PULSE SECUENCIAL
    void setFade(long f);                                        // Asigna el fade 1-1000
    void setPeriod(long p);                                      // Asigna periodo 1-1000
    void setRGB(int R, int G, int B);                            // Asigna las varianles red, green, blue internas
    void setHue(int H);   
    void setSat(int S); 
    void setBri(int B);  
    void Loop();
  

  private:
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDQTY, LEDPIN, NEO_GRB + NEO_KHZ800);
    
    uint32_t _colorNeo;
    int _color;
    int _ledN;
    int _red, _green, _blue;
    int _hue, _sat, _bright;
    int _modo, _fade, _period = 1;
    
    unsigned long _millsNow = 0;
    unsigned long _millsNowDim = 0;
    
    bool _waitToggle;             //onda cuadrada
    bool _waitPulse;              //delta de dirac
    
    void _proximoLed(); 
    void _setBright(int bright);                                  // ajusta RGB con el brillo
    void _setHSB(uint16_t index, uint8_t sat, uint8_t bright); 
    void _colorWipe(uint32_t c, uint8_t wait) ; 
    uint32_t _Wheel(byte WheelPos); 
    void _rainbow(uint8_t wait);
    void _rainbowCycle(uint8_t wait);

    void _ledSelectAll(uint32_t c);              // Activa todos los leds con el color c empaquetado RGB
    
    void _dimmerLeds (int brillo);               //dimeriza segun PWM RGB, _fade y bright 0-100, devuelve true cuando llega
    void _wait(unsigned long tiempo);            //onda cuadrada True False 0-4 seg , señal waitToggle

};

#endif