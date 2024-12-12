//Include the library
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <MQUnifiedsensor.h>
#include "ESP32_FTPClient.h"


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "#Type your own Wifi SSID"
#define WIFI_PASSWORD "Type your own Wifi password"

// FTP credentials
char ftp_server[] = "192.168.1.1";    // Router IP, usually it is this IP address but you should check your own.
char ftp_user[] = "FTP username";             // FTP username
char ftp_pass[] = "FTP password";         // FTP password
char ftp_path[] = "/destination-path/";  // FTP destination path, messages saved here will be then sent as SMS messages

// Insert Firebase project API Key
#define API_KEY "API_KEY"
// Insert Firebase project Database URL
#define DATABASE_URL "Firebase project Database"

#define USER_EMAIL    "EMAIL"
#define USER_PASSWORD "EMAIL PASSWORD"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config1;

// Variable to save USER UID
String uid;
//Variables to save database paths
String databasePath;
String COPath;
String CH4Path;
String BatteryPath;
String PumpPath;
String AlarmPath;
String timePath = "/timestamp/.sv";

// Sensor path (where we'll save our readings)
String testPath;
// JSON object to save sensor readings and timestamp
FirebaseJson json;

//Variables
float CO_data;
float CH4_data;
float voltageBattery;
float voltagePump;
float VoltageAkumulator;
bool Alarm_for_Pump = false;

int adcValue;
int adcValue2;
bool CheckVoltage;


unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = true;
// Define a constant for the threshold voltage
const float voltageThreshold = 1.0; 
// Define a variable to keep track of the start time when voltage exceeds 5V
unsigned long voltageExceedStartTime = 0;
// Define the time limit (in milliseconds) for 15 minutes
const unsigned long timeLimit = 15 * 60 * 1000; // 15 minutes in milliseconds

#define RELAY_PIN 25
unsigned long relayOnTime = 0;
const unsigned long oneHour = 3600000; // 1 hour in milliseconds
const unsigned long threeMinutes = 180000; // 3 minutes in milliseconds
bool relayState = false; // To track the relay state

/************************Hardware Related Macros************************************/
#define         Board                   ("ESP-32") // Wemos ESP-32 or other board, whatever have ESP32 core.
#define         PinMQ_6                     (34)  //IO25 for your ESP32 WeMos Board, pinout here: https://i.pinimg.com/originals/66/9a/61/669a618d9435c702f4b67e12c40a11b8.jpg
#define         PinMQ_7                     (35)  //IO25 for your ESP32 WeMos Board, pinout here: https://i.pinimg.com/originals/66/9a/61/669a618d9435c702f4b67e12c40a11b8.jpg
#define         TypeMQ_6                    ("MQ6") //MQ6 or other MQ Sensor, if change this verify your a and b values.
#define         TypeMQ_7                    ("MQ7") //MQ6 or other MQ Sensor, if change this verify your a and b values.
#define         Voltage_Resolution      (3.3) // 3V3 <- IMPORTANT. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define         ADC_Bit_Resolution      (12) // ESP-32 bit resolution. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define         RatioMQ6CleanAir        (10) // Ratio of your sensor, for this example an MQ-6
#define         RatioMQ7CleanAir        (27.5) //RS / R0 = 27.5 ppm 
#define         Voltage_Akumulator      32    //Voltage akumulator input ADC pin
#define         Voltage_Pumpa           33    //Voltage pumpa input ADC pin

const float R1 = 14000.0;  // Resistance of R1 in ohms
const float R2 = 10000.0;  // Resistance of R2 in ohms

MQUnifiedsensor MQ6(Board, Voltage_Resolution, ADC_Bit_Resolution, PinMQ_6, TypeMQ_6);
MQUnifiedsensor MQ7(Board, Voltage_Resolution, ADC_Bit_Resolution, PinMQ_7, TypeMQ_7);

//FTP Object
ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass);

void MQfun_calibrating();
//void initWifi();

