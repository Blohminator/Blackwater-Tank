# Blackwater Tank Füllstandsüberwachung

[🇬🇧 English Version](README.md)

ESP32-basiertes Blackwater-Tank-Füllstandsüberwachungssystem mit TFmini-S LiDAR-Sensor und SensESP-Framework für Signal K Integration. Entwickelt für maritime Anwendungen mit Echtzeit-Überwachung, Notfall-Override und Relaissteuerung für elektrische Toilettensysteme.

## Überblick

Dieses Projekt bietet eine komplette Lösung zur Überwachung von Blackwater-Tankfüllständen auf Booten und Wohnmobilen. Mit einem berührungslosen LiDAR-Sensor misst es präzise Tankfüllstände und integriert sich nahtlos in Signal K Marine-Datensysteme. Das System umfasst ein OLED-Display für lokale Überwachung, konfigurierbare Alarme und eine Notfall-Override-Funktion für kritische Situationen.

## Funktionen

- **TFmini-S LiDAR-Sensor** für berührungslose Distanzmessung
- **Signal K Integration** über SensESP v3 Framework
- **SSD1306 OLED-Display** (128x64, Software I2C auf GPIO 32/33) zeigt Füllstand und Fortschrittsbalken an
- **Konfigurierbarer Alarm** mit einstellbarem Schwellwert
- **Notbetriebsmodus** mit manueller Überbrückung
- **Relaissteuerung** für elektrische Toilettensystem-Automatisierung
- **Echtzeit-Überwachung** von Tankfüllstand, Volumen und Kapazität
- **Webbasierte Konfiguration** für einfache Einrichtung
- **WiFi-Konnektivität** mit mDNS-Unterstützung
- **Robuste Fehlerbehandlung** und automatische Wiederherstellung

## Hardware-Anforderungen

- **ESP32 Board** - Wemos D1 Mini ESP32 oder kompatibel (ESP32-WROOM-32)
- **TFmini-S LiDAR-Sensor** - Berührungslose Distanzmessung (30-1200cm Reichweite)
- **SSD1306 OLED-Display** - 128x64 Pixel, Software I2C auf GPIO 32/33 (Adresse 0x3C)
- **Relaismodul** - Zur Steuerung elektrischer Toilette oder Pumpe (optional)
- **Alarmgerät** - LED, Summer oder externes Alarmsystem (optional)
- **5V Stromversorgung** - Mindestens 2A empfohlen für stabilen Betrieb
- **Gehäuse** - Wasserdichtes Gehäuse empfohlen für maritime Umgebungen

## Pin-Konfiguration

| Komponente | ESP32 Pin | Beschreibung |
|-----------|-----------|-------------|
| LiDAR RX | GPIO 16 | Empfängt Daten von TFmini-S TX |
| LiDAR TX | GPIO 17 | Sendet Befehle an TFmini-S RX |
| OLED SDA | GPIO 32 | Software I2C Datenleitung |
| OLED SCL | GPIO 33 | Software I2C Taktleitung |
| Alarmausgang | GPIO 23 | Relaisausgang (HIGH = Relais an, HW-482 aktiv-high) |
| Notfalleingang | GPIO 19 | Notfall-Eingang (aktiv LOW, interner Pull-up) |

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

### Tank-Parameter

Die Standard-Tankkonfiguration im Code:
```cpp
float length_cm = 50.0f;   // Tanklänge in Zentimetern
float width_cm = 72.0f;    // Tankbreite in Zentimetern
float height_cm = 87.0f;   // Tankhöhe in Zentimetern
float sensor_offset_cm = 5.0f;  // Abstand vom Sensor zur Tankoberseite
int alarm_threshold_percent = 80;  // Alarm wird bei 80% Füllstand ausgelöst
```

Passen Sie diese Werte in `src/main.cpp` an Ihre Tankabmessungen an.

### Initiale WiFi-Einrichtung

