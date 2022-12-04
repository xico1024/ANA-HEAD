/*
  Music.h - Library for ANA HEAD
  Created by Julian Petrina 2020.
  Deteccion de musica midi
*/
#ifndef Music_h
#define Music_h

#include "Arduino.h"



class Music 
{
  public:      
    Music(void); //constructor
    
    void escribirNoteArray(byte note, bool state, byte vel);
    void detectarEspectro();                            // Cantidad de notas y la vel  en cada tercio del teclado
    int detectarBajos();                                // devuelve la nota mas grave del primer tercio del teclado
    int detectarAgudos();                               // devuelve la nota mas aguda del tercer tercio del teclado
    void decrementarVelArray(int val);                         //decrementa las vel del array
    String procesarNoteArray();
    void escribirPedal(byte pedal);

    
    int gravesNotas, gravesPotencia;
    int mediosNotas, mediosPotencia;
    int agudosNotas, agudosPotencia;
    int potenciaTotal;
    int notaGrave, notaAguda;
    boolean nuevaNota;
    int8_t isConnected = 0;
    byte pedal;


  private:
 
    byte noteIn;
    long noteCont;
    bool noteArray[89];  //la posicion 0 no la uso, por eso hay 89
    int velArray[89];
    String cmd; //usada en el detector de bajos
                        
};

#endif