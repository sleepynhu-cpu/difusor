# App do Difusor (iPhone/Android + ESP32 por Bluetooth) — grátis

O app tem os botões **ABRIR** e **FECHAR** do difusor do carro e pede senha.
O celular conversa com a ESP32 **por Bluetooth**, sem Wi-Fi. Não atrapalha o 4G nem o CarPlay.

- `difusor/difusor.ino`: código da ESP32
- `docs/`: a página do app (publicada de graça no GitHub Pages)
- Link: https://sleepynhu-cpu.github.io/difusor/

## 1. Gravar na ESP32 (Arduino IDE)

1. Instale a [Arduino IDE](https://www.arduino.cc/en/software).
2. Em *Arquivo > Preferências > URLs adicionais*, cole:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. *Ferramentas > Placa > Gerenciador de placas* → instale **esp32 (Espressif)**.
4. **Senha:** copie `difusor/senha.exemplo.h` para `difusor/senha.h` e escreva a senha nele.
   O `senha.h` fica só no seu PC e não vai para o GitHub (o repositório é público).
5. Abra `difusor/difusor.ino`. No topo dá para mudar
   `TEMPO_ABRIR_MS` / `TEMPO_FECHAR_MS`: quanto tempo cada relé fica ligado (hoje 3 s).
6. Selecione a placa **DOIT ESP32 DEVKIT V1**, a porta COM e clique em **Carregar**.

## 2. Como funciona

- **ABRIR**: liga o relé do D13 pelo tempo configurado e desliga.
- **FECHAR**: liga o relé do D15 pelo tempo configurado e desliga.
- Os dois relés **nunca** ligam juntos; ao inverter, há uma pausa de 0,3 s.
- **Botão físico (D26)**: se está aberto, fecha; se está fechado, abre.
- A ESP32 guarda na memória se ficou aberto ou fechado, mesmo desligando o carro.
- **Senha**: sem ela o app não comanda nada. Após 5 erros, bloqueia por 30 s.
  O app pode lembrar a senha no celular ("Lembrar neste aparelho").

## 3. No celular

**iPhone:** instale o **Bluefy – Web BLE Browser** (grátis) e abra o link dentro dele.
**Android:** abra o link no **Google Chrome** (Bluetooth e Localização ligados).

Com o carro ligado: **Conectar** → escolha **Difusor** → digite a senha → use ABRIR / FECHAR.

### Mensagem pronta para o WhatsApp

> Pra controlar o difusor: no iPhone instala o app Bluefy (grátis) e abre este link nele; no Android abre no Chrome: https://sleepynhu-cpu.github.io/difusor/
> Com o carro ligado, toca em Conectar, escolhe "Difusor" e digita a senha.

## 4. Ligação

BC337 de frente (lado reto com o texto, pernas para baixo): **C – B – E**.

Para **cada** relé (ABRIR no D13, FECHAR no D15):

```
D13 (ou D15) ──[1 kΩ]──► Base BC337
                         Base ──[10 kΩ]── Emissor   (mantém desligado na partida)
Emissor ──► GND
Coletor ──► bobina do relé ──► +12 V (ou +5 V, conforme a bobina do relé)
Diodo 1N4007 em paralelo com a bobina: faixa (catodo) no lado +12 V
```

- O **diodo na bobina é obrigatório**: sem ele, o pico ao desligar o relé queima o BC337.
- O resistor de **10 kΩ base-emissor** é importante no **D15**, que fica fraco em HIGH
  enquanto a ESP32 liga (sem ele o relé pode dar um "tique").
- Os contatos dos relés vão no difusor (abrir / fechar), como no circuito original.

**Botão (D26):**

```
3,3 V ──[10 kΩ]──┬── D26
                 ├──[100 nF]── GND
Botão: D26 ──────┘──/ ──── GND
```

O pull-up interno já está ligado no código, mas no carro o resistor de 10 kΩ
e o capacitor evitam toques fantasmas por ruído.

**Alimentação:** conversor buck ajustado em **5,0 V** → VIN da ESP32, entrada
no positivo pós-chave com fusível de 1–2 A. Tudo com **GND em comum**.
