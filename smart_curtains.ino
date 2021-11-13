
#include <ESP8266WiFi.h>

const char* ssid     = "SSID";
const char* password = "PASSWD";

const int pwmMotorA = D1;
const int pwmMotorB = D2;
const int dirMotorA = D3;
const int dirMotorB = D4;

int motorSpeed = 1000;

WiFiServer server(80);

String header;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

String ip = "";

void setup() {
  Serial.begin(115200);
  // Connect to Wi-Fi network with SSID and password
  Serial.println("Smart curtains 0.1");
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  ip = WiFi.localIP().toString();
  Serial.println(ip);
  server.begin();

  Serial.println("Initializing motors...");
  pinMode(pwmMotorA , OUTPUT);
  pinMode(pwmMotorB, OUTPUT);
  pinMode(dirMotorA, OUTPUT);
  pinMode(dirMotorB, OUTPUT);
  Serial.println("Done.");
}

void stop() {
  digitalWrite(pwmMotorA, 0);
  digitalWrite(dirMotorA, LOW);
  digitalWrite(pwmMotorB, 0);
  digitalWrite(dirMotorB, LOW);
}

void open(){
  Serial.println("opening curtains...");
  digitalWrite(pwmMotorA, motorSpeed);
  digitalWrite(dirMotorA, LOW);
  digitalWrite(pwmMotorB, motorSpeed);
  digitalWrite(dirMotorB, LOW);
  delay(1500);
  stop();
}

void close(){
  Serial.println("closing curtains...");
  digitalWrite(pwmMotorA, motorSpeed);
  digitalWrite(dirMotorA, HIGH);
  digitalWrite(pwmMotorB, motorSpeed);
  digitalWrite(dirMotorB, HIGH);
  delay(1500);
  stop();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            Serial.println(header);

            //TODO MOTORISATION
            if (header.indexOf("GET /" + ip + "/open") >= 0) {
              open();
            } else if (header.indexOf("GET /" + ip + "/close") >= 0) {
              close();
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { border: none; color: white; padding: 16px 40px; }");
            client.println(".open { background-color: #195B6A; }");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".close {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Lazy curtains</h1>");
            client.println("<button class=\"button open\">Open</button>");
            client.println("<button class=\"button close\">Close</button>");
            client.println("<script>");
            client.println("const IP = \"" + ip + "\";");
            client.println("function send(state) { const req = new XMLHttpRequest(); req.onload = () => { console.log(\"sent:\", state); }; req.open(\"GET\", `${IP}/${state}`); req.send(); };"); 
            client.println("const run = () => { \n document.getElementsByClassName(\"open\")[0].addEventListener(\"click\", () => { send(\"open\"); });");
            client.println(" document.getElementsByClassName(\"close\")[0].addEventListener(\"click\", () => { send(\"close\"); }); \n};");
            client.println("run()");
            client.println("</script>");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
