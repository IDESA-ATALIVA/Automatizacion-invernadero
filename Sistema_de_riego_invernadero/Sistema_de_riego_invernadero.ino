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
  
}
