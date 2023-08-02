//Author: Lucas Sampaio Leite

#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <time.h>
#include "FirebaseESP32.h"
#include <LiquidCrystal_I2C.h>

#define FIREBASE_HOST "your_host.firebaseio.com"            
#define FIREBASE_AUTH "your_token"  
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"

FirebaseData firebaseData;
FirebaseJson json;
String path = "";

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#define SS_PIN    21
#define RST_PIN   22

int cont = 0;

#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16

int lcdColumns = 20;
int lcdRows = 4;

LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows); 
MFRC522 mfrc522(2, 4); // Create MFRC522 instance 
float tempEnvironment = 0;
float tempStudent = 0;

//card identification                 
char *idCards[] = {"16 fd 59 99", 
                   "86 17 5a 99", 
                   "86 71 46 94",
                   "a6 6e 4a 99", 
                   "86 11 53 99", 
                   "66 e1 52 99",
                   "e6 22 47 94",
                   "96 93 50 94",
                   "16 7b 52 94",
                   "36 38 50 94",
                   "c6 79 4e 99",
                   "d6 09 55 94",
                   "f6 49 50 99",
                   "06 08 55 94",
                   "d6 15 5a 99",
                   "f6 57 57 99",
                   "56 92 56 99",
                   "76 a5 57 94", 
                   "f6 50 57 94",
                   "36 08 4b 94", 
                    };
                    
//association of cards with users
char *myStrings[] = {"5320 - Nome estudante 1 - Turma 105", 
                     "6481 - Nome estudante 2 - Turma 203", 
                     "6152 - Nome estudante 3 - Turma 105",
                     "6481 - Nome estudante 4 - Turma 203", 
                     "5851 - Nome estudante 5 - Turma 104", 
                     "5607 - Nome estudante 6 - Turma 202",
                     "2020 - Nome estudante 7 - Professor",
                     "5352 - Nome estudante 8 - Turma 201",
                     "5301 - Nome estudante 9 - Turma 204",
                     "5516 - Nome estudante 10 - Turma 202",
                     "5418 - Nome estudante 11 - Turma 202",
                     "5534 - Nome estudante 12 - Turma 202",
                     "5439 - Nome estudante 13 - Turma 202",
                     "5449 - Nome estudante 14 - Turma 202",
                     "5487 - Nome estudante 15 - Turma 204",  
                     "6128 - Nome estudante 16 - Turma 203",
                     "5563 - Nome estudante 17 - Turma 202",
                     "5738 - Nome estudante 18 - Turma 202",
                     "5369 - Nome estudante 19 - Turma 201",
                     "0000 - Nome estudante 20 - Turma 000"
                    };
                    
void setup(){
  SPI.begin();                                    // Serial Peripheral Interface
  mfrc522.PCD_Init();                             // initialize MFRC522
  mfrc522.PCD_DumpVersionToSerial();              // Show details of PCD- MFRC522 Card Reader details
  delay(4);                                       // Delay for display
  
  lcd.init();                                     // initialize LCD
  lcd.backlight(); 
  
  Serial.begin(9600);                             // initialize serial
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);           // initialize Wi-Fi
 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    lcd.setCursor(0, 0);
    lcd.print("Connecting to WiFi..");
    delay(500);
  }

  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());                  // print ip address

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);    // initialize the firebase connection
  Firebase.reconnectWiFi(true);
}
 
void loop(){
  
  if ( ! mfrc522.PICC_IsNewCardPresent()) {       // Reset the loop if no new card present on the sensor/reader. 
    lcd.setCursor(0, 0);
    lcd.print("  Colegio Militar   ");
    lcd.setCursor(0, 1);
    lcd.print("--------------------");
    lcd.setCursor(0, 2);
    lcd.print("Posicione o pulso");
    lcd.setCursor(0, 3);
    lcd.print("e aproxime o cartao");
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {         // select one of the cards
    lcd.setCursor(0, 0);
    lcd.print("  Colegio Militar   ");
    lcd.setCursor(0, 1);
    lcd.print("--------------------");
    lcd.setCursor(0, 2);
    lcd.print("Posicione o pulso");
    lcd.setCursor(0, 3);
    lcd.print("e aproxime o cartao");
    return;
  }

String content = "";                              // variable will store the contents of the card

  for (byte i = 0; i < mfrc522.uid.size; i++){    //reads the contents of the card 
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  String person="";                               // variable will store identified person
  for (int i = 0; i < 20; i++){
    String aux=" ";
    aux = aux + idCards[i];                       //String != char*
    if(aux.equals(content)){                      //compares each person's card with the card
      person = person + myStrings[i];             //stores the name of the person found
      Serial.println("Found: " + person);
    }
  }

  Serial.print("Environment = ");                     // temperature sensor reading
  tempEnvironment = mlx.readAmbientTempC();
  Serial.print(tempEnvironment);
  Serial.print("*C\tObject = ");
  tempStudent = mlx.readObjectTempC();
  Serial.print(tempStudent);
  Serial.println("*C");
  Serial.print("Environment = ");
  Serial.print(mlx.readAmbientTempF());
  Serial.print("*F\tObject = ");
  Serial.print(mlx.readObjectTempF());
  Serial.println("*F");

  lcd.clear();

  lcd.setCursor(0, 0);                           // prints the temperature of the room and the student on the display  
  lcd.print(" "+person);
  lcd.setCursor(0, 1);
  lcd.print("Ambiente: ");
  lcd.print(tempEnvironment);
  lcd.print(" C");
  lcd.setCursor(0, 2);
  lcd.print("Estudante: ");
  lcd.print(tempStudent);
  lcd.print(" C");
  lcd.setCursor(0, 3);
  lcd.print(" Dados registrados!");
  
  json.set("Temperatura Ambiente", tempEnvironment);
  json.set("Temperatura Aluno", tempStudent);
  
  if (Firebase.pushJSON(firebaseData, path + "/" + person, json)) { //send data to database
    Serial.println(firebaseData.dataPath());
    Serial.println(firebaseData.pushName());
    Serial.println(firebaseData.dataPath() + "/"+ firebaseData.pushName());
  } else {
    Serial.println("Connection error!");
    Serial.println(firebaseData.errorReason());
  }

Serial.println();
delay(5000);

lcd.clear();
}
