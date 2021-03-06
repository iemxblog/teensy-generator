#include "Arduino.h"
#include "state_machine.h"
#include "screen.h"
#include "config.h"
#include "generator.h"

extern Generator generator;

// This function is a state machine
// It responds to user input, and modifies the global variables above, in order to communicate with the
// generator code in the main module.

void state_machine(Event evt)
{
  enum State {MenuManual_S, MenuFreqSweep_S, MenuAmplSweep_S, MenuYesNo_S, MenuPulse_S, Manual_S, FreqSweep_S, AmplSweep_S, YesNo_S, YesNoWait_S, Yes_S, No_S, Pulse_S};
  static State state = MenuManual_S;
  static int incf = 0, inca = 0, incta=0, inctf=0;

  if(evt != UPDATE_EVT)
    Serial.println(evt);

  switch(state) {
    
    case MenuManual_S:
      clearScreen();
      printScreen(1, "Manuel");
      sendBuffer();
      if(evt == K1R)
        state = MenuFreqSweep_S;
      else if(evt == BT2)
        state = Manual_S;
      break;
        
    case MenuFreqSweep_S:
      clearScreen();
      printScreen(1, "Balayage frequence");
      sendBuffer();
      if(evt == K1L)
        state = MenuManual_S;  
      else if(evt == K1R)
        state = MenuAmplSweep_S;
      else if(evt == BT2)
        state = FreqSweep_S;
      break;

    case MenuAmplSweep_S:
      clearScreen();
      printScreen(1, "Balayage amplitude");
      sendBuffer();
      if(evt == K1L)
        state = MenuFreqSweep_S;  
      else if(evt == K1R)
        state = MenuYesNo_S;
      else if(evt == BT2)
        state = AmplSweep_S;
      break;
      
    case MenuYesNo_S:
      clearScreen();
      printScreen(1, "Deviner");
      sendBuffer();
      if(evt == K1L)
        state = MenuAmplSweep_S;
      if(evt == K1R)
        state = MenuPulse_S;
      else if(evt == BT2)
        state = YesNo_S;
      break;

    case MenuPulse_S:
      clearScreen();
      printScreen(1, "Impulsions");
      sendBuffer();
      if(evt == K1L)
        state = MenuYesNo_S;
      else if(evt == BT2)
        state = Pulse_S;
      break;
      
    case Manual_S:
      static bool sel = true; // true : we act on frequency, false : we act on amplitude
      generator.set_mode(Manual);
      clearScreen();
      printScreen(1, "f = " + String(generator.get_frequency(), 3));
      printScreen(2, "incf = " + String(pow(10, incf), 3));
      printScreen(3, "amp = " + String(generator.get_amplitude()));
      printScreen(4, "inca = " + String((int)pow(10, inca)));
      sendBuffer();
      if(evt == BT1)
        state = MenuManual_S;
      else if (evt == BT2)
        sel = !sel;
      else if(evt==BT3)
        generator.toggle();
      else if(evt == K1R) {
        if(sel==true) {
          float f = generator.get_frequency();
          f = constrain(f+pow(10, incf), 0, 121);
          generator.set_frequency(f);
        }
        else {
          unsigned int amp = generator.get_amplitude();
          amp = constrain(amp+pow(10, inca), 0, maxAmplitude);
          generator.set_amplitude(amp);
          
        }
      }
      else if(evt == K1L) {
        if(sel==true) {
          float f = generator.get_frequency();
          f = constrain(f-pow(10, incf), 0, 121);
          generator.set_frequency(f);
        }
        else {
          unsigned int amp = generator.get_amplitude();
          amp = constrain(amp-pow(10, inca), 0, maxAmplitude);
          generator.set_amplitude(amp);
        }
      }
      else if(evt == K2R) {
        if(sel==true) 
          incf = constrain(incf+1, -2, 2);
        else
          inca = constrain(inca+1, 0, 3);
      }
      else if(evt == K2L) {
        if(sel==true)
          incf = constrain(incf-1, -2, 2);
        else
          inca = constrain(inca-1, 0, 3);
      }
      break;

    case FreqSweep_S:
      generator.set_mode(FrequencySweep);
      clearScreen();
      printScreen(1, "f = " + String(generator.get_frequency(), 3));
      printScreen(2, "amp = " + String(generator.get_amplitude()));
      printScreen(3, "tf = " + String(generator.get_tf()) + " ms");
      printScreen(4, "inctf = " + String((unsigned int)pow(10, inctf)) + " ms");
      sendBuffer();
      if(evt == BT1)
        state = MenuFreqSweep_S;
      else if(evt == BT3)
        generator.toggle();
      break;

    case AmplSweep_S:
      generator.set_mode(AmplitudeSweep);
      clearScreen();
      printScreen(1, "f = " + String(generator.get_frequency(), 3));
      printScreen(2, "amp = " + String(generator.get_amplitude()));
      printScreen(3, "ta = " + String(generator.get_ta()) + " ms");
      printScreen(4, "incta = " + String((unsigned int)pow(10, incta)) + " ms");
      sendBuffer();
      if(evt == BT1)
        state = MenuAmplSweep_S;
      else if(evt == BT3)
        generator.toggle();
      else if(evt == K1R) {
        unsigned int ta = generator.get_ta();
        ta = constrain(ta+(unsigned int)pow(10, incta), 0, 10000);
        generator.set_ta(ta);
      }
      else if(evt == K1L) {
        unsigned int ta = generator.get_ta();
        ta = constrain(ta-(unsigned int)pow(10, incta), 0, 10000);
        generator.set_ta(ta);
      }
      else if(evt == K2R)
        incta = constrain(incta+1, 0, 3);
      else if(evt == K2L)
        incta = constrain(incta-1, 0, 3);
      break;

    case YesNo_S: {
      clearScreen();
      printScreen(1, "Deviner...");
      sendBuffer();
      int r = random(2);
      Serial.print("r=");
      Serial.println(r);
      if(r==0)
        generator.disable();
      else
        generator.enable();
      state = YesNoWait_S;
      break;
    }

    case YesNoWait_S:
      switch(evt) {
        case BT1:
          state = MenuYesNo_S;
          break;
        case BT4:
          state = Yes_S;
          break;
        case BT5:
          state = No_S;
          break;
        default:
          break;
      }
      break;
      
    case Yes_S:
      clearScreen();
      if(generator.isEnabled())
        printScreen(1, "Ok !");
      else
        printScreen(1, "Erreur");
      sendBuffer();
      switch(evt) {
        case BT1:
          state = YesNo_S;
          break;
        default:
          break;
      }
      break;

    case No_S:
      clearScreen();
      if(generator.isEnabled())
        printScreen(1, "Erreur");
      else
        printScreen(1, "Ok !");
      sendBuffer();
      switch(evt) {
        case BT1:
          state = YesNo_S;
          break;
        default:
          break;
      }
      break;

    case Pulse_S:
      generator.set_mode(Pulse);
      clearScreen();
      printScreen(1, "f = " + String(generator.get_frequency(), 3));
      printScreen(2, "amp = " + String(generator.get_amplitude()));
      sendBuffer();
      if(evt == BT1)
        state = MenuPulse_S;
      else if(evt == BT3)
        generator.toggle();
      break;
    
    
    default:
      state=MenuManual_S;
      break;
      
  }
}
