# App do Difusor (iPhone + ESP32 por Bluetooth) — grátis

Um botão que liga e desliga o difusor do carro. O iPhone conversa com a ESP32
**por Bluetooth**, sem Wi-Fi. Não atrapalha o 4G nem o CarPlay.

- `difusor/difusor.ino`: código da ESP32
- `docs/`: a página do botão (publicada de graça no GitHub Pages)

## 1. Gravar na ESP32 (Arduino IDE)

1. Instale a [Arduino IDE](https://www.arduino.cc/en/software).
2. Em *Arquivo > Preferências > URLs adicionais*, cole:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. *Ferramentas > Placa > Gerenciador de placas* → instale **esp32 (Espressif)**.
4. Abra `difusor/difusor.ino`. Já vem configurado para o BC337 no **D13**:
   cada toque no app deixa o D13 em HIGH por 300 ms e volta para LOW.
   Se o difusor não reconhecer o toque, aumente `PULSO_MS` (ex.: 500).
5. Selecione a placa **DOIT ESP32 DEVKIT V1** (ou ESP32 Dev Module), a porta COM e clique em **Carregar**.

Use uma ESP32 comum (ESP32, S3 ou C3). A ESP32-S2 **não tem Bluetooth**.

## 2. No iPhone do dono da BMW

1. Instale o **Bluefy – Web BLE Browser** (grátis na App Store).
2. Abra o link do app **dentro do Bluefy**.
3. Com o carro ligado, toque em **Conectar** e escolha **Difusor**.
4. Toque no botão redondo para ligar ou desligar.

Dica: no Bluefy, salve o link nos favoritos para abrir rápido.

## No Android

Não precisa de app extra: abra o link no **Google Chrome**, deixe o Bluetooth
(e a Localização, que o Android exige para achar aparelhos Bluetooth) ligados e
toque em **Conectar**. Pelo menu ⋮ → **Adicionar à tela inicial** fica com ícone de app.

### Mensagem pronta para o WhatsApp

> Pra controlar o difusor: instala o app Bluefy (grátis na App Store), abre este link dentro dele: https://sleepynhu-cpu.github.io/difusor/
> Com o carro ligado, toca em Conectar, escolhe "Difusor" e usa o botão.

## Ligação (BC337 no botão do difusor)

BC337 de frente (lado reto com o texto, pernas para baixo): **C – B – E**.

| BC337        | Liga em                                      |
|--------------|----------------------------------------------|
| Base (B)     | D13 da ESP32, **por um resistor de ~1 kΩ**   |
| Emissor (E)  | GND (o GND da ESP32 e o do difusor juntos)   |
| Coletor (C)  | lado do botão do difusor que **não** é o GND |

O transistor fica em paralelo com o botão: quando o D13 vai para HIGH, ele
"aperta" o botão. Meça com o multímetro qual lado do botão é o GND do difusor;
o BC337 só funciona se o botão fechar para o GND.

Alimente a ESP32 pela USB do carro (liga e desliga junto com o carro).

Obs.: a ESP32 não "vê" se o difusor está ligado, ela só aperta o botão.
Se o app mostrar o contrário do real, o botão continua funcionando
(cada toque é um aperto). Para acertar, desligue e ligue o carro com o
difusor desligado.
