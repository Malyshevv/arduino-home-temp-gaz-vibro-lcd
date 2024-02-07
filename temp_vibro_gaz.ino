#include <Wire.h>
//Дисплей
#include <GyverOLED.h>
//Температура и влажность
#include <DHT.h>

// #include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BMP280.h>

#define DHTPIN 2
#define SW420 9
#define MQ8 A0
#define MQ8PIN 6
#define SIGNALPIN 12
//#define BMP280_ADDRESS 0x27

//Adafruit_BMP280 bmp280;
DHT dht(DHTPIN, DHT22);
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(MQ8PIN, INPUT);
  pinMode(SW420, INPUT);
  pinMode(SIGNALPIN,OUTPUT);
  //bmp280.begin(BMP280_ADDRESS)

  oled.init();        // инициализация
  oled.clear();       // очистка
  oled.setScale(1);   // масштаб текста (1..4)
  oled.home();        // курсор в 0,0
}

void loop() {
  oled.clear();
  dhtDisplay();
  vibroDisplay();
  gasDisplay();
  //bmpDisplay();
  delay(5000);  // Задержка перед повторным чтением данных
}

void getDHT(float dataArray[]) {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Ошибка чтения с датчика DHT22");
  } else {
    float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    dataArray[0] = humidity;
    //dataArray[1] = temperature;
    dataArray[1] = heatIndex;
  }
}

void dhtDisplay() {
  float sensorData[2];
  getDHT(sensorData);
  
  oled.setCursor(0, 0);

  const float humidity = sensorData[0];
  const float heatIndex = sensorData[1];
  char humidityStr[6];
  char heatIndexStr[6];

  dtostrf(humidity, 6, 2, humidityStr);
  dtostrf(heatIndex, 6, 2, heatIndexStr);
  displayShow(0, 0, "H %: ", humidityStr);  // Updated to use humidityStr instead of humidity
  displayShow(70, 0, "T: ", heatIndexStr);
}

const char* getVibro() {
  int sensorValue = digitalRead(SW420); // Считывание значения с датчика
  
  if (sensorValue == HIGH) {
    zoomerStartStop(1);
    return 1; // Вывод сообщения, если обнаружена вибрация
  } else {
    zoomerStartStop(2);
    return 0; // Вывод сообщения, если вибрация не обнаружена
  }
}

void vibroDisplay() {
  const char* vibro = getVibro();

  displayShow(0, 17, "Вибраций: ", vibro ? "Да" : "Нет");
}

const int getGas(int type) {
  int gasValue = analogRead(MQ8);
  int isDanger = digitalRead(MQ8PIN);
  
  if (type == 1) {
    return gasValue;
  }

  if (type == 2) {
    if (isDanger == 1) {
      zoomerStartStop(2);
      return 0;
    } else {
      zoomerStartStop(1);
      return 1;
    }
  }
}

void gasDisplay() {
  int gasValue = getGas(1);
  const char* isDanger = getGas(2);

  char buffer[10];
  itoa(gasValue, buffer, 10);
  displayShow(0, 37, "N Газа: ", buffer);
  displayShow(0, 55, "Опасен: ", isDanger ? "Да" : "Нет");
}


// const float bmpData(int type) {
//   float temperature = bmp280SPI.readTemperature();
//   float pressure = bmp280SPI.readPressure();
//   float altitude = bmp280SPI.readAltitude(1013.25);

//   if (type == 1) {
//     return temperature;
//   }

//   if (type == 2) {
//     return pressure;
//   }

//   if (type == 3) {
//     return altitude;
//   }
// }

// void bmpDisplay() {
//   float temperature = bmpData(1);
//   float pressure = bmpData(2);
//   float altitude = bmpData(3);
//   displayShow(0, 77, "Температура ", temperature); 
//   displayShow(0, 95, "Давление", pressure);
//   displayShow(0, 120, "Высота", altitude);
// }


void displayShow(int x, int y, const char* text, const char* value) {
  oled.setCursorXY(x, y);
  oled.print(text);
  oled.print(value);

  return true;
}


void zoomerStartStop(int type) {
  if (type == 1) {
    Serial.println("signal on");
    tone(SIGNALPIN, 2000);
  } else {
    Serial.println("signal off");
    noTone(SIGNALPIN);
  }
}


