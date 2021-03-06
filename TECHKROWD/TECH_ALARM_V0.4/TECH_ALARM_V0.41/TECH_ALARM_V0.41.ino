#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define BTN_MODO  0
#define BTN_UP    1
#define BTN_DOWN  2
#define BTN_LIGHT 3

#define Buzzer    9

// Este arreglo contiene los pines utilizados para los botonesuni
uint8_t button[4] = {
  5,
  4,
  3,
  2
};

// Estados del autómata
#define S_HOME              0
#define S_SET_HOUR_CLOCK    1
#define S_SET_MINUTES_CLOCK 2
#define S_SET_HOUR_ALARM    3
#define S_SET_MINUTES_ALARM 4
#define S_SET_ACTIVE_ALARM  5

#define STATES_MAX 6

uint8_t state = S_HOME;
uint8_t button_state[4];

int horasReloj = 0;
int minutosReloj = 0;
int horasAlarma = 0;
int minutosAlarma = 0;
boolean alarmaON = false; 
boolean lightON = false;
boolean buzzerON = false;

/* variables compartidas por el servicio de atención a la interrupción y el flujo normal */
volatile int segundosReloj = 0;
volatile boolean refresh = true;

volatile int contadorlightON = 0;
volatile int contadorbuzzerON = 0;

//Buffer para guardar el formato de fecha
char str_time[9] = "00:00:00";

LiquidCrystal_I2C lcd (0x27, 16, 2);

void setup(){
  // Configurar como PULL-UP para ahorrar resistencias
  pinMode(button[BTN_MODO], INPUT_PULLUP);
  pinMode(button[BTN_UP], INPUT_PULLUP);
  pinMode(button[BTN_DOWN], INPUT_PULLUP);
  pinMode(button[BTN_LIGHT], INPUT_PULLUP);
  pinMode(Buzzer, OUTPUT);
  // Se asume que el estado inicial de los botones es HIGH
  button_state[0] = HIGH;
  button_state[1] = HIGH;
  button_state[2] = HIGH;  
  button_state[3] = HIGH;
  
  Serial.begin(9600);  
  lcd.init();
  lcd.noBacklight();                  // desde el inicio pantalla apagada
  delay(500); 
    
  Timer1.initialize(5000000);
  Timer1.attachInterrupt(ServicioDeInterrupcion);
}

uint8_t Rising_edge(int btn) {
  uint8_t newValue = digitalRead(button[btn]);
  uint8_t result = button_state[btn]!=newValue && newValue == 1;
  button_state[btn] = newValue;
  /*cada vez qe se pulse un boton, reiniciar el contador de luz encendida*/
  if(  digitalRead(button[btn]) == LOW ){ 
    contadorlightON = 0;  
  }  
  return result;
}

void ServicioDeInterrupcion(){
    segundosReloj++;
    /* Incrementando contador de luz*/
    if( lightON == true){
      contadorlightON++; 
    } 
    /*Si la alarma esta activa*/   
    if( digitalRead(Buzzer) == HIGH ){
      contadorbuzzerON++;
    }
    refresh = true;
}

void formatTime(){
  minutosReloj += segundosReloj / 60;
  segundosReloj = segundosReloj % 60;
  horasReloj += minutosReloj / 60;
  minutosReloj = minutosReloj % 60;
  horasReloj = horasReloj % 24;
}  
/* visualizando Reloj*/
void displayTime(){  
  lcd.clear();
  lcd.noBlink();
  lcd.setCursor(0,0);
  sprintf(str_time, "%02d:%02d:%02d", horasReloj, minutosReloj, segundosReloj); 
  lcd.print(str_time);     
}
/* visualizando Alarma*/
void displayAlarm(){
  lcd.setCursor(0,1);
  sprintf(str_time, "%02d:%02d", horasAlarma, minutosAlarma); 
  lcd.print(str_time);  
  /*Mostrando si esta activada o no la alarma*/
  if (alarmaON == true){
    lcd.print(" ON");
  } else{
    lcd.print(" OFF");
  }    
}
void controlLight(){
  if (contadorlightON == 30){  // si ya pasaron 30 seg encendida la luz
    lcd.noBacklight();  
    lightON = false;  
    contadorlightON = 0;      //reiniciando contador de luz
  }    
}
void controlAlarm(){
  if (alarmaON == true){
    /*VERIFICAR SI EL TIEMPO ALARMA COINCIDE CON EL RELOJ*/
    if( segundosReloj == 0 && horasReloj == horasAlarma && minutosReloj == minutosAlarma ){
      digitalWrite(Buzzer, HIGH);
    }  
    if (contadorbuzzerON == 60){  // si ya pasaron 30 seg encendida la luz
      digitalWrite(Buzzer, LOW);
      contadorbuzzerON = 0;      
    }    
  }
}

