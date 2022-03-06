// PowerMonitor.ino

// Test the PowerMonitor class

#include "PowerMonitor.h" 
#include <elapsedMillis.h>          

byte VPin = 3;
byte CPin = 5;
byte SSR_PIN = 8;
double VCal = 14.3;
double CCal = 0.116;
int pha = 280; 
PowerMonitor pm(125, 2, VPin, CPin, VCal, CCal, pha); 

elapsedMillis tmr;    
double Vrms, Irms, Pr, Pa;       

void setup()
{  
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, HIGH);
  Serial.begin(9600);
  pm.Init();
}

void loop()
{
  // delay(30);
  pm.Process();
  if (tmr>2000)
  {
    tmr = 0;
#ifdef TESTING
    Serial.print("MaxSizeBuffer = "); Serial.print(pm.MaxSize());
    Serial.print(" NbSamples = "); Serial.print(pm.NbSamples());
    Serial.print(" MaxProcTime = "); Serial.print(pm.MaxProcTime()); Serial.println("us");
#else
    Vrms = pm.RmsVoltage();
    Irms = pm.RmsCurrent();
    Pr = pm.RealPower();
    Pa = Vrms*Irms;
    Serial.print("Vrms = "); Serial.print(Vrms); Serial.print("V");
    Serial.print(" Irms = "); Serial.print(Irms); Serial.print("A");
    Serial.print(" Pr = "); Serial.print(Pr); Serial.print("W");
    Serial.print(" Pa = "); Serial.print(Vrms*Irms); Serial.print("VA");
    Serial.print(" F = "); Serial.println(abs(Pr)/Pa);
#endif
  }
}
