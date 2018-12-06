# Müschpult
Dokumentation des Mischpults für die Jupibar im Gängeviertel.

## Vorraussetzungen Code
1. Serial mit der DME
2. LEDs & Potis von Frontpanel einlesen / ausgeben

## Benutze Libraries
### Tlc5940
Benutzt für LEDs auf Frontpanel.

## Hardware
### Mux 4051
8 Kanal analog Multiplexer für Input Potis.
Pins für lesen:
- A0
- A1
- A2

Pins für setzen des Zustands:
- A3
- A4
- A5

## DME Setup List

- Talkover für Mikrofone 50 / 51 (mic1, mic2) - binär 1/0
- Mute für Mikrofone 52 / 53 (mic1, mic2) - 0/1
- Fader Volume 54 / 55 (mic1, mic2) -  min -13801, max 1000
- Meter 56 (alle Meter)
  - 0 - alle Meter?
  - 1 - Input DJ Oben
  - 2 - Input Bar
  - 3 - Spare Input
  - 5 - Output Level Oben
  - 6 - Output Level Bar
  - 7 - Output Level Keller
  - 15 - Mic 1 Level
  - 16 - Mic 2 Level
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
