// TestSerial.ino

void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  if(Serial.available()>0)
  {
    char x = Serial.read();
    Serial.println(x);
  }
}
