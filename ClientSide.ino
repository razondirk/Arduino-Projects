#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_PWMServoDriver.h>
#include <EmonLib.h>

EnergyMonitor emon1;                                     // Create an Energy Monitor instance
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(); // called this way, it uses the default address 0x40
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

const char* ssid = "BlastDoors";
const char* password = "woodwork";
char ToolChar = 'A'; //Used to Define the tool being used. A=MiterSaw B=TableSaw C=Jointer/Planer D=Router E=Y1 F=Y2
char ToolAction;
int ServoAction;
int LastServoAction;
int ServoPin = 13;
int CurrentPin = 12;

int gateMinMax[6][2] = {      //Setting the throw of each gate servo  6 Tools/Ys, 2 values min/max
  {250, 415}, //Tool A
  {230, 405}, //Tool B
  {230, 405}, //Tool C
  {285, 425}, //Tool D
  {250, 405}, //Y1
  {250, 415}, //Y2
};


void setup() {
  //Network Setup
  Serial.begin(115200);
  delay(2000);                                                //Delay needed before calling the WiFi.begin

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {                     //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  //Current Sensor Setup
  emon1.current(CurrentPin, 5);                                // current(input pin (D12), calibration)

  //Servo Setup
  pwm.begin();
  pwm.setPWMFreq(60);                                          // Default is 1000mS

}

void loop() {


  double Irms = emon1.calcIrms(500);                          // Calculate Current RMS (Irms) only
  if (Irms > 20) {
    ToolAction = '1';
  }
  else {
    ToolAction = '0';
  }

  Serial.print("Current : ");
  Serial.print(Irms);           // Irms
  Serial.println(" mA");


  if (WiFi.status() == WL_CONNECTED) {                         //Check WiFi connection status

    HTTPClient http;

    http.begin("http://192.168.4.1:80");                        //Specify destination for HTTP request
    http.addHeader("Content-Type", "text/plain" );              //Specify content-type header "text/plain"
    int httpResponseCode = http.POST("A1 END");                 //Send the actual POST request
    Serial.println("Posted A1 END");
    delay(500);

    if (httpResponseCode > 0) {

      String response = http.getString();                       //Get the response to the request
      Serial.println(httpResponseCode);                         //Print return code
      Serial.println(response);                                 //Print request answer

      switch (ToolChar) {
        case 'A':
          ServoAction = (response.substring(0, 1)).toInt();
          pwm.setPWM(ServoPin, 0, gateMinMax[1][ServoAction]);
          break;
        case 'B':
          ServoAction = (response.substring(2, 3)).toInt();
          pwm.setPWM(ServoPin, 0, gateMinMax[2][ServoAction]);
          break;
        case 'C':
          ServoAction = (response.substring(4, 5)).toInt();
          pwm.setPWM(ServoPin, 0, gateMinMax[3][ServoAction]);
          break;
        case 'D':
          ServoAction = (response.substring(5, 6)).toInt();
          pwm.setPWM(ServoPin, 0, gateMinMax[4][ServoAction]);
          break;
        case 'E':
          ServoAction = (response.substring(7, 8)).toInt();
          pwm.setPWM(ServoPin, 0, gateMinMax[5][ServoAction]);
          break;
        case 'F':
          ServoAction = (response.substring(9, 10)).toInt();
          pwm.setPWM(ServoPin, 0, gateMinMax[6][ServoAction]);
          break;
        default:
          Serial.println("The programmer messed up somewhere.");
          break;
      }
      Serial.println(ServoAction);
    }
    else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      http.end();                                                 //Free resources
    }
  }
  else {                                                        //Wifi Connected Else: Reconnection
    Serial.println("Error in WiFi connection");
    WiFi.begin(ssid, password);                                 //Reconnect if Wifi disconnects
    while (WiFi.status() != WL_CONNECTED) {                     //Check for the connection
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
  }
  delay(2000);                                                 //Send a request every 2 seconds
}
