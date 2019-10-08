#include <Servo.h>

// Se define los Pines para el Ultrasonido
#define pinTrigger A0
#define pinEcho A1

// Se define los Pines para los LEDs
#define pinLedVerde 11
#define pinLedRojo 4

//Se define todo el mapa de conecciones para el L298N
#define enA 10
#define in1 8
#define in2 9
#define in3 6
#define in4 7
#define enB 5
#define servoPin 2

//Se crea una clase Motor en donde se define sus 3 variables de control para cada motor creado
class Motor
{
    int enablePin;
    int direccionPin1;
    int direccionPin2;

public:
    //Metodo que define los pines del Motor
    Motor(int enPin, int dPin1, int dPin2)
    {
        enablePin = enPin;
        direccionPin1 = dPin1;
        direccionPin2 = dPin2;
    };
    //Metodo para definir el avance del motor (0-255 avanza, -1-255 retrocede)
    //Si direccionPin1 es LOW y direccionPin2 es HIGH el motor gira en un sentido
    //Si direccionPin1 es HIGH y direccionPin2 es LOW el motor gira en el sentido contrario
    //Si direccionPin1 es LOW y direccionPin2 es LOW el motor no gira
    void Conducir(int velocidad)
    {
        if (velocidad >= 0)
        { // Avanzar
            digitalWrite(direccionPin1, LOW);
            digitalWrite(direccionPin2, HIGH);
        }
        else
        { // Retroceder
            digitalWrite(direccionPin1, HIGH);
            digitalWrite(direccionPin2, LOW);
            velocidad = -velocidad;
        }
        analogWrite(enablePin, velocidad); // Regular velocidad del motor
    }
};
// Se crea 2 Clases Motor y declara con las conecciones del L298N
Motor motorIzquierda = Motor(enA, in1, in2);
Motor motorDerecha = Motor(enB, in3, in4);
// Se crea el objeto del tipo Servo para controlarlo.
Servo myservo;
// Se define un tipo de dato Direcciones
enum Direcciones
{ 
    Avanzar,
    GiroIzquierda,
    GiroDerecha,
    DarVuelta,
    Frenar
};
// Se define y establece variables gloables
Direcciones pasoSiguiente = Avanzar;
const unsigned int MAX_DIST = 23200; // Todo sobre los 400 cm (23200 us pulse) esta "fuera del rango"
bool obstaculo = false;              // Se inicia sin obstaculos
int servoPos = 90;                   // Se inicia en 90 grados
unsigned long anchoPulso;
float cm;