Beim ersten Start erstellt der ESP32 einen WiFi-Access-Point:
- **SSID:** `SensESP-blackwater-tank`
- **Passwort:** `thisisfine`
- Mit diesem Netzwerk verbinden — ein Captive Portal öffnet sich automatisch
- Falls nicht, `http://192.168.4.1` im Browser aufrufen

### Web-Konfiguration

Nach der WiFi-Konfiguration auf die Web-Oberfläche zugreifen:
- **mDNS:** `http://blackwater-tank.local/config`
- **Direkte IP:** Siehe OLED-Display oder seriellen Monitor für IP-Adresse

Die Web-Oberfläche bietet:
- Echtzeit-Tankfüllstand-Visualisierung
- Signal K Verbindungsstatus
- Systeminformationen und Diagnose
- Netzwerkkonfiguration
- Firmware-Update-Funktion

### Signal K Server

Signal K Server-Adresse in der Web-Oberfläche konfigurieren oder im Code anpassen:
```cpp
builder
  .set_hostname("blackwater-tank")
  .set_sk_server("192.168.1.50", 3000)  // Signal K Server IP eintragen
  .get_app();
```

## OLED-Anzeige

Das OLED zeigt Echtzeitinformationen in zwei Modi:

### Normalmodus
- Füllprozentsatz (z.B. `75%`)
- Füllhöhe in Zentimetern
- Vertikaler Füllbalken auf der rechten Seite

### Notfallmodus
- `NOT-BETRIEB` Warnung
- Füllprozentsatz
- Vertikaler Füllbalken auf der rechten Seite

## Notbetrieb

Das System verfügt über eine Notfall-Überbrückungsfunktion:

**Normalbetrieb:**
- Relaisausgang: HIGH (EIN)
- Alarmausgang: Basierend auf Schwellwert
- Anzeige: Zeigt Füllstand und Volumen

**Aktivierung des Notfallmodus:**
Wenn BEIDE Bedingungen erfüllt sind:
1. Füllstand erreicht oder überschreitet Alarmschwelle (Standard 85%)
2. Notfalleingang (GPIO 19) wird auf GND gezogen

**Verhalten im Notfallmodus:**
- Relaisausgang: LOW (AUS) — sicherer Zustand
- Anzeige: Zeigt "NOT-BETRIEB" Meldung
- Seriell: Protokolliert Notfallaktivierung

**Beenden des Notfallmodus:**
Wenn der Notfalleingang (GPIO 19) geöffnet wird (von GND getrennt), kehrt das System automatisch zum Normalbetrieb zurück.

## TFmini-S LiDAR

Der TFmini-S Sensor misst die Entfernung über UART-Kommunikation:
- Baudrate: 115200
- Frame-Format: 9 Bytes mit Prüfsumme
- Messbereich: 30cm bis 1200cm
- Genauigkeit: ±5cm

### Montage

Sensor über der Tanköffnung montieren, nach unten zeigend:

⚠️ **WICHTIG: Feuchtigkeitsschutz erforderlich!**
- Der LiDAR-Sensor darf NICHT in direkten Kontakt mit Wasser oder Feuchtigkeit kommen
- Installieren Sie eine schützende Glas- oder transparente Kunststoffbarriere zwischen Sensor und Tanköffnung
- Verwenden Sie transparente Materialien (Glas, Acryl, Polycarbonat), die Laserlicht durchlassen
- Stellen Sie sicher, dass die Schutzbarriere sauber und frei von Kondenswasser ist für genaue Messungen
- Dichten Sie das Sensorgehäuse ab, um das Eindringen von Feuchtigkeit und Gasen zu verhindern

Zusätzliche Montagehinweise:
1. Freie Sichtlinie zur Flüssigkeitsoberfläche durch die Schutzbarriere sicherstellen
2. Schaum oder turbulente Oberflächen für beste Genauigkeit vermeiden
3. Schutzbarriere schräg montieren, um Kondensationsbildung zu verhindern
4. **Sensor-Offset** Parameter einstellen, um Montageabstand und Barrierendicke zu berücksichtigen
5. Regelmäßige Reinigung der Schutzbarriere erhält die Messgenauigkeit

