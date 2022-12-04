/*
  Music.cpp - Library for ANA HEAD
  Deteccion de musica via MIDI
  Created by Julian Petrina 2022.
 
  */
#include "Music.h"


Music::Music() {                                                        //constructor

}

void Music::escribirNoteArray(byte note, bool state, byte vel)           // Escribe una nota MIDI en el NOTE ARRAY, tambien guarda la VEL (acorde)
{ 
  note = note - 20; // para que el primer La sea 1
  noteArray[note] = state;
  if (state == true) velArray[note] = vel;
  nuevaNota = true;
}

void Music::escribirPedal(byte midiPedal)
{
pedal = midiPedal;
}

int Music::detectarBajos()                  // Deteta la nota mas grave del primer tercio del teclado
{  
  int a = 1;
  while ((noteArray[a] == false) and (a<30)) a++; 
  if (a < 30)return a;
  else return -1;  
}

int Music::detectarAgudos()                 // Deteta la nota mas grave del primer tercio del teclado
{  
    int a = 88;
    while ((noteArray[a] == false) and (a>60)) a -- ; 
    if (a > 60) return a;
    else return -1;   
}

void Music::detectarEspectro()              // Cuenta las notas y la veolocidad en cada tercio del teclado
{ 
    gravesNotas = 0;
    mediosNotas = 0;
    agudosNotas = 0;
    gravesPotencia = 0;
    mediosPotencia = 0;
    agudosPotencia = 0;
    
    for (int a = 1; a < 44; a++)
    {
        if (velArray[a] > 0)
        {
        gravesNotas = gravesNotas + 1;
        gravesPotencia = gravesPotencia + velArray[a];
        }
    }
    for (int a = 30; a < 60; a++)
    {
        if (velArray[a] > 0)
        {
        mediosNotas = mediosNotas + 1;
        mediosPotencia = mediosPotencia + velArray[a];
        }
    }
    for (int a = 45; a <= 88; a++)
  {
    if (velArray[a] > 0)
    {
      agudosNotas = agudosNotas + 1;
      agudosPotencia = agudosPotencia + velArray[a];
    }
  }

potenciaTotal = agudosPotencia + mediosPotencia + gravesPotencia;
}

String Music::procesarNoteArray()           // Actualiza variables del objeto,  potencia, cant de notas, etc
{  
    decrementarVelArray(1);
    detectarEspectro();
    notaGrave = detectarBajos(); 
    notaAguda = detectarAgudos();
  
    String graves = String(gravesPotencia); 
    String agudos = String(agudosPotencia); 
    String medios = String(mediosPotencia);
    String melodia = String(agudosNotas);  
    String bajo = String(gravesNotas);   
   
      
    cmd = "b";
    String mensaje = cmd + graves + "," + medios + "," + agudos + "," + bajo + "," + melodia+ "," + String(pedal); 
    nuevaNota = false;
    return mensaje;  
    
}

void Music::decrementarVelArray(int val)    // decrementa "val" las veocidades, depende del pedal
{
  for (int a = 1; a < 89; a++)
  {
    if (noteArray[a] == false) 
    {
        if  (pedal == 0) velArray[a] = 0;
        if  ((pedal == 127) and  (velArray[a] > 0)) velArray[a] = velArray[a] - val ;        
    }
    if ((noteArray[a] == true) and (velArray[a] > 0)) velArray[a] = velArray[a] - val ;
    
    if (velArray[a] < 0) velArray[a] = 0; 
 }     
}

