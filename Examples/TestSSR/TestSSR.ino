// TestSSR.ino

#define SSR_PIN 2  // SSR1
// #define SSR_PIN 8 // SSR2

void setup() 
{
  pinMode(SSR_PIN, OUTPUT);
}

void loop() 
{
  digitalWrite(SSR_PIN, HIGH);
  delay(2000);                   
  digitalWrite(SSR_PIN, LOW);    
  delay(2000); 
}
