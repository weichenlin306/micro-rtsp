#include "OV2640.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>

#include "SimStreamer.h"
#include "OV2640Streamer.h"
#include "CRtspSession.h"

#include "wifikeys.h"

OV2640 cam;

WebServer server(80);
#define RTSP_PORT 554
WiFiServer rtspServer(RTSP_PORT);

void handle_jpg_stream(void)
{
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (1) {
    cam.run();
    if (!client.connected())
      break;
    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n\r\n";
    server.sendContent(response);

    client.write((char *)cam.getfb(), cam.getSize());
    server.sendContent("\r\n");
    if (!client.connected())
      break;
  }
}

void handle_jpg(void)
{
  WiFiClient client = server.client();

  cam.run();
  if (!client.connected()) {
    return;
  }
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-disposition: inline; filename=capture.jpg\r\n";
  response += "Content-type: image/jpeg\r\n\r\n";
  server.sendContent(response);
  client.write((char *)cam.getfb(), cam.getSize());
}

void handleNotFound()
{
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(200, "text/plain", message);
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  // Supported boards: esp32cam_config, esp32cam_aithinker_config, esp32cam_ttgo_t_config
  camera_config_t config = esp32cam_aithinker_config;
  // Possible frame sizes: UXGA(1600x1200),SXGA(1280x1024),XGA(1024x768),SVGA(800x600),VGA(640x480),CIF(400x296),QVGA(320x240),HQVGA(240x176),QQVGA(160x120)
  config.frame_size = FRAMESIZE_VGA;
  // Clock frequency: 10MHz-20MHz
  config.xclk_freq_hz = 15000000;
  cam.init(config);

  sensor_t *s = esp_camera_sensor_get();
  // s->set_vflip(s, 1);        // Vertical flip
  // s->set_hmirror(s, 1);      // Horizontal mirror
  // s->set_brightness(s, 1);   // -2 ~ 2
  // s->set_saturation(s, -2);  // -2 ~ 2

  IPAddress ip;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  ip = WiFi.localIP();
  Serial.println(F("WiFi connected"));
  Serial.print("IP: ");
  Serial.println(ip);
  Serial.println();

  server.on("/", HTTP_GET, handle_jpg_stream);
  server.on("/jpg", HTTP_GET, handle_jpg);
  server.onNotFound(handleNotFound);
  server.begin();

  rtspServer.begin();

  Serial.println("Usage:");
  Serial.print("MJPEG stream: http://");
  Serial.println(WiFi.localIP());
  Serial.print("Still image: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/jpg");
  Serial.print("RTSP stream: rtsp://");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.print(String(RTSP_PORT));
  Serial.println("/mjpeg/1");
}

CStreamer *streamer;
CRtspSession *session;
WiFiClient client; // FIXME, support multiple clients

void loop()
{
  server.handleClient();

  uint32_t msecPerFrame = 150;
  static uint32_t lastimage = millis();

  // If we have an active client connection, just service that until gone
  // (FIXME - support multiple simultaneous clients)
  if (session) {
    session->handleRequests(0); // we don't use a timeout here,
    // instead we send only if we have new enough frames

    uint32_t now = millis();
    if (now > lastimage + msecPerFrame || now < lastimage) { // handle clock rollover
      session->broadcastCurrentFrame(now);
      lastimage = now;

      // check if we are overrunning our max frame rate
      now = millis();
      if (now > lastimage + msecPerFrame)
        printf("warning exceeding max frame rate of %d ms\n", now - lastimage);
    }

    if(session->m_stopped) {
      delete session;
      delete streamer;
      session = NULL;
      streamer = NULL;
    }
  } else {
    client = rtspServer.accept();

    if (client) {
      //streamer = new SimStreamer(&client, true);             // our streamer for UDP/TCP based RTP transport
      streamer = new OV2640Streamer(&client, cam);             // our streamer for UDP/TCP based RTP transport

      session = new CRtspSession(&client, streamer); // our threads RTSP session and state
    }
  }
}