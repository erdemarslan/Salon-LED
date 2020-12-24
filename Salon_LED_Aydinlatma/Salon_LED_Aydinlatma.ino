#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <AT24CX.h>

#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "BLYNK_AUTH_CODE";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Wifi_SSID";
char pass[] = "Wifi_PASSWORD";

#define LED_PIN D4

#define LED_SAYISI_ON_DUVAR 104
#define LED_SAYISI_SOL_DUVAR 65
#define LED_SAYISI_ARKA_DUVAR 104
#define LED_SAYISI_SAG_DUVAR 65
#define LED_SAYISI LED_SAYISI_ON_DUVAR + LED_SAYISI_SOL_DUVAR + LED_SAYISI_ARKA_DUVAR + LED_SAYISI_SAG_DUVAR


volatile byte r, g, b = 0;
volatile int on_duvar, sol_duvar, arka_duvar, sag_duvar = 0;
volatile byte parlaklik = 0;
volatile byte calisma_modu = 0;
volatile int led_sayisi = 100;

// EEPROM KAYIT BÖLGELERİ
#define E_CALISMA_MODU 10
#define E_PARLAKLIK 11
#define E_ON_DUVAR 20
#define E_SOL_DUVAR 25
#define E_ARKA_DUVAR 30
#define E_SAG_DUVAR 35
#define E_R 12
#define E_G 13
#define E_B 14

// EEPROM object
AT24C32 mem;
// NeoPIXEL object
//Adafruit_NeoPixel led;
Adafruit_NeoPixel led = Adafruit_NeoPixel(LED_SAYISI, LED_PIN, NEO_GRB + NEO_KHZ400);  // WS2811

BlynkTimer rainbowTimer;
int timerID = 0;
long firstPixelHue = 0;

// İlk verileri EEPROM'a kaydet -- Yerleri belirle...
void eepromIlkKayit() {
  mem.write(E_CALISMA_MODU, 4); // Çalışma Modu
  mem.write(E_PARLAKLIK, 50); // Parlaklık
  
  mem.write(E_R, 255); // Kırmızı
  mem.write(E_G, 255); // Yeşil
  mem.write(E_B, 255); // Mavi

  mem.writeInt(E_ON_DUVAR, LED_SAYISI_ON_DUVAR); // Duvardaki led sayısı ön
  mem.writeInt(E_SOL_DUVAR, LED_SAYISI_SOL_DUVAR); // Duvardaki led sayısı ön
  mem.writeInt(E_ARKA_DUVAR, LED_SAYISI_ARKA_DUVAR); // Duvardaki led sayısı ön
  mem.writeInt(E_SAG_DUVAR, LED_SAYISI_SAG_DUVAR); // Duvardaki led sayısı ön
}

void eepromdanOku() {
  calisma_modu = mem.read(E_CALISMA_MODU);
  delay(20);
  parlaklik = mem.read(E_PARLAKLIK);
  delay(20);
  
  on_duvar = mem.readInt(E_ON_DUVAR);
  delay(20);
  sol_duvar = mem.readInt(E_SOL_DUVAR);
  delay(20);
  arka_duvar = mem.readInt(E_ARKA_DUVAR);
  delay(20);
  sag_duvar = mem.readInt(E_SAG_DUVAR);
  delay(20);

  r = mem.read(E_R);
  delay(20);
  g = mem.read(E_G);
  delay(20);
  b = mem.read(E_B);
  delay(20);
}

void eepromdanCalis() {
  led_sayisi = on_duvar + sol_duvar + arka_duvar + sag_duvar;

  led.begin();
  led.setBrightness(parlaklik);
  led.show();
  
  ledGoster();
}

void ledGoster() {
  switch(calisma_modu) {
    case 1:
      if(rainbowTimer.isEnabled(timerID)) {
        rainbowTimer.disable(timerID);
      }
      led.fill(led.Color(255, 255, 255), 0, led_sayisi);
      led.show();  
    break;
    
    case 2:
      if(rainbowTimer.isEnabled(timerID)) {
        rainbowTimer.disable(timerID);
      }
      led.fill(led.Color(255, 0, 0), 0, led_sayisi);
      led.show();
    break;
    
    case 3:
      if(rainbowTimer.isEnabled(timerID)) {
        rainbowTimer.disable(timerID);
      }
      led.fill(led.Color(0, 255, 0), 0, led_sayisi);
      led.show();
    break;
    
    case 4:
      if(rainbowTimer.isEnabled(timerID)) {
        rainbowTimer.disable(timerID);
      }
      led.fill(led.Color(0, 0, 255), 0, led_sayisi);
      led.show();
    break;
    /**/
    case 5:
      // Renk Geçişi
      firstPixelHue = 0;
      rainbowTimer.enable(timerID);
    break;
    
    case 6:
      if(rainbowTimer.isEnabled(timerID)) {
        rainbowTimer.disable(timerID);
      }
      // Kullanıcı Tanımlı Renk
      led.fill(led.Color(r, g, b), 0, led_sayisi);
      led.show();
    break;
  }
}

