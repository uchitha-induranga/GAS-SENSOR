#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x26,16,2);



int smoke;

void setup() {
pinMode(A0,INPUT);
Serial.begin(9600);
pinMode(2,OUTPUT);
lcd.init();
lcd.backlight();
lcd.setCursor(0,0);
lcd.print("HRTE PROJECTS");
delay(1000);
lcd.clear();
delay(1000);
lcd.setCursor(2,1);
lcd.print("by Uchitha");
delay(1000);
lcd.clear();
delay(500);
}

void loop() {
smoke = analogRead(A0);

lcd.setCursor(0,0);
lcd.print("HRTE LPG METER ");
lcd.setCursor(0,1);
lcd.print(smoke);

if (smoke > 300)
{
  digitalWrite(2,HIGH);
  lcd.setCursor(5,1);
  lcd.print(" Alert!!...");
  lcd.noBacklight();
  delay(500);
  lcd.backlight(); 
  }
else
{
  digitalWrite(2,LOW);
  }
  delay(500);
  lcd.clear();
}
