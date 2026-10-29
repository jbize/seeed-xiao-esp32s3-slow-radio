// Slow radio check for a Seeed Studio XIAO ESP32S3 Sense.
// Produced by Godswind Consulting.
//
// Leave the camera module unplugged. This sketch never starts it.
// Flash the sketch. Join the open Wi-Fi network XIAO-RADIO.
// Then, on the computer, run:  python3 slow_radio.py
//
// The board sends the same 24 KB block ten times. 24 KB is the size of
// one JPEG from the camera that was slow. The bytes are placed in internal
// memory first, because that is the only memory the Wi-Fi block can send.
// The serial monitor at 115200 prints the same times the Python script prints.
// A good board finishes a send in about 30 ms. A bad board averages 80 ms
// or more, and some sends take hundreds of milliseconds.

#include <WiFi.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>

static const int kBytes = 24 * 1024;
static const int kSends = 10;
static const char *kSsid = "XIAO-RADIO";

static WiFiServer server(80);
static uint8_t *buffer = nullptr;

static bool writeAll(WiFiClient &client) {
  size_t sent = 0;
  while (sent < kBytes) {
    int wrote = client.write(buffer + sent, kBytes - sent);
    if (wrote <= 0) {
      return false;
    }
    sent += wrote;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("slow radio");

  buffer = (uint8_t *)heap_caps_malloc(kBytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (buffer == nullptr) {
    Serial.println("no internal memory");
    return;
  }
  for (int i = 0; i < kBytes; i++) {
    buffer[i] = i & 0xff;
  }

  // Keep the access point in RAM. The Wi-Fi library would otherwise save it in NVS.
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  if (!WiFi.softAP(kSsid)) {
    Serial.println("access point failed");
    return;
  }
  server.begin();
  Serial.println("Join XIAO-RADIO (no password), then run slow_radio.py");
  Serial.println("Board address http://192.168.4.1");
}

void loop() {
  if (buffer == nullptr) {
    delay(1000);
    return;
  }
  WiFiClient client = server.accept();
  if (!client) {
    delay(10);
    return;
  }
  client.setNoDelay(true);
  for (int n = 1; n <= kSends; n++) {
    int64_t started = esp_timer_get_time();
    if (!writeAll(client)) {
      Serial.println("write failed");
      break;
    }
    int ms = (int)((esp_timer_get_time() - started) / 1000);
    client.printf("%d\n", ms);
    Serial.printf("send %d: %d ms\n", n, ms);
  }
  client.stop();
}
