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


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 0,5 second.
int botRequestDelay = 14000;
unsigned long lastTimeBotRan;
String chat_id;

const int buzzer = 2;
const int foto = 32;
int buzzState = 0;
int fotoState = analogRead(foto); 

// Handle what happens when you receive new messages
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

    if (fotoState >= 1000) {
      buzzState = 1; 
      bot.sendMessage(chat_id, "ALARM utløst", "");
      tone(buzzer, 5000);
      
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
    
    if (text == "/buzzer_off") {
      buzzState = 0;
      bot.sendMessage(chat_id, "BUZZER is turned OFF", "");
      noTone(buzzer);
      
    }
    
    if (text == "/status") {
      if (buzzState = 0){
        bot.sendMessage(chat_id, "BUZZER is OFF", "");
      }
      else if (buzzState = 1) {
        bot.sendMessage(chat_id, "BUZZER is ON", "");
      }
    }
  }
}



void setup() {
  Serial.begin(115200);

  pinMode(2, OUTPUT);
  pinMode(foto, INPUT);
  digitalWrite(buzzer, buzzState);

  
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
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
    
  }
  
  fotoState = hallRead();
  if (fotoState >= 40) {
  Serial.println(fotoState);
  }else if(fotoState <= 40){
  buzzState = 0;
  //Serial.println(fotoState);
  }
  //fotoState = analogRead(foto);

  
  if (fotoState >= 90 && buzzState == 0) {
    buzzState = 1;
    bot.sendMessage(CHAT_ID, "ALARM utløst", "");
    delay(10000);
  }
}