// funcion para inicializar variables del Motor
void inicializarMotor()
{
    pinMode(enA, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(enB, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    // Se establece la direccion y velocidad inicial
    digitalWrite(enA, LOW);
    digitalWrite(enB, LOW);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
}
// funcion para inicializar variables del Ultrasonido
void inicializarUltrasonido()
{
    pinMode(pinTrigger, OUTPUT);
    digitalWrite(pinTrigger, LOW);
}
// funcion para inicializar variables de los LEDs
void inicializarLeds()
{
    pinMode(pinLedVerde, OUTPUT);
    digitalWrite(pinLedVerde, LOW);
    pinMode(pinLedRojo, OUTPUT);
    digitalWrite(pinLedRojo, LOW);
}

//SETUP--------------------------------------------------------------------------
void setup()
{
    Serial.begin(9600);
    inicializarUltrasonido();
    inicializarLeds();
    inicializarMotor();
    myservo.attach(servoPin);
    Direcciones pasoSiguiente = Avanzar;
}

// MAIN LOOP ---------------------------------------------------------------------
void loop()
{
    verificarDistancia();
    verificarDireccion();
    Conducir();
}

// FUNCIONES ---------------------------------------------------------------------

// Funcion que verifica la distancia que mide el ultrasonido
void verificarDistancia()
{

    digitalWrite(pinTrigger, HIGH);      // Activa el trigger
    delayMicroseconds(10);               // Mantiene el Trigger activo por 10ms
    digitalWrite(pinTrigger, LOW);       // Desactiva el trigger
    anchoPulso = pulseIn(pinEcho, HIGH); // Mide cuanto tiempo el pin Echo estuvo activo (ancho del pulso en ms)
    cm = anchoPulso / 58.2;              // Calcula la distancia en centimetros (se asume que la velocidad del sonido en el aire a nivel del mar es ~340 m/s)
    if (anchoPulso > MAX_DIST)
    { // Imprime los resultados
        Serial.println("Out of range");
    }
    else
    {
        Serial.print(cm);
        Serial.print(" cm \t");
    }

    delay(60); // Espera al menos 60ms para la siguiente medicion
    if (cm <= 20)
    { // Si la distancia es menor a 20 cm establece que hay un obstaculo
        obstaculo = true;
        Serial.println("Problema adelante");
    }
    else
    { // De ser mayor a 20 cm establece que no hay un obstaculo
        obstaculo = false;
    }
}

// Funcion que verifica la direccion hacia donde girar
void verificarDireccion()
{
    Serial.println("Verificar direccion");
    if (obstaculo == true)
    { // si al frente esta bloqueado, se revisa a la izquierda
        pasoSiguiente = Frenar;
        Conducir();           // llama a la funcion Conducir con la orden de frenar
        myservo.write(180);   // le indica ql servo que vaya a la posicion 180 grados (izquierda)
        delay(500);           // espera medio segundo para que el servo llegue a su posicion
        verificarDistancia(); // verifica la distancia
        if (obstaculo == false)
        { // si a la izquierda esta libre, girara hacia esa direccion
            pasoSiguiente = GiroIzquierda;
            Serial.println("Siguiente paso es GiroIzquierda");
            myservo.write(90); // restablece la posicion del servo en 90 grados (en frente)
            delay(500);        // espera medio segundo para que el servo llegue a su posicion
        }
        else
        {                         // si a la izquierda esta bloqueado, se revisa a la derecha
            myservo.write(0);     // le indica ql servo que vaya a la posicion 0 grados (derecha)
            delay(1000);          // espera un segundo para que el servo llegue a su posicion
            verificarDistancia(); // verifica la distancia
            if (obstaculo == false)
            { // si a la derecha esta libre, girara hacia esa direccion
                pasoSiguiente = GiroDerecha;
                Serial.println("Siguiente paso es GiroDerecha");
                myservo.write(90); // restablece la posicion del servo en 90 grados (en frente)
                delay(500);        // espera medio segundo para que el servo llegue a su posicion
            }
            else
            { // si a la derecha tambien esta bloqueado, debera dar media vuelta
                pasoSiguiente = DarVuelta;
                myservo.write(90); // restablece la posicion del servo en 90 grados (en frente)
                delay(500);        // espera medio segundo para que el servo llegue a su posicion
                Serial.println("Siguiente paso es dar la vuelta");
            }
        }
    }
    else
    { //Sin obstaculo adelante
        pasoSiguiente = Avanzar;
    }
}

// Funcion que acciona los motores DC conectados
void Conducir()
{
    switch (pasoSiguiente)
    {
    case Avanzar: // Enciende el Led Verde, y pone en marcha los 2 motores
        digitalWrite(pinLedVerde, HIGH);
        motorIzquierda.Conducir(255); // motor izquierda hacia adelante
        motorDerecha.Conducir(255);   // motor derecha hacia adelante
        Serial.println("Avanzar");
        break;

    case GiroIzquierda: // Parpadea el Led Verde, y pone en marcha el motor derecho y en reversa el izquierdo hasta girar 90 grados
        led(2, pinLedVerde);
        motorIzquierda.Conducir(-255);           // motor izquierda en reversa
        motorDerecha.Conducir(255);              // motor derecha hacia adelante
        Serial.println(" Girar a la Izquierda"); //
        delay(300);                              //
        motorIzquierda.Conducir(0);              // motor izquierda detenido
        motorDerecha.Conducir(0);                // motor derecha detenido
        delay(50);
        break;

    case GiroDerecha: // Parpadea el Led Rojo, y pone en marcha el motor izquierdo y en reversa el derecho hasta girar 90 grados
        led(2, pinLedRojo);
        motorIzquierda.Conducir(255);          // motor izquierda hacia adelante
        motorDerecha.Conducir(-255);           // motor derecha en reversa
        Serial.println(" Girar a la Derecha"); //
        delay(300);                            //
        motorIzquierda.Conducir(0);            // motor izquierda detenido
        motorDerecha.Conducir(0);              // motor derecha detenido
        delay(50);
        break;

    case DarVuelta: // Parpadea ambos Leds, y pone en reversa ambos motores y luego pone en marcha el motor izquierdo y en reversa el derecho hasta girar 180 grados
        led(2, pinLedVerde, pinLedRojo);
        motorIzquierda.Conducir(-255);             // motor izquierda en reversa
        motorDerecha.Conducir(-255);               // motor derecha en reversa
        delay(250);                                //
        motorIzquierda.Conducir(255);              // motor izquierda hacia adelante
        motorDerecha.Conducir(-255);               // motor derecha en reversa
        Serial.println(" Dar la vuelta completa"); //
        delay(600);                                //
        motorIzquierda.Conducir(255);              // motor izquierda hacia adelante
        motorDerecha.Conducir(255);                // motor derecha hacia adelante
        delay(250);
        break;

    case Frenar: // Enciende el Led Rojo, y detiene los 2 motores
        digitalWrite(pinLedVerde, LOW);
        digitalWrite(pinLedRojo, HIGH);
        motorIzquierda.Conducir(0); // motor izquierda detenido
        motorDerecha.Conducir(0);   // motor derecha detenido
        Serial.println("Detenido");
    }
}

// funcion que permite parpadear el led del lado donde va girar
void led(int numParpadeos, int led1)
{
    digitalWrite(pinLedVerde, LOW);
    digitalWrite(pinLedRojo, LOW);
    delay(200);
    for (int t = 1; t <= numParpadeos; t++)
    { // bucle que permite parpadear un led
        digitalWrite(led1, HIGH);
        delay(200);
        digitalWrite(led1, LOW);
        delay(200);
    }
}

// funcion que permite parpadear ambos led para indicar que dara media vuelta
void led(int numParpadeos, int led1, int led2)
{
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    delay(200);
    for (int t = 1; t <= numParpadeos; t++)
    { // bucle que permite parpadear ambos led
        digitalWrite(led1, HIGH);
        digitalWrite(led2, HIGH);
        delay(200);
        digitalWrite(led1, LOW);
        digitalWrite(led2, LOW);
        delay(200);
    }
}