## Fehlerbehebung

### OLED zeigt nichts an
- I2C-Adresse überprüfen (Standard 0x3C, manche verwenden 0x3D)
- Verkabelung prüfen: SDA → GPIO 32, SCL → GPIO 33
- Mit I2C-Scanner-Sketch testen

### Keine LiDAR-Messwerte
- UART-Verkabelung prüfen (RX/TX könnten vertauscht sein)
- TFmini-S Stromversorgung überprüfen (5V)
- Serielle Ausgabe auf Frame-Fehler überwachen
- **Schutzbarriere prüfen:** Sicherstellen, dass Glas-/Kunststoffbarriere sauber und transparent ist
- **Feuchtigkeitsschaden:** Wenn Sensor Wasser ausgesetzt war, kann er dauerhaft beschädigt sein

### Ungenaue oder schwankende Messwerte
- Schutzglas-/Kunststoffbarriere reinigen
- Auf Kondenswasser an der Schutzbarriere prüfen
- Überprüfen, dass Sensor senkrecht zur Flüssigkeitsoberfläche montiert ist
- Sicherstellen, dass Barrierenmaterial geeignet ist (klares Glas oder Acryl empfohlen)
- Sensor-Offset-Parameter anpassen, wenn sich Barrierendicke geändert hat

### Signal K verbindet nicht
- WiFi-Verbindung überprüfen
- Signal K Server IP und Port prüfen
- Sicherstellen, dass Signal K Server läuft und erreichbar ist

### Alarm funktioniert nicht
- GPIO 23 Verbindung prüfen
- Alarmschwellen-Einstellung überprüfen
- Mit Multimeter oder LED testen

## Technische Details

### Systemarchitektur

Das System arbeitet mit einem 200ms Update-Zyklus:
1. Distanz vom TFmini-S LiDAR-Sensor über UART auslesen
2. Füllstand, Prozentsatz und Volumen berechnen
3. OLED-Display mit aktuellen Werten aktualisieren
4. Alarmbedingungen und Notfallmodus prüfen
5. Daten an Signal K Server senden
6. SensESP Event-Loop verarbeiten

### Kommunikationsprotokolle

- **UART:** TFmini-S LiDAR (115200 Baud, 8N1)
- **I2C:** OLED-Display (Software I2C, GPIO 32/33, Adresse 0x3C)
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Signal K:** WebSocket über HTTP (Standard-Port 3000)

## Entwicklung

### Projektstruktur

```
├── platformio.ini          # PlatformIO-Konfiguration
├── src/
│   └── main.cpp           # Hauptanwendungscode
├── include/               # Header-Dateien
├── lib/                   # Eigene Bibliotheken
├── DOCUMENTATION_INDEX.md  # Dokumentationsübersicht
├── EMERGENCY_MODE_LOGIC.md # Notfallmodus-Details
└── config_example.h       # Konfigurationsvorlage
```

### Abhängigkeiten

- [SensESP](https://github.com/SignalK/SensESP) v3.2.0+
- [U8g2](https://github.com/olikraus/u8g2) (OLED-Treiber)
- [ArduinoJson](https://arduinojson.org/) v7.0.0+

## Mitwirken

Beiträge sind willkommen! Bitte zögern Sie nicht, Pull Requests einzureichen oder Issues für Bugs und Feature-Anfragen zu öffnen.

## Lizenz

Dieses Projekt ist Open Source. Prüfen Sie die individuellen Bibliothekslizenzen für Abhängigkeiten.

## Autor

Erstellt von Stefan Blohm (Blohminator)

## Changelog

Siehe [CHANGELOG.md](CHANGELOG.md) für Versionsverlauf und Updates.

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