void setup() {
  // put your setup code here, to run once:
  //Init the serial port communication - to debug the library
  Serial.begin(115200); //Init serial port
   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();

  // Connect to FTP
  Serial.println("Connecting to FTP server...");
  ftp.OpenConnection();
  if (ftp.isConnected()) {
    Serial.println("FTP connection established");
  } else {
    Serial.println("Failed to connect to FTP server");
    return;
  }
  //Set the binary mode and choose right FTP directory
  ftp.InitFile("Type I");
  // String list[128];
  Serial.println("Changing working directory...");
  ftp.ChangeWorkDir(ftp_path);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure relay is off initially

  /* Assign the api key (required) */
  config1.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config1.database_url = DATABASE_URL;

    /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  fbdo.setResponseSize(4096);

   /* Assign the RTDB URL (required) */
  config1.database_url = DATABASE_URL;

   /* Assign the callback function for the long running token generation task */
  config1.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config1.max_token_generation_retry = 5;

  Firebase.begin(&config1, &auth);

  Firebase.reconnectWiFi(true);

 // Getting the user UID might take a few seconds
 Serial.println("Getting User UID");
 while ((auth.token.uid) == "") {
 Serial.print('.');
 delay(1000);
 }
 // Print user UID
 uid = auth.token.uid.c_str();
 Serial.print("User UID: ");
 Serial.print(uid);

// Update database path
 databasePath = "/path/" + uid;
  // Define database path for sensor readings
 // --> UsersData/<user_uid>/test/temperature
 AlarmPath = "/Alarm";
 // --> UsersData/<user_uid>/test/humidity
 CH4Path = "/CH4";
 // --> UsersData/<user_uid>/test/pressure
 COPath = "/CO";
// --> UsersData/<user_uid>/test/humidity
 BatteryPath = "/VoltageBattery";
 // --> UsersData/<user_uid>/test/pressure
 PumpPath = "/VoltagePump";
//  --> Update database path for sensor readings
testPath = databasePath +"/test/";


  analogReadResolution(12);  // Set ADC resolution to 12 bits
  
  MQfun_calibrating();   
}

void loop() 
{
  // bool alarmCondition = (parseInt(Alarm_Reading) == 1);
  // bool lowBatteryCondition = (voltageAkumulator < 9);
  // bool lowBatteryCondition2 = (voltageAkumulator2 < 9);
  // bool timeCondition = (millis() - sendDataPrevMillis > oneHour || sendDataPrevMillis == 0);

  // // Check if any condition is true
  // if (Firebase.ready() && (alarmCondition || lowBatteryCondition || lowBatteryCondition2 || timeCondition)) {
  //   sendDataPrevMillis = millis();

  //   // Turn on the relay
  //   digitalWrite(RELAY_PIN, HIGH);
  //   relayOnTime = millis();
  //   relayState = true;
  // }

  // // Turn off the relay after 5 minutes
  // if (relayState && millis() - relayOnTime > fiveMinutes) {
  //   digitalWrite(RELAY_PIN, LOW);
  //   relayState = false;
  // }


  // put your main code here, to run repeatedly:
  //5 min 300000
  //1 min 60000 
  //10 min 600000
  
  
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 600000  || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    MQ6.update(); // Update data, the arduino will read the voltage from the analog pin
    json.set(CH4Path.c_str(), String(MQ6.readSensor()));
    if (MQ6.readSensor() > 50000){
      ftp.InitFile("Type I");
      ftp.NewFile("send_sms_CH4.sms");
      ftp.Write("To: +YourphoneNumber \n\nCH4 is at dangerous level, evacuate the ship!");
      ftp.CloseFile();
    }

    MQ7.update(); // Update data, the Arduino will read the voltage from the analog pin
    json.set(COPath.c_str(), String(MQ7.readSensor()));
     if (MQ7.readSensor() > 400){
      ftp.InitFile("Type I");
      ftp.NewFile("send_sms_CO.sms");
      ftp.Write("To: +YourphoneNumber \n\nCO is at dangerous level, evacuate the ship!");
      ftp.CloseFile();
    }

    adcValue = analogRead(Voltage_Akumulator);  // Read the ADC value
    VoltageAkumulator = adcToVoltage(adcValue);
    json.set(BatteryPath.c_str(), String(VoltageAkumulator));

    if (VoltageAkumulator < 9 && VoltageAkumulator > 1 ){
      ftp.InitFile("Type I");
      ftp.NewFile("send_sms_Akumulator.sms");
      ftp.Write("To: +YourphoneNumber \n\nBattery Voltage is at critical level, under 9V!");
      ftp.CloseFile();
    }
 
    adcValue2 = analogRead(Voltage_Pumpa);
    voltagePump = adcToVoltage_Pump(adcValue2);  
    json.set(PumpPath.c_str(), String(voltagePump));
 
    CheckVoltage = checkPumpAndExecute(voltagePump);
    if (CheckVoltage){
      json.set(AlarmPath.c_str(), "1");
      // Create the file new and write a string into it
      ftp.InitFile("Type I");
      ftp.NewFile("send_sms_ALARM.sms");
      ftp.Write("To: +YourphoneNumber \n\nWater Leakeage in a ship!");
      ftp.CloseFile();
    }
    else{
      json.set(AlarmPath.c_str(), "0");
    }
    json.set(timePath, "timestamp");

    Serial.printf("Set json... %s\n", Firebase.RTDB.pushJSON(&fbdo, testPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());

  }
}

