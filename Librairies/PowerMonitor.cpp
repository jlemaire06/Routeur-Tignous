// PowerMonitor.cpp

#include "PowerMonitor.h"

/***********************************************************************************
ISR 
***********************************************************************************/

PowerMonitor* pPowerMonitor; // As static

ISR(TIMER1_OVF_vect)
{
  pPowerMonitor->FuncISR();
}

/***********************************************************************************
 Class PowerMonitor
***********************************************************************************/

PowerMonitor::PowerMonitor(int _period, int _nCycl, byte  _VPin, byte  _CPin, double _VCal, double _CCal, int _pha)
{
  pPowerMonitor = this;
  period = _period;
  nCycl = _nCycl;
  VPin = _VPin;
  CPin = _CPin;  
  VCal = _VCal;
  CCal = _CCal;
  pha = _pha;
}

void PowerMonitor::Init()
{
  // Set ADC parameters (prescaler, voltage reference), enable and start it with the volatge source
  ADCSRA = (1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); // Prescaler = 128 (104us/conversion and max accuracy)
  ADCSRA |= (1<<ADEN);                       // Enable ADC
  ADMUX = 0x40;                              // Voltage reference = AVcc with external capacitor at AREF pin 
  ADMUX |= VPin;                             // Set ADPS to the voltage pin
  ADCSRA |= (1<<ADSC);                       // Start the conversion
  delay(1);                                  // Wait to perform this conversion
  
  // Set Timer1 interrupts 
  noInterrupts();                            // Disable all interrupts
  TCCR1A = 0;                                // Normal port operation (no PWM outputs)
  TCCR1B = (1<<WGM13);                       // Phase and frequency correct PWM mode with TOP in ICR1
  TCCR1B |= (1<<CS10);                       // No prescaling
  ICR1 = 8*period;                           // TOP
  TIMSK1 = (1<<TOIE1);                       // Enable timer OVF interrupt (TOV1 flag set on at BOTTOM)
  interrupts();                              // Enable all interrupts
  
  // Init values for members
  iCycl = 0;
  iChan = 0;
  nSum = 0;
  nSavSum = 0;
  offV = 510L*256;
  sumV = 0;
  sumVV = 0;
  offC = 510L*256;
  sumC = 0;
  sumCC = 0;
  sumVC = 0;
  preV = 220L*64;
  
#ifdef TESTING
    maxSize = 0;
    maxProcTime = 0;
#endif
}

void PowerMonitor::Process()
{
  
#ifdef TESTING
    maxSize = max(maxSize, rbDig.Size());
#endif

  while (rbDig.Size() > 0)
  {

#ifdef TESTING
    long startTime = micros();
#endif

    // Pop the digitized values
    cli();
    rbDig.Pop(tDig);
    sei();

    // Voltage and current values (x256 and centering)
    V = (((long)tDig[0])<<8)- offV;
    C = (((long)tDig[1])<<8)- offC;

    // New half alternance test
    if ((V >= 0) &&  (pol == NEGATIVE)) // Start POSITIVE (new cycle)
    {
      // Polarity new value
      pol = POSITIVE;
      
      iCycl++;
      if (iCycl == nCycl)
      {
        // Save the sums of squared values and VC products, then reinit them
        savSumVV = sumVV;
        savSumCC = sumCC;
        savSumVC = sumVC;
        nSavSum = nSum;
        sumVV = 0;
        sumCC = 0;
        sumVC = 0;
        nSum = 0;
        
        // Reinit iCycl
        iCycl = 0;
      }
    }
    else if ((V < 0) &&  (pol == POSITIVE)) // Start NEGATIVE
    {
      // Polarity new value
      pol = NEGATIVE;

      // Offsets update using low pass filters 
      offV += (sumV>>12);
      offC += (sumC>>12);
      sumV = 0;
      sumC = 0;
    }
    
    // Sums incrementation
    sumV += V;
    V = (V>>2);
    sumVV += ((V*V)>>6);
    sumC += C;
    C = (C>>2);
    sumCC += ((C*C)>>6);
    phaV = preV + (((V - preV)*pha)>>8);
    sumVC += ((phaV*C)>>6);
    preV = V;
    nSum++;

#ifdef TESTING
    maxProcTime = max(maxProcTime, micros()-startTime);
#endif

  }
} 

double PowerMonitor::RmsVoltage()
{
  return VCal*sqrt(((double)savSumVV)/(64.*nSavSum));
}

double PowerMonitor::RmsCurrent()
{
  return CCal*sqrt(((double)savSumCC)/(64.*nSavSum));
}

double PowerMonitor::RealPower()
{
  return VCal*CCal*(((double)savSumVC)/(64.*nSavSum));
}

#ifdef TESTING
byte PowerMonitor::MaxSize()
{
  byte savMaxSize = maxSize;
  maxSize = 0;
  return savMaxSize;
}

int PowerMonitor::NbSamples()
{
  return nSavSum;
}
           
long PowerMonitor::MaxProcTime()
{
  long savMaxProcTime = maxProcTime;
  maxProcTime = 0;
  return savMaxProcTime;
}
#endif

void PowerMonitor::FuncISR()
{
  // Digitized value
  tDigISR[iChan++] = ADC; 
  
  // Push the digitized values in the buffer
  if(iChan == 2) 
  {
    iChan = 0;
    ADMUX = 0x40 | VPin;
    rbDig.Push(tDigISR);
  }
    
  // Start the next conversion 
  ADMUX = 0x40 | CPin;
  ADCSRA |= (1<<ADSC); 
}  
