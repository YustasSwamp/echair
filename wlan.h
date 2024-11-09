#include <WiFi.h>

#define ACCESS_POINT

#ifdef ACCESS_POINT

# ifdef ECHAIR_4x4
const char* ssid = "eChair 4x4";
# else
const char* ssid = "eChair";
# endif
#include "password.h"

#else
//const char* ssid = "<WiFi network to connect to>";
//const char* password = "<password>";
#endif


void wifi_setup() {
#ifdef ACCESS_POINT
  WiFi.mode(WIFI_AP);
  if(! WiFi.softAP(ssid, password)) {
    Serial.println("WiFi AP creation failed! Rebooting...");
    ESP.restart(); //reset because there's no good reason for setting up an AP to fail
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
#else
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif


}
