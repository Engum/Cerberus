#include "Cerberus.h"

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT); // Sets the ledPin for system testing as an Output
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(sirenPin, OUTPUT); // Sets the trigPin as an Output
  digitalWrite(sirenPin, HIGH);

  initWiFi();

  xTaskCreatePinnedToCore(
             pingThomas, /* Task function. */
             "Task1",   /* name of task. */
             10000,     /* Stack size of task */
             NULL,      /* parameter of the task */
             1,         /* priority of the task */
             &Task1,    /* Task handle to keep track of created task */
             1);        /* pin task to core 0 */
}

void loop() {
  checkMessages();
  switch (SystemState) {
    case activation:
      getDistance("stabilize");
      break;
    case active:
      getDistance("active");
      break;
    case inactive:
      sirenOFF();
  }
}

//-----------------------Functions--------------------//

void pingThomas(void* parameter){
  for(;;){
    switch (SystemState) {
    case active:
      if (ENABLEPING) {PingForArrival();}
      break;
    case inactive:    
      if (ENABLEPING) {PingForDeparture();}
      break;
    }
    switch (MessageHelper) {
      case alarmAlert:
        Serial.println("MESSAGEHELPER");
        bot.sendMessage(CHAT_ID, "Cerberus Engaged \n\n to rewiew options: /start ", "");
        MessageHelper = none;
        break;
      default:
        Serial.print("");
        break;   
    }
  }
}
void PingForArrival(){
  if (millis() > lastPing + PINGTIMER){
      Serial.print("Ping: ");
    if (Ping.ping("ThomasXIV.net")){
      Serial.println("1");
      bot.sendMessage(CHAT_ID, "Welcome Home Master Thomas!", "");
      SystemState = inactive; 
      Serial.println("deactivation");
    }else{
      Serial.println("0");      
    }
  }
}
void PingForDeparture(){//System inactive
  if (millis() > lastPing + PINGTIMER){
      Serial.print("Ping: ");
    if (Ping.ping("ThomasXIV.net")||(Ping.ping("Vulcan.net"))){
      Serial.println("1");
      lastPing = lastPing + BIGTIME;
      Serial.print("Bigtime");
    }else{
      Serial.println("0");
      departureCounter++;
      if (departureCounter == DEPARTURE_COUNTER_LIMIT){ //240 default 
        bot.sendMessage(CHAT_ID, "Activation Test", "");
      }      
      if (departureCounter >= 5){
        bot.sendMessage(CHAT_ID, "SYSTEM ACTIVATON INIT", "");
        if (!Ping.ping("ThomasXIV.net")){
          SystemState = active;     
          Serial.print("activation");  
        }
        departureCounter = 0; 
      }
    lastPing = millis();
    }
  }
}
void getDistance(char* mode) {
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
    if (mode == "stabilize"){
      stabilize(distanceCmInt); 
      return;
    }
    distanceCheck(distanceCmInt);
  }
}

void stabilize(int distanceCmInt){
  organizeMovingAverage();
  if (movingAverage[MA_ArraySize-1] == 0){      //exclude intronumbers
    return;
  }

  for (int i = 0; i < MA_ArraySize; i++) {          //check for ArraySize of a kind
    if (movingAverage[i] != movingAverage[0]){
      return;
    }
  }
  SystemState = active;
}

void organizeMovingAverage(){
  //-----------------------ShiftReg------------------//
  for (int i = 0; i < DM_ArraySize-1; i++) {
      distanceVals[DM_ArraySize-1-i] = distanceVals[DM_ArraySize-2-i]; //exclude distanceVals[0]
  }distanceVals[0] = distanceCmInt;                                    //add distanceVals[0]

  //-----------------------MovingAverage------------------//  
  for (int i = 0; i < MA_ArraySize-1; i++) {
      movingAverage[MA_ArraySize-1-i] = movingAverage[MA_ArraySize-2-i]; //exclude movingAverage[0]
  }
  movingAverage[0] = distanceVals[0];
  for (int i = 1; i < DM_ArraySize-1; i++) {
    movingAverage[0] += distanceVals[i];
  }
}

void distanceCheck(int distanceCmInt) {
  organizeMovingAverage();
  #if (DEBUG)  
    for (int i = 0; i < MA_ArraySize; i++) {
      Serial.print(String(movingAverage[i]) + " ");
    }Serial.println("");  
  #endif
    //Serial.print(movingAverage[0]);
  
  //-----------------------DetectionLogic------------------//
  for (int i = 0; i < MA_ArraySize-1; i++) {  
    if (abs(movingAverage[i] - movingAverage[i+1]) >= DISTANCE_GAP_SENSITIVITY){
      threatLevel++;
    }
  }
  #if (DEBUG)  
  Serial.print(String(threatLevel)+"   ");
  #endif
  if (threatLevel >= MEASURE_COUNT_SENSITIVITY){
    runSiren(true);  //start siren                   --set sirentime in define SIREN_TIMER
    //Serial.println(threatLevel);
  }else{
    runSiren(false); //check if siren should turn off
  }  
  threatLevel = 0;
}
void runSiren(bool reset){
  sirenStatus = reset;
  if ((sirenStatus == true) && (millis() > AlarmStartMillis + ALARM_TIMER)){
    Serial.print("alarm" + String(alarmAlert));
    MessageHelper = alarmAlert;
    Serial.print(alarmAlert);
    AlarmStartMillis = millis();
  }
  if (sirenStatus == true){
    sirenON();
    sirenStatus = false;
    sirenStartMillis = millis();
    SystemState = activation;
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
      bot.sendMessage(chat_id, "SirenPin25 is currently " + String(digitalRead(sirenPin)), + "Where 1 means relay is inactive" "");
    }
    if (text == "/awaken") {
      SystemState = activation;
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