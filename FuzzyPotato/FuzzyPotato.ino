//
//    FILE: FuzzyPotato.ino
//  AUTHOR: Jaewoong Mun (happybono@outlook.com)
// CREATED: April 25, 2021
//
// Released to the public domain
//

#include <WiFi.h>
#include <HTTPClient.h>

#include <esp_task_wdt.h>
#include "esp_system.h"
#include <algorithm>
#include <iostream>
#include <Arduino.h>
#include <DHT12.h>
#include <Wire.h>
#include <BH1750.h>

#define DHT12_PIN           16
#define BATT_ADC            33
#define SALT_PIN            34
#define SOIL_PIN            32
#define POWER_CTRL          4
#define I2C_SDA             25
#define I2C_SCL             26

#define AVGCNT              30
#define PERVALUE            0.0244141

DHT12 dht12(DHT12_PIN, true);
BH1750 lightMeter(0x23);

//60 seconds WDT
#define WDT_TIMEOUT 60

const char* ssid = "[Wi-Fi SSID]";
const char* password = "[Wi-Fi Password]";

// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://api.thingspeak.com/update";
// Service API Key
String apiKey = "[ThingSpeak Write API Key]";

void sleepGo() {
//  const int sleepPeriod = 10800;    /* 3 hour */
//  const int sleepPeriod = 3600;     /* 1 hour */
    const int sleepPeriod = 1800;     /* 30 minutes */

	uint64_t time_in_us;

	WiFi.mode(WIFI_OFF);

	time_in_us = 1000000 * (uint64_t)sleepPeriod; /* 1Sec(microSecond) * Sec */

	esp_sleep_enable_timer_wakeup( time_in_us );
	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
	
	// After entering Deep Sleep sleep mode, the setup() function is automatically executed when a set period of time has elapsed.
	// Deep Sleep 절전 모드 진입 후 지정된 시간이 경과하면 setup() 함수를 자동으로 실행합니다.
	esp_deep_sleep_start();
}

float readTemp()
{
  uint8_t samples = AVGCNT;
  float   array[AVGCNT];
  float   temp = 0.0;

  for (int i = 0; i < samples; i++) {
    while (1) {
      float _t12 = 0.0;

      _t12 = dht12.readTemperature();

      if (!isnan(_t12)) {
        array[i] = _t12;
        break;
      }
      delay(100);
    }
    delay(2);
  }

  std::sort(array, array + samples);
  for (int i = 0; i < samples; i++) {
    if (i == 0 || i == samples - 1)
      continue;
    temp += array[i];
  }

  temp /= samples - 2;
  return temp;
}

float readHumi()
{
  uint8_t samples = AVGCNT;
  float   array[AVGCNT];
  float   humi = 0.0;

  for (int i = 0; i < samples; i++) {
    while (1) {
      float _h12 = 0.0;

      _h12 = dht12.readHumidity();

      if (!isnan(_h12)) {
        array[i] = _h12;
        break;
      }
      delay(100);
    }
    delay(2);
  }

  std::sort(array, array + samples);
  for (int i = 0; i < samples; i++) {
    if (i == 0 || i == samples - 1)
      continue;
    humi += array[i];
  }

  humi /= samples - 2;
  return humi;
}

float readSalt()
{
  uint8_t samples = AVGCNT;
  float   array[AVGCNT];
  float   soil_s = 0.0;

  for (int i = 0; i < samples; i++) {
    uint32_t soil = 0;
    float    per_soil = 0.0;

    soil = analogRead(SALT_PIN);
    per_soil = soil * PERVALUE;

    array[i] = per_soil;

    delay(2);
  }

  std::sort(array, array + samples);
  for (int i = 0; i < samples; i++) {
    if (i == 0 || i == samples - 1)
      continue;
    soil_s += array[i];
  }

  soil_s /= samples - 2;
  return soil_s;
}

float readSoil()
{
  uint8_t samples = AVGCNT;
  float   array[AVGCNT];
  float   soil_m = 0.0;

  for (int i = 0; i < samples; i++) {
    uint32_t soil = 0;
    uint32_t mapsoil = 0;
    float per_soil = 0.0;

    soil = analogRead(SOIL_PIN);
	  
    // 토양 습도 데이터를 백분율 (%) 로 표시합니다.
    mapsoil = map(soil, 0, 4095, 4095, 0);
    per_soil = mapsoil * PERVALUE;

    array[i] = per_soil;

    delay(2);
  }

  std::sort(array, array + samples);
  for (int i = 0; i < samples; i++) {
    if (i == 0 || i == samples - 1)
      continue;
    soil_m += array[i];
  }

  soil_m /= samples - 2;
  return soil_m;
}

