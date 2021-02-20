#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd (0x27, 16, 2);

#define BTN_MODO  0
#define BTN_UP    1
#define BTN_DOWN  2
#define BTN_LIGHT 3

// Pines para los botones
uint8_t button[4] = {
  5,
  4,
  3,
  2
};
#define BUZZER    12
// Estados del autómata
#define S_HOME              0
#define S_SET_CLOCK_HOUR    1
#define S_SET_CLOCK_MINUTES 2
#define S_SET_ALARM_HOUR    3
#define S_SET_ALARM_MINUTES 4
#define S_ALARM_ON_OFF      5
#define STATES_MAX          6

uint8_t state = S_HOME;
// Base historica de botoneslínea
uint8_t button_state[4];

int horas = 0;
int minutos = 0;
/* variables compartidas por el servicio de atención a la interrupción y el flujo normal */
volatile int segundos = 0;
volatile boolean actualizar = true;

int horas_alarma = 0;
int minutos_alarma = 0;
// no tiene sentido los segundos en la alarma
boolean alarmaOn = false;         // Para activar y desactivar la alarma (modo 6);

boolean lightOn = false;
int contadorLuz = 0;
int contadorAlarma = 0;           // contar solo cuando suena la alarma

//Buffer para guardar el formato de fecha
char str_time[9] = "00:00:00";

void setup(){
  // Configurar como PULL-UP para ahorrar resistencias
  pinMode(button[BTN_MODO], INPUT_PULLUP);
  pinMode(button[BTN_UP], INPUT_PULLUP);
  pinMode(button[BTN_DOWN], INPUT_PULLUP);
  pinMode(button[BTN_LIGHT], INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  // Se asume que el estado inicial de los botones es HIGH
  button_state[0] = HIGH;
  button_state[1] = HIGH;
  button_state[2] = HIGH;  
  button_state[3] = HIGH; 

  Serial.begin(9600);  
  lcd.init();
  lcd.backlight();
  delay(500); 
  
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(controladorTimer);
}

uint8_t Rising_edge(int btn) {
  uint8_t newValue = digitalRead(button[btn]);
  uint8_t result = button_state[btn]!=newValue && newValue == 1;
  button_state[btn] = newValue;
  /* cada vez que hay una pulsación, el contador de luz se reinicia a 0*/
  if(  digitalRead(button[btn]) == HIGH ){    
     contadorLuz = 0;
     //lcd.backlight();
  }
  return result;
}

void controladorTimer(){
    segundos++;
    if( lightOn == true ){
      contadorLuz++;   
    }    
    if( digitalRead(BUZZER)== HIGH ){
      contadorAlarma++;
    }
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
  /* Visualizar la alarma*/
  lcd.setCursor(0,1);
  sprintf(str_time, "%02d:%02d", horas_alarma, minutos_alarma); 
  lcd.print(str_time);  
  actualizar = false;   
}

void estadoAlarma(){
  if (alarmaOn == true){
    /* Verificando el tiempo de la alarma*/
      if( segundos == 0 && horas == horas_alarma && minutos == minutos_alarma){
        digitalWrite(BUZZER, HIGH);
      }
    lcd.print(" ON");
  } else{    
    lcd.print(" OFF");
  }
/* Si pasan 60 seg de la alarma, apagar el buzzer */  
  if (contadorAlarma == 60){
    digitalWrite(BUZZER, LOW);
    contadorAlarma = 0;
  }
  /* Si pasan 30 seg y la luz está encendida, se apaga */
  if( contadorLuz == 30 ){
    lcd.noBacklight();
    lightOn = false;
    contadorLuz = 0;
  } 
}
/* Ajuste de reloj*/
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

/* Ajuste de Alarma */
void incrementarHorasAlarma(){
  horas_alarma ++;
  horas_alarma = horas_alarma % 24; 
  actualizar = true; 
}
void decrementarHorasAlarma(){
  horas_alarma --;
  if(horas_alarma < 0){
    horas_alarma = 23;  
  }  
  actualizar = true;
}
void incrementarMinutosAlarma(){
  minutos_alarma ++;
  minutos_alarma = minutos_alarma % 60; 
  actualizar = true; 
}
void decrementarMinutosAlarma(){
  minutos_alarma --;
  if(minutos_alarma < 0){
    minutos_alarma = 59;  
  }  
  actualizar = true;
}

/* PARPADEO DE PANTALLA */
void fijarCursorModo(){
  switch(state){
    case S_HOME: 
      lcd.noBlink();
    break;
    case S_SET_CLOCK_HOUR: 
      lcd.setCursor(1,0);
      lcd.blink();
    break;
    case S_SET_CLOCK_MINUTES: 
      lcd.setCursor(4,0);
      lcd.blink();
    break;  
    case S_SET_ALARM_HOUR: 
      lcd.setCursor(1,1);
      lcd.blink();
    break;
    case S_SET_ALARM_MINUTES: 
      lcd.setCursor(4,1);
      lcd.blink();
    break;      
    case S_ALARM_ON_OFF: 
      lcd.setCursor(7,1);
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
/* Estas líneas colocarlo en la máquina*/
/* Lógica para el botón de luz*/
  if ( Rising_edge(BTN_LIGHT) ){
    /* Si el  buzzer esta sonando, apagarlo*/
    if( digitalRead(BUZZER) == HIGH ){
      digitalWrite(BUZZER, LOW); 
      contadorAlarma = 0;     
    }
    if(lightOn == false){
        lcd.backlight();
    } else {
        lcd.noBacklight();
    }
    lightOn = !lightOn; 
  }

  /* La luz se apaga después de 5 seg de haber presionado el último botón*/
  
  switch(state){    
    case S_SET_CLOCK_HOUR: 
       if( Rising_edge(BTN_UP) ){
         incrementarHoras();          
      }
      if( Rising_edge(BTN_DOWN) ){
         decrementarHoras();         
      }
    break;    
    case S_SET_CLOCK_MINUTES: 
      if( Rising_edge(BTN_UP) ){
        incrementarMinutos();        
      }
      if( Rising_edge(BTN_DOWN) ){
        decrementarMinutos();        
      }
    break;
  /* ALARMA */
    case S_SET_ALARM_HOUR: 
       if( Rising_edge(BTN_UP) ){
         incrementarHorasAlarma();          
      }
      if( Rising_edge(BTN_DOWN) ){
         decrementarHorasAlarma();         
      }
    break;
    case S_SET_ALARM_MINUTES: 
      if( Rising_edge(BTN_UP) ){
        incrementarMinutosAlarma();        
      }
      if( Rising_edge(BTN_DOWN) ){
        decrementarMinutosAlarma();        
      }
    break;    
    case S_ALARM_ON_OFF:      
      if( Rising_edge(BTN_UP) ){
        actualizar = true;
        alarmaOn = !alarmaOn;
      }
    break;  
  };

  if(actualizar == true){
    formatTime();
    estadoAlarma();
    fijarCursorModo();
  }  
}
