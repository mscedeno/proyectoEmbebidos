#include <Arduino.h>
#include <ESP32Servo.h>
#include "memoria.h" //se aloja las funciones para escribir/leer en la memoria eeprom
//#include <EEPROM.h>

#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/RTDBHelper.h>
/* 1. Define the WiFi credentials */
#define WIFI_SSID "SOLANGE-YIGA5-2.4G"
#define WIFI_PASSWORD "Solang1720"
/* 2. Define the RTDB URL */
#define DATABASE_URL "fir-se-ceabc-default-rtdb.firebaseio.com"

//Variables modo automatico
int gas = 36;
Servo servo_1;
int boton=15;

int pinVerde = 27;
int pinAmarillo = 25;
int pinRojo = 33;

int sonido = 21;
int lastState = HIGH;
int lastState2 = HIGH;

int pinServo = 26;
int Signal = 0;

int motor1 = 19;
int motor2 = 16;

// Setting PWM properties
const int freq = 30000;
const int canalPWM1 = 0;
const int canalPWM2 = 1;
const int resolution = 8;

//variables modo manual
/* 3. Define the Firebase Data object */
FirebaseData fbdo;
/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;
/* Define the FirebaseConfig data for config data */
FirebaseConfig config;
int valor = 0;
int porcentual = 0;
String porcen = "";

String Nmodo = "0";
String Npuerta = "0";
String Nextractor = "0";
String Nventilador = "0";

int pos=50;
int pos2=70;
String usuario="usuario123";
String contra="clave123";

void setup()
{
    Serial.begin(115200);

    EEPROM.begin(512);  //inicializar el tamano de memoria eeprom a usar
    escribirStringEnEEPROM(usuario, pos);
    escribirStringEnEEPROM(contra, pos2);

    Serial.println("la lectura del espacio de la eeprom 50 es: ");
    Serial.println(leerStringDeEEPROM(pos));

  pinMode(pinVerde, OUTPUT);
  pinMode(pinRojo, OUTPUT);
  pinMode(pinAmarillo, OUTPUT);
  pinMode(boton, INPUT_PULLUP);
  pinMode(sonido, OUTPUT);

  // Configurar la señal PWM
  ledcSetup(canalPWM1, freq, resolution);
  ledcSetup(canalPWM2, freq, resolution);

  // Asignar el pin al canal PWM
  ledcAttachPin(motor1, canalPWM1);
  ledcAttachPin(motor2, canalPWM2);

  servo_1.attach(pinServo, 500, 2500); 
  servo_1.write(0);

  //modo wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.database_url = DATABASE_URL;
    config.signer.test_mode = true;
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096 , 1024 );
    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);

}

