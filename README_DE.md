# Blackwater Tank Füllstandsüberwachung

[🇬🇧 English Version](README.md)

ESP32-basiertes Tank-Füllstandsüberwachungssystem mit TFmini-S LiDAR-Sensor und SensESP-Framework für Signal K Integration.

## Funktionen

- **TFmini-S LiDAR-Sensor** für berührungslose Distanzmessung
- **Signal K Integration** über SensESP v3 Framework
- **LCD-Display** (16x2 I2C) zeigt Füllstand und Volumen an
- **Konfigurierbarer Alarm** mit einstellbarem Schwellwert
- **Notbetriebsmodus** mit manueller Überbrückung
- **Relaissteuerung** für Pumpen- oder Ventilautomatisierung
- **Echtzeit-Überwachung** von Tankfüllstand, Volumen und Kapazität

## Hardware-Anforderungen

- ESP32 Board (Wemos D1 Mini ESP32 oder kompatibel)
- TFmini-S LiDAR-Sensor
- 16x2 LCD mit PCF8574 I2C-Backpack (Adresse 0x27)
- Alarmausgabegerät (LED, Summer, Relais, etc.)
- 5V Stromversorgung

## Pin-Konfiguration

| Komponente | ESP32 Pin | Beschreibung |
|-----------|-----------|-------------|
| LiDAR RX | GPIO 16 | Empfängt Daten von TFmini-S TX |
| LiDAR TX | GPIO 17 | Sendet Befehle an TFmini-S RX |
| LCD SDA | GPIO 21 | I2C Datenleitung |
| LCD SCL | GPIO 22 | I2C Taktleitung |
| Alarmausgang | GPIO 23 | Alarmausgang (HIGH bei Schwellwertüberschreitung) |
| Alarmeingang | GPIO 19 | Notfall-Alarmeingang (aktiv LOW, interner Pull-up) |
| Relaisausgang | GPIO 18 | Relaissteuerung (HIGH=EIN im Normalbetrieb, LOW=AUS im Notfall) |

## Signal K Pfade

Das System veröffentlicht auf folgenden Signal K Pfaden:

- `tanks.blackWater.0.currentLevel` - Tank-Füllverhältnis (0.0 bis 1.0)
- `tanks.blackWater.0.capacity` - Tankkapazität in Kubikmetern (m³)
- `tanks.blackWater.0.currentVolume` - Aktuelles Volumen in Kubikmetern (m³)

## Installation

### Voraussetzungen