float readLux()
{
  uint8_t samples = AVGCNT;
  float   array[AVGCNT];
  float   lux = 0.0;

  // Calculates a value equal to the size of the samples array at 2ms per second.
  // 1 초당 2 ms 씩 samples 배열 크기 만큼 값을 계산합니다.
  for(int i=0; i<samples; i++) {
    array[i] = lightMeter.readLightLevel();
    delay(2);
  }
  
  // Sort the sample data using the sort function included in the C++ standard library.
  // C++ 의 표준라이브러리에 포함된 sort 함수를 사용해 샘플 데이터들을 정렬합니다.
  std::sort(array, array + samples);
  for(int i=0; i<samples; i++) {
    if (i == 0 || i == samples-1)
      continue;
    lux += array[i];
  }

  lux /= samples - 2;
  return lux;
}

float readBattery()
{
  uint8_t samples = AVGCNT;
  float   array[AVGCNT];
  float   batt_adc = 0;

  for (int i = 0; i < samples; i++) {
    int vref = 1100;
    uint16_t volt = analogRead(BATT_ADC);
	  
    // When detecting the remaining battery level, the maximum value is 3.3V, so the current operating voltage of 3.7V cannot be detected.
    // Therefore, the parallel circuit calculation value is derived, so the result must be multiplied by 2.
    // 배터리 잔량 감지 시, 최고값이 3.3V 이므로 현재 작동 전압인 3.7V 를 감지할 수 없습니다, 
    // 따라서 회로 자체가 병렬로 검사한 결과로 나오기 때문에, 결과값에 2 를 곱해 주어야 합니다.
    float battery_voltage = ((float)volt / 4095.0) * 2.0 * 3.3 * (vref);

    array[i] = battery_voltage;
    delay(2);
  }

  std::sort(array, array + samples);
  for (int i = 0; i < samples; i++) {
    if (i == 0 || i == samples - 1)
      continue;
    batt_adc += array[i];
  }

  batt_adc /= samples - 2;
  return batt_adc;
}

void setup() {
	Serial.begin(115200);

	/* Watchdog Timer Setup */
	Serial.println("Configuring WDT...");
	// enable panic so ESP32 restarts
	// watchDog 시작 기능을 60 초로 초기화합니다.
	esp_task_wdt_init(WDT_TIMEOUT, true);   
	// add current thread to WDT watch
	// 현재 프로세스 Thread 를 watchDog 에 등록합니다.
	esp_task_wdt_add(NULL);
	

  //! Sensor power control pin , use deteced must set high
  pinMode(POWER_CTRL, OUTPUT);
  digitalWrite(POWER_CTRL, 1);
  delay(10); 

  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(10);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE); 

  delay(1000);  

	/* WiFi Setup */
	WiFi.begin(ssid, password);
	Serial.println("Connecting");
	while(WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Connected to WiFi network with IP Address: ");
	Serial.println(WiFi.localIP());

  /* Soil Temperature And Humidity Photometric Electrolyte Sensor Setup */
  dht12.begin();
}

void loop() {
  /* Sensor Values */
  float t12 = 0.0;
  float h12 = 0.0;
  float soil = 0.0;
  float salt = 0.0;
  float lux = 0.0;
  float bat = 0.0;

	/* Watchdog Reset */
	// 실행하면, 2 초 이상 소요되지 않습니다. 
	esp_task_wdt_reset();

  /* Get Sensor Data */
  // Read temperature as Fahrenheit (isFahrenheit = true)
  t12 = readTemp();
  h12 = readHumi();
  soil = readSoil();
  salt = readSalt();
  lux = readLux();
  bat = readBattery();

  Serial.println("Temperature = " + String(t12, 2));
  Serial.println("Humidity    = " + String(h12, 2));
  Serial.println("soil = " + String(soil, 2));
  Serial.println("salt = " + String(salt, 2));
  Serial.println("lux = " + String(lux, 2));
  Serial.println("bat = " + String(bat, 2));

	//Check WiFi connection status
	if(WiFi.status()== WL_CONNECTED){
		/* Watchdog Reset */
		esp_task_wdt_reset();

		HTTPClient http;

		// Your Domain name with URL path or IP address with path
		http.begin(serverName);

		// Specify content-type header
		http.addHeader("Content-Type", "application/x-www-form-urlencoded");
		// Data to send with HTTP POST
		String httpRequestData = "api_key=" + apiKey + "&field1=" + String(t12, 2) + "&field2=" + String(h12, 2) + "&field3=" + String(lux, 2) + "&field4=" + String(soil, 2) + "&field5=" + String(salt, 2) + "&field6=" + String(bat, 2);           
		// Send HTTP POST request
		int httpResponseCode = http.POST(httpRequestData);

		Serial.print("HTTP Response code: ");
		Serial.println(httpResponseCode);

		// Free resources
		http.end();
	}
	else {
		Serial.println("WiFi Disconnected");
	}

	delay(1000);	
	/* Start Deep Sleep Mode */
  digitalWrite(POWER_CTRL, 0);
  delay(10);
  sleepGo();
}
