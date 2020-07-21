#include "Arduino.h"
#include "M5StickC.h"
#include "WiFi.h"

/**
 * not under source control, credentials.h needs the following constants:
 * const char* ssid="yourwifissid";
 * const char* wifi_pw="yourwifipassword";
 * const char* api_key="yournewsapi.orgkey";
 **/

#include "credentials.h"

void displayNonsense(void *params)
{
  TFT_eSprite *sprite = (TFT_eSprite *)params;
  sprite->createSprite(120, 80);
  sprite->setTextColor(TFT_BLUE);
  sprite->setTextSize(3);
  for (;;)
  {
    sprite->fillSprite(TFT_BLACK);
    sprite->setCursor(0, 20);
    sprite->printf("%d", rand() % 100);
    sprite->pushSprite(0, 0);
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

void displayVoltage(void *params)
{
  TFT_eSprite *sprite = (TFT_eSprite *)params;
  sprite->createSprite(40, 15);
  sprite->setTextColor(TFT_GREENYELLOW);
  sprite->setTextSize(1);
  for (;;)
  {
    sprite->fillSprite(TFT_BLACK);
    sprite->setCursor(0, 0);
    sprite->printf("%.3fv", M5.Axp.GetBatVoltage());
    sprite->pushSprite(120, 0);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void maintainWiFi(void *params)
{
  int wifiRecoverTimeMs = 30000;
  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connected to the WiFi network");
      vTaskDelay(pdMS_TO_TICKS(60000));
      continue;
    }
    WiFi.begin(ssid, wifi_pw);
    unsigned long beginTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - beginTime < wifiRecoverTimeMs)
    {
      ;
    }
    vTaskDelay(pdMS_TO_TICKS(wifiRecoverTimeMs));
  }
}

void setup()
{
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.setRotation(3);
  TFT_eSprite *voltageSprite = new TFT_eSprite(&M5.Lcd);
  TFT_eSprite *dummySprite = new TFT_eSprite(&M5.Lcd);
  xTaskCreate(
      maintainWiFi,
      "maintainWiFi",
      5000,
      nullptr,
      1,
      nullptr);

  xTaskCreate(
      displayNonsense,
      "displayNonsense",
      2048,
      dummySprite,
      1,
      nullptr);

  //Consecutive Voltage measurements don't seem to work on core 0
  xTaskCreatePinnedToCore(
      displayVoltage,
      "displayVoltage",
      2048,
      voltageSprite,
      1,
      nullptr, 1);
}

void loop()
{
  vTaskDelete(nullptr);
}