//Bibliotecas:
#include <DHT.h> //DHT
#include <Adafruit_GFX.h> //DISPLAY
#include <Adafruit_SSD1306.h> //DISPLAY
#include <Servo.h> //SERVO MOTOR

//Pinos:
//Sensores
const int pinDHT = 3;
const char pinNivel = A0;
const char pinLDR = A1;

//Servo Motor
const int pinSer = 11;
Servo ser; //Objeto do Servo

//LEDs
const int pinLN = 8; //Led Nivel d'água
const int pinLUmi = 9; //Led Umidade 
const int pinLRol = 10; //Led Rolagem
const int pinLO = 13; //Led Ovoscópio
const char pinLLDR = A2; //Led sensor LDR

//interruptores/botões
const int pinRele = 2; //Relé
const int pinInO = 12; //Interruptor Ovoscópio
const char pinInR = A3;
const int confir = 4; //Botão para confirmar configurações
const int cima = 5; //Botão para somar
const int baixo = 6; //Botão para subtrair
const int mudar = 7; //Botão para Mudar configurações

//váriaveis:
//interruptores/botões
int cimaEstado;
int baixoEstado;
int confirEstado;
int mudarEstado;
int interruptorOvo;
int interruptorRol;

//sensores
int valorLDR;
int leituraNivel;
int umidade;
float temperatura;

//Posição Servo Motor
int pos = 0;

//tela
int tela = 0;

//variáveis principais
float grau = 38;
int umidadeMin = 50;
int umidadeMax = 55;
unsigned long ultimaRol = 0; //marca quando foi a última rolagem
unsigned long intervaloRol = 10000; //o intervalo (4 horas)
unsigned long tempoRol; //o tempo em milisegundos
bool Click = false;

//coisas de biblioteca
#define DHTVERSAO DHT22
DHT dht(pinDHT, DHTVERSAO);
Adafruit_SSD1306 display = Adafruit_SSD1306();



void setup() {

  //INPUT:
  pinMode(pinDHT, INPUT);
  pinMode(pinLDR, INPUT);
  pinMode(pinNivel, INPUT);

  //INPUT_PULLUP:
  pinMode(confir, INPUT_PULLUP);
  pinMode(cima, INPUT_PULLUP);
  pinMode(baixo, INPUT_PULLUP);
  pinMode(mudar, INPUT_PULLUP);
  pinMode(pinInO, INPUT_PULLUP);
  pinMode(pinInR, INPUT_PULLUP);

  //OUTPUT:
  pinMode(pinLUmi, OUTPUT);
  pinMode(pinLRol, OUTPUT);
  pinMode(pinLO, OUTPUT);
  pinMode(pinLLDR, OUTPUT);
  pinMode(pinRele, OUTPUT);
  pinMode(pinLN, OUTPUT);

  //BIBLIOTECAS:
  dht.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //inicia o display com endereço I2C 0x3C
  display.setTextColor(WHITE); //define cor 
  display.clearDisplay(); //limpa as informações do display

  ser.attach(pinSer); //associar o pino do Servo ao objeto
  ser.write(pos);

}

