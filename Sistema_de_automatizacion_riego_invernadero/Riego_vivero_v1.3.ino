//Importación de librerías LCD
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//Libreria DHT11
#include <DFRobot_DHT11.h>

//Definición de objeto DHT y pin del sensor
DFRobot_DHT11 DHT;
#define SENSOR_DHT 9

//Defino en que pin del arduino esta conectado el sensor de humedad de suelo
const int sensor_hum_1 = A0;
const int sensor_hum_2 = A1;
const int sensor_hum_3 = A2;
const int pote_conf = A3;

//Defino un pin para la bomba de riego
const int bomba_riego = 13;

//Defino los pines para las luces, luces infrarrojas y ventiladores.
const int luces = 12;
const int luces_inf = 11;
const int ventiladores = 10;

//Defino los pines para los picos
const int pico_1 = 6;
const int pico_2 = 7;
const int pico_3 = 8;

//Defino pines para sensor de nivel de tanque, nivel de luz y pulsador para cambiar de pantalla
const int sensor_nivel_tanq = 5;
const int sensor_nivel_luz = 4;
const int camb_pantalla = 3; 
const int camb_conf = 2;

//Arreglo para pasar 
volatile int data[12] = {};
//0-cont/1-hum1/2-hum2/3-hum3/4-hum_amb/5-temp_amb/6-luces/7-tanque
int data_conf[10] = {0, 80, 80, 80, 70, 20, 1};

int ult_valor_analog = 0;

//Variables anti-rebote
int estado_pulsador; 
int ultimo_estado_pulsador = LOW;

long ultimo_tiempo_rebote = 0;
long retardo_rebote = 5;

static unsigned long lastInterruptTime = 0;

volatile bool estado_boton = false;
volatile bool estado_boton_conf = false;

bool movimiento_pote = false;

bool estado_pico_1 = false;
bool estado_pico_2 = false;
bool estado_pico_3 = false;


LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display


void antirrebote_ISR();
void antirrebote_ISR_2();
void show_lcd(int data_sensor[]);
void cambio_configuracion(int data_conf[], int data_sensor[]);

void setup() 
{
  Serial.begin(115200);  //Inicio la comunicación serial 
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  //Configuro los pines como salidas digitales
  pinMode(bomba_riego, OUTPUT);  
  pinMode(luces, OUTPUT);
  pinMode(luces_inf, OUTPUT);
  pinMode(ventiladores, OUTPUT);
  pinMode(pico_1, OUTPUT);
  pinMode(pico_2, OUTPUT);
  pinMode(pico_3, OUTPUT);

  pinMode(camb_conf, INPUT);
  pinMode(camb_pantalla, INPUT);
  pinMode(sensor_nivel_tanq, INPUT);
  pinMode(sensor_nivel_luz, INPUT);
  
  
  attachInterrupt(digitalPinToInterrupt(camb_conf), antirrebote_ISR_2, RISING); 
  attachInterrupt(digitalPinToInterrupt(camb_pantalla), antirrebote_ISR, RISING);  
   
  data[0] = 0;
  data_conf[0] = 0;


  ult_valor_analog = analogRead(pote_conf);

}

