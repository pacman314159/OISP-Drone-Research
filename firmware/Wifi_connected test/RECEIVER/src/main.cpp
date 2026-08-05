#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <LoRa.h>
#include <WebServer.h>

#define NODE_ID 1

const char* ssid = "RECEIVER";
const char* password = "Boboiboy123";

#define NSS 13
#define RST 6
#define DIO0 1
#define SCK 9
#define MISO 10
#define MOSI 11
#define FREQUENCY 433E6

WebServer server(80);
String chatHistory = "";
bool lora_ok = false;

//=======WEB======
void handleRoot(){
  //Start and end of a raw string
  String html = R"rawliteral(
    <html>
    <head>
    <!-- Screen specification: space for interface the  width = screen width; scale = 100% -->
    <meta name = "viewport" content = "width=device-width, initial-scale=1">
    <style>
  <!-- Interface -->
      body { 
        font-family:Arial;
        background:#111; 
        color:#fff; 
        }
      .chat { 
        max-height:300px; 
        overflow-y:auto; 
        border:1px solid #444; 
        padding:10px; }
      .msgA { 
        color:#0f0; 
        }
      .msgB {
        color:#0ff;
        }
      input {
        width:70%;
        padding:8px;
        }
      button {
        padding:8px;
        }
    </style>
    </head>
    <body>
  
  <!-- Heading of website for RECEVER -->
    <h2>LoRa Chat - RECEIVER </h2>
    <p style='font-size:16px;'>
    <a href='https://youtube.com/@veritasium' style='color:white;'> 
    Youtube.com/Veritasium</a>
    </p>

  <!-- Texting interface, typing area, sending button -->
    //Create empty space <div> for message
    <div class="chat" id="chat"></div>

    <!-- Message -->
    <input id="msg" placeholder="Type message">

    <!-- When clicking button, Java send back to sendMsg function -->
    <button onclick="sendMsg()">Send</button>

    <script>
    function loadChat(){
      //Sending request for esp32, waiting for respond
      fetch('/get').then(function(response){
        return response.text();
        }).then(function(data){
        document.getElementById('chat').innerHTML = data;

      <!-- Save chatbox in a variable -->
      var chatBox = document.getElementById('chat');

      <!-- Autmatically scrolling down -->
      chatBox.scrollTop = chatBox.scrollHeight;
        }
      );
    }

    function sendMsg(){
      <!-- Variable m contains text when user type -->
      let m = document.getElementById('msg').value;
      fetch('/send?msg=' + encodeURIComponent(m));
    }

    <!-- Call loadChat every second -->
    setInterval(loadChat, 1000);
    </script>

    </body>
    </html>
    )rawliteral";

    //Send web to html: statusCode, contentType, content);
    server.send(200, "text/html", html);
}

//Send
void handleSend(){
  //Take messege
  String msg = server.arg("msg");

  if (lora_ok){
    LoRa.beginPacket();
    LoRa.print("Receiver:" + msg);
    LoRa.endPacket();
  }

  chatHistory += "<div class='msgA'>Receiver: " + msg + "</div>";
  server.send(200, "text/plain", "OK");
}

//GET CHAT
void handleGet(){
  server.send(200, "text/html", chatHistory);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());
  
  SPI.begin(SCK, MISO, MOSI, NSS);
  LoRa.setPins(NSS, RST, DIO0);

  if(LoRa.begin(FREQUENCY)){
    lora_ok = true;
    Serial.println("Connected");
  }else{
    Serial.println("Disconnected");
  }
  
  //Run handleRoot when accessing to link
  server.on("/", handleRoot);
  //Run handleSend when accessing /send
  server.on("/send", handleSend);
  server.on("/get", handleGet);
  //Restart web server
  server.begin();
}

void loop() {
  //Check if web have send request to esp32 or not
  server.handleClient();

  if(lora_ok){
    //Checking new packet
    int packetSize = LoRa.parsePacket();
    if (packetSize){
      String msg = "";
      while (LoRa.available()) 
        msg += (char)LoRa.read();
      chatHistory += "<div class='msgB'>" + msg + "</div>";
      Serial.println("RX: " + msg);
    }
  }
}

