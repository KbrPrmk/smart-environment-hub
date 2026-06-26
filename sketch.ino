#include <Wire.h> // I2C haberlesmesi icin gerekli kutuphane
#include <LiquidCrystal_I2C.h>// I2C LCD kontrolu icin gerekli kutuphane
#include <DHTesp.h> // DHT22 sicaklik ve nem sensoru icin gerekli kutuphane

// LCD nesnesi olusturulur
// 0x27 = I2C adresi, 16 = sutun sayisi, 2 = satir sayisi
LiquidCrystal_I2C lcd(0x27, 16, 2);

// DHT sensoru icin nesne olusturulur
DHTesp dhtSensor;

#define DHT_PIN 15 // DHT22 veri pininin ESP32 uzerindeki baglandigi pin
#define LDR_PIN 34 // LDR sensorunun analog cikisinin baglandigi pin 
#define RED_PIN 25 // RGB LED'in kirmizi kanalinin baglandigi pin
#define GREEN_PIN 26 // RGB LED'in yesil kanalinin baglandigi pin 
#define BLUE_PIN 27 // RGB LED'in mavi kanalinin baglandigi pin
#define BUTTON_PIN 18 // Butonun baglandigi pin 

 
unsigned long lcdSure = 2000; // LCD'de her bir ekranin varsayilan gosterim suresi milisaniye cinsinden
unsigned long oncekiLcdZamani = 0; // LCD ekraninin son ne zaman guncellendigini tutan zaman degiskeni 
unsigned long oncekiSerialZamani = 0; // Seri monitore son ne zaman veri yazdirildigini tutan zaman degiskeni 
unsigned long sonButonDegisimZamani = 0; // Butonda son durum degisiminin ne zaman oldugunu tutan zaman degiskeni 
const unsigned long debounceSuresi = 50; // Buton ziplamasini engellemek icin debounce suresi 

// LCD'de hangi ekranin gosterilecegini belirleyen sira degiskeni 
// 0 = nem, 1 = sicaklik, 2 = isik
int ekranSirasi = 0;

bool sonButonOkumasi = HIGH; // Butondan okunan son ham deger 
bool kararliButonDurumu = HIGH; // Debounce sonrasi kabul edilen kararlı buton durumu 

float sicaklik = 0.0; // Sicaklik verisini tutmak icin degisken 
float nem = 0.0;// Nem verisini tutmak icin degisken 
int isikLux = 0; // Lux benzeri isik degerini tutmak icin degisken 
int ldrHamDeger = 0; // LDR'den okunan ham analog degeri tutmak icin degisken 

// LDR'den okunan ham analog degeri yaklasik lux degerine donusturen fonksiyon 
int luxHesapla(int analogDeger) {
  // 0-4095 araligindaki ADC degeri 0-1000 araligina haritalanir
  int lux = map(analogDeger, 4095, 0, 0, 1000);

  // Hesaplanan lux degeri sifirdan kucukse sifira sabitlenir
  if (lux < 0) {
    lux = 0;
  }

  // Hesaplanan lux degeri 1000'den buyukse 1000'e sabitlenir
  if (lux > 1000) {
    lux = 1000;
  }

  // Sonuc olarak hesaplanan lux degeri geri dondurulur
  return lux;
}

// Nem degerine gore LCD'de gosterilecek durum bilgisini donduren fonksiyon 
String nemDurumuGetir(float nemDegeri) {
  // Nem degeri 60 ve altindaysa LOW durumu dondurulur
  if (nemDegeri <= 60.0) {
    return "LOW";
  }

  // Nem degeri 60'tan buyukse HIGH durumu dondurulur
  return "HIGH";
}

// Sicaklik degerine gore LCD'de gosterilecek durum bilgisini donduren fonksiyon 
String sicaklikDurumuGetir(float sicaklikDegeri) {
  // Sicaklik sifirin altindaysa VERY LOW dondurulur
  if (sicaklikDegeri < 0.0) {
    return "VERY LOW";
  }

  // Sicaklik 0 ile 30 arasindaysa NORMAL dondurulur
  if (sicaklikDegeri <= 30.0) {
    return "NORMAL";
  }

  // Sicaklik 30'un ustundeyse HIGH dondurulur
  return "HIGH";
}

