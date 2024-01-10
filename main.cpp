#include <Arduino.h>

#include <ESP32Servo.h>

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
//motor
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

}

void loop()
{

  Signal = analogRead(gas);
  //Mostramos la señal del sensor de gas
  Serial.println(Signal); //muestra el valor del sensor de gas
  delay(1000);

  int pwm_value = map(Signal, 1700, 2200, 90, 190);

  if(Signal < 1720) //calidad del aire buena
  {
    digitalWrite(pinVerde, HIGH);
    digitalWrite(pinAmarillo, LOW);
    digitalWrite(pinRojo, LOW);
    digitalWrite(sonido,LOW);
    servo_1.write(0);
    ledcWrite(canalPWM1, 0);
    ledcWrite(canalPWM2, 0);
    
  }

  if ((Signal > 1720) and (Signal < 1730)) //calidad del aire regular
  {
    digitalWrite(pinVerde, LOW);
    digitalWrite(pinAmarillo, HIGH);
    digitalWrite(pinRojo, LOW);
    digitalWrite(sonido,LOW);
    servo_1.write(0);
    ledcWrite(canalPWM1, pwm_value);

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

}
