#include <PubSubClient.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <OneWire.h>

OneWire ds(A0);
// -------------------------------------- BEGIN - Пины Arduino ----------------------------------------------

#define Relay1_pin A3                    
#define Relay2_pin A2                      
#define Relay3_pin 2
#define Relay4_pin 7
#define Relay5_pin 6
#define Relay6_pin 5
#define Relay7_pin 4
#define Relay8_pin 3
#define Relay9_pin A4
#define Relay10_pin A5
// -------------------------------------- END - Пины Arduino ------------------------------------------------


// -------------------------------------- BEGIN - Глобальные переменные -------------------------------------
//Переменная для хранения состояния светодиода
boolean Relay1 = HIGH;                    //Переменная для хранения состояния Реле 1
boolean Relay2 = HIGH;                    //Переменная для хранения состояния Реле 2
boolean Relay3 = HIGH;
boolean Relay4 = HIGH;
boolean Relay5 = HIGH;
boolean Relay6 = HIGH;
boolean Relay7 = HIGH;
boolean Relay8 = HIGH;
boolean Relay9 = HIGH;
boolean Relay10 = HIGH;
// -------------------------------------- END - Глобальные переменные ---------------------------------------


// -------------------------------------- BEGIN - Установка параметров сети ---------------------------------
void callback(char* topic, byte* payload, unsigned int length);
// Установить MAC адресс для этой Arduino (должен быть уникальным в вашей сети)
byte mac[] = { 0xA8, 0x61, 0x0A, 0x5F, 0xFB, 0xEC }; //a8:61:0a:4f:fa:ed

// Утановить IP адресс для этой Arduino (должен быть уникальным в вашей сети)
IPAddress ip(192, 168, 1, 33);

// Уставновить IP адресс MQTT брокера
byte server[] = { 192, 168, 1, 21 };

// Уставновить Логин и Пароль для подключения к MQTT брокеру
const char* mqtt_username = "stasdizzi";
const char* mqtt_password = "dIzzi7166899";

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
// --------------------------------------- END - Установка параметров сети ----------------------------------


// --------------------------------------- BEGIN - Подключение и подписка на MQTT broker ----------------------------------
boolean reconnect() {
  //Serial.println("reconnect...");
  if (client.connect("irrigationgate", mqtt_username, mqtt_password)) {
    client.subscribe("/irrigationgate/relay1");
    client.subscribe("/irrigationgate/relay2");
    client.subscribe("/irrigationgate/relay3");
    client.subscribe("/irrigationgate/relay4");
    client.subscribe("/irrigationgate/relay5");
    client.subscribe("/irrigationgate/relay6");
    client.subscribe("/irrigationgate/relay7");
    client.subscribe("/irrigationgate/relay8");
    client.subscribe("/irrigationgate/relay9");
    client.subscribe("/irrigationgate/relay10");
    //    Serial.println("MQTT connected");
  }
  return client.connected();
}
// --------------------------------------- END - Подключение и подписка на MQTT broker ----------------------------------
String addrToStr(byte addr[8]) {
  String result = "";
  for (byte i = 0; i < 8; i++) {
    byte a = addr[i];
    auto s = String(a, HEX);
    if (a < 0x10) {
      result += "0";
    }
    result += s;
    if (i < 7) {
      result += ":";
    }
  }
  return result;
}
// --------------------------------------- BEGIN - void setup() -------------------------------------------
void setup()
{
  digitalWrite(Relay1_pin, LOW); // Решение проблемы с LOW статусом пинов при загрузке ардуино
  digitalWrite(Relay2_pin, LOW); // Решение проблемы с LOW статусом пинов при загрузке ардуино
  digitalWrite(Relay3_pin, LOW);
  digitalWrite(Relay4_pin, LOW);
  digitalWrite(Relay5_pin, LOW);
  digitalWrite(Relay6_pin, LOW);
  digitalWrite(Relay7_pin, LOW);
  digitalWrite(Relay8_pin, LOW);
  digitalWrite(Relay9_pin, LOW);
  digitalWrite(Relay10_pin, LOW);

  pinMode(Relay1_pin, OUTPUT);
  pinMode(Relay2_pin, OUTPUT);
  pinMode(Relay3_pin, OUTPUT);
  pinMode(Relay4_pin, OUTPUT);
  pinMode(Relay5_pin, OUTPUT);
  pinMode(Relay6_pin, OUTPUT);
  pinMode(Relay7_pin, OUTPUT);
  pinMode(Relay8_pin, OUTPUT);
  pinMode(Relay9_pin, OUTPUT);
  pinMode(Relay10_pin, OUTPUT);

  Serial.begin(9600); // Open serial communications
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    //    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  //  Serial.println("connecting...");

  Serial.print("My ip address: ");
  Serial.println(Ethernet.localIP());

  reconnect(); // Подключение к брокеру, подписка на прописанные выше темы

}
// --------------------------------------- END - void setup() ---------------------------------------------

