#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h> 

// Replace with your network credentials
const char* ssid = "ASUS";
const char* password = "vikommeretterdeg";

// Initialize Telegram BOT
#define BOTtoken "5931880125:AAHvkLIx7FOMdAu10QBGtnkazs_KtplCb4k"  // min Bot Token (Get from Botfather) Roberto 5931880125:AAHvkLIx7FOMdAu10QBGtnkazs_KtplCb4k

#define CHAT_ID "5942012745"//Thomas ID 5942012745

const int trigPin = 5;
const int echoPin = 18;
const int sirenPin = 25;


//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define SIREN_TIMER 500 //Time for siren to be active in ms
#define DM_TIMER 100

long duration;
float distanceCm;
float prevDistanceCm;
bool sirenStatus = false;
unsigned long sirenStartMillis = millis();
unsigned long prevDistanceMeassure = millis();


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 0,5 second.
int botRequestDelay = 14000;
unsigned long lastTimeBotRan;
String chat_id;

const int foto = 32;
int buzzState = 0;
int magnetState = analogRead(foto); 

void setup() {
  Serial.begin(115200);

  pinMode(2, OUTPUT);
  pinMode(foto, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(sirenPin, OUTPUT); // Sets the trigPin as an Output
  digitalWrite(sirenPin, HIGH); // Sets the trigPin as an Output

  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  checkMessages();
  getDistance();
  alarmCheck();
  
}
//-----------------------Functions--------------------//


void getDistance() {
  //if   (millis() <= prevDistanceMeassure + DM_TIMER){
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    
    // Calculate the distance
    distanceCm = duration * SOUND_SPEED/2;
      
    // Prints the distance in the Serial Monitor
    Serial.print("Distance (dm): ");
    Serial.println(int(distanceCm/10));
  //  prevDistanceMeassure = millis();
  //}
}

void runSiren(bool reset){
  sirenStatus = reset;
  if (millis() > sirenStartMillis + SIREN_TIMER){
    sirenOFF();
    sirenStatus = false;
  }else if (sirenStatus == true){
    sirenON();
    sirenStartMillis = millis();
  }
}

void alarmCheck() {
  if (int(distanceCm)/10 != prevDistanceCm){
    bot.sendMessage(CHAT_ID, "ALARM utløst", "");
    runSiren(true);  //start siren                   --set sirentime in define SIREN_TIMER
  }else{
    runSiren(false); //check if siren should turn off
  }  
  prevDistanceCm = int(distanceCm)/10;
}


void sirenToggle(){
  digitalWrite(sirenPin, !digitalRead(sirenPin));
}
void sirenON(){
  digitalWrite(sirenPin, LOW);
  Serial.println("siren on   ++++++");
}
void sirenOFF(){
  digitalWrite(sirenPin, HIGH);
  //Serial.println("siren off  ------");
}

void checkMessages(){
    if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;


    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/System_test to flash onboard LED \n";
      welcome += "/buzzer_off to turn GPIO OFF \n";
      welcome += "/status to request current GPIO status \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (magnetState >= 1000) {
      bot.sendMessage(chat_id, "ALARM utløst", "");
    }

    if (text == "/System_test") {
      bot.sendMessage(chat_id, "System message test active", "");
      for (int i = 0; i <= 8; i++){
        digitalWrite(2, HIGH);
        delay(200);
        digitalWrite(2, LOW);
        delay(200);
      }
    }
    
    if (text == "siren_toggle") {
      sirenToggle();
      bot.sendMessage(chat_id, "Siren is turned OFF", "");
      
    }
  }
}