// Lux degerine gore LCD'de gosterilecek isik durumunu donduren fonksiyon 
String isikDurumuGetir(int luxDegeri) {
  // Lux degeri 10'dan kucukse ortam KARANLIK kabul edilir
  if (luxDegeri < 10) {
    return "KARANLIK";
  }

  // Diger durumlarda ortam AYDINLIK kabul edilir
  return "AYDINLIK";
}

// RGB LED'in tum renklerini kapatan fonksiyon 
void rgbKapat() {
  digitalWrite(RED_PIN, LOW); // Kirmizi kanal kapatilir
  digitalWrite(GREEN_PIN, LOW); // Yesil kanal kapatilir
  digitalWrite(BLUE_PIN, LOW); // Mavi kanal kapatilir
}

// RGB LED'e istenen renk kombinasyonunu yazan fonksiyon 
void rgbYaz(bool kirmizi, bool yesil, bool mavi) {
  // Kirmizi parametresi true ise HIGH, degilse LOW yazilir
  digitalWrite(RED_PIN, kirmizi ? HIGH : LOW);

  // Yesil parametresi true ise HIGH, degilse LOW yazilir
  digitalWrite(GREEN_PIN, yesil ? HIGH : LOW);

  // Mavi parametresi true ise HIGH, degilse LOW yazilir
  digitalWrite(BLUE_PIN, mavi ? HIGH : LOW);
}

// Mevcut isik ve sicaklik durumuna gore RGB LED rengini guncelleyen fonksiyon 
void rgbGuncelle() {
  // Ortam karanliksa RGB LED mavi yakilir
  if (isikLux < 10) {
    rgbYaz(false, false, true);
    return;
  }

  // Ortam aydinlik ve sicaklik 0'in altindaysa acik maviye yakin renk yakilir
  if (sicaklik < 0.0) {
    rgbYaz(false, true, true);
    return;
  }

  // Ortam aydinlik ve sicaklik 0 ile 30 arasindaysa yesil yakilir
  if (sicaklik <= 30.0) {
    rgbYaz(false, true, false);
    return;
  }

  // Ortam aydinlik ve sicaklik 30 ile 40 arasindaysa sari/turuncuya yakin renk yakilir
  if (sicaklik <= 40.0) {
    rgbYaz(true, true, false);
    return;
  }

  // Ortam aydinlik ve sicaklik 40'in ustundeyse kirmizi yakilir
  rgbYaz(true, false, false);
}

// DHT22 ve LDR sensorlerinden veri okuyup sistem degiskenlerini guncelleyen fonksiyon 
void sensorleriOku() {
  // DHT22 sensorunden sicaklik ve nem verileri okunur
  TempAndHumidity veri = dhtSensor.getTempAndHumidity();

  // Okunan sicaklik ve nem degerleri gecerliyse ana degiskenler guncellenir
  if (!isnan(veri.temperature) && !isnan(veri.humidity)) {
    // Sicaklik degiskenine sensor sicakligi atanir
    sicaklik = veri.temperature;

    // Nem degiskenine sensor nemi atanir
    nem = veri.humidity;
  }

  // LDR'den ham analog deger okunur
  ldrHamDeger = analogRead(LDR_PIN);

  // Okunan ham deger lux benzeri deger haline donusturulur
  isikLux = luxHesapla(ldrHamDeger);

  // Yeni sensor verilerine gore RGB LED durumu guncellenir
  rgbGuncelle();
}

// Sensor verilerini seri monitore formatli sekilde yazdiran fonksiyon 
void serialYazdir() {
  Serial.print("Nem: "); // Nem etiketi yazdirilir
  Serial.print(nem, 2); // Nem degeri iki ondalikli olarak yazdirilir
  Serial.print("% || Sicaklik: "); // Nem birimi ve ayirici yazdirilir
  Serial.print(sicaklik, 2); // Sicaklik degeri iki ondalikli olarak yazdirilir
  Serial.print("C || Isik: "); // Sicaklik birimi ve ayirici yazdirilir
  Serial.print(isikLux); // Isik degeri yazdirilir
  Serial.println(" lux"); // Lux birimi yazdirilir ve satir sonu eklenir
}

