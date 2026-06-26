<h1 align="center">🌡️ Smart Environment Hub</h1>
<p align="center"><b>Uyarlanabilir RGB geri bildirimli ESP32 tabanlı ortam izleme istasyonu</b></p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32-00979D?style=flat&logo=espressif&logoColor=white" />
  <img src="https://img.shields.io/badge/Simulated%20on-Wokwi-1A1A1A?style=flat&logo=arduino&logoColor=white" />
  <img src="https://img.shields.io/badge/Language-C%2B%2B%20(Arduino)-blue?style=flat" />
  <img src="https://img.shields.io/badge/License-MIT-green?style=flat" />
</p>

<p align="center">
  <a href="https://wokwi.com/projects/461371419163577345"><b>▶️ Canlı simülasyonu Wokwi'de çalıştır</b></a>
</p>

---

## 📌 Genel Bakış

**Smart Environment Hub**, ESP32 üzerinde çalışan; **sıcaklık, nem ve ortam ışığını** sürekli olarak ölçen bir simülasyon projesidir. Ölçüm sonuçları iki kanaldan iletilir: dönüşümlü ekranlar gösteren bir **16x2 I2C LCD** ve duruma göre renk değiştiren bir **RGB LED**. Tek bir buton, LCD ekranlarının görüntülenme süresini ayarlamaya yarar — basit bir sensör demosunu yapılandırılabilir bir izleme aracına dönüştüren küçük ama etkili bir kullanıcı deneyimi dokunuşu.

---

## ✨ Özellikler

- 🌡️ **DHT22** ile gerçek zamanlı sıcaklık ve nem ölçümü
- 💡 **LDR** ile ortam ışığı algılama, yaklaşık lux değerine dönüştürülmüş
- 📟 Canlı durum etiketleriyle otomatik dönen LCD ekranları (Nem → Sıcaklık → Işık)
- 🚦 Hem ışık hem sıcaklık koşullarına tepki veren **RGB LED durum göstergesi**
- 🔘 LCD yenileme süresini (2sn ↔ 4sn) değiştiren debounce'lu buton
- 📊 Hata ayıklama / veri inceleme için her 3 saniyede bir seri monitör loglaması

---

## 🧰 Donanım Bileşenleri

| Bileşen | Görevi | ESP32 Pini |
|---|---|---|
| ESP32 DevKit V1 | Ana kontrolcü | — |
| DHT22 | Sıcaklık & nem sensörü | GPIO 15 (veri), 3V3, GND |
| Fotodirenç (LDR) | Ortam ışığı sensörü | GPIO 34 (analog giriş), 3V3, GND |
| 16x2 LCD (I2C, adres `0x27`) | Durum ekranı | GPIO 21 (SDA), GPIO 22 (SCL), 3V3, GND |
| RGB LED (ortak katot) | Görsel durum göstergesi | GPIO 25 (R), 26 (G), 27 (B) — 220Ω dirençler üzerinden |
| Buton | Ekran süresi değiştirme | GPIO 18 (`INPUT_PULLUP`) |

---

## ⚙️ Nasıl Çalışır

**1. Sensör Okuma**
Her döngüde DHT22'den sıcaklık/nem okunur ve LDR'nin ham analog değeri (0–4095) yaklaşık **0–1000 lux** aralığına haritalanıp bu sınırlar içinde sabitlenir.

**2. Durum Sınıflandırması**

| Ölçüm | Eşik Değerleri |
|---|---|
| Nem | ≤ %60 → `LOW` · > %60 → `HIGH` |
| Sıcaklık | < 0°C → `VERY LOW` · 0–30°C → `NORMAL` · > 30°C → `HIGH` |
| Işık | < 10 lux → `KARANLIK` · ≥ 10 lux → `AYDINLIK` |

**3. RGB Geri Bildirim Mantığı**

| Koşul | LED Rengi |
|---|---|
| Karanlık ortam | 🔵 Mavi |
| Aydınlık + sıcaklık < 0°C | 🩵 Camgöbeği |
| Aydınlık + 0°C ≤ sıcaklık ≤ 30°C | 🟢 Yeşil |
| Aydınlık + 30°C < sıcaklık ≤ 40°C | 🟡 Sarı |
| Aydınlık + sıcaklık > 40°C | 🔴 Kırmızı |