void loop()
{
  //Datos importantes
  Signal = analogRead(gas);
  Serial.println(Signal); //muestra el valor del sensor de gas
  delay(600);

  int pwm_value = map(Signal, 200, 900, 90, 190);

  valor = map(Signal, 0, 900, 0, 100);
  porcentual = map(valor, 0, 100, 100, 0);
  porcen = porcentual;
  Firebase.setString(fbdo,"/ProyectoSEV2/porcentaje", porcen);

  //ESTADOS
  Firebase.getString(fbdo,"/ProyectoSEV2/Npuerta");
  Npuerta = fbdo.stringData();
  
  Firebase.getString(fbdo,"/ProyectoSEV2/Nextractor");
  Nextractor = fbdo.stringData();

  Firebase.getString(fbdo,"/ProyectoSEV2/Nventilador");
  Nventilador = fbdo.stringData();

  //MODOS
  Firebase.getString(fbdo,"/ProyectoSEV2/Nmodo");
  Nmodo = fbdo.stringData();
  
  
  // Modo automatico

  if (Nmodo.equals("0")){
    if(Signal < 300) //calidad del aire buena
    {
        digitalWrite(pinVerde, HIGH);
        digitalWrite(pinAmarillo, LOW);
        digitalWrite(pinRojo, LOW);
        digitalWrite(sonido,LOW);
        servo_1.write(0);
        ledcWrite(canalPWM1, 0);
        ledcWrite(canalPWM2, 0);
        Firebase.setString(fbdo,"/ProyectoSEV2/calidad","Buena");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadopuerta","cerrado");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoextractor","desactivado");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoventilador","desactivado");   
    }

    else if ((Signal > 300) and (Signal < 600)) //calidad del aire regular
    {
        digitalWrite(pinVerde, LOW);
        digitalWrite(pinAmarillo, HIGH);
        digitalWrite(pinRojo, LOW);
        delay(25);
        digitalWrite(sonido,LOW);
        servo_1.write(0);
        ledcWrite(canalPWM1, pwm_value);
        Firebase.setString(fbdo,"/ProyectoSEV2/calidad","Regular");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadopuerta","cerrado");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoextractor","activado");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoventilador","activado");
        digitalWrite(pinAmarillo, LOW);

    }

    else if(Signal > 600) //calidad del aire mala
    {
        digitalWrite(pinVerde, LOW);
        digitalWrite(pinAmarillo, LOW);
        digitalWrite(pinRojo, HIGH);
        delay(15);
        servo_1.write(90); //abre la puerta cuando exite mala calidad de mala
        digitalWrite(sonido, lastState);
        delay(100);
        ledcWrite(canalPWM1, pwm_value);
        ledcWrite(canalPWM2, pwm_value);
        Firebase.setString(fbdo,"/ProyectoSEV2/calidad","Mala");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadopuerta","abierto");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoextractor","activado");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoventilador","activado");
        digitalWrite(pinRojo, LOW);
    
    }

    int buttonState = digitalRead(boton); // read new state
    Serial.println(buttonState);
    if (buttonState == LOW) {
        Serial.println("The button is being pressed");
        lastState = LOW;
        digitalWrite(sonido, lastState); // turn on
    }
    int currentState = digitalRead(sonido); //estado actual
    lastState = currentState;
    delay(30);
  }
  
  //Modo manual


  if (Nmodo.equals("1")){

    if(Signal < 300) //calidad del aire buena
    {
        digitalWrite(pinVerde, HIGH);
        servo_1.write(0);
        Firebase.setString(fbdo,"/ProyectoSEV2/calidad","Buena");

        if(Nventilador.equals("1")){
            ledcWrite(canalPWM1, 100);
        }
        else if(Nextractor.equals("1")){
            ledcWrite(canalPWM2, 100);
        }
        else if(Npuerta.equals("1")){
            servo_1.write(90);
        }
        else if(Nventilador.equals("0")){
            ledcWrite(canalPWM1, 0);
        }
        else if(Nextractor.equals("0")){
            ledcWrite(canalPWM2, 0);
        }
        else if(Npuerta.equals("0")){
            servo_1.write(0);
        }
    }
    else if ((Signal > 300) and (Signal < 600)) //calidad del aire regular
    {
        digitalWrite(pinVerde, LOW);
        digitalWrite(pinAmarillo, HIGH);
        digitalWrite(pinRojo, LOW);
        digitalWrite(sonido,LOW);
        servo_1.write(0);
        Firebase.setString(fbdo,"/ProyectoSEV2/calidad","Regular");

        if(Nventilador.equals("1")){
            ledcWrite(canalPWM1, 100);
        }
        else if(Nextractor.equals("1")){
            ledcWrite(canalPWM2, 100);
        }
        else if(Npuerta.equals("1")){
            servo_1.write(90);
        }
        else if(Nventilador.equals("0")){
            ledcWrite(canalPWM1, 0);
        }
        else if(Nextractor.equals("0")){
            ledcWrite(canalPWM2, 0);
        }
        else if(Npuerta.equals("0")){
            servo_1.write(0);
        }
    }
    else if(Signal > 600) //calidad del aire mala
    {
        digitalWrite(pinVerde, LOW);
        digitalWrite(pinAmarillo, LOW);
        digitalWrite(pinRojo, HIGH);
        digitalWrite(sonido, lastState2);
        Firebase.setString(fbdo,"/ProyectoSEV2/calidad","Mala");

        if(Nventilador.equals("1")){
            ledcWrite(canalPWM1, 100);
        }
        else if(Nextractor.equals("1")){
            ledcWrite(canalPWM2, 100);
        }
        else if(Npuerta.equals("1")){
            servo_1.write(90);
        }
        else if(Nventilador.equals("0")){
            ledcWrite(canalPWM1, 0);
        }
        else if(Nextractor.equals("0")){
            ledcWrite(canalPWM2, 0);
        }
        else if(Npuerta.equals("0")){
            servo_1.write(0);
        }
    }
    int buttonState = digitalRead(boton); // read new state
    Serial.println(buttonState);
    if (buttonState == LOW) {
        Serial.println("The button is being pressed");
        lastState2 = LOW;
        digitalWrite(sonido, lastState2); // turn on
    }
    int currentState2 = digitalRead(sonido); //estado actual
    lastState2 = currentState2;
  }
}
