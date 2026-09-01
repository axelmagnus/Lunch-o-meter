
// libraries used for the display
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include <QRCodeGFX.h>  // QR-code generating library
#include "U8g2_for_Adafruit_GFX.h"
// libraries used for wifi
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <esp_eap_client.h>
#include <ArduinoJson.h>
#include "AdafruitIO_WiFi.h"

#define TFT_CS SS  // pin 6
#define TFT_DC 3
#define TFT_RST -1
#define TFT_MOSI MOSI  // pin 8
#define TFT_SCK SCK    //pin 10
#define TFT_MISO -1    // Custom pin (MISO is often -1 or not used for displays)


Adafruit_HX8357 tft(TFT_CS, TFT_DC, TFT_RST);  // for drawing shapes and other graphics
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;   // used to draw text
QRCodeGFX qrcode(tft);                         // Initialize QR-code functionallity

int buttonPins[5] = { 25, 7, 26, 1, 0 };  // the pins used for the buttons
int votes[5] = { 0, 0, 0, 0, 0 };
const char* voteLabels[5] = { "HATE", "DISLIKE", "NEUTRAL", "LIKE", "LOVE" };  // labels used to assign a name to each vote cast
char dagens[128] = "laddar...";                                                // stores the food of the day

// --- Batch posting state ---
int batchVotes[5] = { 0, 0, 0, 0, 0 };  // counts per label inside current batch
int batchCount = 0;                     // total votes accumulated in current batch (max 10)
void sendBatchPost();
// === Daily getData scheduler ===
// Set the time (24-hour format) when getData() should run each day
#define DAILY_UPDATE_HOUR 6  // Hour (0-23)
#define DAILY_UPDATE_MIN 0   // Minute (0-59)
// Track when getData was last ran to avoid running multiple times per day
unsigned long lastDataUpdateTime = 0;


#include "credentials.h"

void setup() {
  Serial.begin(115200);
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_SCK, OUTPUT);
  pinMode(TFT_MOSI, OUTPUT);

  for (int i = 0; i < 5; i++) {  // sets the LEDpins to give an output
    pinMode(buttonPins[i], INPUT);
  }
  // Display initialization
  tft.begin();
  delay(200);
  tft.setRotation(3);
  tft.setTextSize(2);

  u8g2_for_adafruit_gfx.begin(tft);  // connect u8g2 procedures to Adafruit GFX
  startUpScreen();
  delay(5000);
  tft.fillScreen(HX8357_BLACK);

  // Setup WiFi connection
 
  // WiFi event debug logger
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
      Serial.print("Disconnected! Reason code: ");
      Serial.println(info.wifi_sta_disconnected.reason);
    }
  });

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  // Native ESP32 Enterprise initialization:
  // WiFi.begin(SSID, method, outer_identity, username, password)
  WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, WIFI_USERNAME, WIFI_USERNAME, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting to WiFi... ");
    Serial.println(WIFI_SSID);
    tft.print("Connecting to ");
    tft.println(WIFI_SSID);
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  tft.println("Connected to WiFi");

  // POSIX Timezone String for Sweden / Central European Time (Europe/Stockholm)
  const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

  // Set timezone rules and NTP server together
  configTzTime(TZ_INFO, "pool.ntp.org", "time.nist.gov");
  // Wait briefly for time to be set
  Serial.println("Waiting for time sync...");
  tft.println("Waiting for time sync...");

  // 3. Loop until epoch time updates past timestamp 1600000000 (Sep 2020)
  for (int i = 0; i < 20 && time(nullptr) < 1600000000; ++i) {
    delay(500);
    Serial.print('.');
    tft.print(".");
  }
  tft.println();

  if (time(nullptr) > 1600000000) {
    tft.println("\nTime successfully synchronized!");
    Serial.println("\nTime successfully synchronized!");
  } else {
    tft.println("\nNTP sync timed out.");
    Serial.println("\nNTP sync timed out.");
  }
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char dateBuffer[30];

    // Format: Tuesday, Sep 01, 2026
    strftime(dateBuffer, sizeof(dateBuffer), "%A, %b %d, %Y", &timeinfo);

    Serial.print("Current Date: ");
    Serial.println(dateBuffer);

    tft.println(dateBuffer);  // Easy to print on TFT display too
    tft.println("Waiting for daily sync @ 6 am..");  // Easy to print on TFT display too
    
  }

  // One-time GET to populate dagens and initial votes
  getData();
  lastDataUpdateTime = time(nullptr);  // Record the initial update time
}