bool checkPumpAndExecute(float voltage){

  if (voltage > voltageThreshold) {
    // Check if the timer has not started yet
    if (voltageExceedStartTime == 0) {
      // Start the timer
      voltageExceedStartTime = millis();
    } else if (millis() - voltageExceedStartTime >= timeLimit) {
      // Reset the timer after execution
      voltageExceedStartTime = 0; // Resetting to prevent repeated execution

      // If the voltage has been over 5V for more than 15 minutes, execute the action and return 1
      return 1;
    }
  } else {
    // Reset the timer if the voltage drops below the threshold
    voltageExceedStartTime = 0;
    return 0;
  }

}

float adcToVoltage_Pump(int adcValue) {
  Serial.print("ADC Value pumpe: ");
  Serial.print(adcValue);
  // Convert the 12-bit ADC value to the input voltage
  float adcVoltage = (adcValue * 5.0) / 4095.0;  // Convert ADC value to voltage (0-5V range)
  float pumpVoltage = adcVoltage * (R1 + R2) / R2;  // Calculate the battery voltage using the voltage divider ratio
  
  Serial.print("  Voltage pumpe: ");
  Serial.println(pumpVoltage, 3);  // Print the voltage with 3 decimal places
  return pumpVoltage;
}



float adcToVoltage(int adcValue) {
  Serial.print("ADC Value akumulatora: ");
  Serial.print(adcValue);
  // Convert the 12-bit ADC value to the input voltage
  float adcVoltage = (adcValue * 5.0) / 4095.0;  // Convert ADC value to voltage (0-5V range)
  float batteryVoltage = adcVoltage * (R1 + R2) / R2;  // Calculate the battery voltage using the voltage divider ratio
  
  Serial.print("  Voltage akumulatora: ");
  Serial.println(batteryVoltage, 3);  // Print the voltage with 3 decimal places
  return batteryVoltage;
}


void MQfun_calibrating()
{
  //CH4
  MQ6.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ6.setA(2127.2); MQ6.setB(-2.526); // Configure the equation to to calculate CH4 concentration
  //CO
  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setA(99.042); MQ7.setB(-1.518); // Configure the equation to calculate CO concentration value
  MQ6.init();
  MQ7.init();
    Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  float calcR1 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ6.update(); // Update data, the arduino will read the voltage from the analog pin
    MQ7.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ6.calibrate(RatioMQ6CleanAir);
    calcR1 += MQ7.calibrate(RatioMQ7CleanAir);
    Serial.print(".");
  }
  MQ6.setR0(calcR0/10);
  MQ7.setR0(calcR1/10);
  Serial.println("  done!.");

  MQ6.serialDebug(true);
  MQ7.serialDebug(true);
}