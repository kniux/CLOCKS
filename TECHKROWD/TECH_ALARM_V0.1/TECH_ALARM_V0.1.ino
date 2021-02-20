#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd (0x27, 16, 2);

int horas = 0;
int minutos= 0;
/* variable compartida en flujo normal y atención a interrupción*/
volatile int segundos = 0; 
volatile boolean actualizar = true;

/* se ejecuta cada segundo*/
void manejadoraTimer(){
  segundos++;
  actualizar = true;          // Para refrezco de pantalla
}
/* Formato del reloj*/
void actualizarReloj(){
  minutos += segundos / 60 ;          // división entre tipos "int", si el resultado da entero se suma y se actualiza minutos. 
  segundos = segundos % 60 ;         // resto de la división se guarda en seg.
  
  horas += minutos / 60 ;
  minutos = minutos % 60 ;
  
  horas = horas % 24 ;
}

void setup(){
  Wire.begin();
  lcd.init();
  lcd.clear();
  lcd.backlight();

  Timer1.initialize(1000000);
  Timer1.attachInterrupt(manejadoraTimer);  
}

void loop(){
  if(actualizar == true){
    actualizarReloj();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(horas);
    lcd.print(":");
    lcd.print(minutos);
    lcd.print(":");
    lcd.print(segundos);
    actualizar = false;
  }
}