void loop() {
  // Check if it's time to run daily getData update
  checkAndRunDailyUpdate();

  // read buttonPins
  for (int i = 0; i < 5; i++) {
    if (digitalRead(buttonPins[i]) == 0) {
      votes[i]++;
      Serial.print(i);
      Serial.print(": ");
      Serial.println(votes[i]);
      thankYouScreen();

      // Accumulate vote into batch; when batchCount reaches 10 send batch POST
      batchVotes[i]++;
      batchCount++;
      Serial.print("Batch count: ");
      Serial.println(batchCount);
      if (batchCount >= 5) {
        sendBatchPost();
      }
      welcomeScreen(dagens);
    }
  }
}

// === Daily scheduler helper ===
void checkAndRunDailyUpdate() {
  // Get current time
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  // Check if current time matches the scheduled hour and minute
  if (timeinfo->tm_hour == DAILY_UPDATE_HOUR && timeinfo->tm_min == DAILY_UPDATE_MIN) {
    // Check if we haven't already run getData today
    // (lastDataUpdateTime will be earlier in the day)
    if (now - lastDataUpdateTime > 3600) {  // More than 1 hour since last run
      Serial.println("Running scheduled daily getData...");
      getData();
      lastDataUpdateTime = now;
    }
  }
}

void startUpScreen() {
  tft.fillScreen(HX8357_WHITE);
  u8g2_for_adafruit_gfx.setBackgroundColor(HX8357_WHITE);
  u8g2_for_adafruit_gfx.setForegroundColor(HX8357_RED);  // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setFont(u8g2_font_osb35_tf);     // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2_for_adafruit_gfx.setCursor(80, 70);
  u8g2_for_adafruit_gfx.print("Lunch-o-Meter");
  u8g2_for_adafruit_gfx.setCursor(80, 130);
  u8g2_for_adafruit_gfx.print("Dags & Simbas");
  u8g2_for_adafruit_gfx.setCursor(80, 190);
  u8g2_for_adafruit_gfx.print("Gymnasiearbete");
  u8g2_for_adafruit_gfx.setCursor(80, 250);
  u8g2_for_adafruit_gfx.print("TE23TE");
}

void thankYouScreen() {
  tft.fillScreen(HX8357_BLACK);
  tft.fillRoundRect(20, 20, 440, 280, 20, tft.color565(100, 0, 100));
  u8g2_for_adafruit_gfx.setBackgroundColor(tft.color565(100, 0, 100));
  u8g2_for_adafruit_gfx.setCursor(100, 100);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_osb21_tf);
  u8g2_for_adafruit_gfx.print("Tack för att du röstat!");
  u8g2_for_adafruit_gfx.setCursor(50, 150);
  u8g2_for_adafruit_gfx.print("Klicka på en knapp nedan");
  u8g2_for_adafruit_gfx.setCursor(50, 175);
  u8g2_for_adafruit_gfx.print("om du vill se vad andra tyckt");

  // Blocking wait for a button press (with timeout). This allows immediate response to a press
  // while the thank-you screen is visible and ensures background tasks pause until the user
  // interacts.
  unsigned long start = millis();
  const unsigned long timeout = 3000;  // ms
  while (millis() - start < timeout) {
    for (int i = 0; i < 5; i++) {
      if (digitalRead(buttonPins[i]) == 0) {
        // simple debounce
        delay(50);
        if (digitalRead(buttonPins[i]) == 0) {
          // wait for release
          while (digitalRead(buttonPins[i]) == 0) delay(10);
          // show aggregated data screen
          displayData();
          return;
        }
      }
    }
    delay(50);
  }
}