/* AJUSTE RELOJ*/
void incrementarhorasReloj(){
  horasReloj ++;
  horasReloj = horasReloj % 24; 
  refresh = true; 
}
void decrementarhorasReloj(){
  horasReloj --;
  if(horasReloj < 0){
    horasReloj = 23;  
  }  
  refresh = true;
}
void incrementarminutosReloj(){
  minutosReloj ++;
  minutosReloj = minutosReloj % 60; 
  refresh = true; 
}
void decrementarminutosReloj(){
  minutosReloj --;
  if(minutosReloj < 0){
    minutosReloj = 59;  
  }  
  refresh = true;
}
/* AJUSTE ALARMA */
void incrementarhorasAlarma(){
  horasAlarma ++;
  horasAlarma = horasAlarma % 24; 
  refresh = true; 
}
void decrementarhorasAlarma(){
  horasAlarma--;
  if(horasAlarma < 0){
    horasAlarma = 23;  
  }  
  refresh = true;
}
void incrementarminutosAlarma(){
  minutosAlarma ++;
  minutosAlarma = minutosAlarma % 60; 
  refresh = true; 
}
void decrementarminutosAlarma(){
  minutosAlarma --;
  if(minutosAlarma < 0){
    minutosAlarma = 59;  
  }  
  refresh = true;
}

/* PANTALLA PARPADEA*/
void fijarCursor(){
  switch(state){
    case S_HOME: 
      lcd.noBlink();
    break;
    case S_SET_HOUR_CLOCK: 
      lcd.setCursor(1,0);
      lcd.blink();
    break;
    case S_SET_MINUTES_CLOCK: 
      lcd.setCursor(4,0);
      lcd.blink();
    break;  
    case S_SET_HOUR_ALARM: 
      lcd.setCursor(1,1);
      lcd.blink();
    break;
    case S_SET_MINUTES_ALARM: 
      lcd.setCursor(4,1);
      lcd.blink();
    break;      
    case S_SET_ACTIVE_ALARM: 
      lcd.setCursor(6,1);
      lcd.blink();
    break;     
  };
}

void loop(){
/* validando opciones*/
  if( Rising_edge(BTN_MODO) ){    
    state++;
    state = state % STATES_MAX;
    fijarCursor();    
  }

  if( Rising_edge(BTN_LIGHT) ){    
    /* Si la alarma esta sonnado*/
    if( digitalRead(Buzzer) == HIGH ){
      digitalWrite(Buzzer, LOW);
      contadorbuzzerON = 0;
    }
    if( lightON == false){
      lcd.backlight();
    } else{
      lcd.noBacklight();
    }
      lightON = !lightON;
  }
  switch(state){    
    /* CASOS RELOJ*/
    case S_SET_HOUR_CLOCK: 
       if( Rising_edge(BTN_UP) ){        
         incrementarhorasReloj();          
      }
      if( Rising_edge(BTN_DOWN) ){
        decrementarhorasReloj();         
      }
    break;    
    case S_SET_MINUTES_CLOCK: 
      if( Rising_edge(BTN_UP) ){
        incrementarminutosReloj();        
      }
      if( Rising_edge(BTN_DOWN) ){
        decrementarminutosReloj();        
      }
    break;
    /* CASOS ALARM*/
    case S_SET_HOUR_ALARM: 
       if( Rising_edge(BTN_UP) ){
         incrementarhorasAlarma();          
      }
      if( Rising_edge(BTN_DOWN) ){
         decrementarhorasAlarma();         
      }
    break;    
    case S_SET_MINUTES_ALARM: 
      if( Rising_edge(BTN_UP) ){
        incrementarminutosAlarma();        
      }
      if( Rising_edge(BTN_DOWN) ){
        decrementarminutosAlarma();        
      }
    break;  
    /* ACTIVAR/DESACTIVAR ALARMA*/  
    case S_SET_ACTIVE_ALARM: 
      if( Rising_edge(BTN_UP) ){
        alarmaON = !alarmaON;    
        refresh = true ;       // refrezca en el momento (no espera un segundo)    
      }      
    break;  
  };

  if(refresh == true){
      formatTime();
      displayTime();
      controlLight();
      displayAlarm();   
      controlAlarm(); 
      fijarCursor();
      refresh = false;        
  }  
}
