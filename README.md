# Hauswasserwerk

___

![](assets/img/hauswasserwerk.png)

Das Hauswasserwerk besteht aus mehreren Pumpen, welche beide über WiFi Smart Aktoren angesteuert werden.
Die Master-Einheit besteht aus einem Arduino Uno mit W5500 Ethernet Driver.

Die Konfigurationen werden jeweils auf den Geräten Lokal gespeichert, eine Steuerung wird über HomeAssistant
bereitgestellt.

## Todos:

- Telnet Server
- Automatische Befüllung des Pufferspeichers.
- Automatische Befüllung nach Vorrang.
- Fehlermeldung (Anhand Füllstand, bei BSPW. 10% Druckspeicherpumpe nicht mehr einschalten.)
- Fehlermeldung (Anhand Leistungsaufnahme)
- Füllstandüberwachung (Ultraschall)
- Wassermelder
- PUSH Benachrichtigungen

## Struktur:

```mermaid
graph TD
    Master(Master) --> Netzwerk
    Netzwerk --> Shelly1(Shelly)
    Netzwerk --> Shelly2(Shelly)
    Shelly1 --> Pumpe1(Tauchpumpe)
    Shelly2 --> Pumpe2(Tauchpumpe)
    Master --> Aktoren
    Aktoren --> Pumpe3(Druckspeicherpumpe)
    Aktoren --> Lampe1(Fehler LED)
    Aktoren --> Lampe2(Status LED)
    Master --> Sensoren
    Sensoren --> Pegelschalter
    Sensoren --> Wassermelder
```