void loop() 
{
  int hum_suelo_1 = analogRead(sensor_hum_1);   //Leo y guardo el valor de humedad del sensor
  int hum_suelo_2 = analogRead(sensor_hum_2);
  int hum_suelo_3 = analogRead(sensor_hum_3);
  //int nivel_tanq = analogRead(sensor_nivel_tanq);
  //int nivel_luz = analogRead(sensor_nivel_luz);
  DHT.read(SENSOR_DHT);

  Serial.print(DHT.humidity);
  Serial.print("   ");
  Serial.println(DHT.temperature);

  //Serial.println(hum_suelo_1);

  data[1] = map(hum_suelo_1, 0, 1023, 100, 0);
  data[2] = map(hum_suelo_2, 0, 1023, 100, 0);
  data[3] = map(hum_suelo_3, 0, 1023, 100, 0);


  if(estado_boton_conf == true)
  {
    data_conf[0] = 0;
    cambio_configuracion(data_conf, data);
  }
  
  
  if(digitalRead(sensor_nivel_tanq) == LOW)
  {
      //Si la humedad del suelo es baja y el nivel de tanque es alto
    if(data[1] <= data_conf[1])
    {
      digitalWrite(bomba_riego, HIGH);         //Enciendo la bomba de riego
      digitalWrite(pico_1, HIGH);
      data[4] = 1; 
    }

    else if(data[1] > data_conf[1])
    {
      digitalWrite(bomba_riego, LOW);         //Enciendo la bomba de riego
      digitalWrite(pico_1, LOW);
      data[4] = 0;
    }

    //Si la humedad del suelo es baja y el nivel de tanque es alto
    if(data[2] <= data_conf[2])
    {
      digitalWrite(bomba_riego, HIGH);         //Enciendo la bomba de riego
      digitalWrite(pico_2, HIGH);
      data[5] = 1;
    }

    else if(data[2] > data_conf[2])
    {
      //digitalWrite(bomba_riego, LOW);         //Enciendo la bomba de riego
      digitalWrite(pico_2, LOW);
      data[5] = 0;
    }
    
    
    
    if(data[3] <= data_conf[3])
    {
      digitalWrite(bomba_riego, HIGH);         //Enciendo la bomba de riego
      digitalWrite(pico_3, HIGH);
      data[6] = 1;
    }

    else if(data[3] > data_conf[3])
    {
      //digitalWrite(bomba_riego, LOW);         //Enciendo la bomba de riego
      digitalWrite(pico_3, LOW);
      data[6] = 0;
    }
  }


    //si la humedad de suelo es alta o el tanque esta vacio
  else
  {
    digitalWrite(bomba_riego, LOW);
    digitalWrite(pico_1, LOW);
    digitalWrite(pico_2, LOW);
    digitalWrite(pico_3, LOW);
     
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
  }

  

  

  //control de luces y luces infrarrojas
  if(digitalRead(sensor_nivel_luz) == LOW && data_conf[6] == 1)
  {
    digitalWrite(luces, HIGH);
    digitalWrite(luces_inf, HIGH);
    data[7] = 0;
  }

  else
  {
    digitalWrite(luces, LOW);
    digitalWrite(luces_inf, LOW);

    data[7] = 1;
  }


  //Control de humedad y temperatura ambiente
  if(DHT.temperature > data_conf[5] || DHT.humidity > data_conf[4])
  {
    digitalWrite(ventiladores, HIGH);
    data[8] = 1;
  }

  else 
  {
    digitalWrite(ventiladores, LOW);
    data[8] = 0;
  }


  if(digitalRead(sensor_nivel_tanq) == LOW)
  {
    data[11] = 1;
  }

  else
  {
    data[11] = 0; 
  }


  data[9] = DHT.humidity;
  data[10] = DHT.temperature;


  if(estado_boton_conf == false)
  {
    show_lcd(data);
  }
  


  
  

}


// Función de interrupción
void antirrebote_ISR() {
  //static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  
  // Si el tiempo entre interrupciones es mayor que el tiempo de antirrebote
  if (interruptTime - lastInterruptTime > 300) {
    data[0] ++;
    data_conf[0] ++;

    if(data[0]>2)
    {
      data[0] = 0;
    }

    if(data_conf[0]>8)
    {
      data_conf[0] = 0;
    }
  }

  lastInterruptTime = interruptTime;

  
}

// Función de interrupción
void antirrebote_ISR_2() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  
  // Si el tiempo entre interrupciones es mayor que el tiempo de antirrebote
  if (interruptTime - lastInterruptTime > 200) {
    estado_boton_conf = !estado_boton_conf;
  }
  lastInterruptTime = interruptTime;

}


void show_lcd(int data_sensor[])
{
  switch (data_sensor[0]) 
  {
    case 0:
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("H1:");
    lcd.setCursor(3, 0);
    lcd.print(data_sensor[1]);
    lcd.setCursor(6, 0); 
    lcd.print("%");

    lcd.setCursor(8, 0); 
    lcd.print("H2:");
    lcd.setCursor(11, 0);
    lcd.print(data_sensor[2]);
    lcd.setCursor(14, 0); 
    lcd.print("%");

    lcd.setCursor(0, 1); 
    lcd.print("H3:");
    lcd.setCursor(3, 1);
    lcd.print(data_sensor[3]);
    lcd.setCursor(6, 1); 
    lcd.print("%");

    if(data_sensor[4] == 1)
    {
      lcd.setCursor(8, 1);
      lcd.print("P1");
    }

    if(data_sensor[5] == 1)
    {
      lcd.setCursor(11, 1);
      lcd.print("P2");
    }

    if(data_sensor[6] == 1)
    {
      lcd.setCursor(14, 1);
      lcd.print("P3");
    }

    break;



    case 1: 
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Hum:");
    lcd.setCursor(4, 0);
    lcd.print(data_sensor[9]);
    lcd.setCursor(7, 0); 
    lcd.print("%");

    lcd.setCursor(8, 0); 
    lcd.print("Temp:");
    lcd.setCursor(13, 0);
    lcd.print(data_sensor[10]);
    lcd.setCursor(15, 0); 
    lcd.print("c");   //Ver

    lcd.setCursor(0, 1); 
    lcd.print("Luz:");
    lcd.setCursor(4, 1);
    if(data_sensor[7] == 1)
    {
      lcd.print("Si");
    }
    else
    {
      lcd.print("No");
    }

    //VER
    lcd.setCursor(7, 1); 
    lcd.print("Tn:");
    lcd.setCursor(10, 1);
    if(data_sensor[11] == 1)
    {
      lcd.print("Si"); 
    }
    else
    {
      lcd.print("No");
    }

    break;

    default:
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Default");
    break;

  }

}