unsigned long previousTime = 0;
// --------------------------------------- BEGIN - void loop() --------------------------------------------
void loop() {


  byte i;
  byte data[12];
  byte addr[8];

  while (!ds.search(addr)) {
    ds.reset_search();
    //    Serial.println("No more sensors");
    delay(1000);
  }

  for ( i = 0; i < 8; i++) { // we need 9 bytes, last one for CRC
    //    Serial.print(addr[i], HEX);
    //    Serial.print(":");
  }
  //  Serial.print(" ");


  // Configure chip to read VAD and not VDD
  ds.reset();
  ds.select(addr);
  ds.write(0x4E);
  ds.write(0x00);
  ds.write(0x00);

  // Convert voltage
  ds.reset();
  ds.select(addr);
  ds.write(0xB4);
  delay(20);

  // Start temperature conversion
  ds.reset();
  ds.select(addr);
  ds.write(0x44);
  delay(20);

  // Current readings are updated automatically

  // Recall
  ds.reset();
  ds.select(addr);
  ds.write(0xB8);
  ds.write(0x00);

  // Read Scratchpad
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  ds.write(0x00);

  for ( i = 0; i < 9; i++) { // we need 9 bytes, last one for CRC
    data[i] = ds.read();
    //     Serial.print(data[i], HEX);
    //     Serial.print(" ");
  }

  // CRC
  if ( OneWire::crc8(data, 8) != data[8]) {
    //    Serial.println("CRC is not valid!");
    return;
  }

  Serial.print("Temp=");
  int temp = (((((int16_t) data[2]) << 8) | data[1]) >> 3) * 0.03125;
  Serial.print(temp);

  //  Serial.print(" VAD=");
  float vad = ((((int16_t)data[4]) << 8) | data[3]) * 0.01;
  //  Serial.print(vad);
  //

  // Configure chip to read VDD and not VAD
  ds.reset();
  ds.select(addr);
  ds.write(0x4E);
  ds.write(0x00);
  ds.write(0x0F);

  // Convert voltage
  ds.reset();
  ds.select(addr);
  ds.write(0xB4);
  delay(20);

  // Recall
  ds.reset();
  ds.select(addr);
  ds.write(0xB8);
  ds.write(0x00);

  // Read Scratchpad
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  ds.write(0x00);

  //  Serial.print(" ");
  for ( i = 0; i < 9; i++) { // we need 9 bytes, last one for CRC
    data[i] = ds.read();
  }

  // CRC
  if ( OneWire::crc8(data, 8) != data[8]) {
    //    Serial.println("CRC is not valid!");
    return;
  }

  Serial.print(" VDD=");
  float vdd = ((((int16_t)data[4]) << 8) | data[3]) * 0.01;
  Serial.print(vdd);

  Serial.print(" Hum=");
  int trueHum = (vad * 100);
  trueHum = map(trueHum, 283, 150, 0, 100);
  Serial.print(trueHum);

  //  Serial.println();

  auto time = millis();
  if (time >= previousTime + 60000) {
    previousTime = time;

    client.publish((addrToStr(addr) + "/Hum").c_str(), String(trueHum).c_str());
    client.publish((addrToStr(addr) + "/Temp").c_str(), String(temp).c_str());
  }

  client.loop();
}
// --------------------------------------- END - void loop() ----------------------------------------------


// --------------------------------------- BEGIN - void callback ------------------------------------------
// Чтение данных из MQTT брокера
void callback(char* topic, byte* payload, unsigned int length) {
  // проверка новых сообщений в подписках у брокера
  payload[length] = '\0';
  Serial.print("Topic: ");
  Serial.print(String(topic));
  Serial.println(" - ");


  if (String(topic) == "/irrigationgate/relay1") {
    String value = String((char*)payload);
    Relay1 = value == "1";
    Serial.println(Relay1);
    digitalWrite(Relay1_pin, Relay1 ? HIGH : LOW);
  }

  if (String(topic) == "/irrigationgate/relay2") {
    String value = String((char*)payload);
    Relay2 = value == "1";
    Serial.println(Relay2);
    digitalWrite(Relay2_pin, Relay2 ? HIGH : LOW);
  }

  if (String(topic) == "/irrigationgate/relay3") {
    String value = String((char*)payload);
    Relay3 = value == "1";
    Serial.println(Relay3);
    digitalWrite(Relay3_pin, Relay3 ? HIGH : LOW);
  }

  if (String(topic) == "/irrigationgate/relay4") {
    String value = String((char*)payload);
    Relay4 = value == "1";
    Serial.println(Relay4);
    digitalWrite(Relay4_pin, Relay4 ? HIGH : LOW);
  }

  if (String(topic) == "/irrigationgate/relay5") {
    String value = String((char*)payload);
    Relay5 = value == "1";
    Serial.println(Relay5);
    digitalWrite(Relay5_pin, Relay5 ? HIGH : LOW);
  }

  if (String(topic) == "/irrigationgate/relay6") {
    String value = String((char*)payload);
    Relay6 = value == "1";
    Serial.println(Relay6);
    digitalWrite(Relay6_pin, Relay6 ? HIGH : LOW);
  }

  if (String(topic) == "/irrigationgate/relay7") {
    String value = String((char*)payload);
    Relay7 = value == "1";
    Serial.println(Relay7);
    digitalWrite(Relay7_pin, Relay7 ? HIGH : LOW);
  }

  if (String(topic) == "/irrigationgate/relay8") {
    String value = String((char*)payload);
    Relay8 = value == "1";
    Serial.println(Relay8);
    digitalWrite(Relay8_pin, Relay8 ? HIGH : LOW);
  }

  if (String(topic) == "/irrigationgate/relay9") {
    String value = String((char*)payload);
    Relay9 = value == "1";
    Serial.println(Relay9);
    digitalWrite(Relay9_pin, Relay9 ? HIGH : LOW);
  }

  if (String(topic) == "/irrigationgate/relay10") {
    String value = String((char*)payload);
    Relay10 = value == "1";
    Serial.println(Relay10);
    digitalWrite(Relay10_pin, Relay10 ? HIGH : LOW);
  }
}
// ---------------------------------------- END - void callback -------------------------------------------
