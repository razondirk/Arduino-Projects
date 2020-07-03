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
String output13State = "off";
String output2State = "off";

// Assign output variables to GPIO pins
const int output13 = 13;
const int output2 = 2;

// For your received data
const byte charLimit = 32;
char receivedChars[charLimit]; // an array to store the received data
bool newData = false;

void setup()
{
  Serial.begin(115200);

  // Initialize the output variables as outputs
  pinMode(output13, OUTPUT);
  pinMode(output2, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output13, LOW);
  digitalWrite(output2, LOW);

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
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      while (client.available()) {
        static byte ndx = 0;
        char c = client.read();
        if (c != '\n') {
          receivedChars[ndx] = c;
          ndx++;
          if (ndx >= charLimit) {
            ndx = charLimit - 1;
          }
        }
        else {
          receivedChars[ndx] = '\0'; // terminate the string
          ndx = 0;
          newData = true;
        }

        // This just shows what you have received so you know the parser works.
        if (newData == true) {
          Serial.print("This is what you have ");
          Serial.println(receivedChars);
          newData = false;
        }
      }
    }
    Serial.println("Disconnected");
  }
}
