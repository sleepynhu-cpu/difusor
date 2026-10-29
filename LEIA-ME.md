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
4. Abra `difusor/difusor.ino` e, se precisar, ajuste o topo:
   - `PINO_DIFUSOR` → GPIO que você ligou (padrão 26).
   - `ATIVO_EM_ALTO` → `false` se o módulo relé liga com LOW (comum nos módulos azuis).
   - `MODO_PULSO` → `true` se a ESP32 "aperta" o botão do difusor em vez de ligar a energia dele.
5. Selecione a placa **DOIT ESP32 DEVKIT V1** (ou ESP32 Dev Module), a porta COM e clique em **Carregar**.

Use uma ESP32 comum (ESP32, S3 ou C3). A ESP32-S2 **não tem Bluetooth**.

## 2. No iPhone do dono da BMW

1. Instale o **Bluefy – Web BLE Browser** (grátis na App Store).
2. Abra o link do app **dentro do Bluefy**.
3. Com o carro ligado, toque em **Conectar** e escolha **Difusor**.
4. Toque no botão redondo para ligar ou desligar.

Dica: no Bluefy, salve o link nos favoritos para abrir rápido.

### Mensagem pronta para o WhatsApp

> Pra controlar o difusor: instala o app Bluefy (grátis na App Store), abre este link dentro dele: https://sleepynhu-cpu.github.io/difusor/
> Com o carro ligado, toca em Conectar, escolhe "Difusor" e usa o botão.

## Ligação sugerida (relé)

| ESP32  | Módulo relé 5V |
|--------|----------------|
| VIN/5V | VCC            |
| GND    | GND            |
| GPIO26 | IN             |

Alimente a ESP32 pela USB do carro (liga e desliga junto com o carro).
Os contatos COM/NA do relé entram em série com a alimentação do difusor
(ou em paralelo com o botão dele, no modo pulso).
