import numpy as np
import matplotlib.pyplot as plt
from engine import co2_emission
from sampling import SamplingSystem
from sensor import MHZ19B
from controller import ESP8266

# --- Настройки симуляции ---
DURATION_SEC = 600        # 10 минут поездки
DT = 5                    # шаг 5 секунд (как у MH-Z19B)

# --- Создаём компоненты ---
sampler = SamplingSystem()
sensor = MHZ19B()
esp = ESP8266(sample_period_sec=DT)

# --- Массивы для графиков ---
times, true_ppm_list, measured_ppm_list, temps = [], [], [], []

# --- Цикл симуляции ---
for step in range(0, DURATION_SEC, DT):
    # 1. Меняем режим движения (ускорение/торможение/стоянка)
    speed = 60 + 30 * np.sin(step / 100)       # км/ч
    fuel = 25 + 10 * np.sin(step / 80)         # л/ч
    
    # 2. Считаем «истинный» выброс
    true_ppm = co2_emission(step, speed, fuel)
    
    # 3. Разбавляем и охлаждаем
    diluted = sampler.dilute(true_ppm)
    T = sampler.cool(450, step % 60)
    
    # 4. Измеряем сенсором
    measured = sensor.read(diluted)
    
    # 5. Отправляем в контроллер
    esp.add_sample(measured, T, speed)
    
    # 6. Запоминаем для графиков
    times.append(step)
    true_ppm_list.append(diluted)
    measured_ppm_list.append(measured)
    temps.append(T)

# --- Сохраняем данные ---
esp.flush_to_pc("trip_data.json")

# --- Строим графики ---
fig, ax = plt.subplots(3, 1, figsize=(10, 8))

ax[0].plot(times, true_ppm_list, label="True concentation (duited)")
ax[0].plot(times, measured_ppm_list, label="MH-Z19B indications", alpha=0.7)
ax[0].set_ylabel("CO₂, ppm")
ax[0].legend()
ax[0].grid(True)

ax[1].plot(times, temps, color="orange")
ax[1].set_ylabel("Sample temperature, °C")
ax[1].grid(True)

ax[2].plot(times, [60 + 30 * np.sin(t/100) for t in times], color="green")
ax[2].set_ylabel("Speed, km/h")
ax[2].set_xlabel("Time, sec")
ax[2].grid(True)

plt.tight_layout()
plt.savefig("digital_twin_result.png", dpi=150)
plt.show()