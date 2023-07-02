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
#define SIREN_TIMER 100 //4000 //Time for siren to be active in ms
#define DM_TIMER 20       // how often to meassure distance  and run distanceCheck
#define DM_ArraySize 20  //HIGHER IS MORE SENSITIVE default: 20 // ammount of meassurements that are checked for movement. Higher number gives more time-range to detect movement        
#define MA_ArraySize 20  //
//alarm sensitivity-requirement settings: LOWER IS MORE SENSITIVE. (more alarm)
#define DISTANCE_GAP_SENSITIVITY 2 // required distance difference for alarm-trip default: 2   aprox: number*10cm  example: DISTANCE_GAP_SENSITIVITY at 2 means distance has to change 40cm from measure to measure to upp the treatlevel by one. 
#define MEASURE_COUNT_SENSITIVITY 5 // ammout of required threatLevel-points, default: 5 choose between 0 and DM_ArraySize - 1      4 or less-unstable
#define MISSREAD_LIMIT 70 // exclude unreasonable jumps in measurements from the detection logic

long duration;
float distanceCm;
int distanceCmInt;
bool sirenStatus = false;
unsigned long sirenStartMillis = millis();
unsigned long prevDistanceMeassure = millis();
enum systemState {active = 0, inactive = 1};
systemState SystemState = active;
int distanceVals[DM_ArraySize];
int threatLevel = 0;
int distanceAggreagte = 0;
int movingAverage[MA_ArraySize];

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 0,5 second.
int botRequestDelay = 14000;
unsigned long lastTimeBotRan;
String chat_id;

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT); // Sets the ledPin for system testing as an Output
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(sirenPin, OUTPUT); // Sets the trigPin as an Output
  digitalWrite(sirenPin, HIGH); // Sets the trigPin as an Output

  initWiFi();
}

void loop() {
  checkMessages();
  switch (SystemState) {
    case active:
      getDistance();
      break;
    case inactive:
      sirenOFF();
      break;
  }
}


//-----------------------Functions--------------------//
void getDistance() {
  if   (millis() >= prevDistanceMeassure + DM_TIMER){
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
    distanceCmInt = int(distanceCm/10);

      
    // Prints the distance in the Serial Monitor
    //    Serial.print("Distance (dm): ");
    //    Serial.println(int(distanceCm/10));
    prevDistanceMeassure = millis();
    distanceCheck(distanceCmInt);
  }
}

void distanceCheck(int distanceCmInt) {
  //-----------------------ShiftReg------------------//
  for (int i = 0; i < DM_ArraySize-1; i++) {
      distanceVals[DM_ArraySize-1-i] = distanceVals[DM_ArraySize-2-i]; //exclude distanceVals[0]
  }distanceVals[0] = distanceCmInt;                                    //add distanceVals[0]
  //-----------------------Printer------------------//
  /*  
  for (int i = 0; i < DM_ArraySize; i++) {
      Serial.print(distanceVals[i]);
  }Serial.println("");  //*/

  //-----------------------MovingAverage------------------//  
  for (int i = 0; i < MA_ArraySize-1; i++) {
      movingAverage[MA_ArraySize-1-i] = movingAverage[MA_ArraySize-2-i]; //exclude movingAverage[0]
  }
    movingAverage[0] = distanceVals[0];
  for (int i = 1; i < DM_ArraySize-1; i++) {
    movingAverage[0] += distanceVals[i];
  }
  for (int i = 0; i < DM_ArraySize; i++) {
  }Serial.println("");  //*/

    Serial.print(movingAverage[0]);
  
  //-----------------------DetectionLogic------------------//
  for (int i = 0; i < DM_ArraySize-1; i++) {  
    if (abs(movingAverage[i] - movingAverage[i+1]) >= DISTANCE_GAP_SENSITIVITY){
      threatLevel++;
    }
  } 

  if (threatLevel >= MEASURE_COUNT_SENSITIVITY){
    runSiren(true);  //start siren                   --set sirentime in define SIREN_TIMER
    Serial.println(threatLevel); 
    Serial.println(""); 
  }else{
    runSiren(false); //check if siren should turn off
  }  
  threatLevel = 0;
}
void runSiren(bool reset){
  sirenStatus = reset;
  if (sirenStatus == true){
    sirenON();
    sirenStatus = false;
   // bot.sendMessage(CHAT_ID, "ALARM utløst", "");
    sirenStartMillis = millis();
  }else if (millis() > sirenStartMillis + SIREN_TIMER){
    sirenOFF();
    sirenStatus = false;
  }
}
void sirenToggle(){
  digitalWrite(sirenPin, !digitalRead(sirenPin));
  delay(1000);
}
void sirenON(){
  digitalWrite(sirenPin, LOW);
  //Serial.println("siren on   ++++++");
}
void sirenOFF(){
  digitalWrite(sirenPin, HIGH);
  //Serial.println("siren off  ------");
}
void sirenBURST(){
  digitalWrite(sirenPin, LOW);
  delay(100);
  digitalWrite(sirenPin, HIGH);
  delay(100);
  digitalWrite(sirenPin, LOW);
  delay(100);
  digitalWrite(sirenPin, HIGH);
  delay(100);
  digitalWrite(sirenPin, LOW);
  delay(100);
  digitalWrite(sirenPin, HIGH);
  delay(100);
  digitalWrite(sirenPin, LOW);
  delay(100);
  digitalWrite(sirenPin, HIGH);
  delay(100);
}
String getSystemState(){
  if (SystemState == 1){
    return "Asleep";
  }else if (SystemState == 0){
    return "in Senty-Mode";
  }else{
    return "in errorstate: Out of state";
  }
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
      String welcome = "Welcome, master " + from_name + ".\n";
      welcome += "Cerberus is currently " + getSystemState() + " \n\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/System_test to flash onboard LED \n";
      welcome += "/siren_burst to play a butiful siren hymn \n";
      welcome += "/status to request current GPIO status \n";
      welcome += "/awaken to activate sentry-mode \n";
      welcome += "/sleep to deactivate \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/System_test") {
      for (int i = 0; i <= 8; i++){
        digitalWrite(2, HIGH);
        delay(200);
        digitalWrite(2, LOW);
        delay(200);
      }
      bot.sendMessage(chat_id, "System message test active", "");
    }
    
    if (text == "/siren_toggle") {
      sirenToggle();
      bot.sendMessage(chat_id, "Siren is toggled", "");
    }
    if (text == "/siren_burst") {
      sirenBURST();
      bot.sendMessage(chat_id, "Siren is bursting", "");
    }
    if (text == "/status") {
      bot.sendMessage(chat_id, "SirenPin25 is currently " + String(digitalRead(sirenPin)), "");
    }
    if (text == "/awaken") {
      SystemState = active;
      bot.sendMessage(chat_id, "Sentry-mode Active", "");
    }
    if (text == "/sleep") {
      SystemState = inactive;
      bot.sendMessage(chat_id, "Cerberus is asleep", "");
    }
  }
}
void initWiFi(){
  
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