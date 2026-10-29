// Slow radio check for a Seeed Studio XIAO ESP32S3 Sense.
// Produced by Godswind Consulting.
//
// Leave the camera module unplugged. This sketch never starts it.
//
// kJoinRouter, below, chooses the radio.
// true: the board joins the network in secrets.h. Copy secrets.h.example
// to secrets.h and set the name and password. secrets.h is gitignored.
// This is the test that shows a slow board. The serial monitor prints the
// address. Run:  python3 slow_radio.py http://that-address
// false: the board opens an open hotspot named XIAO-RADIO at 192.168.4.1.
// Run:  python3 slow_radio.py
// That hotspot can look fast on a slow board, including when the signal is
// no stronger than the router link that stalls. It is not the test that shows
// the fault. secrets.h must still be present so the sketch can compile.
// The password is not used.
//
// The board sends the same 24 KB block until the computer hangs up. 24 KB is the size of
// one JPEG from the camera that was slow. The bytes are placed in internal
// memory first, because that is the only memory the Wi-Fi block can send.
// The serial monitor at 115200 prints the same times the Python script prints.
// A good board stays near 20 to 30 ms. A bad board stalls again and again,
// and those stalls last hundreds of milliseconds. Signal strength is not the fault.

#include "secrets.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>

// true: join the router in secrets.h. false: open the XIAO-RADIO hotspot.
const bool kJoinRouter = true;

static const int kBytes = 24 * 1024;
static const char *kHotspotSsid = "XIAO-RADIO";

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

  // Keep the network settings in RAM. The Wi-Fi library would otherwise save them in NVS.
  WiFi.persistent(false);
  WiFi.setSleep(false);
  if (kJoinRouter) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("Joining %s\n", WIFI_SSID);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - start > 20000) {
        Serial.println("wifi failed");
        return;
      }
      delay(200);
    }
    Serial.printf("ready http://%s\n", WiFi.localIP().toString().c_str());
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    if (!WiFi.softAP(kHotspotSsid)) {
      Serial.println("access point failed");
      return;
    }
    Serial.println("Join XIAO-RADIO (no password), then run slow_radio.py");
    Serial.println("Board address http://192.168.4.1");
  }
  server.begin();
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
  if (kJoinRouter) {
    Serial.printf("signal %d dBm\n", WiFi.RSSI());
  } else {
    wifi_sta_list_t stations = {};
    if (esp_wifi_ap_get_sta_list(&stations) == ESP_OK && stations.num > 0) {
      Serial.printf("signal %d dBm\n", stations.sta[0].rssi);
    }
  }
  int n = 0;
  while (client.connected()) {
    n++;
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
