#include <Arduino.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Preferences.h> // Bibliothek für den Dauerspeicher

// ===========================
// --- PIN-KONFIGURATION ---
// ===========================
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define DATA_OUT_TX_PIN 25
#define DATA_OUT_RX_PIN 26
#define SD_CS_PIN 5
#define TFT_RST 4
#define TFT_DC  2
#define TFT_CS  15 
#define BUTTON_PIN 27 

// ===========================
// --- GLOBALE OBJEKTE & VARIABLEN ---
// ===========================
HardwareSerial GpsSerial(2);
HardwareSerial DataOutSerial(1);
TinyGPSPlus gps;
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);
Preferences preferences; // Objekt für den NVS-Speicher

int currentPage = 0;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
bool sdReady = false;
String serialBuffer = "";

unsigned long lastSaveAttempt = 0;
const long saveInterval = 10000; // 10 Sekunden, denn die Speicherung der daten jede Sekunde hat den ESP zum abstürzen gebracht. 
String lastSaveStatus = "Warte...";
unsigned long savedRecordCount = 0;

struct SatelliteInfo {
  int id = 0; int elevation = 0; int azimuth = 0; int snr = 0;
};
SatelliteInfo satellites[16];
int totalSatsInView = 0;

// ===========================
// --- VORWÄRTSDEKLARATION ---
// ===========================
void updateTftDisplay();
void saveFixToSD();
void parseGsvSentence(String sentence);

// ===========================
// --- SETUP ---
// ===========================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n--- ESP32 GPS-Sensor | Final mit Zeitstempel & sicherem Zähler ---");

  // Zähler aus dem Dauerspeicher laden
  preferences.begin("gps-logger", false);
  savedRecordCount = preferences.getULong("count", 0);
  preferences.end();
  Serial.printf("Zählerstand aus NVS geladen: %lu\n", savedRecordCount);

  GpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  DataOutSerial.begin(9600, SERIAL_8N1, DATA_OUT_RX_PIN, DATA_OUT_TX_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("❌ SD: Initialisierung fehlgeschlagen!");
    sdReady = false;
    lastSaveStatus = "Keine SD!";
  } else {
    Serial.println("✅ SD: bereit.");
    sdReady = true;
  }

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);
}

// ===========================
// --- HAUPTSCHLEIFE ---
// ===========================
void loop() {
  while (GpsSerial.available() > 0) {
    char c = GpsSerial.read();
    DataOutSerial.write(c);
    Serial.write(c);
    gps.encode(c);
    serialBuffer += c;
    if (c == '\n') {
      parseGsvSentence(serialBuffer);
      serialBuffer = "";
    }
  }

  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    if (millis() - lastDebounceTime > 200) {
      Serial.println("Button pressed!");
      int maxSatPages = (totalSatsInView > 0) ? (totalSatsInView + 3) / 4 : 1;
      currentPage = (currentPage + 1) % (maxSatPages + 1);
      lastDebounceTime = millis();
      updateTftDisplay(); // Sofort aktualisieren
    }
  }
  lastButtonState = currentButtonState;

  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 1000) {
    lastDisplayUpdate = millis();
    updateTftDisplay();
  }

  if (millis() - lastSaveAttempt >= saveInterval) {
    lastSaveAttempt = millis();
    // Speichern nur, wenn wir einen Fix UND eine gültige Zeit haben
    if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) {
        saveFixToSD();
    } else {
        lastSaveStatus = "Kein Fix/Zeit!";
    }
  }
}