// Nem bilgisini LCD'de gosteren fonksiyon 
void nemEkraniGoster() {
  lcd.clear(); // LCD temizlenir

  lcd.setCursor(6, 0); // Imlec birinci satir 6. sutuna ayarlanir
  lcd.print("NEM");// Baslik olarak NEM yazdirilir
  lcd.setCursor(15, 0); // Imlec birinci satir 15. sutuna ayarlanir
  lcd.print(lcdSure / 1000); // Gosterim suresi saniye cinsinden yazdirilir
  lcd.setCursor(0, 1); // Imlec ikinci satir basina ayarlanir
  lcd.print(nem, 2); // Nem degeri iki ondalikli olarak yazdirilir
  lcd.print("-"); // Ayirici karakter yazdirilir
  lcd.print(nemDurumuGetir(nem)); // Nem durumu yazdirilir
}

// Sicaklik bilgisini LCD'de gosteren fonksiyon 
void sicaklikEkraniGoster() {
  lcd.clear(); // LCD temizlenir

  lcd.setCursor(3, 0); // Imlec birinci satir 3. sutuna ayarlanir
  lcd.print("SICAKLIK"); // Baslik olarak SICAKLIK yazdirilir
  lcd.setCursor(15, 0); // Imlec birinci satir 15. sutuna ayarlanir
  lcd.print(lcdSure / 1000); // Gosterim suresi saniye cinsinden yazdirilir
  lcd.setCursor(0, 1); // Imlec ikinci satir basina ayarlanir
  lcd.print(sicaklik, 2); // Sicaklik degeri iki ondalikli olarak yazdirilir
  lcd.print("-"); // Ayirici karakter yazdirilir
  lcd.print(sicaklikDurumuGetir(sicaklik)); // Sicaklik durumu yazdirilir
}

// Isik bilgisini LCD'de gosteren fonksiyon 
void isikEkraniGoster() {
  lcd.clear(); // LCD temizlenir

  lcd.setCursor(6, 0); // Imlec birinci satir 6. sutuna ayarlanir
  lcd.print("ISIK"); // Baslik olarak ISIK yazdirilir
  lcd.setCursor(15, 0); // Imlec birinci satir 15. sutuna ayarlanir
  lcd.print(lcdSure / 1000); // Gosterim suresi saniye cinsinden yazdirilir
  lcd.setCursor(0, 1); // Imlec ikinci satir basina ayarlanir
  lcd.print(" "); // Baslangicta bir bosluk yazdirilir
  lcd.print(isikDurumuGetir(isikLux));// Isik durumu yazdirilir
}

// LCD'de hangi ekranin gosterilecegine karar veren fonksiyon 
void lcdGuncelle() {
  // Ekran sirasi 0 ise nem ekrani gosterilir
  if (ekranSirasi == 0) {
    nemEkraniGoster();
  }

  // Ekran sirasi 1 ise sicaklik ekrani gosterilir
  else if (ekranSirasi == 1) {
    sicaklikEkraniGoster();
  }

  // Ekran sirasi 2 ise isik ekrani gosterilir
  else if (ekranSirasi == 2) {
    isikEkraniGoster();
  }

  // Beklenmeyen bir durumda sira sifirlanir ve nem ekrani gosterilir
  else {
    ekranSirasi = 0;
    nemEkraniGoster();
  }
}

