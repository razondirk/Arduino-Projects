#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "BlastDoors";
const char* password = "woodwork";

// The IP address will be dependent on your local network:
IPAddress local_IP( 192, 168, 11, 1 ); //Static IP
IPAddress gateway( 192, 168, 11, 1 );
IPAddress subnet ( 255, 255, 255, 0 );

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
WiFiServer server(80);

// Auxiliar variables to store the current output state
String VacuumRelayPinState = "off";
String output2State = "off";

// Assign output variables to GPIO pins
const int VacuumRelayPin = 13;
const int output2 = 2;

// For your received data
const byte charLimit = 1000;
char receivedChars[charLimit]; // an array to store the received data
bool newData = false;
char ToolChar;
char ToolStatus;
String GateCommand;
String Endcase = "start";   //arbitrary string to start; !="END"

void setup()
{
  Serial.begin(115200);

  // Initialize the output variables as outputs
  pinMode(VacuumRelayPin, OUTPUT);
  pinMode(output2, OUTPUT);
  // Set outputs to LOW


  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  delay(2000);
  server.begin();

  Serial.println("setup finished");
}

void loop()
{
  // listen for incoming clients
  WiFiClient client = server.available();         // Listen for incoming clients
  if (client) {
    Serial.println("New Client.");                // If a new client connects,
    // an http request ends with a blank line
    while (client.connected()) {
      while (client.available()) {
        static byte ndx = 0;
        char m = client.read();                    //Reads the next character of POST Tx
        receivedChars[ndx] = m;                  //Adds Char to the Recieved Array
        ndx++;                                   //Increments Array position by 1

        if (ndx >= charLimit) {                  //Prevents Array Index (position) from being greater than Character Limit
          ndx = charLimit - 1;
        }
        Endcase = String(receivedChars[ndx - 4, ndx - 3]) + String(receivedChars[ndx - 3, ndx - 2]) + String(receivedChars[ndx - 2, ndx - 1]);
        if (Endcase == "END") {                                //Builds last 3 characters of Tx, Checks for "END" of Tx
          Endcase = "start";                                 //resets Endcase to arbitrary value that is not "END"
          ToolChar = receivedChars[ndx - 7, ndx - 6]; //Builds tool character prior to "END"; the desired payload
          ToolStatus = receivedChars[ndx - 6, ndx - 5]; //Builds status character prior to "END"; the desired payload
          receivedChars[ndx] = '\0'; // terminate the string build (if not here, it would add on using the next Tx coming in)
          ndx = 0;                   // resets the Array position to 0, next transmission starts from beginning of array
          newData = true;            // Boolean used to only print when a new POST is recieved

          switch (ToolChar) {                 //Logic to Determine which gates are open/closed
            case 'A':
              GateCommand = "1,0,0,0,1,1";
              break;
            case 'B':
              GateCommand = "0,1,0,0,0,1";
              break;
            case 'C':
              GateCommand = "0,0,1,0,0,0";
              break;
            case 'D':
              GateCommand = "0,0,0,1,1,1";
              break;
            default:
              client.println("HTTP/1.0 200 OK");
              client.println("Content-Type: text/html");
              client.println("Error Recieving POST Switch");
              Serial.println(ToolChar);
              break;
          }
        }

        client.println("HTTP/1.0 200 OK");
        client.println("Content-Type: text/html");
        client.println();
        client.println(GateCommand);
        client.stop();

        // This just shows what you have received so you know the parser works.
        if (newData == true) {                          //IF statement to print if POST is new
          Serial.print("This is your newest POST: ");
          Serial.print(ToolChar);
          Serial.print(ToolStatus);
          Serial.println();
          newData = false;                              //Resets Boolean to False until a new POST exists
        }
      }
    }
    Serial.println("Disconnected");

    //Logic to turn Dust collection on/off
    /*switch (ToolStatus) {
      case '1':
           digitalWrite(VacuumRelayPin,1);
           break;
      case '2':
           digitalWrite(VacuumRelayPin,0);
           break;
      default:
           digitalWrite(VacuumRelayPin,0);
           Serial.println("Error processing POST");
           Serial.println(Endcase);
           Serial.println();
           Serial.println(receivedChars);
           break;*/
  }
}
