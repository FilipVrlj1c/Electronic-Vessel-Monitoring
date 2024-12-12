// Define a variable to keep track of the start time when voltage exceeds 5V
unsigned long voltageExceedStartTime = 0;
// Define the time limit (in milliseconds) for 15 minutes
const unsigned long timeLimit = 15 * 60 * 1000; // 15 minutes in milliseconds

#define RELAY_PIN 25
unsigned long sendData = 0;
unsigned long sendDataPrevMillis = 0;
unsigned long DataPrepTime = 0;
unsigned long relayOnTime = 0;
const unsigned long oneHour = 3600000; // 1 hour in milliseconds
const unsigned long threeMinutes = 180000; // 3 minutes in milliseconds
const unsigned long oneMinute_for_prep = 60000; // minute in milliseconds
bool relayState = false; // To track the relay state

bool alarmCondition;
bool lowBatteryCondition;
bool lowBatteryCondition;
bool timeCondition;
bool First_time_connecting = true;

void SendData_to_DataBase();

void setup()
{
  //Ovdje su predstavljeni uvjeti, znači prvi uvjet je ako je Alarm jednak 1 odnosno upaljen, drugi i treći uvjet je ako su oba akumulatora ispod vrijednosti napona od 9 Volti i 
  //zadnji uvjet je timeCondition odnosno da je prošlo više od sat vremena.

  alarmCondition = (parseInt(Alarm_Reading) == 1);
  lowBatteryCondition = (voltageAkumulator < 9);
  lowBatteryCondition2 = (voltageAkumulator2 < 9);
  timeCondition = (millis() - sendDataPrevMillis > oneHour || sendDataPrevMillis == 0);


}

void loop() 
{
// Mi ćemo uvijek čitat podatke, jer ESP32 pločica troši usporedivo manje energije nego 4G LTE router i može raditi u više različitih energetskih režima

  MQ6.update();
  MQ6_read = MQ6.readSensor();
  MQ7.update();
  MQ7_read = MQ7.readSensor()
  adcValue = analogRead(Voltage_Akumulator);
  voltageAkumulator = adcToVoltage(adcValue);
  adcValue2 = analogRead(Voltage_Pumpa);
  voltagePump = adcToVoltage_Pump(adcValue2);
  Alarm_Reading = checkPumpAndExecute(voltagePump)



  // Check if any condition is true
  if ((alarmCondition || lowBatteryCondition || timeCondition)) 
  { 
    // Turn on the relay, RELAY_PIN je zamišljeni pin koji bi bio povezan na relay i koji bi spajao strujni krug između Routera i Akumulatora broj 2.
    // Ovo je kod napisan za teorijsku izvedbu jer ja trenutno nemam potrebnu strukturu za to izvest ali je logika jasna.
    // Turn on the relay
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;

    //First time router is turing on,after being turn off.
    if (First_time_connecting == true)
    {
    //Router needs time to configure, 1 minute is usually enough time
    // Ako ste kad promatrali router ili modem kad se resetiraju mogli ste primjetit kako oni ne prorade instantno, treba im neko vrijeme. O tome ovdje vodimo računa.

      if (millis() - DataPrepTime > oneMinute_for_prep)
      {
        DataPrepTime = millis();
        Serial.begin(115200); //Init serial port
        //Connect to internet again
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        Serial.print("Connecting to WiFi ..");
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print('.');
          delay(1000);
        }
        Serial.println(WiFi.localIP());
        Serial.println();
        First_time_connecting = false;
    //Not connecting first time, we dont need to wait for configuration anymore
    }else
    {
      // Ako postoji problem možemo namjestit da se poruke šalju češće, ovdje je 1 minuta slanje poruka. Znači sat vremena je pauze onda idu poruke minut za minut.
      if(Firebase.ready() && (millis() - sendData > 60000  || sendData == 0))
      {
          sendData = millis();
          SendData_to_DataBase();
          //Turn off the relay after 5 minutes
          //Ipak ne želimo beskonačno slat poruke, bit će poslane po sekvenci sve ukupno 5 jer nakon 5 minuta je zamišljeno da se relej gasi. Ali to neće biti izvedeno ovdje.
          
          if (relayState && millis() - relayOnTime > fiveMinutes) 
          {
            relayOnTime = millis();
            relayState = false;
          }
          // Gašenje releja je hardwareska izvedba i želimo biti sto posto sigurni da je sve uredu kad prekinemo konekciju. Ovdje se vodi računa da nijedan alarm još ne gori  odnosno
          // da nema problema na brodu, tek onda možemo izgasiti strujni krug i ponovo restartirati vrijeme na čekanje od 1h.
          // Only turn off the relay if none of the critical conditions are true
          if (!alarmCondition && !lowBatteryCondition && !relayState) 
          {
            digitalWrite(RELAY_PIN, LOW);
            sendDataPrevMillis = millis();
            First_time_connecting = true;
          }
      }
    } 
  }
  

}

void SendData_to_DataBase(){
  MQ6.update(); // Update data, the arduino will read the voltage from the analog pin
  json.set(CH4Path.c_str(), String(MQ6.readSensor()));

  MQ7.update(); // Update data, the Arduino will read the voltage from the analog pin
  json.set(COPath.c_str(), String(MQ7.readSensor()));

  adcValue = analogRead(Voltage_Akumulator);  // Read the ADC value
  json.set(BatteryPath.c_str(), String(adcToVoltage(adcValue)));

  adcValue2 = analogRead(Voltage_Pumpa);
  voltagePump = adcToVoltage_Pump(adcValue2);  
  json.set(PumpPath.c_str(), String(voltagePump));

  CheckVoltage = checkPumpAndExecute(voltagePump);
  if (CheckVoltage){
    json.set(AlarmPath.c_str(), "1");
  }
  else{
    json.set(AlarmPath.c_str(), "0");
  }
  json.set(timePath, "timestamp");

  Serial.printf("Set json... %s\n", Firebase.RTDB.pushJSON(&fbdo, testPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
}