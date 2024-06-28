#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
const char* ssid = "Chertezh";
const char* password = "a1234509876";
const int id = 1;
const char* host = "192.168.78.123";
const uint16_t port = 80;

#define rele_pin D4
#define piezo_pin D8
#define Open_button D2
#define door_button D3
#define RST_PIN D9 // Configurable, see typical pin layout above
#define SS_PIN D10 // Configurable, see typical pin layout above

MFRC522 rfid(SS_PIN, RST_PIN); // Create MFRC522 instance.
int Time_open = 0;
bool locked = true;
int lock_timeout = 1000;

void setup() {
 Serial.begin(9600);
 SPI.begin();
 rfid.PCD_Init();
 pinMode(rele_pin, OUTPUT);
 pinMode(piezo_pin, OUTPUT);
 pinMode(Open_button, INPUT);
 pinMode(door_button, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

 close();
 indicate(2);
}

void loop() {
 static uint32_t TimeDelays;
 if (rfid.PICC_IsNewCardPresent() and rfid.PICC_ReadCardSerial()) 
 {
  Serial.print("Open");
  for (uint8_t i = 0; i< rfid.uid.size;i++)
  {
  Serial.print(rfid.uid.uidByte[i],HEX);
  Serial.print(" ");
  }
  if(locked)
  {
   if (foundTag(rfid.uid.uidByte, rfid.uid.size))
   {
    Time_open = millis();
    Serial.print("Open");
    open();
    indicate(1);
    locked = false;
   } 
   else if (millis()-TimeDelays >= 500)
   {
    indicate(2);
   }
  }
  TimeDelays = millis();
 }
 if (digitalRead(Open_button) == HIGH)
  {
   Time_open = millis();
   open();
   indicate(1);
   locked = false;
  } 
  if(!isOpen())
  {
  Time_open = millis();
  }
 
  if (!locked and millis() - Time_open > lock_timeout) {
   close();
   indicate(2);
   locked = true;
  }
 static uint32_t rfidReboot = millis();
 if (millis() - rfidReboot > 3000)
 {
  rfidReboot = millis();
  digitalWrite(RST_PIN,HIGH);
  delay(1);
  digitalWrite(RST_PIN,LOW);
  rfid.PCD_Init();
 }
}

void indicate(uint8_t signal)
{
 switch (signal) 
 {
  case 2:
   Serial.print("Deline");
   for (int i = 0; i < 2; i++) 
   {
    tone(piezo_pin, 100);
    delay(300);
    noTone(piezo_pin);
    delay(100);
   }
   return;
  case 1:
   Serial.print("Success");
   tone(piezo_pin, 890);
   delay(300);
   noTone(piezo_pin);
   delay(100);
   return;
 }

}

int foundTag(uint8_t *key, uint8_t size) {
 
 String stroka = "";
 for(int i = 0; i < size;i++)
 {
 stroka +=String(key[i],HEX);
 }
 Serial.println(stroka);
 WiFiClient client;
 if (client.connect(host,port))
 {
 client.print("POST /arrive?");
 client.print("roomId=");
 client.print(id);
 client.print("&face=");
 client.print(stroka);
 client.print("&timeOut=-1");
 client.print("\n");
 String result = "false";
   //while (client.available()) {
    delay(2000);
    result = client.readString();
    Serial.print(result);
  //}
 client.println("Host: " + String(host));
 client.println("Connection: close");
 client.println();
 client.stop();
 client.flush();
 
 if (result == "false") {
 return false;
 } else {
  return true;
 }
 } else {
  Serial.print("Connection failed");
  return false;
 }
}

bool compareUIDs(uint8_t *in1, uint8_t *in2, uint8_t size) {
 for (uint8_t i = 0; i < size; i++) {
  if (in1[i] != in2[i]) {return false;}
 }
 return true;
}

void open() 
 {
  digitalWrite(rele_pin,HIGH);
  delay(1000);
 }
 void close() 
 {
  digitalWrite(rele_pin,LOW);
  delay(1000);
 }
  bool isOpen()
 {
  return digitalRead(door_button);
 }
