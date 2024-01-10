#include <Arduino.h>
#include <ESP32Servo.h>

#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/RTDBHelper.h>
/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"
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
int pinServo = 26;
int Signal = 0;
int motor1 = 18;
int motor12 = 19;
int motor2 = 17;
int motor22 = 16;
// Setting PWM properties
const int freq = 30000;
const int canalPWM1 = 0;
const int canalPWM2 = 1;
const int canalPWM3 = 0;
const int canalPWM4 = 1;
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
String modo = "";

void setup()
{
  Serial.begin(115200);

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
  ledcAttachPin(motor12, canalPWM2);
  ledcAttachPin(motor2, canalPWM3);
  ledcAttachPin(motor22, canalPWM4);

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
  //Mostramos la señal del sensor de gas
  Serial.println(Signal); //muestra el valor del sensor de gas
  delay(1000);

  int pwm_value = map(Signal, 1700, 2200, 90, 190);

  valor = map(Signal, 1700, 2200, 0, 100);
  porcentual = map(valor, 0, 100, 100, 0);
  char porcen = porcentual;
  Firebase.setString(fbdo,"/ProyectoSEV2/porcentaje", porcen);

  //ESTADOS


// Modo automatico
    if(Signal < 1720) //calidad del aire buena
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

    if ((Signal > 1720) and (Signal < 1730)) //calidad del aire regular
    {
        digitalWrite(pinVerde, LOW);
        digitalWrite(pinAmarillo, HIGH);
        digitalWrite(pinRojo, LOW);
        digitalWrite(sonido,LOW);
        servo_1.write(0);
        ledcWrite(canalPWM1, pwm_value);
        Firebase.setString(fbdo,"/ProyectoSEV2/calidad","Regular");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadopuerta","cerrado");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoextractor","desactivado");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoventilador","desactivado");
    }

    if(Signal > 2000) //calidad del aire mala
    {
        digitalWrite(pinVerde, LOW);
        digitalWrite(pinAmarillo, LOW);
        digitalWrite(pinRojo, HIGH);
        servo_1.write(90); //abre la puerta cuando exite mala calidad de mala
        digitalWrite(sonido, lastState);
        //delay(1000);
        ledcWrite(canalPWM1, pwm_value);
        ledcWrite(canalPWM4, pwm_value);
        Firebase.setString(fbdo,"/ProyectoSEV2/calidad","Mala");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadopuerta","abierto");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoextractor","activado");
        Firebase.setString(fbdo,"/ProyectoSEV2/estadoventilador","activado");
    
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
  
  //Modo manual

}