void loop() {
  //Tempo Rolagem
  tempoRol = millis();

  //DHT11:
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();

  //Botões e Interruptor:
  cimaEstado = digitalRead(cima);
  baixoEstado = digitalRead(baixo);
  confirEstado = digitalRead(confir);
  mudarEstado = digitalRead(mudar);
  interruptorOvo = digitalRead(pinInO);
  interruptorRol = analogRead(pinInR);

  //Sensores:
  valorLDR = analogRead(pinLDR);
  leituraNivel = analogRead(pinNivel);

  //Ovoscópio:
  if(interruptorOvo == 0){
    digitalWrite(pinLO, HIGH);
  }else{
    digitalWrite(pinLO, LOW);
  }

  //Confirmar se o interruptor está clicado
  if(Click){
    ultimaRol = tempoRol;
    Click = false;
  }

  if(tempoRol == 0){
    ultimaRol = tempoRol;
  }

  //interruptor da rolagem de ovos
  if(interruptorRol < 100){
    digitalWrite(pinLRol, HIGH);

    if(tempoRol - ultimaRol >= intervaloRol){
      
      if(pos == 0){
        pos = 90;
      }else{
        pos = 0;
      }
      ultimaRol = tempoRol;
      ser.write(pos);

    }
  }else{
    digitalWrite(pinLRol, LOW);
    Click = true;
  }

  //Tela principal:
  if(tela == 0){
    
    //Caso aperte o botão de editar configurações
    if(mudarEstado == 0){
        tela = 1;
    }

    //Display mostrando Umidade e Temperatura
    display.clearDisplay(); //Limpa o Display
    display.setTextSize(2); //define o tamanho da fonte
    display.drawRect(0, 0, 128, 32, 1); //dessenha um retângulo
    display.setCursor(44, 1); //posição do texto no display
    display.print(temperatura, 1); //escreve o texto de temperatura no display
    display.setCursor(44, 16);
    display.print(umidade); //escreve o texto da Umidade no display
    display.setCursor(72, 16); 
    display.print("%"); //escreve o "%" no display
    display.setCursor(104, 1);
    display.print("C"); //escreve o "C" no display
    display.setCursor(14, 16); 
    display.print("U"); //escreve o "U" no display
    display.display(); //permite a mudança no display

    //Verifica se a temperatura do DHT é maior (ou menor) que a temperatura máxima
    if(temperatura > grau){
      digitalWrite(pinRele, HIGH);
    }else if(temperatura < grau){
      digitalWrite(pinRele, LOW);
    }

    //Verifica o Nível d'água
    if(leituraNivel > 950){
      digitalWrite(pinLN, HIGH);
    } else if(leituraNivel < 949){
      digitalWrite(pinLN, LOW);
    }

    //Verifica se a umidade é Maior que a porcentagem máxima, ou menor que a mínima
    if(umidade < umidadeMin || umidade > umidadeMax){
      digitalWrite(pinLUmi, HIGH);
    } else{
      digitalWrite(pinLUmi, LOW);
    }

    //Verifica se a lampâda queimou (aonde o fotorresistor fica a menos que 500) ou não
    if(valorLDR < 500 && temperatura < grau){
      analogWrite(pinLLDR, 254);
    }else{
      analogWrite(pinLLDR, 0);
    }

  //Tela para mudar a temperatura máxima
  }else if(tela == 1){
    display.clearDisplay();
    display.setTextSize(1); 
    display.drawRect(0, 0, 128, 32, 1);
    display.setCursor(10, 2); 
    display.print("definir variaveis");
    display.setTextSize(2);
    display.setCursor(35, 12);
    display.print(grau, 1); 
    display.setCursor(93, 12); 
    display.print("C"); 
    display.display(); 

    //somar ou subtrair 0.5 da temperatura máxima
    if(cimaEstado == 0){
      grau += 0.5;
    }else if(baixoEstado == 0){
      grau -= 0.5;
    }

    //Caso confirmar ele vai para a tela de Umidade Mínima
    if(confirEstado == 0){
      tela = 2;
    }

  //tela para mudar o intervalo da rolagem dos ovos
  }else if (tela == 2){
    
    display.clearDisplay();
    display.setTextSize(1); 
    display.drawRect(0, 0, 128, 32, 1);
    display.setCursor(10, 2); 
    display.print("definir variaveis"); 
    display.setTextSize(1);
    display.setCursor(22, 12); 
    display.print(intervaloRol/60000); //transforma em minutos
    display.setCursor(52, 12); 
    display.print("Minutos");  
    display.setCursor(16, 23); 
    display.print("rolagem de ovos"); 
    display.display(); 

    //somar ou subtrair 1 minuto (60000 milisegundos) do intervalo
    if(cimaEstado == 0){
      intervaloRol += 60000;
    }else if(baixoEstado == 0){
      intervaloRol -= 60000;
    }

    //Caso confirmar ele vai para a tela de Umidade Minima
    if(confirEstado == 0){
      tela = 3;
    }

  //Tela para mudar a Umidade Mínima
  }else if(tela == 3){
    display.clearDisplay();
    display.setTextSize(1); 
    display.drawRect(0, 0, 128, 32, 1);
    display.setCursor(10, 2); 
    display.print("definir variaveis"); 
    display.setTextSize(2);
    display.setCursor(32, 12); 
    display.print(umidadeMin); 
    display.setCursor(63, 12); 
    display.print("U.min"); 
    display.display(); 

    //somar ou subtrair 1 da Umidade Mínima
    if(cimaEstado == 0){
      umidadeMin += 1;
    }else if(baixoEstado == 0){
      umidadeMin -= 1;
    }

    //Caso confirmar ele vai para a tela de Umidade Maxima
    if(confirEstado == 0){
      tela = 4;
    }

  //Tela para mudar Umidade Máxima
  }else if(tela == 4){
    display.clearDisplay();
    display.setTextSize(1); 
    display.drawRect(0, 0, 128, 32, 1);
    display.setCursor(10, 2); 
    display.print("definir variaveis"); 
    display.setTextSize(2);
    display.setCursor(32, 12); 
    display.print(umidadeMax); 
    display.setCursor(63, 12); 
    display.print("U.max"); 
    display.display(); 

    //somar ou subtrair 1 da Umidade Maxima
    if(cimaEstado == 0){
      umidadeMax += 1;
    }else if(baixoEstado == 0){
      umidadeMax -= 1;
    }

    //Caso confirmar ele vai verificar se a Umidade Minima está maior que a Umidade Maxima, caso isso ocorra ele vai dar um aviso
    if(confirEstado == 0){
      if(umidadeMin > umidadeMax){
        display.clearDisplay();
        display.setTextSize(1); 
        display.drawRect(0, 0, 128, 32, 1);
        display.setCursor(6, 2); 
        display.print("ERRO! Umidade minima maior que umidade    maxima permitida"); 
        display.display(); 
        delay(8000); //vai exibir a mensagem por 8 segundos

        tela = 2; //e vai voltar pra tela de Umidade Mínima
      }else{
        tela = 0; //caso o contrário ele vai pra tela normal com tudo salvo
      }
    }
  }

}