void welcomeScreen(const char* food) {
  tft.fillScreen(HX8357_BLACK);
  u8g2_for_adafruit_gfx.setBackgroundColor(HX8357_BLACK);
  u8g2_for_adafruit_gfx.setForegroundColor(HX8357_WHITE);


  int displayW = tft.width();

  // Title
  u8g2_for_adafruit_gfx.setFont(u8g2_font_osb35_tf);
  int titleW = u8g2_for_adafruit_gfx.getUTF8Width("Välkommen!");
  int titleX = max(0, (displayW - titleW) / 2);
  u8g2_for_adafruit_gfx.setCursor(155, 70);
  u8g2_for_adafruit_gfx.print("Välkommen!");

  // Subtitle
  u8g2_for_adafruit_gfx.setFont(u8g2_font_osb21_tf);
  u8g2_for_adafruit_gfx.setCursor(170, 120);
  u8g2_for_adafruit_gfx.print("Dagens lunch är:");

  // Wrap food into max 2 lines
  const int maxLines = 2;
  String lines[maxLines];
  int margin = 8;  // left/right margin
  int availW = displayW - margin * 2;

  // make sure the font used for menu measurement & rendering is set
  u8g2_for_adafruit_gfx.setFont(u8g2_font_osb21_tf);
  int n = wrapTextToLines(food, availW, lines, maxLines);

  // center & draw wrapped lines
  int y0 = 190;
  int lineHeight = 36;  // adjust to match font size
  for (int i = 0; i < n; ++i) {
    int w = u8g2_for_adafruit_gfx.getUTF8Width(lines[i].c_str());
    int x = max(0, (displayW - w) / 2);
    u8g2_for_adafruit_gfx.setCursor(x, y0 + i * lineHeight);
    u8g2_for_adafruit_gfx.print(lines[i]);
  }
  // Configure appearance using method chaining
  qrcode.setScale(3)                        // from 1 to 20
    .setColors(HX8357_BLACK, HX8357_WHITE)  // background, foreground
    .setRotation(QRCodeRotation::R0);       // 0°, 90°, 180°, or 270°

  // Configure qrcode generation
  qrcode.getGenerator()
    .setErrorCorrectionLevel(QRCodeECCLevel::Medium)
    .setVersion(7);  // force specific version, the QR-code version corresponds to how much information is able to be stored. The URL in this application is long and so a slightly higher QR-code is needed than if the URL had been www.google.com

  // Draw and check for errors
  String text = "https://docs.google.com/spreadsheets/d/1A9jNZFbBt8WnfRm9xxziX8ge5XegCjXRrjOzbJCytzs/edit?gid=0#gid=0";

  bool success = qrcode.draw(text, 5, 5);

  if (!success) {
    // Error generating QR code!
    // Possible causes:
    // - Text too long for selected version
    // - Not enough memory
  }
}
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

void getData() {
  // 1. Create secure client and bypass CA certificate checks
  WiFiClientSecure client;
  client.setInsecure();

  // 2. Initialize HTTPClient with the secure client and your server URL
  HTTPClient http;

  // Google Apps Script requires following HTTP 302/307 redirects
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (http.begin(client, serverName)) {
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.print("Data received: ");
      Serial.println(payload);
    } else {
      Serial.print("HTTP GET failed: ");
      Serial.println(http.errorToString(httpCode).c_str());
    }
    http.end();  // Always close connection
  } else {
    Serial.println("Unable to initialize connection to server URL");
  }
}

// Send aggregated batch of votes as JSON array: ["YYYY-MM-DD",Hate,Dislike,Neutral,Like,Love]
void sendBatchPost() {
  if (batchCount <= 0) return;  // nothing to send
  // build date string YYYY-MM-DD using system time
  time_t now;
  struct tm timeinfo;
  char dateBuf[32] = { 0 };
  time(&now);
  localtime_r(&now, &timeinfo);
  snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);

  // Build JSON array string
  String json = "[\"" + String(dateBuf) + "\"," + String(batchVotes[0]) + "," + String(batchVotes[1]) + "," + String(batchVotes[2]) + "," + String(batchVotes[3]) + "," + String(batchVotes[4]) + "]";
  Serial.print("Batch JSON: ");
  Serial.println(json);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(json);
    if (code > 0) {
      Serial.print("Batch POST response: ");
      Serial.println(code);
      String resp = http.getString();
      Serial.println(resp);
    } else {
      Serial.print("Batch POST failed: ");
      Serial.println(code);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected - batch not sent");
  }

  // Clear batch
  for (int i = 0; i < 5; ++i) batchVotes[i] = 0;
  batchCount = 0;
}

