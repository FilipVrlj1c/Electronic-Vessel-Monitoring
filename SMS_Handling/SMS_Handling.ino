#include <WiFi.h>
#include "ESP32_FTPClient.h"


const char* ssid     = "WIFI SSID";
const char* password = "WIFI PASSWORD";

// FTP credentials
char ftp_server[] = "192.168.1.1";    // Router IP
char ftp_user[] = "[Insert your username]";             // FTP username
char ftp_pass[] = "[Inser your password]";         // FTP password
char ftp_path[] = "/var/spool/sms/outgoing/";  // FTP destination path

ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass);

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to FTP
  Serial.println("Connecting to FTP server...");
  ftp.OpenConnection();
  if (ftp.isConnected()) {
    Serial.println("FTP connection established");
  } else {
    Serial.println("Failed to connect to FTP server");
    return;
  }

  ftp.InitFile("Type I");
  // String list[128];
  Serial.println("Changing working directory...");
  ftp.ChangeWorkDir(ftp_path);
  
  // Create the file new and write a string into it
  ftp.InitFile("Type I");
  ftp.NewFile("send_sms_FV.sms");
  ftp.Write("To: +["Insert your number, without "" "] \n\nHello From ESP32-TEST");
  ftp.CloseFile();

  ftp.CloseConnection();
  Serial.println("DONE!");
}

void loop() {
  // Nothing needed here
}
