/*
O seguinte programa utiliza um ESP32, juntamente com um relê e um display OLED
para controlar de maneira remota (via website) um inversor de frequência WEG CFw500

O CFW500 conta com um módulo de entradas e saídas analógicas e digitais
Por meio de uma entrada digital, e utilizando um relê, controlamos a função de start/stop do motor
Por meio de uma entrada analógica, e utilizando uma saída digital com PWM do ESP32, controlamos
a velocidade do motor.
*/

// Bibliotecas e constantes para conexão WIFI
#include <WiFi.h>
const char* ssid     = "Caio"; // Nome da rede WIFI
const char* password = "12345678"; // Senha da rede WIFI
WiFiServer server(80);

// Bibliotecas e definições para display OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_RESET -1

Adafruit_SH1106G tela(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// INC, UD e CS são para controle de velocidade com potenciometro eletronico
// O novo modelo de controle não utiliza o potenciometro eletronico, mas sim as portas analógicas do ESP32
//#define INC   18
//#define UD    5
//#define CS    19

// Pino de controle do Relê, que controla o START/STOP do motor 
#define RELE  26

// Definição do PWM para controle de velocidade
int pwmChannel = 0; // Selects channel 0
int frequence = 1000; // PWM frequency of 1 KHz
int resolution = 8; // 8-bit resolution, 256 possible values
int pwmPin = 25;
int pwm = 0;


void setup()
{
    tela.begin(SCREEN_ADDRESS, true);
    tela.clearDisplay();
    tela.setTextSize(2);
    tela.setTextColor(SH110X_WHITE);
    tela.setCursor(0, 0);
    tela.print("Iniciando ...");
    tela.display();
    ledcSetup(pwmChannel, frequence, resolution);
    ledcAttachPin(pwmPin, pwmChannel);

    //pinMode(INC, OUTPUT);
    //pinMode(UD, OUTPUT);
    //pinMode(CS, OUTPUT);
    //digitalWrite(CS, HIGH);
    
    Serial.begin(115200);
    pinMode(RELE, OUTPUT);      

    delay(10);

    // We start by connecting to a WiFi network
    tela.clearDisplay();
    tela.setTextSize(2);
    tela.setTextColor(SH110X_WHITE);
    tela.setCursor(0, 0);
    tela.println("Conectando a:");
    tela.println(ssid);
    tela.display();
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    tela.clearDisplay();
    tela.setTextSize(2);
    tela.setTextColor(SH110X_WHITE);
    tela.setCursor(0, 0);
    tela.println("Wifi conectado:");
    tela.println("Endereço IP: ");
    tela.print(WiFi.localIP());
    tela.display();
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

}

void loop(){
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Aperte <a href=\"/START\">aqui</a> para ligar o motor.<br>");
            client.print("<br>");
            client.print("Aperte <a href=\"/STOP\">aqui</a> para desligar o motor.<br>");
            client.print("<br>");
            client.print("Aperte <a href=\"/VEL+\">aqui</a> para aumentar a velocidade.<br>");
            client.print("<br>");
            client.print("Aperte <a href=\"/VEL-\">aqui</a> para diminuir a velocidade.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /START")) {
          digitalWrite(RELE, LOW);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /STOP")) {
          digitalWrite(RELE, HIGH);                // GET /L turns the LED off
        }
        if (currentLine.endsWith("GET /VEL+")) {
          for(int i = 0; i <= 2; i++) {
           pwm = constrain(pwm + 10, 0, 255);;
           ledcWrite(pwmChannel, pwm); 
          }
          
        }
       if (currentLine.endsWith("GET /VEL-")) {
        for(int i = 0; i <= 2; i++) {
           pwm = constrain(pwm - 10, 0, 255);
           ledcWrite(pwmChannel, pwm);
        }
          
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
