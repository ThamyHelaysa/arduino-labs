# Ferramentas de fonte

Testam e convertem fontes U8g2 sem precisar gravar a placa. Precisam só de Python 3.

## preview

Gera um PNG do tamanho da tela e informa se o texto cabe. Termina com erro se faltar glifo ou espaço.

```sh
python3 tools/fonts/preview.py "VAI CHOVER. LEVA GUARDA-CHUVA" \
  --font Pixellari_tf --invert --center --max-lines 2 --size 296x40
```

Para ver as fontes instaladas:

```sh
python3 tools/fonts/preview.py --list pixel
```

Por padrão, as fontes vêm de `~/Documents/Arduino/libraries/U8g2_for_Adafruit_GFX/src/u8g2_fonts.c`. Use `--library` para outro caminho. A largura é a soma dos avanços dos glifos, então pode diferir em 1 px da medida na placa. Só são lidos glifos de código até 255.

## convert

Transforma TTF, OTF ou BDF em um `.h` para o sketch. Na primeira vez, compile o `bdfconv`:

```sh
tools/fonts/setup.sh
brew install otf2bdf   # só para TTF e OTF
```

```sh
python3 tools/fonts/convert.py VT323-Regular.ttf --name fonte_banner --size 16
python3 tools/fonts/preview.py "Teste de acentuação" --font-file fonte_banner.h
```

Em fonte pixel, `--size` precisa ser o tamanho nativo da fonte ou um múltiplo dele, senão as letras deformam.

Antes de publicar um `.h` gerado, confira a licença da fonte e guarde o arquivo de licença junto dele.
