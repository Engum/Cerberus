#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <ESP32Ping.h>

#define DEBUG 0
#define ENABLEPING 0

const char* ssid = "ASUS";
const char* password = "vikommeretterdeg";


#define BOTtoken "5931880125:AAHvkLIx7FOMdAu10QBGtnkazs_KtplCb4k"  // min Bot Token (Get from Botfather) Roberto 5931880125:AAHvkLIx7FOMdAu10QBGtnkazs_KtplCb4k
#define CHAT_ID "5942012745"//Thomas ID 5942012745

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define SIREN_TIMER 4000 //4000 //Time for siren to be active in ms
#define ALARM_TIMER 20000 // max alowed alarm delay to reduce spam (presumably)
#define DEPARTURE_COUNTER_LIMIT 240 // ammount of times to PING 0 before Systemactivation. This * PINGTIMER = minTime for systemactivation
#define PINGTIMER 5000      //240 times 5000ms => 20minutes to activate the system
#define BIGTIME 1200000   // time im ms to wait after sucsessfull ping // 1200000ms = 20minutes
#define DM_TIMER 20       // how often to meassure distance  and run distanceCheck
#define DM_ArraySize 20  //HIGHER IS MORE SENSITIVE default: 20 // ammount of meassurements that are checked for movement. Higher number gives more time-range to detect movement        
#define MA_ArraySize 20  //
//alarm sensitivity-requirement settings: LOWER IS MORE SENSITIVE. (more alarm)
#define DISTANCE_GAP_SENSITIVITY 4 // required distance difference for alarm-trip default: 2   aprox: number*10cm  example: DISTANCE_GAP_SENSITIVITY at 2 means distance has to change 40cm from measure to measure to upp the treatlevel by one. 
#define MEASURE_COUNT_SENSITIVITY 10 // ammout of required threatLevel-points, default: 5 choose between 0 and DM_ArraySize - 1      4 or less-unstable
#define MISSREAD_LIMIT 70 // exclude unreasonable jumps in measurements from the detection logic



//------------------------------------PINS--------------------------------//
const int trigPin = 5;
const int echoPin = 18;
const int sirenPin = 25;

//------------------------------------Variables--------------------------------//
long duration;
float distanceCm;
int distanceCmInt;
bool sirenStatus = false;
unsigned long sirenStartMillis = millis();
unsigned long prevDistanceMeassure = millis();
unsigned long AlarmStartMillis = millis();
unsigned long lastPing = millis();
enum systemState {activation = 0, active = 1, inactive = 2};
systemState SystemState = activation;
enum messageHelper {none = 0, alarmAlert = 1};
messageHelper MessageHelper = none;
int distanceVals[DM_ArraySize];
int threatLevel = 0;
int distanceAggreagte = 0;
int movingAverage[MA_ArraySize];
int departureCounter;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 0,5 second.
int botRequestDelay = 9000;
unsigned long lastTimeBotRan;
String chat_id;

TaskHandle_t Task1;


// Function prototypes
void PingForArrival(void* parameter);
void PingForDeparture(void* parameter);
void getDistance();
void distanceCheck(int distanceCmInt);
void runSiren(bool reset);
void sirenToggle();
void sirenON();
void sirenOFF();
void sirenBURST();
String getSystemState();
void checkMessages();
void handleNewMessages(int numNewMessages);
void initWiFi();
String macToString(const uint8_t* mac);

void setup();
void loop();
