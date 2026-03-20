#include <WiFi.h>
#include <WebServer.h>

// Network settings
const char* ssid = "DMX-Tester-Pro";
IPAddress apIP(4, 3, 2, 1); // Тот самый IP как у WLED
WebServer server(80);

// pins
const uint8_t PIN_COUNT = 3;
const uint8_t sendPins[PIN_COUNT] = {13, 12, 14}; // DMX female outputs
const uint8_t readPins[PIN_COUNT] = {27, 26, 25}; // DMX male inputs
const uint8_t buttonPin = 0;   // ESP32 "boot" button
const uint8_t buzzerPin = 33;  

const char* labels[PIN_COUNT] = {"GND (1)", "DATA- (2)", "DATA+ (3)"};
int testCounter = 0;
String currentCardHtml = "";

void setup() {
  Serial.begin(115200);
  
  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    pinMode(sendPins[i], OUTPUT);
    digitalWrite(sendPins[i], LOW);

    pinMode(readPins[i], INPUT_PULLDOWN); // В ESP32 есть встроенные Pull-down!
  }
  
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  // Access point settings
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  
  server.on("/", handleRoot);
  server.on("/getLatest", handleLatest);
  server.begin();

  Serial.println("HTTP server started");
  Serial.println("AP Started. Connect to 'DMX-Tester-Pro' and go to 4.3.2.1");
}

void loop() {
  server.handleClient();

  if (digitalRead(buttonPin) == LOW) {
    delay(200);
    runTest();
    while (digitalRead(buttonPin) == LOW) delay(1);
  }
}

void handleRoot() {
  String html = "<html><head><meta charset='utf-8' name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:sans-serif; background:#121212; color:#eee; margin:0; padding:15px;}";
  html += ".card{border-left:5px solid #444; background:#1e1e1e; padding:15px; margin-bottom:12px; border-radius:6px; ";
  html += "box-shadow:0 3px 6px rgba(0,0,0,0.3); opacity:0; transform:translateY(-20px); transition: all 0.4s ease-out;}";
  html += ".show{opacity:1; transform:translateY(0);}";
  html += ".pass{border-left-color:#2ecc71;}";
  html += ".err{border-left-color:#e74c3c; color:#ff8080;}";
  html += "h1{text-align:center; color:#00AAFF; margin-bottom:20px; font-weight:300;}";
  html += "</style></head><body>";
  html += "<h1>DMX Pro Tester</h1><div id='logContainer'></div>";

  // ajax script for update page content without full page reload
  html += "<script>";
  html += "let lastId = 0;";
  html += "function checkNewTest(){";
  html += "  fetch('/getLatest').then(res => res.json()).then(data => {";
  html += "    if(data.id > lastId){";
  html += "      lastId = data.id;";
  html += "      let container = document.getElementById('logContainer');";
  html += "      let tempDiv = document.createElement('div');";
  html += "      tempDiv.innerHTML = data.cardHtml;";
  html += "      let card = tempDiv.firstChild;";
  html += "      container.insertBefore(card, container.firstChild);";
  html += "      setTimeout(() => card.classList.add('show'), 10);";
  html += "      if(container.children.length > 15) container.lastChild.remove();";
  html += "    }";
  html += "  });";
  html += "}";
  html += "setInterval(checkNewTest, 500);";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

// JSON build handler
void handleLatest() {
  String json = "{";
  json += "\"id\":" + String(testCounter) + ",";
  json += "\"cardHtml\":\"" + currentCardHtml + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void runTest() {
  testCounter++;
  bool hasError = false;
  bool lineConnected[PIN_COUNT] = {false};
  String detailLog = "";
  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    digitalWrite(sendPins[i], HIGH);
    delay(30);
    for (uint8_t j = 0; j < PIN_COUNT; j++) {
      if (digitalRead(readPins[j])) {
        if (i == j) {
          lineConnected[i] = true;
        } else {
          detailLog += "• SHORT: Pin " + String(i+1) + " &rarr; " + String(j+1) + "<br>";
          hasError = true;
        }
      }
    }
    digitalWrite(sendPins[i], LOW);
  }

  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    if (!lineConnected[i]) {
      detailLog += "• OPEN: " + String(labels[i]) + "<br>";
      hasError = true;
    }
  }

  String cardClass = hasError ? "card err" : "card pass";
  String resText = hasError ? "FAILED" : "PASSED";

  String htmlEntry = "<div class='" + cardClass + "'>";
  htmlEntry += "<b>TEST #" + String(testCounter) + " — " + resText + "</b><br>";

  if (hasError) {
    htmlEntry += "<div style='margin-top:5px; font-size:0.9em;'>" + detailLog + "</div>";
    tone(buzzerPin, 500, 500); // buzzer sound notification about cable problem
  } else {
    htmlEntry += "<div style='margin-top:5px; font-size:0.9em; color:#2ecc71;'>All pins connected correctly.</div>";
    tone(buzzerPin, 2500, 100); // buzzer sound notification about successful cable test
    delay(150); 
    tone(buzzerPin, 2500, 100);
  }
  htmlEntry += "</div>";
  currentCardHtml = htmlEntry; 
  Serial.println("Test #" + String(testCounter) + " finished. Result: " + (hasError ? "FAIL" : "OK"));
}