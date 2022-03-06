//  V6.ino

// D'après v6_cours.ino (https://forum-photovoltaique.fr/)

#include <avr/wdt.h>

enum Polarities {POSITIVE, NEGATIVE};

// Constant variables
byte VPin = A3;                     // Voltage pin
byte CPin = A5;                     // Current pin
byte SSR1Pin = 2;                   // SSR1 pin
byte SSR2Pin = 8;                   // SSR2 pin
float POWERCAL = 0.18;              // Digital to Watt conversion coefficient  
float VOLTAGECAL = (float)679/471;  // Digital to Volt conversion coefficient
float PHASECAL = 1;                 // Phase shift coefficient for the voltage
float L1CAL = (float)8000/2400;     // Power coefficient of load 1 (8000/power)
float high_thrL2 = 500;             // High threshold to activate load 2 (W)
float low_thrL2 = 100;              // Low threshold to deactivate load 2 (W)
float capB = 1000;                  // Capacity of the bucket (W)
int minNStart = 100;                // Minimum number of cycles in the starting phase
float freq = 50;                    // AC current frequency (Hz)
int maxNSumP = 250;                 // Maximum number of cycles to smooth the real power values 
float recovPWR = 500 ;              // ?

// Energy calculus variables
float V, prev_V, C;                 // Voltage [previous] and current values (centered)
float off, prev_off;                // Offset [previous] value for centering
float filtV, prev_filtV;            // Filtered voltage [previous] values
Polarities polV;                    // Polarity of the voltage alternance
float sumV;                         // Sum in the cycle of the voltage values  
float sumVC;                        // Sum in the cycle of the products voltage (with phase correction) x current values
int  nSum;                          // Number of values in these sums

// Other variables
float nrgyB;                        // Energy in the bucket (W)
bool okStart;                       // True when the router is operational
int nStart;                         // Number of cycles in the starting phase
float sumP;                         // Sum of the power values
int nSumP;                          // Number of values in this sum
boolean startAlt;                   // First loop in the alternance
unsigned long Tc;                   // micros() on starting an alternance
boolean flg2;                       // Flag to activate SSR2 
float P, prev_P;                    // Power [previous] on the previous alternance   
float E;                            // Energy

// ?
unsigned long Fd;                   // Firing delay 
int SMC;                            // Safety Margin Compensation 
float ret;                          // retard
float imaP;                         // image de P pour calcul de SMC
boolean phaseAngleTriggerActivated; // Utile ?

void setup()
{ 
  wdt_enable(WDTO_8S);
  pinMode(SSR1Pin, OUTPUT); 
  pinMode(SSR2Pin, OUTPUT); 

  okStart = false;  
  nStart = 0;
  sumV = 0;
  sumVC = 0;
  nSum = 0;
  nrgyB = 0;
  sumP = 0;
  nSumP = 0;
  SMC = -60;
  ret = 0 ; 
  imaP = 0;  
  flg2 = false;
}  


void loop() 
{
  wdt_reset();

  // Saving previous values and new values for the voltage and the current
  prev_V = V;            
  prev_filtV = filtV;      
  V = (float)analogRead(VPin) - off; 
  C = (float)analogRead(CPin) - off;
  filtV = 0.996*(prev_filtV + V - prev_V); // High pass filter

  digitalWrite(SSR1Pin, LOW); // Utile ?
 
  // Treatments on alternance starts
  if ((filtV >= 0) && (polV == NEGATIVE))       // Start POSITIVE (new cycle)
  {
    // Polaritiy new value
    polV = POSITIVE;
    
    if (!okStart) nStart++; 
    startAlt = true;   // Starting new alternance
    off += 0.015*sumV; // Low pass filter

    P = POWERCAL*sumVC /(float)nSum;  
    E = P/freq;                      
    if ((P-prev_P) > recovPWR) nrgyB = 300 ; 
    prev_P = P ;      
 
    if (okStart)
    { 
      // Update bucket
      nrgyB += E;   
      nrgyB -= SMC/freq;       
      if (nrgyB > capB) nrgyB = capB;  
      else if (nrgyB < 0) nrgyB = 0;    
    }
    else if(nStart > minNStart) okStart = true; 
      
    if (nrgyB <= 100) Fd = 99999; 
    else 
    {
      if (nrgyB >= 1000) Fd = 200;
      else
      {
        Fd = 10 * (1020 - nrgyB); 
        ret = Fd;
        if (ret >= 8000) ret = 8000; 
        imaP = 8000 - ret;
        SMC = -60 ;   
        if (Fd > 7300) SMC = -30 ; 
        if (Fd > 8500) Fd = 99999; 
      }
    }
    
    sumP += imaP/L1CAL  ;
    nSumP++;
    if (nSumP > maxNSumP) 
    { 
      sumP /= (float)nSumP ; 
      if (sumP >= high_thrL2) flg2 = true; 
      if (sumP <= low_thrL2) flg2 = false ; 
      sumP = 0; 
      nSumP = 0;
    } 
    
    // Reinit sums
    sumVC = 0;
    sumV = 0;
    nSum = 0;
  }
  
  else if ((filtV < 0) && (polV == POSITIVE))   // Start NEGATIVE
  {
    // Polaritiy new value
    polV = NEGATIVE;  
    
    // Starting new alternance
    startAlt = true;
  }
  
  // *******************************************************************************************
  // Other treatments
  // *******************************************************************************************
  
  // Loads command
  unsigned long To = micros(); 
  if (flg2) digitalWrite(SSR2Pin, HIGH);
  else digitalWrite(SSR2Pin, LOW);
  if (startAlt)           
  {  
    Tc = To; 
    startAlt = false;
    phaseAngleTriggerActivated = false;
    if (Fd > 200) digitalWrite(SSR1Pin, LOW); 
  }
  if (phaseAngleTriggerActivated)
  {
    if (Fd > 200) digitalWrite(SSR1Pin, LOW);
    else
    {  
      if (To >= (Tc+Fd))
      {  
        digitalWrite(SSR1Pin, HIGH); 
        phaseAngleTriggerActivated = true;  // Utile ?
      }
    }
  }
  
  // Sums incrementation
  sumVC += (prev_V+PHASECAL*(V-prev_V))*C;     
  sumV += V;
  nSum++;
} 