**4. Ekran Döngüsü**
LCD, varsayılan olarak **2000ms** olan `lcdSure` süresi boyunca Nem → Sıcaklık → Işık ekranları arasında dönüşümlü olarak geçiş yapar.

**5. Buton Kontrolü**
Debounce'lu (50ms) buton, `lcdSure` değerini **2000ms ile 4000ms** arasında değiştirir; değişim sırasında ekranda kısa bir onay mesajı gösterilip ardından normal döngüye geri dönülür.

**6. Seri Loglama**
Her 3 saniyede bir, anlık nem, sıcaklık ve lux değerleri `115200` baud hızında Seri Monitöre yazdırılır.

---

## 🔌 Devre Şeması

Tüm kablolama ve bileşen yerleşimi canlı simülasyonda mevcuttur:

👉 **[Wokwi'de Aç](https://wokwi.com/projects/461371419163577345)**

<details>
<summary>📐 Pin bağlantı özeti</summary>

```
ESP32 D15  ──── DHT22 SDA
ESP32 3V3  ──── DHT22 VCC
ESP32 GND  ──── DHT22 GND

ESP32 3V3  ──── LDR VCC
ESP32 GND  ──── LDR GND
ESP32 D34  ──── LDR AO

ESP32 D25 ── 220Ω ── RGB LED (R)
ESP32 D26 ── 220Ω ── RGB LED (G)
ESP32 D27 ── 220Ω ── RGB LED (B)
RGB LED COM ──── GND

ESP32 D21 (SDA) ──── LCD SDA
ESP32 D22 (SCL) ──── LCD SCL
ESP32 3V3       ──── LCD VCC
ESP32 GND       ──── LCD GND

ESP32 D18 ──── Buton ──── GND
```
</details>

---

## 📊 Örnek Seri Çıktısı

```
Nem: 45.20% || Sicaklik: 23.10C || Isik: 412 lux
Nem: 45.00% || Sicaklik: 23.15C || Isik: 408 lux
LCD suresi degistirildi: 4 sn
```

---

## 🚀 Başlarken

**Simülasyonda çalıştırma (donanım gerekmez):**
1. [Wokwi proje linkini](https://wokwi.com/projects/461371419163577345) aç
2. Simülasyonu başlatmak için ▶️ **Play** butonuna tıkla
3. LCD ve RGB'nin canlı tepkisini görmek için simülatördeki DHT22/LDR değerlerini değiştir
4. Ekran süresini değiştirmek için ekrandaki butona tıkla

**Gerçek donanımda çalıştırma:**
1. Bileşenleri yukarıdaki [pin bağlantı özetine](#-devre-şeması) göre kabloya
2. Gerekli kütüphaneleri yükle: `LiquidCrystal I2C`, `DHT sensor library for ESPx`
3. `sketch.ino` dosyasını Arduino IDE veya PlatformIO ile ESP32'ye yükle
4. Canlı verileri görmek için Seri Monitörü `115200` baud hızında aç

---

## 📁 Proje Yapısı

```
smart-environment-hub/
├── sketch.ino          # Ana firmware mantığı
├── diagram.json         # Wokwi devre/kablolama tanımı
└── libraries.txt        # Gerekli Arduino kütüphaneleri
```

---

## 🛠️ Teknoloji Yığını

<p>
  <img src="https://img.shields.io/badge/ESP32-00979D?style=flat&logo=espressif&logoColor=white" />
  <img src="https://img.shields.io/badge/Arduino%20C%2B%2B-00979D?style=flat&logo=arduino&logoColor=white" />
  <img src="https://img.shields.io/badge/I2C-LCD%201602-blue?style=flat" />
  <img src="https://img.shields.io/badge/DHT22-Sensor-orange?style=flat" />
  <img src="https://img.shields.io/badge/Wokwi-Simulator-1A1A1A?style=flat" />
</p>

---

## 📄 Lisans

Bu proje **MIT Lisansı** ile lisanslanmıştır.

---

## 🙋 Geliştirici

**Kübra Parmak**
- GitHub: [@KbrPrmk](https://github.com/KbrPrmk)
- Wokwi: [@kbrprmk](https://wokwi.com/makers/kbrprmk)
