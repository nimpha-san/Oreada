#include <Arduino.h>

// Буфер для накопления данных
struct Sample {
  unsigned long time_ms;
  int ppm;
  float temp_c;
  float speed_kmh;
};

Sample samples[200];   // до 200 точек (примерно 15 минут)
int sample_count = 0;

// Виртуальные датчики (для эмуляции условий)
float fake_speed = 60.0;
float fake_temp = 45.0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("=== Ореада: старт ===");
  Serial.println("time_ms,ppm,temp_c,speed_kmh");
}

int read_co2() {
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  Serial2.write(cmd, 9);
  
  delay(100);
  
  if (Serial2.available() >= 9) {
    byte resp[9];
    Serial2.readBytes(resp, 9);
    if (resp[0] == 0xFF && resp[1] == 0x86) {
      return resp[2] * 256 + resp[3];
    }
  }
  return -1;   // ошибка
}

void loop() {
  int ppm = read_co2();
  
  // Имитация изменения условий (как будто машина едет)
  fake_speed = 60 + 30 * sin(millis() / 10000.0);
  fake_temp = 40 + 10 * sin(millis() / 15000.0);
  
  if (ppm > 0 && sample_count < 200) {
    samples[sample_count].time_ms = millis();
    samples[sample_count].ppm = ppm;
    samples[sample_count].temp_c = fake_temp;
    samples[sample_count].speed_kmh = fake_speed;
    
    // Вывод в формате CSV — удобно для Excel и Python
    Serial.print(millis());
    Serial.print(",");
    Serial.print(ppm);
    Serial.print(",");
    Serial.print(fake_temp, 1);
    Serial.print(",");
    Serial.println(fake_speed, 1);
    
    sample_count++;
  }
  
  // Каждые 10 точек — вывод статистики
  if (sample_count > 0 && sample_count % 10 == 0) {
    long sum = 0;
    for (int i = 0; i < sample_count; i++) sum += samples[i].ppm;
    Serial.print("# Среднее за ");
    Serial.print(sample_count);
    Serial.print(" точек: ");
    Serial.print(sum / sample_count);
    Serial.println(" ppm");
  }
  
  delay(5000);
}