# Painel

Painel de mesa com tela e-ink de 2.9" que mostra o tempo, uma frase do dia e cheat sheets de comandos. Os dados do tempo vêm do [Open-Meteo](https://open-meteo.com/), sem chave de API.

## Peças

- ESP32
- Tela e-ink 2.9" de 3 cores, 296x128 (driver `GxEPD2_290_C90c`)
- Um botão

## Ligações

| Tela | GPIO |
| --- | --- |
| CS | 5 |
| DC | 22 |
| RST | 21 |
| BUSY | 4 |
| SCK | 18 |
| SDI/MOSI | 23 |

O botão liga o GPIO 32 ao GND.

## Instalação

1. Copie `segredos.exemplo.h` para `segredos.h`.
2. Preencha Wi-Fi, latitude e longitude. O fuso horário fica em `clima.h`.
3. Grave na placa, trocando a porta:

```sh
arduino-cli compile --profile esp32 --upload -p /dev/cu.usbserial-XXXX
```

## Uso

- Aperto curto: alterna entre a tela principal e as cheat sheets.
- Aperto longo (3 s) na tela principal: sorteia outra frase, se não houver recado do tempo.

Frases ficam em `frases.h` e comandos em `cheatsheets.h`.
