#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>


LiquidCrystal_I2C lcd (0x27, 16, 2);

#define BTN_MODO  0
#define BTN_UP    1
#define BTN_DOWN  2

// Este arreglo contiene los pines utilizados para los botones
uint8_t button[3] = {
  5,
  4,
  3
};

// Estados del autómata
#define S_HOME        0
#define S_SET_HOUR    1
#define S_SET_MINUTES 2
#define STATES_MAX 3

uint8_t state = S_HOME;
// Base historica de botones
uint8_t button_state[4];

int horas = 0;
int minutos = 0;
/* variables compartidas por el servicio de atención a la interrupción y el flujo normal */
volatile int segundos = 0;
volatile boolean actualizar = true;

//Buffer para guardar el formato de fecha
char str_time[9] = "00:00:00";

void setup(){
  // Configurar como PULL-UP para ahorrar resistencias
  pinMode(button[BTN_MODO], INPUT_PULLUP);
  pinMode(button[BTN_UP], INPUT_PULLUP);
  pinMode(button[BTN_DOWN], INPUT_PULLUP);
  // Se asume que el estado inicial de los botones es HIGH
  button_state[0] = HIGH;
  button_state[1] = HIGH;
  button_state[2] = HIGH;  

  Serial.begin(9600);  
  lcd.init();
  lcd.backlight();
  delay(500);  
  
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(acumuladorTimer);
}

uint8_t Rising_edge(int btn) {
  uint8_t newValue = digitalRead(button[btn]);
  uint8_t result = button_state[btn]!=newValue && newValue == 1;
  button_state[btn] = newValue;
  return result;
}

void acumuladorTimer(){
    segundos++;
    actualizar = true;
}

void formatTime(){
  minutos += segundos / 60;
  segundos = segundos % 60;
  horas += minutos / 60;
  minutos = minutos % 60;
  horas = horas % 24;
  lcd.clear();
  lcd.noBlink();
  lcd.setCursor(0,0);
  sprintf(str_time, "%02d:%02d:%02d", horas, minutos, segundos); 
  lcd.print(str_time);  
  actualizar = false;    
}

void incrementarHoras(){
  horas ++;
  horas = horas % 24; 
  actualizar = true; 
}

void decrementarHoras(){
  horas --;
  if(horas < 0){
    horas = 23;  
  }  
  actualizar = true;
}

void incrementarMinutos(){
  minutos ++;
  minutos = minutos % 60; 
  actualizar = true; 
}

void decrementarMinutos(){
  minutos --;
  if(minutos < 0){
    minutos = 59;  
  }  
  actualizar = true;
}

void fijarCursorModo(){
  switch(state){
    case S_HOME: 
      lcd.noBlink();
    break;
    case S_SET_HOUR: 
      lcd.setCursor(1,0);
      lcd.blink();
    break;
    case S_SET_MINUTES: 
      lcd.setCursor(4,0);
      lcd.blink();
    break;  
  };
}

void loop(){
/* validando opciones*/
  if( Rising_edge(BTN_MODO) ){
    state++;
    state = state % STATES_MAX;
    fijarCursorModo();    
  }

  switch(state){    
    case S_SET_HOUR: 
       if( Rising_edge(BTN_UP) ){
         incrementarHoras();          
      }
      if( Rising_edge(BTN_DOWN) ){
         decrementarHoras();         
      }
    break;
    
    case S_SET_MINUTES: 
      if( Rising_edge(BTN_UP) ){
        incrementarMinutos();        
      }
      if( Rising_edge(BTN_DOWN) ){
        decrementarMinutos();        
      }
    break;
  };

  if(actualizar == true){
    formatTime();
    fijarCursorModo();
  }  
}
