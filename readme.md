# Müschpult 2
Dokumentation des Mischpults für die Jupibar im Gängeviertel. Hier werden 3 Stereo Ausgänge für verschiedene Barbereiche mit eigener Lautstärkerregelung benötigt. Das Grundgerät ist eine Yamaha DME 24N. Hierfür wurde mithilfe eines Arduino Nanos eine RS232 Fernbedienung gebaut.

## Anforderung
1. RS232 mit DME sprechen
2. Input von Potis & Schaltern auf DME setzen
3. DME Werte auf LEDs anzeigen

## Benutze Libraries
### Tlc5940
Benutzt für LEDs auf Frontpanel.

## Hardware
### Mux 4051
8 Kanal analog Multiplexer für Input Potis. Hiervon wurden 3 Stück eingesetzt. Alle 3 werden zeitgleich gesetzt und dann am entsprechenden Pin ausgelesen.
Pins für lesen:
- A0
- A1
- A2

Pins für setzen des Zustands:
- A3
- A4
- A5

## DME Setup Liste

- Talkover für Mikrofone 50 / 51 (mic1, mic2) - binär 1/0
- Mute für Mikrofone 52 / 53 (mic1, mic2) - 0/1
- Fader Volume 54 / 55 (mic1, mic2) -  min -13801, max 1000
- Meter 56 (alle Meter)
  - 0 - alle Meter?
  - 1 - Input DJ Oben
  - 2 - Input Bar
  - 3 - Spare Input
  - 4 - Output Level Oben
  - 5 - Output Level Bar
  - 6 - Output Level Keller
  - 7 - Mic 1 Level
  - 8 - Mic 2 Level
- Input Level Stereo in 1 - 58/59  min -13801, max 1000
- Input Level Stereo in 2 - 60/61 min -13801, max 1000
- Input Level Stereo in 3 - 62/63 min -13801, max 1000
- Masterlevel Oben - 64/65
- Masterlevel Bar - 66/67
- Masterlevel Unten - 68/69
- EQ Oben:
  - High 70
  - Mid 71
  - Low 72
- EQ Bar:
  - High 73
  - Mid 74
  - Low 75
- EQ Keller
  - High 76
  - Mid 77
  - Low 78

## Inputs Arduino
### MUX 1
| Name           | Pin  | DME  |
| -------------- | ---- | ---- |
| Talkover Mic1  | 0    | 50   |
| Mute Mic2      | 1    | 53   |
| Input DJ       | 2    | 58   |
| Output Oben    | 3    | 64   |
| EQ Oben High   | 4    | 70   |
| EQ Bar High    | 5    | 73   |
| EQ Keller High | 6    | 76   |
| Delay Switch   | 7    | ??   |

## MUX 2
| Name          | Pin  | DME  |
| ------------- | ---- | ---- |
| Talkover Mic2 | 0    | 51   |
| Volume Mic1   | 1    | 54   |
| Input Bar     | 2    | 60   |
| Output Bar    | 3    | 66   |
| EQ Oben Mid   | 4    | 71   |
| EQ Bar Mid    | 5    | 74   |
| EQ Keller Mid | 6    | 77   |
|               | 7    |      |

## MUX 3
| Name          | Pin  | DME  |
| ------------- | ---- | ---- |
| Mute Mic1     | 0    | 52   |
| Volume Mic2   | 1    | 55   |
| Input Spare   | 2    | 62   |
| Output Unten  | 3    | 68   |
| EQ Oben Low   | 4    | 72   |
| EQ Bar Low    | 5    | 75   |
| EQ Keller Low | 6    | 78   |
|               | 7    |      |

## DME Meter
Beispielantwort der DME auf ein `GMT 0 56 0`.
```
MTR 0 56 0 CUR -13801 -13801 -13801 -6000 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 HOLD -9322 -9741 -9332 -13801 -7421 -8943 -8943 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801

MTR 0 56 0 CUR -13801 -13801 -13801 -13801 -13801 -13801 -13801 -13801 HOLD -9322 -9741 -9332 -13801 -7421 -8943 -8943 -13801
MTR 0 56 0 CUR 1 1 1 1 1 1 1 1 HOLD -9322 -9741 -9332 -13801 -7421 -8943 -8943 -13801

```
