
#include "Leds.h"

Leds::Leds() {                                                        //constructor, setup pines
  strip.begin();
  //strip.setBrightness(50);
  //strip.show(); // Initialize all pixels to 'off'

  //_setHSB(random(760),255,255);

}

void Leds::setHue(int H) {
  if (H > 760) H = 760;
  _hue = hue = H;
  _setHSB(_hue,_sat,_bright); 

}
void Leds::setSat(int S) {
  if (S > 255) S = 255;
  _sat = saturation = S;

 _setHSB(_hue,_sat,_bright); 
} 
void Leds::setBri(int B) {
  if (B > 255) B = 255;
  _bright = brightnes = B;
  _setHSB(_hue,_sat,_bright); 
}  

void Leds::setRGB(int R, int G, int B) {                              // valores 0 a 999
  if (R > 255) R = 255;
  if (G > 255) G = 255;
  if (B > 255) B = 255;
  _red = red = R;
  _green = green = G ;
  _blue = blue = B;
  _colorNeo = strip.Color(_red,_green,_blue);
  strip.show();
}

void Leds::setMode(int m) {_modo = modo = m;}
void Leds::setFade(long f) {_fade = fade =  128-f;}  
void Leds::setPeriod(long p) {_period = period = 128-p;}

void Leds::Loop() {                                                    // modo 0 Off, ,1 ON, 2 PULSE, 3 PULSE SECUENCIAL

  if (_modo == 0) {_ledSelectAll(0);}
  if (_modo == 1) {_ledSelectAll(_colorNeo);}
 
  if (_modo == 2) {
    _wait(_period);                                           // genero un pulso __waitToggle
    if (_waitToggle) _ledSelectAll(_colorNeo);
    else _ledSelectAll(0);
  }
  if (_modo == 3) {
    _wait(_period);                                           // genero un pulso __waitToggle
    if (_waitPulse) _proximoLed();
    if (_waitToggle) _setBright(100); 
    else _setBright(30); 
  }

}

void  Leds::init()
{
  _colorWipe(strip.Color(255, 0, 0), 50); // Red
  //_rainbow(20);
  //strip.setBrightness(50);
  _ledSelectAll(strip.Color(0, 0, 255)) ;

  //_setHSB(random(760),255,255);

}


//FUNCIONES PRIVADAS
void Leds::_setHSB(uint16_t index, uint8_t sat, uint8_t bright) {      //Transforma de HSB a RGB y lo aplica con strip.show
	
  uint16_t r_temp, g_temp, b_temp;
	uint8_t index_mod;
	uint8_t inverse_sat = (sat ^ 255);
	
  hue = _hue = index;
  saturation = _sat = sat;
  brightnes = _bright = bright;
  
  index = index % 768;
	index_mod = index % 256;

	if (index < 256)
	{
		r_temp = index_mod ^ 255;
		g_temp = index_mod;
		b_temp = 0;
	}
	else if (index < 512)
	{
		r_temp = 0;
		g_temp = index_mod ^ 255;
		b_temp = index_mod;
	}
	else if ( index < 768)
	{
		r_temp = index_mod;
		g_temp = 0;
		b_temp = index_mod ^ 255;
	}
	else
	{
		r_temp = 0;
		g_temp = 0;
		b_temp = 0;
	}
	r_temp = ((r_temp * sat) / 255) + inverse_sat;
	g_temp = ((g_temp * sat) / 255) + inverse_sat;
	b_temp = ((b_temp * sat) / 255) + inverse_sat;
	r_temp = (r_temp * bright) / 255;
	g_temp = (g_temp * bright) / 255;
	b_temp = (b_temp * bright) / 255;
  
  _red = red = (uint8_t)r_temp;
  _green = green = (uint8_t)g_temp;
  _blue = blue = (uint8_t)b_temp;

  _colorNeo = strip.Color(_red, _green, _blue);
  _ledSelectAll(_colorNeo); 
  strip.show();
}

void Leds::_setBright(int b) {                                         // multiplica x brillo (0-100) a RGB
  strip.setBrightness(b);
  strip.show(); 
}

void Leds::_proximoLed() {
  for (int i = 0; i < strip.numPixels(); i++) strip.setPixelColor(i, 0);                  // Apago todo los leds
  strip.show();
  if (_ledN < strip.numPixels() ) _ledN++;
  else _ledN = 0;
  strip.setPixelColor(_ledN, _colorNeo); //enciendo el siguiente led
  strip.show();
}

void Leds::_ledSelectAll(uint32_t col) 
{                                  
  for (uint16_t  i = 0; i < strip.numPixels(); i++)  
  {
  strip.setPixelColor(i, col);                                                         // mayor a 7,para que salte en proximo led
  strip.show();
  }
}

void Leds::_dimmerLeds (int b) {                                       //dimeriza segun PWM RGB, _fade y bright 0-100, devuelve true cuando llega

  int brightAux = _bright;
  int incB = 0;
  unsigned long aux;
  if (b != _bright){
    if (fade == 0) fade = 1;                                           // para no dividir pr cero nunca
    aux= _period * 10 / _fade;
    if (b > _bright) incB  = (1) * aux;
    if (b < _bright) incB  = (-1) * aux;
  
    
    if (millis() - _millsNowDim > aux) {          //fade hasta la mitad del pulso
      _millsNowDim = millis();
      brightAux = _bright + incB;
      if (brightAux > 100) brightAux = 100;
      if (brightAux < 0) brightAux = 0;
      _setBright(brightAux);
    }
  }
}

void Leds::_wait(unsigned long tiempo) {                               //onda cuadrada True False 0-999 seg, señal _waitToggle
  _waitPulse = false;
  
  if ((millis() - _millsNow) > (tiempo * 10)) {
    _millsNow = millis();
    _waitPulse = true;
    _waitToggle = !_waitToggle;

  }
}

void  Leds::_colorWipe(uint32_t c, uint8_t _wait) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(_wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Leds::_Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void Leds::_rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, _Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void Leds::_rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) 
  { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, _Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}