void eepromByteKaydet(int bolge, byte veri) {
  byte b = mem.read(bolge);
  if(b != veri) {
    mem.write(bolge, veri);
  }
}

void eepromIntKaydet(int bolge, int veri) {
  int i = mem.readInt(bolge);
  if(i != veri) {
    mem.writeInt(bolge, veri);
  }
}

void rainbow() {
  if(firstPixelHue >= 5*65536) {
    firstPixelHue = 0;
  }
  for(int i = 0; i < led.numPixels(); i++) {
    int pixelHue = firstPixelHue + (i * 65536L / led.numPixels());
    led.setPixelColor(i, led.gamma32(led.ColorHSV(pixelHue)));
  }
  led.show();
  firstPixelHue += 256;
}

void setup() {
  delay(1000);
  // Önce serial ekranı açalım...
  Serial.begin(9600);
  Serial.println("Salon LED Aydınlatma modülü başlıyor...");

  //eepromIlkKayit(); // ilk çalıştırmadan sonra sil...

  eepromdanOku(); // değerleri eepromdan al ve yerlerine koy. Sonra da blynk e at...

  // eeprom a göre ilk led çalışmasını yap. Blynk bağlanmasa bile en azından hafızadan çalışalım...
  eepromdanCalis();

  // Blynk Başlasın...
  Blynk.begin(auth, ssid, pass);

  timerID = rainbowTimer.setInterval(25L, rainbow);
  Serial.print("Timer ID: ");
  Serial.println(timerID);

  if(calisma_modu != 5) {
    rainbowTimer.disable(timerID);
  }
  
}

BLYNK_CONNECTED() {
  Blynk.virtualWrite(V0, calisma_modu);
  Blynk.virtualWrite(V1, parlaklik);
  Blynk.virtualWrite(V20, r);
  Blynk.virtualWrite(V21, g);
  Blynk.virtualWrite(V22, b);

  Blynk.virtualWrite(V3, 1);
  Blynk.virtualWrite(V4, 1);
  Blynk.virtualWrite(V5, 1);
  Blynk.virtualWrite(V6, 1);

  Blynk.virtualWrite(V7, on_duvar);
  Blynk.virtualWrite(V8, sol_duvar);
  Blynk.virtualWrite(V9, arka_duvar);
  Blynk.virtualWrite(V10, sag_duvar);
  Blynk.virtualWrite(V11, on_duvar + sol_duvar + arka_duvar + sag_duvar);
}

// Çalışma Modu
BLYNK_WRITE(V0) {
  calisma_modu = param.asInt();
  eepromByteKaydet(E_CALISMA_MODU, calisma_modu);
  ledGoster();
}

// Parlaklık Değişimi
BLYNK_WRITE(V1) {
  parlaklik = param.asInt();
  eepromByteKaydet(E_PARLAKLIK, parlaklik);
  led.setBrightness(parlaklik);
  led.show();
}

// Kırmızı Renk Skalası
BLYNK_WRITE(V20) {
  
  r = param.asInt();
  calisma_modu = 6;
  eepromByteKaydet(E_CALISMA_MODU, calisma_modu);
  Blynk.virtualWrite(V0, calisma_modu);
  eepromByteKaydet(E_R, r);
  ledGoster();
}

// Yeşil Renk Skalası
BLYNK_WRITE(V21) {
  g = param.asInt();
  calisma_modu = 6;
  eepromByteKaydet(E_CALISMA_MODU, calisma_modu);
  Blynk.virtualWrite(V0, calisma_modu);
  eepromByteKaydet(E_G, g);
  ledGoster();
}

// Mavi Renk Skalası
BLYNK_WRITE(V22) {
  b = param.asInt();
  calisma_modu = 6;
  eepromByteKaydet(E_CALISMA_MODU, calisma_modu);
  Blynk.virtualWrite(V0, calisma_modu);
  eepromByteKaydet(E_B, b);
  ledGoster();
}

BLYNK_WRITE(V3) {
  int durum = param.asInt();

  if(durum == 1) {
    // aç...
    if(calisma_modu != 5) {
      switch(calisma_modu) {
        case 1:
          led.fill(led.Color(255, 255, 255), 0, on_duvar);
          led.show();
        break;

        case 2:
          led.fill(led.Color(255, 0, 0), 0, on_duvar);
          led.show();
        break;

        case 3:
          led.fill(led.Color(0, 255, 0), 0, on_duvar);
          led.show();
        break;

        case 4:
          led.fill(led.Color(0, 0, 255), 0, on_duvar);
          led.show();
        break;

        case 6:
          led.fill(led.Color(r, g, b), 0, on_duvar);
          led.show();
        break;
      }

      //Blynk.virtualWrite(V3, 1);
    }
    
    
  } else {
    // kapat...
    if(calisma_modu != 5) {
      //Blynk.virtualWrite(V3, 0);
      led.fill(led.Color(0,0,0), 0, on_duvar);
      led.show();
    } else {
      Blynk.virtualWrite(V3, 1);
    }
  }
}

