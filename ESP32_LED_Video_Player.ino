#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>
#include <FastLED.h>

#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define LED_PIN 27
#define WIDTH 16
#define HEIGHT 16
#define NUM_LEDS (WIDTH * HEIGHT)
#define BRIGHTNESS 30
#define VIDEO_FPS 15
#define SERPENTINE true
#define FRAME_BYTES (NUM_LEDS * 3)
#define FRAME_INTERVAL (1000UL / VIDEO_FPS)

const char* AP_SSID = "LED-Video-Player";
const char* AP_PASSWORD = "LEDplayer2026"; // Change before sharing publicly
WebServer server(80);
CRGB leds[NUM_LEDS];
File videoFile;
String currentVideo;
bool playing = false;
bool sdReady = false;
unsigned long lastFrame = 0;
uint8_t frameBuffer[FRAME_BYTES];

int pixelIndex(int x, int y) {
  return y * WIDTH + ((SERPENTINE && (y & 1)) ? WIDTH - 1 - x : x);
}

void stopVideo() {
  playing = false;
  if (videoFile) videoFile.close();
  currentVideo = "";
  FastLED.clear(true);
}

String htmlEscape(String s) {
  s.replace("&", "&amp;"); s.replace("<", "&lt;");
  s.replace(">", "&gt;"); s.replace("\"", "&quot;");
  s.replace("'", "&#39;");
  return s;
}

String urlEncode(const String& s) {
  String out;
  for (size_t i = 0; i < s.length(); ++i) {
    unsigned char c = s[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.') out += char(c);
    else { char buf[4]; snprintf(buf, sizeof(buf), "%%%02X", c); out += buf; }
  }
  return out;
}

void handleRoot() {
  String html = "<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'><title>ESP32 LED Player</title><style>body{font-family:Arial;background:#111;color:#eee;text-align:center;padding:24px}a{display:block;background:#1682c5;color:white;padding:14px;margin:12px auto;max-width:380px;border-radius:9px;text-decoration:none}.stop{background:#bd3434}</style></head><body><h1>ESP32 LED Video Player</h1>";
  if (!sdReady) html += "<p>SD card not detected. Restart after checking the card.</p>";
  else {
    File root = SD.open("/");
    int count = 0;
    if (root) {
      File entry = root.openNextFile();
      while (entry) {
        String name = String(entry.name());
        String lower = name; lower.toLowerCase();
        if (!entry.isDirectory() && lower.endsWith(".rgb")) {
          String bare = name.startsWith("/") ? name.substring(1) : name;
          html += "<a href='/play?file=" + urlEncode(bare) + "'>" + htmlEscape(bare) + "</a>";
          count++;
        }
        entry.close();
        entry = root.openNextFile();
      }
      root.close();
    }
    if (!count) html += "<p>No .rgb files in SD card root.</p>";
  }
  if (playing) html += "<p>Playing: " + htmlEscape(currentVideo) + "</p>";
  html += "<a class='stop' href='/stop'>Stop</a></body></html>";
  server.send(200, "text/html", html);
}

void handlePlay() {
  if (!sdReady) { server.send(503, "text/plain", "SD card unavailable"); return; }
  String filename = server.arg("file");
  if (!filename.length() || filename.indexOf('/') >= 0 || filename.indexOf('\\') >= 0 || filename.indexOf("..") >= 0) {
    server.send(400, "text/plain", "Invalid filename"); return;
  }
  String lower = filename; lower.toLowerCase();
  if (!lower.endsWith(".rgb")) { server.send(400, "text/plain", "Only .rgb files supported"); return; }
  stopVideo();
  videoFile = SD.open("/" + filename, FILE_READ);
  if (!videoFile || videoFile.isDirectory()) {
    if (videoFile) videoFile.close();
    server.send(404, "text/plain", "Video not found"); return;
  }
  if (videoFile.size() < FRAME_BYTES || videoFile.size() % FRAME_BYTES != 0) {
    videoFile.close(); server.send(400, "text/plain", "Invalid frame data size"); return;
  }
  currentVideo = filename;
  playing = true;
  lastFrame = millis() - FRAME_INTERVAL;
  Serial.println("Playing " + filename);
  server.sendHeader("Location", "/"); server.send(303, "text/plain", "");
}

void handleStop() {
  stopVideo();
  server.sendHeader("Location", "/"); server.send(303, "text/plain", "");
}

void updateVideo() {
  if (!playing || !videoFile) return;
  unsigned long now = millis();
  if (now - lastFrame < FRAME_INTERVAL) return;
  lastFrame = now;
  if (videoFile.available() < FRAME_BYTES) videoFile.seek(0);
  if (videoFile.read(frameBuffer, FRAME_BYTES) != FRAME_BYTES) { stopVideo(); return; }
  for (int y = 0; y < HEIGHT; ++y)
    for (int x = 0; x < WIDTH; ++x) {
      int source = (y * WIDTH + x) * 3;
      leds[pixelIndex(x, y)] = CRGB(frameBuffer[source], frameBuffer[source+1], frameBuffer[source+2]);
    }
  FastLED.show();
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);
  FastLED.clear(true);
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  sdReady = SD.begin(SD_CS, SPI, 1000000);
  Serial.println(sdReady ? "SD CARD DETECTED" : "SD CARD FAILED");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("Open http://"); Serial.println(WiFi.softAPIP());
  server.on("/", HTTP_GET, handleRoot);
  server.on("/play", HTTP_GET, handlePlay);
  server.on("/stop", HTTP_GET, handleStop);
  server.begin();
}

void loop() {
  server.handleClient();
  updateVideo();
  delay(1);
}