// ===========================
// --- SD-LOGGING ---
// ===========================
void saveFixToSD() {
  if (!sdReady) {
    lastSaveStatus = "Keine SD!";
    return;
  }

  StaticJsonDocument<2048> doc;
  
  // Echten Zeitstempel erstellen und hinzufügen
  char datetime_str[20];
  sprintf(datetime_str, "%04d-%02d-%02d %02d:%02d:%02d",
          gps.date.year(), gps.date.month(), gps.date.day(),
          gps.time.hour(), gps.time.minute(), gps.time.second());
  doc["datetime"] = datetime_str;

  doc["fix"] = true;
  doc["lat"] = gps.location.lat();
  doc["lon"] = gps.location.lng();
  doc["sats_in_use"] = gps.satellites.value();
  doc["sats_in_view"] = totalSatsInView;
  doc["hdop"] = gps.hdop.hdop();
  doc["alt_m"] = gps.altitude.meters();
  doc["speed_kmph"] = gps.speed.kmph();
  doc["ms"] = millis();

  JsonArray satsArray = doc.createNestedArray("satellites");
  for (int i = 0; i < totalSatsInView; ++i) {
    if (satellites[i].id > 0) {
        JsonObject satObject = satsArray.createNestedObject();
        satObject["id"] = satellites[i].id;
        satObject["elev"] = satellites[i].elevation;
        satObject["azim"] = satellites[i].azimuth;
        satObject["snr"] = satellites[i].snr;
    }
  }

  String line;
  serializeJson(doc, line);

  File f = SD.open("/gpslog.jsonl", FILE_APPEND);
  if (f) {
    f.println(line);
    f.close();
    savedRecordCount++;
    
    // Zählerstand im Dauerspeicher sichern
    preferences.begin("gps-logger", false);
    preferences.putULong("count", savedRecordCount);
    preferences.end();

    lastSaveStatus = "OK @" + String(millis() / 1000) + "s";
    Serial.println("💾 SD: Daten erfolgreich gespeichert.");
  } else {
    lastSaveStatus = "Fehler!";
    Serial.println("❌ SD: Öffnen von /gpslog.jsonl fehlgeschlagen.");
  }
}

// ===========================
// --- TFT-DARSTELLUNG ---
// ===========================
void updateTftDisplay() {
    tft.fillScreen(ST7735_BLACK);
    int maxSatPages = (totalSatsInView > 0) ? (totalSatsInView + 3) / 4 : 1;
    if (currentPage >= (maxSatPages + 1)) { currentPage = 0; }

    if (currentPage < maxSatPages) {
        // Satelliten-Seiten
        tft.setCursor(5, 5); tft.setTextColor(ST7735_YELLOW); tft.setTextSize(2);
        tft.print("Fix: ");
        tft.setTextSize(2); tft.setCursor(60, 5);
        if (gps.location.isValid()) { tft.setTextColor(ST7735_GREEN); tft.print("JA "); }
        else { tft.setTextColor(ST7735_RED); tft.print("NEIN"); }
        tft.setTextSize(1); tft.setTextColor(ST7735_YELLOW);
        tft.setCursor(100, 10);
        tft.printf("Sats: %d/%d", gps.satellites.value(), totalSatsInView);
        tft.setTextColor(ST7735_WHITE); tft.setCursor(5, 30);
        tft.printf("Lat: %.6f", gps.location.isValid() ? gps.location.lat() : 0.0);
        tft.setCursor(5, 40);
        tft.printf("Lon: %.6f", gps.location.isValid() ? gps.location.lng() : 0.0);
        tft.setCursor(5, 60); tft.setTextColor(ST7735_CYAN);
        int startSat = currentPage * 4;
        int endSat = min(startSat + 4, totalSatsInView);
        tft.printf("--- Sats (%d-%d / %d) ---", (totalSatsInView ? startSat + 1 : 0), endSat, totalSatsInView);
        tft.setCursor(5, 75); tft.setTextColor(ST7735_WHITE);
        tft.println("ID | Elev | Azim | SNR");
        tft.println("-------------------------");
        for (int i = startSat; i < endSat; ++i) {
            if (satellites[i].id > 0) {
                tft.printf("%-3d| %-5d| %-5d| %d\n", satellites[i].id, satellites[i].elevation, satellites[i].azimuth, satellites[i].snr);
            }
        }
    } else {
        // Status-Seite
        tft.setCursor(5, 5); tft.setTextColor(ST7735_YELLOW); tft.setTextSize(2);
        tft.println("System Status");
        tft.drawFastHLine(5, 22, tft.width() - 10, ST7735_YELLOW);
        tft.setCursor(5, 40); tft.setTextColor(ST7735_WHITE); tft.setTextSize(1);
        tft.println("Letzter Speicherversuch:");
        tft.setCursor(10, 60); tft.setTextSize(2);
        if (lastSaveStatus.startsWith("OK")) tft.setTextColor(ST7735_GREEN);
        else if (lastSaveStatus.startsWith("Fehler") || lastSaveStatus.startsWith("Keine SD")) tft.setTextColor(ST7735_RED);
        else tft.setTextColor(ST7735_ORANGE);
        tft.print(lastSaveStatus);
        tft.setCursor(5, 90); tft.setTextColor(ST7735_WHITE); tft.setTextSize(1);
        tft.println("Gespeicherte Datensaetze:");
        tft.setCursor(10, 110); tft.setTextSize(2);
        tft.setTextColor(ST7735_CYAN);
        tft.print(savedRecordCount);
    }
}