BLYNK_WRITE(V4) {
  int durum = param.asInt();

  if(durum == 1) {
    // aç...
    if(calisma_modu != 5) {
      switch(calisma_modu) {
        case 1:
          led.fill(led.Color(255, 255, 255), on_duvar, sol_duvar);
          led.show();
        break;

        case 2:
          led.fill(led.Color(255, 0, 0), on_duvar, sol_duvar);
          led.show();
        break;

        case 3:
          led.fill(led.Color(0, 255, 0), on_duvar, sol_duvar);
          led.show();
        break;

        case 4:
          led.fill(led.Color(0, 0, 255), on_duvar, sol_duvar);
          led.show();
        break;

        case 6:
          led.fill(led.Color(r, g, b), on_duvar, sol_duvar);
          led.show();
        break;
      }

      //Blynk.virtualWrite(V3, 1);
    }
    
    
  } else {
    // kapat...
    if(calisma_modu != 5) {
      //Blynk.virtualWrite(V3, 0);
      led.fill(led.Color(0,0,0), on_duvar, sol_duvar);
      led.show();
    } else {
      Blynk.virtualWrite(V3, 1);
    }
  }
}

BLYNK_WRITE(V5) {
  int durum = param.asInt();

  if(durum == 1) {
    // aç...
    if(calisma_modu != 5) {
      switch(calisma_modu) {
        case 1:
          led.fill(led.Color(255, 255, 255), on_duvar + sol_duvar, arka_duvar);
          led.show();
        break;

        case 2:
          led.fill(led.Color(255, 0, 0), on_duvar + sol_duvar, arka_duvar);
          led.show();
        break;

        case 3:
          led.fill(led.Color(0, 255, 0), on_duvar + sol_duvar, arka_duvar);
          led.show();
        break;

        case 4:
          led.fill(led.Color(0, 0, 255), on_duvar + sol_duvar, arka_duvar);
          led.show();
        break;

        case 6:
          led.fill(led.Color(r, g, b), on_duvar + sol_duvar, arka_duvar);
          led.show();
        break;
      }
    }
    
    
  } else {
    // kapat...
    if(calisma_modu != 5) {
      //Blynk.virtualWrite(V3, 0);
      led.fill(led.Color(0,0,0), on_duvar + sol_duvar, arka_duvar);
      led.show();
    } else {
      Blynk.virtualWrite(V3, 1);
    }
  }
}

BLYNK_WRITE(V6) {
  int durum = param.asInt();

  if(durum == 1) {
    // aç...
    if(calisma_modu != 5) {
      switch(calisma_modu) {
        case 1:
          led.fill(led.Color(255, 255, 255), on_duvar + sol_duvar + arka_duvar, sag_duvar);
          led.show();
        break;

        case 2:
          led.fill(led.Color(255, 0, 0), on_duvar + sol_duvar + arka_duvar, sag_duvar);
          led.show();
        break;

        case 3:
          led.fill(led.Color(0, 255, 0), on_duvar + sol_duvar + arka_duvar, sag_duvar);
          led.show();
        break;

        case 4:
          led.fill(led.Color(0, 0, 255), on_duvar + sol_duvar + arka_duvar, sag_duvar);
          led.show();
        break;

        case 6:
          led.fill(led.Color(r, g, b), on_duvar + sol_duvar + arka_duvar, sag_duvar);
          led.show();
        break;
      }
    }
    
    
  } else {
    // kapat...
    if(calisma_modu != 5) {
      //Blynk.virtualWrite(V3, 0);
      led.fill(led.Color(0,0,0), on_duvar + sol_duvar + arka_duvar, sag_duvar);
      led.show();
    } else {
      Blynk.virtualWrite(V3, 1);
    }
  }
}

BLYNK_WRITE(V7) {
  on_duvar = param.asInt();

  eepromIntKaydet(E_ON_DUVAR, on_duvar);

  led_sayisi = on_duvar + sol_duvar + arka_duvar + sag_duvar;
  Blynk.virtualWrite(V11, led_sayisi);
  led.updateLength(led_sayisi);
}

BLYNK_WRITE(V8) {
  sol_duvar = param.asInt();

  eepromIntKaydet(E_SOL_DUVAR, sol_duvar);

  led_sayisi = on_duvar + sol_duvar + arka_duvar + sag_duvar;
  Blynk.virtualWrite(V11, led_sayisi);
  led.updateLength(led_sayisi);
}

BLYNK_WRITE(V9) {
  arka_duvar = param.asInt();

  eepromIntKaydet(E_ARKA_DUVAR, arka_duvar);

  led_sayisi = on_duvar + sol_duvar + arka_duvar + sag_duvar;
  Blynk.virtualWrite(V11, led_sayisi);
  led.updateLength(led_sayisi);
}

BLYNK_WRITE(V10) {
  sag_duvar = param.asInt();

  eepromIntKaydet(E_SAG_DUVAR, sag_duvar);

  led_sayisi = on_duvar + sol_duvar + arka_duvar + sag_duvar;
  Blynk.virtualWrite(V11, led_sayisi);
  led.updateLength(led_sayisi);
}

void loop() {
  Blynk.run();
  rainbowTimer.run();
}
