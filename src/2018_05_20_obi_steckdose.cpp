
#include "Arduino.h"
#include <ESP.h>
#include <ESP8266WiFi.h>
#include "myiot_DeviceConfig.h"
#include "myiot_webServer.h"
#include "myiot_ota.h"
#include "myiot_timer_system.h"
#include "myiot_mqtt.h"
#include "ObiRelay.h"
#include "StatusLED.h"
#include "DeviceButton.h"

enum
{
  LED = 4,            // <-- blaue LED im Geh�use
  BUTTON = 14,
  RELAY_OFF = 5,
  RELAY_ON = 12
};

MyIOT::DeviceConfig config;
MyIOT::TimerSystem tsystem;
MyIOT::Mqtt mqtt;
MyIOT::OTA ota;
MyIOT::WebServer webServer;

ObiRelay relay;
StatusLED statusLed;
DeviceButton button(BUTTON);




void dump();




void setup()
{
  Serial.begin(76800);
  statusLed.setup(LED);

  statusLed.setError(StatusLED::Setup);
  config.setup(BUTTON, LOW);
  statusLed.setError(StatusLED::Ok);

  ota.setup(config.getDeviceName());
  webServer.setup(config);
  mqtt.setup(config.getDeviceName(), config.getMqttServer());

  pinMode(BUTTON, INPUT); // Push button, released: LOW, pressed: HIGH
  //pinMode(LED, OUTPUT); // Blue LED

  relay.setup();
  button.setup([](){relay.toggle();});

  tsystem.add(&ota, MyIOT::TimerSystem::TimeSpec(0, 100e6));  // 100ms
  tsystem.add(&webServer, MyIOT::TimerSystem::TimeSpec(0, 100e6));  // 100ms
  tsystem.add(&mqtt, MyIOT::TimerSystem::TimeSpec(0, 100e6));  // 100ms

  tsystem.add([](){dump();}, MyIOT::TimerSystem::TimeSpec(5));  // 5 s
  tsystem.add([](){relay.expire();}, MyIOT::TimerSystem::TimeSpec(0, 10e6));  // 10ms
  tsystem.add(&button, MyIOT::TimerSystem::TimeSpec(0, 100e6));  // 100ms

  mqtt.subscribe("control",[](const char*msg){
    if (0 == strcmp(msg, "on")) relay.enable(true);
    else if (0 == strcmp(msg, "off")) relay.enable(false);
    else if (0 == strcmp(msg, "toggle")) relay.toggle();
    else {
      int seconds = ::atoi(msg);
      if (seconds>0)
      {
        relay.timed_on(seconds);
      }
    }
  });
}

void dump()
{
  IPAddress ip = WiFi.localIP();
  Serial.print("localIp: "); Serial.println(ip.toString());

  String mac = WiFi.macAddress();
  Serial.print("mac: "); Serial.println(mac);

  uint32_t flashSize = ESP.getFlashChipSize();
  String  sFlashSize(flashSize);
  Serial.print("flashSize: "); Serial.println(sFlashSize);

  uint32_t realFlashSize = ESP.getFlashChipRealSize();
  String sRealFlashSize(realFlashSize);
  Serial.print("realFlashSize: "); Serial.println(sRealFlashSize);

  int32_t rssi = WiFi.RSSI(); // receive signal strength
  String sRSSI(rssi);
  Serial.print("rssi: "); Serial.println(sRSSI);


  //digitalWrite(LED , ! digitalRead(LED));
  //digitalWrite(RELAY_OFF , ! digitalRead(RELAY_OFF));  ... rote LED schaltet mit "RELAY_OFF"
//  digitalWrite(RELAY_ON , ! digitalRead(RELAY_ON));

  Serial.println("Hello");
  bool button = digitalRead(BUTTON);
  if (button)
  {
    Serial.println("Button released");
  }
  else
  {
    Serial.println("Button pressed");
  }
}



void loop()
{
  tsystem.run_loop(1, 1);
}
