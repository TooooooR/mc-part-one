#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Home_2.4G";
const char* password = "t1a2r3a4s5";

ESP8266WebServer server(80);

int8_t leds[3] = {D5, D4, D3};
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

void handleWeb() {
  String html = "<html><head>"
                "<style>"
                "body { display: flex; flex-direction: column; align-items: center; justify-content: start; font-family: Arial, sans-serif; background-color: #f4f4f4; }"
                "h1 { margin-top: 20px; color: #333; }"
                "button { background-color: #008CBA; color: white; border: none; padding: 15px 30px; font-size: 18px; border-radius: 5px; cursor: pointer; margin-top: 20px; }"
                "button:hover { background-color: #005f73; }"
                "</style>"
                "</head><body>"
                "<h1>ESP8266 LED Control</h1>"
                "<button onclick=\"fetch('/click')\">Click</button>"
                "</body></html>";
  server.send(200, "text/html", html);
}

void handleClick() {
  clickLeds();
  server.send(200, "text/plain", "OK");
}

void checkButton() {
  static bool lastButtonState = HIGH;
  static bool buttonHandled = false;
  static uint32_t lastDebounceTime = 0;

  bool currentButtonState = digitalRead(button);

  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentButtonState == LOW && !buttonHandled) { 
      clickLeds(); // Виконується тільки при першому натисканні
      buttonHandled = true;
    } 
    else if (currentButtonState == HIGH) {
      buttonHandled = false;
    }
  }

  lastButtonState = currentButtonState;
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

void updateLeds() {
  if (!running) {
    return;
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
}


void setup() {
  WiFi.begin(ssid, password);
  Serial.begin(115200);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());

  for (int i = 0; i < 3; i++) {
    pinMode(leds[i], OUTPUT);
  } 
  pinMode(button, INPUT);

  server.on("/", handleWeb); // запуск сайту
  server.on("/click", handleClick); // реагування на натиск кнопки на сайті ппп
  server.begin();
}

void loop() {
  server.handleClient(); // перевірка натискки на кнопці на сайті
  checkButton(); // перевірка фізичної кнопки
  updateLeds();
}
