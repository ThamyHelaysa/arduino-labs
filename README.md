# arduino-labs

[![Compile](https://github.com/ThamyHelaysa/arduino-labs/actions/workflows/compile.yml/badge.svg)](https://github.com/ThamyHelaysa/arduino-labs/actions/workflows/compile.yml)

Projetos e experimentos com Arduino e ESP32.

## Projetos

| Projeto | Descrição |
| --- | --- |
| [painel](projects/painel) | Painel de mesa com tela e-ink: tempo, frase do dia e cheat sheets. |

## Ferramentas

| Ferramenta | Descrição |
| --- | --- |
| [fonts](tools/fonts) | Testa em PNG se um texto cabe na tela e converte fontes para U8g2. |

## Como compilar

Cada projeto fixa placa e bibliotecas em `sketch.yaml`. Com o [Arduino CLI](https://arduino.github.io/arduino-cli/) instalado:

```sh
cd projects/painel
cp segredos.exemplo.h segredos.h   # preencha com os seus dados
arduino-cli compile --profile esp32
```

## Licença

Código sob [MIT](LICENSE). Fontes de terceiros mantêm a licença própria.