- [PlatformIO](https://platformio.org/) installiert
- USB-Kabel für ESP32-Programmierung

### Schritte

1. Repository klonen:
   ```bash
   git clone https://github.com/Blohminator/Blackwater-Tank.git
   cd Blackwater-Tank
   ```

2. Projekt in PlatformIO öffnen (VS Code oder CLI)

3. ESP32 Board per USB verbinden

4. Kompilieren und hochladen:
   ```bash
   pio run -t upload
   ```

5. Serielle Ausgabe überwachen:
   ```bash
   pio device monitor
   ```

## Konfiguration

### Initiale WiFi-Einrichtung

Beim ersten Start erstellt der ESP32 einen WiFi-Access-Point:
- SSID: `SensESP-blackwater-tank`
- Mit diesem Netzwerk verbinden und WiFi-Zugangsdaten konfigurieren

### Web-Konfiguration

Zugriff auf die Konfigurationsoberfläche unter:
- `http://blackwater-tank.local/config` (mDNS)
- Oder die auf dem LCD/seriellen Monitor angezeigte IP-Adresse verwenden

Folgende Parameter konfigurieren:
- **Tanklänge** (cm)
- **Tankbreite** (cm)
- **Tankhöhe** (cm)
- **Sensor-Offset** (cm) - Abstand von Sensormontage zur Tankoberseite
- **Alarmschwelle** (%) - Prozentsatz, bei dem der Alarm aktiviert wird

### Signal K Server

Signal K Server-Adresse in der Web-Oberfläche konfigurieren oder im Code anpassen:
```cpp
builder
  .set_hostname("blackwater-tank")
  .set_sk_server("192.168.1.50", 3000)  // Signal K Server IP eintragen
  .get_app();
```

## LCD-Anzeige

Das LCD zeigt Echtzeitinformationen in zwei Modi:

### Normalmodus
**Zeile 1:** `Fill:XXcm YYY%`
- Füllhöhe in Zentimetern
- Füllprozentsatz (0-100%)

**Zeile 2:** `Vol:XXXXXXL`
- Aktuelles Volumen in Litern

### Notfallmodus
**Zeile 1:** `Fill:XXcm YYY%`
- Füllhöhe in Zentimetern
- Füllprozentsatz (0-100%)

**Zeile 2:** `EMERGENCY MODE`
- Zeigt an, dass der Notbetrieb aktiv ist

## Notbetrieb

Das System verfügt über eine Notfall-Überbrückungsfunktion:

**Normalbetrieb:**
- Relaisausgang: HIGH (EIN)
- Alarmausgang: Basierend auf Schwellwert
- Anzeige: Zeigt Füllstand und Volumen

**Aktivierung des Notfallmodus:**
Wenn BEIDE Bedingungen erfüllt sind:
1. Füllstand überschreitet Alarmschwelle (Standard 80%)
2. Alarmeingang (GPIO 19) ist geschlossen (mit GND verbunden)

**Verhalten im Notfallmodus:**
- Relaisausgang: LOW (AUS) - schaltet in Normal-/Sicherheitszustand
- Alarmausgang: HIGH (aktiv)
- Anzeige: Zeigt "EMERGENCY MODE" Meldung
- Seriell: Protokolliert Notfallaktivierung

**Beenden des Notfallmodus:**
Wenn der Alarmeingang (GPIO 19) geöffnet wird (getrennt), kehrt das System automatisch zum Normalbetrieb zurück.

## TFmini-S LiDAR

Der TFmini-S Sensor misst die Entfernung über UART-Kommunikation:
- Baudrate: 115200
- Frame-Format: 9 Bytes mit Prüfsumme
- Messbereich: 30cm bis 1200cm
- Genauigkeit: ±5cm

### Montage

Sensor über der Tanköffnung montieren, nach unten zeigend:
1. Freie Sichtlinie zur Flüssigkeitsoberfläche sicherstellen
2. Schaum oder turbulente Oberflächen für beste Genauigkeit vermeiden
3. **Sensor-Offset** Parameter einstellen, um Montageabstand zu berücksichtigen

## Fehlerbehebung

### Keine LiDAR-Messwerte
- UART-Verkabelung prüfen (RX/TX könnten vertauscht sein)
- TFmini-S Stromversorgung überprüfen (5V)
- Serielle Ausgabe auf Frame-Fehler überwachen

### LCD zeigt nichts an
- I2C-Adresse überprüfen (Standard 0x27, manche verwenden 0x3F)
- I2C-Verkabelung prüfen (SDA/SCL)
- Mit I2C-Scanner-Sketch testen

### Signal K verbindet nicht
- WiFi-Verbindung überprüfen
- Signal K Server IP und Port prüfen
- Sicherstellen, dass Signal K Server läuft und erreichbar ist

### Alarm funktioniert nicht
- GPIO 23 Verbindung prüfen
- Alarmschwellen-Einstellung überprüfen
- Mit Multimeter oder LED testen

## Entwicklung

### Projektstruktur

```
├── platformio.ini          # PlatformIO-Konfiguration
├── src/
│   └── main.cpp           # Hauptanwendungscode
├── include/               # Header-Dateien (falls benötigt)
└── lib/                   # Eigene Bibliotheken (falls benötigt)
```

### Abhängigkeiten

- [SensESP](https://github.com/SignalK/SensESP) v3.2.0+
- [LiquidCrystal_PCF8574](https://github.com/mathertel/LiquidCrystal_PCF8574) v2.2.0+
- [ArduinoJson](https://arduinojson.org/) v7.0.0+

## Lizenz

Dieses Projekt ist Open Source. Prüfen Sie die individuellen Bibliothekslizenzen für Abhängigkeiten.

## Referenzen

- [SensESP Dokumentation](https://signalk.org/SensESP/)
- [Signal K](https://signalk.org/)
- [TFmini-S Datenblatt](https://www.benewake.com/en/tfmini-s.html)
- [ESP32 Dokumentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)

## Support

Für Probleme und Fragen:
- Issue auf [GitHub](https://github.com/Blohminator/Blackwater-Tank/issues) öffnen
- SensESP: [GitHub Issues](https://github.com/SignalK/SensESP/issues)
- Signal K: [Community Forum](https://github.com/SignalK/signalk-server/discussions)