void cambio_configuracion(int data_conf[], int data_sensor[])  //Ver data_sensor
{
  while(estado_boton_conf == true)
  {
    //Serial.println(data_sensor[0]);

    Serial.print(data[0]);
    Serial.print("   ");
    Serial.println(data_conf[0]);

    int valor_analog = analogRead(pote_conf);
    int valor_analog_map = map(valor_analog, 0, 1023, 0, 100);

    if(valor_analog > ult_valor_analog * 1.1 || valor_analog < ult_valor_analog * 0.9)
    {
      movimiento_pote = true;
      ult_valor_analog = valor_analog;
    }

    else
    {
      movimiento_pote = false;
    }

     

    switch (data_conf[0])
    {
      //Configuración sensor humedad 1
      case 0:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Conf umbral Hum1");
      
      if(movimiento_pote == true)
      {
        data_conf[1] = valor_analog_map;//map(valor_analog, 0, 1023, 0, 100);
        lcd.setCursor(0, 1);
        lcd.print("Hum1: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[1]);
        lcd.setCursor(9, 1);
        lcd.print("%");
      }

      else 
      {
        lcd.setCursor(0, 1);
        lcd.print("Hum1: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[1]);
        lcd.setCursor(9, 1);
        lcd.print("%");
      }
      
      break;



      //Configuración sensor de humedad 2
      case 1:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Conf umbral Hum2");

      if(movimiento_pote == true)
      {
        data_conf[2] = valor_analog_map;//map(valor_analog, 0, 1023, 0, 100);
        lcd.setCursor(0, 1);
        lcd.print("Hum2: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[2]);
        lcd.setCursor(9, 1);
        lcd.print("%");
      }

      else 
      {
        lcd.setCursor(0, 1);
        lcd.print("Hum2: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[2]);
        lcd.setCursor(9, 1);
        lcd.print("%");
      }
      break;


      //Configuración sensor de humedad 3
      case 2:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Conf umbral Hum3");

      if(movimiento_pote == true)
      {
        data_conf[3] = valor_analog_map;//map(valor_analog, 0, 1023, 0, 100);
        lcd.setCursor(0, 1);
        lcd.print("Hum3: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[3]);
        lcd.setCursor(9, 1);
        lcd.print("%");
      }

      else 
      {
        lcd.setCursor(0, 1);
        lcd.print("Hum3: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[3]);
        lcd.setCursor(9, 1);
        lcd.print("%");
      }
      break;


      //Configuración humedad ambiente
      case 3:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Conf umbral HumA");

      if(movimiento_pote == true)
      {
        data_conf[4] = valor_analog_map;//map(valor_analog, 0, 1023, 0, 100);
        lcd.setCursor(0, 1);
        lcd.print("HumA: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[4]);
        lcd.setCursor(9, 1);
        lcd.print("%");
      }

      else 
      {
        lcd.setCursor(0, 1);
        lcd.print("HumA: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[4]);
        lcd.setCursor(9, 1);
        lcd.print("%");
      }
      break;


      //Configuración temperatura ambiente
      case 4:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Conf umbral Temp");

      if(movimiento_pote == true)
      {
        data_conf[5] = valor_analog_map;//map(valor_analog, 0, 1023, 0, 100);
        lcd.setCursor(0, 1);
        lcd.print("Temp: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[5]);
        lcd.setCursor(9, 1);
        lcd.print("c");
      }

      else 
      {
        lcd.setCursor(0, 1);
        lcd.print("Temp: ");
        lcd.setCursor(6, 1);
        lcd.print(data_conf[5]);
        lcd.setCursor(9, 1);
        lcd.print("c");
      }
      break;

      
      
      //Configuración luces
      case 5:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Conf cont luces");

      if(movimiento_pote == true)
      {
        if(valor_analog_map > 50)
        {
          data_conf[6] = 1;
          lcd.setCursor(0, 1);
          lcd.print("Luces: ");
          lcd.setCursor(7, 1);
          lcd.print("Si");
        }

        else
        {
          data_conf[6] = 0;
          lcd.setCursor(0, 1);
          lcd.print("Luces: ");
          lcd.setCursor(7, 1);
          lcd.print("No");
        }
      }

      else 
      {
        lcd.setCursor(0, 1);
        lcd.print("Luces: ");
        lcd.setCursor(7, 1);

        if(data_conf[6] == 1)
        {
          lcd.print("Si");
        }

        else
        {
          lcd.print("No");
        }
      }
      break;



      default:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("default 2");
      break;

    }

    delay(200);

  



  }

}