// ===========================
// --- GSV-PARSING ---
// ===========================
void parseGsvSentence(String sentence) {
  if (sentence.startsWith("$GPGSV") || sentence.startsWith("$GLGSV") || sentence.startsWith("$GAGSV") || sentence.startsWith("$GQGSV")) {
    int part_index = 0; String part = ""; int msg_num = 0;
    int temp_msg_num_start = sentence.indexOf(',', 7) + 1;
    int temp_msg_num_end = sentence.indexOf(',', temp_msg_num_start);
    int temp_msg_num = sentence.substring(temp_msg_num_start, temp_msg_num_end).toInt();
    if (temp_msg_num == 1) {
      for(int i=0; i<16; i++) { satellites[i] = {0,0,0,0}; }
    }
    for (int i = 0; i < sentence.length(); i++) {
      if (sentence.charAt(i) == ',' || sentence.charAt(i) == '*') {
        if (part.length() > 0 || part_index == 7 || part_index == 11 || part_index == 15 || part_index == 19) {
          switch (part_index) {
            case 2: msg_num = part.toInt(); break;
            case 3: totalSatsInView = part.toInt(); break;
            case 4: if (msg_num > 0) satellites[(msg_num-1)*4 + 0].id = part.toInt(); break;
            case 5: if (msg_num > 0) satellites[(msg_num-1)*4 + 0].elevation = part.toInt(); break;
            case 6: if (msg_num > 0) satellites[(msg_num-1)*4 + 0].azimuth = part.toInt(); break;
            case 7: if (msg_num > 0) satellites[(msg_num-1)*4 + 0].snr = part.toInt(); break;
            case 8: if (msg_num > 0) satellites[(msg_num-1)*4 + 1].id = part.toInt(); break;
            case 9: if (msg_num > 0) satellites[(msg_num-1)*4 + 1].elevation = part.toInt(); break;
            case 10: if (msg_num > 0) satellites[(msg_num-1)*4 + 1].azimuth = part.toInt(); break;
            case 11: if (msg_num > 0) satellites[(msg_num-1)*4 + 1].snr = part.toInt(); break;
            case 12: if (msg_num > 0) satellites[(msg_num-1)*4 + 2].id = part.toInt(); break;
            case 13: if (msg_num > 0) satellites[(msg_num-1)*4 + 2].elevation = part.toInt(); break;
            case 14: if (msg_num > 0) satellites[(msg_num-1)*4 + 2].azimuth = part.toInt(); break;
            case 15: if (msg_num > 0) satellites[(msg_num-1)*4 + 2].snr = part.toInt(); break;
            case 16: if (msg_num > 0) satellites[(msg_num-1)*4 + 3].id = part.toInt(); break;
            case 17: if (msg_num > 0) satellites[(msg_num-1)*4 + 3].elevation = part.toInt(); break;
            case 18: if (msg_num > 0) satellites[(msg_num-1)*4 + 3].azimuth = part.toInt(); break;
            case 19: if (msg_num > 0) satellites[(msg_num-1)*4 + 3].snr = part.toInt(); break;
          }
        }
        part = ""; part_index++;
      } else { part += sentence.charAt(i); }
    }
  }
}
