#ifndef __DEVICE_CONFIG_H__
#define __DEVICE_CONFIG_H__

#include <FS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

namespace MyIOT
{
class DeviceConfig
{
  static constexpr const char *CONFIG_FILE = "/config.json";
  
  public:
  DeviceConfig():deviceName{0}, mqttServer{0}, state{0} {}

  const char* getDeviceName() const{return deviceName;}
  const char* getMqttServer() const {return mqttServer;}
  const char* getState() const {return state;}

  void setDeviceName(const char* name) { secureCopy(deviceName, name, sizeof(deviceName)); }
  void setMqttServer(const char* server) { secureCopy(mqttServer, server, sizeof(mqttServer)); }
  void setState(const char* server) { secureCopy(state, server, sizeof(state)); }
  
  void setup(uint8_t configurePin = 0, int checkPinState = LOW )
  {
     fsReadConfig();

     String hostname(this->getDeviceName());
     WiFi.mode(WiFiMode::WIFI_STA);
     WiFi.hostname(hostname);
     WiFi.begin();
     pinMode(configurePin, INPUT);

     unsigned long counter = 0;
     while (WiFi.status() != WL_CONNECTED)
     {
       counter++;
       delay(1);
       yield();
       if (checkPinState == digitalRead(configurePin))
       {
         if (8 < hostname.length())
         {
           hostname += ESP.getChipId();
         }

         info("Starting config portal");
         startConfigPortal(hostname.c_str(), "geheim1234");
         break;
       }
       if (0 == counter%5000)
       {
         info("Try to connect to SSID: ", WiFi.SSID().c_str());
       }
     }
     info("WiFi connected: ", WiFi.SSID().c_str() );
  }

  void save(){fsSaveConfig();}
  
  private:

    void startConfigPortal(const char* hostname, const char* password)
    {
      WiFiManager wifiManager;
      WiFiManagerParameter custom_device_name("device", "Device Name", deviceName, sizeof(deviceName));
      WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqttServer, sizeof(mqttServer));

      wifiManager.addParameter(&custom_device_name);
      wifiManager.addParameter(&custom_mqtt_server);

      wifiManager.setSaveConfigCallback([](){saveConfig = true;});

      wifiManager.setConfigPortalTimeout(180);

      if (!wifiManager.autoConnect(hostname, password))
      {
        Serial.println("Configuration failed. -> reboot");
        ESP.restart();
      }

      if (saveConfig)
      {
        strncpy(deviceName, custom_device_name.getValue(), sizeof(deviceName));
        strncpy(mqttServer, custom_mqtt_server.getValue(), sizeof(mqttServer));
        fsSaveConfig();
      }
      WiFi.hostname(this->getDeviceName());
      IPAddress localIp = WiFi.localIP();
      Serial.print("localIp: ");
      Serial.println(localIp.toString());
    }

    void error(const char* msg)
    {
      Serial.print("DeviceConfig error: ");
      Serial.println(msg);
    }
  
    void info(const char* msg)
    {
      Serial.print("DeviceConfig info: ");
      Serial.println(msg);
    }
    void info(const char* msg1, const char* msg2)
    {
      Serial.print("DeviceConfig info: ");
      Serial.print(msg1);
      Serial.print(" ");
      Serial.println(msg2);
    }

    void secureCopy(char* buffer, const char* value, size_t bufferLength)
    {
	strncpy(buffer, value, bufferLength);
	buffer[bufferLength-1] = 0;
    }

    bool readValue(DynamicJsonDocument& json, const char* valueName, char* buffer, size_t bufferLength)
    {
      const char* stringValue = json[valueName];
      if (!stringValue) return false;
      secureCopy(buffer, stringValue, bufferLength);
      info(valueName, stringValue);
      return true;
    }

    void fsReadConfig()
    {
      info("fsReadConfig()");
      if (SPIFFS.begin())
      {
        if (SPIFFS.exists(CONFIG_FILE))
        {
          File configFile = SPIFFS.open(CONFIG_FILE, "r");
          if (configFile)
          {
            size_t size = configFile.size();
            std::unique_ptr<char[]> buffer(new char[size]); // dynamic memory !!!
            configFile.readBytes(buffer.get(), size);
            configFile.close();
            
            DynamicJsonDocument  doc(1024);
            auto jerror = deserializeJson(doc, buffer.get());
            if (!jerror)
            {
        	    readValue(doc, "device_name", deviceName, sizeof(deviceName));
        	    readValue(doc, "mqtt_server", mqttServer, sizeof(mqttServer));
        	    readValue(doc, "state", state, sizeof(state));
            }
            else error("failed to parse config file data");
          }
          else error("failed to open config file");
        }
        else info("no config file");
      }
      else error("SPIFFS.begin() failed");
    }

    void fsSaveConfig()
    {
      info("fsSaveConfig()");
      DynamicJsonDocument doc(1024);

      info("device_name", deviceName);
      info("mqtt_server", mqttServer);
                      
      doc["device_name"] = deviceName;
      doc["mqtt_server"] = mqttServer;
      doc["state"] = state;
      File configFile = SPIFFS.open(CONFIG_FILE, "w");
      if (configFile)
      {
        serializeJson(doc, configFile);
        configFile.close();
      } 
      else error ("failed to save config file");
    }

     char deviceName[40];
     char mqttServer[40];
     char state[40];
     static bool saveConfig;
};

bool DeviceConfig::saveConfig = false;
}
#endif