void displayData() {
  tft.fillScreen(HX8357_BLACK);
  tft.setCursor(0, 0);
  u8g2_for_adafruit_gfx.setBackgroundColor(HX8357_BLACK);
  u8g2_for_adafruit_gfx.setForegroundColor(HX8357_WHITE);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox3cb_tf);
  for (int i = 1; i < 7; i++) {
    tft.drawFastHLine(0, i * 50 - 10, 480, HX8357_WHITE);
    u8g2_for_adafruit_gfx.setCursor(5, i * 50 - 15);
    u8g2_for_adafruit_gfx.print(300 - i * 50);
  }
  for (int i = 0; i < 5; i++) {
    if (i == 1) {
      u8g2_for_adafruit_gfx.setCursor(i * 90 + 30, 310);
      u8g2_for_adafruit_gfx.print(voteLabels[i]);
      tft.fillRect(i * 90 + 60, 291 - votes[i], 30, votes[i], tft.color565(i * 50, 0, 100));
      tft.drawRect(i * 90 + 60, 291 - votes[i], 30, votes[i], HX8357_WHITE);
    } else if (i == 3) {
      u8g2_for_adafruit_gfx.setCursor(i * 90 + 70, 310);
      u8g2_for_adafruit_gfx.print(voteLabels[i]);
      tft.fillRect(i * 90 + 80, 291 - votes[i], 30, votes[i], tft.color565(i * 50, 0, 100));
      tft.drawRect(i * 90 + 80, 291 - votes[i], 30, votes[i], HX8357_WHITE);
    } else {
      u8g2_for_adafruit_gfx.setCursor(i * 90 + 50, 310);
      u8g2_for_adafruit_gfx.print(voteLabels[i]);
      tft.fillRect(i * 90 + 60, 291 - votes[i], 30, votes[i], tft.color565(i * 50, 0, 100));
      tft.drawRect(i * 90 + 60, 291 - votes[i], 30, votes[i], HX8357_WHITE);
    }
  }
  delay(3000);
}

// Wrap text into up to maxLines lines using pixel width from u8g2.
// Returns number of lines produced (<= maxLines).
int wrapTextToLines(const char* text, int maxWidth, String lines[], int maxLines) {
  String s = String(text);
  int lineCount = 0;
  String current = "";

  int pos = 0;
  while (pos <= s.length() && lineCount < maxLines) {
    // extract next word (space separated)
    int next = s.indexOf(' ', pos);
    String word;
    if (next == -1) {
      word = s.substring(pos);
      pos = s.length() + 1;  // end loop
    } else {
      word = s.substring(pos, next);
      pos = next + 1;
    }

    String candidate = current.length() ? (current + " " + word) : word;
    // measure candidate width (ensure font is set on u8g2_for_adafruit_gfx before calling)
    if (u8g2_for_adafruit_gfx.getUTF8Width(candidate.c_str()) <= maxWidth) {
      current = candidate;
    } else {
      if (current.length()) {
        lines[lineCount++] = current;
        current = word;
      } else {
        // single word longer than width -> break it roughly by characters
        String part = "";
        for (int i = 0; i < word.length() && lineCount < maxLines; ++i) {
          part += word[i];
          if (u8g2_for_adafruit_gfx.getUTF8Width(part.c_str()) > maxWidth) {
            // remove last char that overflowed
            part.remove(part.length() - 1);
            if (part.length()) {
              lines[lineCount++] = part;
            }
            part = String(word[i]);  // start new chunk with current char
          }
        }
        // leftover from broken word becomes current
        current = part;
      }
    }

    // final flush if at end
    if (pos > s.length() && lineCount < maxLines && current.length()) {
      lines[lineCount++] = current;
      break;
    }
  }

  return lineCount;
}
