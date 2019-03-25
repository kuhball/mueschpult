# Müschpult 2
Dokumentation des Mischpults für die Jupibar im Gängeviertel. Hier werden 2 Stereo Ausgänge für zwei Barbereiche mit eigener Lautstärkerregelung benötigt. Das Grundgerät ist eine Yamaha DME 24N. Hierfür wurde mithilfe eines Arduino Nanos eine RS232 Fernbedienung gebaut.

## Anforderung
1. RS232 mit DME sprechen
2. Input von Potis & Schaltern auf DME setzen
3. DME Werte auf LEDs anzeigen

## geänderte Header Dateien
HardwareSerial.h

#define SERIAL_RX_BUFFER_SIZE 256

Der Compiler erkennt u.U. nicht das diese Datei geändert wurde. 
Es ist also ratsam an entsprechnder stelle im Projekt die vom Compuler erstellte Objektdateie HardwareSerial.cpp.o zu löschen, damit sie neu kompiliert wird.

## Hardware
### Mux 74HC4051
8 Kanal analog Multiplexer für Input Potis. Hiervon wurden 3 Stück eingesetzt. Alle 3 werden zeitgleich gesetzt und dann am entsprechenden Pin ausgelesen.
Pins für lesen:
- A3
- A4
- A5

Pins für setzen des Zustands:
- A0
- A1
- A2

### Ledbar Treiber Shiftregister 74HC595
3 in Reihe geschaltete Schieberegister betreiben zwei 10 segment Ledbars die den Ausgangspregel der DME an die Anlage darstellen.
- Clock Pin D2
- Latch Pin D3
- Data Pin D4


## DME Setup Liste

- Talkover für Mikrofone 50 / 51 (mic1, mic2) - binär 1/0

- Fader Volume 54 / 55 (mic1, mic2) -  min -13801, max 1000
- Meter 56 (alle Meter)
  - 0 - alle Meter?
  - 1 - Input DJ Oben
  - 2 - Input Bar
  - 3 - Spare Input
  - 4 - Output Level Oben
  - 5 - Output Level Bar
  - 7 - Mic 1 Level
  - 8 - Mic 2 Level
- Input Level Stereo in 1(DJ) - 58/59  min -13801, max 1000
- Input Level Stereo in 2(Bar) - 60/61 min -13801, max 1000
- Input Level Stereo in 3(Spare) - 62/63 min -13801, max 1000
- Masterlevel Oben - 64/65
- Masterlevel Bar - 66/67

- EQ Oben:
  - High 70
  - Mid 71
  - Low 72
- EQ Bar:
  - High 73
  - Mid 74
  - Low 75
  
In der DME angelegt aber am Müschpult nicht vorhanden  
- 6 - Output Level Keller
- Mute für Mikrofone 52 / 53 (mic1, mic2) - 0/1
- EQ Keller
  - High 76
  - Mid 77
  - Low 78
- Masterlevel Keller - 68/69


## Inputs Arduino
### MUX 0
| Name           | Pin  | DME  |
| -------------- | ---- | ---- |
| n.c.           | 0    | 50   |
| n.c.           | 1    | 53   |
| n.c.           | 2    | 58   |
| n.c.           | 3    | 64   |
| n.c.           | 4    | 70   |
| Talkover Mic2  | 5    | 73   |
| Delay Switch   | 6    | 76   |
| Talkover Mic1  | 7    | ??   |

## MUX 1
| Name          | Pin  | DME  |
| ------------- | ---- | ---- |
| n.c.          | 0    | 51   |
| Vol in Spare  | 1    | 54   |
| Vol in Bar    | 2    | 60   |
| n.c.          | 3    | 66   |
| Vol in DJ     | 4    | 71   |
| n.c.          | 5    | 74   |
| n.c.          | 6    | 77   |
| n.c.          | 7    |      |

## MUX 2
| Name          | Pin  | DME  |
| ------------- | ---- | ---- |
| EQ_Bar_Lo     | 0    | 52   |
| EQ_Bar_Mid    | 1    | 55   |
| EQ_Bar_Hi     | 2    | 62   |
| Vol_Bar       | 3    | 68   |
| EQ_Oben_Hi    | 4    | 72   |
| EQ_Oben_Mid   | 5    | 75   |
| EQ_Oben_Mid   | 6    | 78   |
| EQ_Oben_Lo    | 7    |      |

## DME Meter

!!! Zum Verarbeiten des langen Antwortstrings der DME wurde der Serielle Buffer vergrößert.
Hierfür wurde die variable SERIAL_RX_BUFFER_SIZE in der HardwareSerial.h auf 256 gesetzt.
http://shelvin.de/arduino-serial-buffer-size-aendern/


Beispielantwort der DME auf ein `GMT 0 56 0`.
```
MTR 0 56 0 CUR -13801 -13801 -13801 -6000 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 HOLD -9322 -9741 -9332 -13801 -7421 -8943 -8943 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801

MTR 0 56 0 CUR -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 HOLD -9322 -9741 -9332 -13801 -7421 -8943 -8943 -13801
MTR 0 56 0 CUR 1 1 1 1 1 1 1 1 HOLD -9322 -9741 -9332 -13801 -7421 -8943 -8943 -13801

```
