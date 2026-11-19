#if !defined(CONFIG_IDF_TARGET_ESP32S3)
#error "Select ESP32S3 Dev Module for this v2 sketch"
#endif

#include <WiFi.h>
#include <WebServer.h>

const char* WIFI_NAME = "S3-Hello-Team01";
const char* WIFI_PASSWORD = "esp32wifi";

WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(500);

  // ESP32가 휴대폰이 접속할 Wi-Fi를 만듭니다.
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(WIFI_NAME, WIFI_PASSWORD)) {
    Serial.println("Wi-Fi start failed");
    return;
  }

  Serial.print("Wi-Fi name: ");
  Serial.println(WIFI_NAME);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // 브라우저에서 첫 화면을 요청하면 HTML을 보냅니다.
  server.on("/", HTTP_GET, []() {
    server.send(
      200,
      "text/html; charset=utf-8",
      "<!doctype html><html lang='ko'>"
      "<meta charset='utf-8'>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>ESP32 Hello</title>"
      "<h1>Hello, World!</h1>"
      "<p>ESP32 Wi-Fi 연결 성공!</p>"
      "</html>"
    );
  });

  server.begin();
  Serial.println("Open http://192.168.4.1");
}

void loop() {
  // 브라우저 요청을 계속 처리합니다.
  server.handleClient();
}
