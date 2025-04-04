#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <WebSocketsServer.h> 

const char* ssid = "Redmi Note 8 Pro";
const char* password = "t1a2r3a4s55";

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

int8_t leds[3] = {D5, D3, D2};
const int8_t button = D0;

bool reverseOrder = false;
bool running = false;
bool ledState = false;

uint32_t lastPress = 0;
const int32_t debounceDelay = 50;
uint32_t previousMillis = 0;
const int32_t ledOnTime = 100;
const int32_t ledOffTime = 500;
int8_t ledIndex = 0;
uint32_t pressStartTime = 0;
bool isButtonPressed = false;


void handleFileRequest(String path) {
    if (path.endsWith("/")) path += "index.html";

    String contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";

    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
    } else {
        server.send(404, "text/plain", "File Not Found");
    }
}


void handleClick() {
    clickLeds();
    server.send(200, "text/plain", "OK");
}


void checkButton() {
    bool buttonState = digitalRead(button);

    if (buttonState == LOW && !isButtonPressed) { 
        isButtonPressed = true;
        pressStartTime = millis();
    } 
    else if (buttonState == HIGH && isButtonPressed) { 
        if (millis() - pressStartTime >= debounceDelay) { 
            clickLeds();
        }
        isButtonPressed = false;
    }
}


void clickLeds() {
    if (!running) {
        running = true;
        reverseOrder = !reverseOrder;
        ledIndex = 0;
        ledState = true;
        previousMillis = millis();
    }
}


bool updateLeds() {
    if (!running) {
        return false;
    }

    uint32_t now = millis();
    uint32_t delayTime = ledOffTime;

    if (ledState) {
        delayTime = ledOnTime;
    }

    if (now - previousMillis >= delayTime) {
        previousMillis = now;

        int index = ledIndex;
        if (reverseOrder) {
            index = 2 - ledIndex;
        }

        if (ledState) {
            digitalWrite(leds[index], HIGH);
        } else {
            digitalWrite(leds[index], LOW);
            ledIndex++;
        }

        ledState = !ledState;

        if (ledIndex >= 3) {
            running = false;
        }
    }
    return true;
}

//робота з uart
void handleUARTSend() {
    Serial.write('D');
    server.send(200, "text/plain", "Sent D");
}

void checkUARTReceive() {
    if (Serial.available()) {
        uint8_t received = Serial.read();
        
          if (received == 'A') {
            clickLeds();
            updateLeds();
        }
    }
}

//надсилання станів кнопок через webSocket
void updateLEDsWebSocket() {
    String json = "{\"leds\":[";

    for (int i = 0; i < 3; i++) {
        json += digitalRead(leds[i]) ? "1" : "0";
        json += (i < 2) ? "," : "";
    }

    json += "]}";
    webSocket.broadcastTXT(json);
}


void setup() {
    for (int i = 0; i < 3; i++) {
        pinMode(leds[i], OUTPUT);
    }
    pinMode(button, INPUT);

    WiFi.begin(ssid, password);
    Serial.begin(115200);

    if (!LittleFS.begin()) {
        Serial.println("LittleFS Mount Failed");
        return;
    }

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    Serial.println("ESP8266 IP Address: ");
    Serial.println(WiFi.localIP());

    server.onNotFound([]() { handleFileRequest(server.uri()); });
    server.on("/click", handleClick);
    server.on("/sendD", handleUARTSend);

    webSocket.begin();
    webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t length) { });

    server.begin();
}


void loop() {
    server.handleClient();
    webSocket.loop();
    checkUARTReceive();

    checkButton();
    updateLeds();

    updateLEDsWebSocket();
}
