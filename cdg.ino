#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

//MP3 - MUDAR O 10, 11 
//Inicia a serial por software nos pinos 10 e 11
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX

DFRobotDFPlayerMini myDFPlayer;

//-------------SINAL-------
#define LED_VERDE 7
#define LED_AMARELO 6
#define LED_VERMELHO 4


//RFID - Leitor do cartão

 // --- Bibliotecas Auxiliares ---
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>


// --- Mapeamento de Hardware ---
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Cria instância com MFRC522
 

// --- Variáveis Globais --- 
char st[20];

//////////////////////////////////////////////////////////////////////

void setup() {
 
  Serial.begin(9600);   // Inicia comunicação Serial em 9600 baud rate
 
  //DFPlay mini mp3
  mySoftwareSerial.begin(9600);

  //Verifica se o modulo esta respondendo e se o
  //cartao SD foi encontrado
  Serial.println(F("Inicializando modulo DFPlayer... (3~5 segundos)"));
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Nao inicializado:"));
    Serial.println(F("1.Cheque as conexoes do DFPlayer Mini"));
    Serial.println(F("2.Insira um cartao SD"));
    while (true);
  }
  Serial.println();
  Serial.println(F("Modulo DFPlayer Mini inicializado!"));


  //-----------------------------------------------------------------------------------------
   //RFID
  Wire.begin();
  SPI.begin();          // Inicia comunicação SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522
  //-----------------------------------------------------------------------------------------
  

  //Definicoes iniciais
  myDFPlayer.setTimeOut(500); //Timeout serial 500ms
  myDFPlayer.volume(100); //Volume 5
  myDFPlayer.EQ(0); //Equalizacao normal
  
  //-----------------------------------------------------------------------------------------
    //Sinal
    pinMode(7,OUTPUT); 
    pinMode(6,OUTPUT);    
    pinMode(4,OUTPUT);   
  //-----------------------------------------------------------------------------------------
}


#define TEMPO_AMARELO 200
#define TEMPO_NORMAL 400
#define TEMPO_IDOSO 400
#define TEMPO_DEFICIENTE 300

int tempoExtra = 0;
int tempo = TEMPO_NORMAL;
int numLed = 1;//1 - verde, 2 - amarelo, 3 - vermelho
void loop() {

  if (tempoExtra == 0){
    tempoExtra = rfid();
  if (tempoExtra > 0){
    identificado();
    delay(3000);
    }
  if (tempoExtra > 0){
      espere();
      delay(3000);
    }
  }
  
  if (numLed == 3 && tempo == (TEMPO_NORMAL+TEMPO_IDOSO)-100){
    pode_atravessar();
    delay(3000);
    contagem();
  }

  
  if (numLed == 1){
    led(false, LED_VERMELHO);
    led(true, LED_VERDE);
  } else 
  if (numLed == 2){
    led(false, LED_VERDE);
    led(true, LED_AMARELO);
  } else {
    led(false, LED_AMARELO);
    led(true, LED_VERMELHO);
  }

  Serial.print(numLed);
  Serial.print(" tempo: ");
  Serial.print(tempo/100);
  Serial.print(" tempoExtra: ");
  Serial.print(tempoExtra/100);
  Serial.println();

  if (tempo > 0){
    tempo--;
  } else {
    numLed++;
    tempo = TEMPO_NORMAL;
    if (numLed == 3){
      tempo += tempoExtra;
    } else 
    if (numLed == 2){
      tempo = TEMPO_AMARELO;
    } else
    if (numLed == 4){
      numLed = 1;
      tempoExtra = 0;
    }
  }

}
///////////////////////////- Cód DFPlayer mini -////////////////////////////////////////////
void identificado(){
  myDFPlayer.play(1);
}
void contagem(){
  myDFPlayer.play(2);
}
void espere(){
  myDFPlayer.play(3);
}
void pode_atravessar(){
  myDFPlayer.play(4);
}

/////////////////////////////////////- RFID -//////////////////////////////////////////////

int rfid(){
  // Verifica novos cartões
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  // Seleciona um dos cartões
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  
  // Mostra UID na serial
  Serial.print("UID da tag :");
  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++)  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase();
  
  if (conteudo.substring(1) == "20 16 3B 52") { //UID 1 - Chaveiro
    Serial.println("Chaveiro identificado!");
    Serial.println();
    return TEMPO_DEFICIENTE;
     
  } else
  if (conteudo.substring(1) == "04 50 B0 AB") { //UID 2 - Cartao
    Serial.println("Cartao identificado");
    Serial.println();
    return TEMPO_IDOSO;
  } else {
    Serial.println("*** Não identificado");
  }

  return 0;
}


//////////////////////////////- Sinal -///////////////////////////////////////////


void led(bool ligar, int porta){
   //Controle do led verde
  if (ligar){
    digitalWrite(porta,HIGH);  //acende o led
  } else {           
    digitalWrite(porta,LOW);   //apaga o led
  }
}
