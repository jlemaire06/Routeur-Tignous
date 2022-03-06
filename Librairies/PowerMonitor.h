// PowerMonitor.h

#pragma once

#include <Arduino.h>
#include "RingBuffer.hpp"

#define M 100 // Buffer size <256
//#define TESTING   

/***********************************************************************************
  Class PowerMonitor
************************************************************************************

  Rms voltage, rms current and real power estimations for voltage and current channels
  
  Inspired from :
    Robin Emley's Cal_bothDisplays_2.ino sketch  ( https://www.mk2pvrouter.co.uk )
    EmonLib library ( https://github.com/openenergymonitor/EmonLib )
    EmonLibCM library ( https://github.com/openenergymonitor/EmonLibCM )

  Constructor parameters :
 
    _period      = sampling period for the channels (in us and 105<.<8192)
    _nCycl       = number of cycles to construct the estimations (>=1)
    _VPin, _CPin = Arduino analog pins for the voltage and the current (Ax)
    _VCal, _CCal = Calculus coefficients to convert the digitized values
    _pha         = Phase shift coefficient of the voltage (int)
    NB : _pha=256 => no shift, _pha<256 => right shift, _pha>256 => left shift
*/

enum Polarities {POSITIVE, NEGATIVE}; // Alternance polarities

extern "C" void TIMER1_OVF_vect();

class PowerMonitor
{
  public:
    // Constructor
    PowerMonitor(int _period, int _nCycl, byte  _VPin, byte  _CPin, double _VCal, double _CCal, int _pha);  
    
    void Init();              // Initialisation (to be called in the setup)
    void Process();           // Process calculus outside the ISR (to be called in the loop()
    double RmsVoltage();      // Vrms (V)
    double RmsCurrent();      // Irms (A)
    double RealPower();       // Pr (W)
  
#ifdef TESTING
    byte MaxSize();           // Maximum size of the circular buffer, on entering Process()
    int NbSamples();          // Number of samples in estimations
    long MaxProcTime();       // Max time to treat a sample (us)
#endif

  private:
    unsigned int period;      // Sampling period for the channels
    int nCycl, iCycl;         // Number of cycles and index of the cycle 
    byte VPin, CPin;          // Arduino pins for the voltage and the current
    double VCal, CCal;        // Calculus coefficients to convert the digitized vales
    int pha;                  // Phase shift coefficients array of the voltage 
 
    int iChan;                // Index of the channel in the digitalization
    RingBuffer<M, 2> rbDig;   // Circular buffer for the digitized values
    int tDigISR[2];           // Digitized values array pushed in the ISR 
    int tDig[2];              // Digitized values array poped in the Process method 
    long V, C;                // Voltage and current values (<<8 and centered)
    Polarities pol;           // Polarity of the alternance
 
    long offV, offC;          // Offsets for centering
    long sumV, sumC;          // Sums of the voltage and current values 
    long sumVV, sumCC;        // Sums of the squared voltage and current values (>>10)
    long savSumVV, savSumCC;  // Saved sum of the squared voltage and current values (>>10)    
    long sumVC;               // Sum of the VC products(>>10)
    long savSumVC;            // Saved sum of the VC products(>>10)
    long preV;                // Previous voltage value (>>2)
    long phaV;                // Phase shifted voltage value (>>2)
    int nSum;                 // Number (saved) of values in the sums VV, CC and VC
    int nSavSum;              // Number (saved) of values in the sums VV, CC and VC
    void FuncISR();           // ISR function

#ifdef TESTING
    byte maxSize;             // Maximum size of the circular buffer, on entering Process()
    long maxProcTime;         // Maximum of processing time
#endif
    
  friend void TIMER1_OVF_vect(); // To use the private FuncISR() in the ISR
};