// Buton durumunu okuyup LCD suresini degistiren fonksiyon 
void butonKontrol() {
  // Butondan o anki ham okuma degeri okunur
  bool okunanButon = digitalRead(BUTTON_PIN);

  // Eger butonun ham degeri degismisse zaman damgasi guncellenir
  if (okunanButon != sonButonOkumasi) {
    sonButonDegisimZamani = millis();
  }

  // Degisimden sonra debounce suresi kadar beklenip durumun kararlı olup olmadigi kontrol edilir
  if ((millis() - sonButonDegisimZamani) > debounceSuresi) {
    // Kararlı buton durumu degismisse yeni durum kabul edilir
    if (okunanButon != kararliButonDurumu) {
      kararliButonDurumu = okunanButon;

      // Butona basildiysa LOW okunur cunku INPUT_PULLUP kullanilmistir
      if (kararliButonDurumu == LOW) {
        // Eger mevcut sure 2 saniyeyse 4 saniyeye cikarilir
        if (lcdSure == 2000) {
          lcdSure = 4000;
        }

        // Diger durumda sure tekrar 2 saniyeye dondurulur
        else {
          lcdSure = 2000;
        }

        // Seri monitore LCD suresinin degistigi bilgisi yazdirilir
        Serial.print("LCD suresi degistirildi: ");
        // Yeni sure saniye cinsinden yazdirilir
        Serial.print(lcdSure / 1000);
        // Birim bilgisi yazdirilir ve satir sonu eklenir
        Serial.println(" sn");

        lcd.clear(); // LCD temizlenir

        lcd.setCursor(0, 0); // Imlec birinci satir basina ayarlanir
        lcd.print("LCD suresi"); // Bilgilendirme basligi yazdirilir
        lcd.setCursor(0, 1); // Imlec ikinci satir basina ayarlanir
        lcd.print("degistirildi:"); // Bilgilendirme mesaji yazdirilir
        lcd.print(lcdSure / 1000); // Yeni sure LCD'de gosterilir
        
        delay(1000); // Bilgilendirme mesaji 1 saniye boyunca gosterilir

        oncekiLcdZamani = millis(); // LCD ekran zaman damgasi guncellenir

        lcdGuncelle(); // Normal ekrana geri donulur
      }
    }
  }

  sonButonOkumasi = okunanButon; // Son ham buton okuma degeri saklanir
}

// Program baslangicinda bir kez calisan setup fonksiyonu 
void setup() {
  // Seri haberlesme 115200 baud hizinda baslatilir
  Serial.begin(115200);

  // I2C haberlesmesi SDA=21 ve SCL=22 pinleri ile baslatilir
  Wire.begin(21, 22);

  lcd.init(); // LCD baslatilir
  lcd.backlight(); // LCD arka isigi acilir

  // DHT22 sensoru ilgili pin ve sensor tipi ile baslatilir
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  // Buton pini dahili pull-up direnci ile giris olarak ayarlanir
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // RGB LED'in kirmizi pini cikis olarak ayarlanir
  pinMode(RED_PIN, OUTPUT);
  // RGB LED'in yesil pini cikis olarak ayarlanir
  pinMode(GREEN_PIN, OUTPUT);
  // RGB LED'in mavi pini cikis olarak ayarlanir
  pinMode(BLUE_PIN, OUTPUT);

  rgbKapat(); // Baslangicta RGB LED kapatilir

  sensorleriOku(); // Baslangicta sensorden ilk veriler okunur

  lcdGuncelle(); // LCD'de ilk ekran gosterilir
}

// Program calistiktan sonra surekli tekrar eden ana dongu 
void loop() {
  butonKontrol(); // Buton durumu her dongude kontrol edilir
  sensorleriOku(); // Sensor verileri her dongude guncellenir

  // Eger son seri yazdirimdan beri 3 saniye gecmisse yeni veri yazdirilir
  if (millis() - oncekiSerialZamani >= 3000) {
    oncekiSerialZamani = millis(); // Seri yazdirim zaman damgasi guncellenir

    serialYazdir(); // Sensor verileri seri monitore yazdirilir
  }

  // Eger LCD gosterim suresi dolmussa bir sonraki ekrana gecilir
  if (millis() - oncekiLcdZamani >= lcdSure) {
    oncekiLcdZamani = millis(); // LCD ekran zaman damgasi guncellenir

    ekranSirasi++; // Ekran sirasi bir arttirilir

    // Sira 2'yi gectiyse tekrar basa donulur
    if (ekranSirasi > 2) {
      ekranSirasi = 0;
    }

    lcdGuncelle(); // Yeni ekrana gore LCD guncellenir
  }
}