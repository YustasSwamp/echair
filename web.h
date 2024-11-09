#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include <ESPmDNS.h>

AsyncWebServer server(80);

bool next_word(uint8_t *data, size_t len, int *i) {
    String str = "";

    /* Skip spaces */
    for(;*i < len; *i++) {
      if (char(data[*i]) == ' ');
        continue;
    }

    for(;*i < len; *i++) {
      char ch = char(data[*i]);
      if (ch == ' ')
        break;
      str += ch;
    }
    return str;
  }

/* Message callback of WebSerial */
void recvMsg(uint8_t *data, size_t len) {
  String word;
  int i = 0;

  auto next_word = [&] () -> bool {
    word = "";

    /* Skip spaces */
    for(;i < len; i++) {
      if (char(data[i]) != ' ')
        break;
    }

    for(;i < len; i++) {
      char ch = char(data[i]);
      if (ch == ' ')
        break;
      word += ch;
    }
    return word.length() != 0;
  };

#if GYRO_ENABLED
  next_word();
  if (word == "pid") {
    double kp, ki, kd;
    if (!next_word()) {
      /* No args, print current values */
      WebSerial.printf("pid %f %f %f\n", pid.GetKp(), pid.GetKi(), pid.GetKd());
      return;
    }

    kp = word.toFloat();

    if (!next_word())
      goto err;
    ki = word.toFloat();
    
    if (!next_word())
      goto err;
    kd = word.toFloat();
    pid.SetTunings(kp, ki, kd);
    WebSerial.println("pid command completed");
  }
#endif

  return;

err:
  WebSerial.println("Invalid arguments");
}

void webserial_setup() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! This is WebSerial demo. You can access webserial interface at http://" + WiFi.softAPIP().toString() + "/webserial");
  });

  // WebSerial is accessible at "http://<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  /* Attach Message Callback */
  WebSerial.onMessage(recvMsg);
//  server.onNotFound([](AsyncWebServerRequest* request) { request->redirect("/webserial"); });
  server.begin();

#if 0
  // Set up mDNS responder. WebSerial can be reached at http://echair.local/webserial
  if (!MDNS.begin("echair")) {
    ESP.restart(); //reset because there's no good reason for setting up MDNS to fail
  }
  MDNS.addService("http", "tcp", 80);
#endif
}

void webserial_handle() {
  WebSerial.loop();
}