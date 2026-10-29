// Difusor Bluetooth — ESP32
// Dois BC337 acionam dois reles: um ABRE e outro FECHA o difusor, por tempo.
// O app (Bluefy no iPhone / Chrome no Android) so comanda depois da senha.
// Um botao fisico (manda GND) alterna entre abrir e fechar.

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Preferences.h>
#include "senha.h"  // fica so no seu PC (nao vai para o GitHub); modelo em senha.exemplo.h

// ======== CONFIGURE AQUI ========
const char* NOME_BLUETOOTH = "Difusor";  // nome que aparece na hora de conectar
const char* SENHA = SENHA_DIFUSOR;       // senha pedida pelo app (definida em senha.h)

const int PINO_ABRIR = 15;    // D15 -> base do BC337 do rele ABRIR (resistor ~1k)
                              // D15 fica fraco em HIGH na partida: use 10k base-emissor
const int PINO_FECHAR = 13;   // D13 -> base do BC337 do rele FECHAR (resistor ~1k)
const int PINO_BOTAO = 26;    // D26 <- botao que manda GND (pull-up)

const unsigned long TEMPO_ABRIR_MS = 500;    // quanto tempo o rele ABRIR fica ligado
const unsigned long TEMPO_FECHAR_MS = 500;   // quanto tempo o rele FECHAR fica ligado
const unsigned long PAUSA_INVERSAO_MS = 300; // espera entre desligar um rele e ligar o outro
// ================================

// Precisam ser iguais aos do app (index.html)
#define UUID_SERVICO "8f1d0001-6c2e-4b7a-9d3f-2a5b1c0e7d41"
#define UUID_ESTADO  "8f1d0002-6c2e-4b7a-9d3f-2a5b1c0e7d41"
#define UUID_SENHA   "8f1d0003-6c2e-4b7a-9d3f-2a5b1c0e7d41"

// Estados enviados ao app
enum Estado : uint8_t { FECHADO = 0, ABERTO = 1, ABRINDO = 2, FECHANDO = 3, BLOQUEADO = 0xFF };

BLECharacteristic* caracEstado;
Preferences memoria;

Estado estado = FECHADO;        // posicao atual (salva na memoria)
unsigned long inicioMovimento = 0;
unsigned long liberarEm = 0;    // fim da pausa entre reles
int pendente = -1;              // comando aguardando a pausa: 1 abrir, 0 fechar

volatile bool autenticado = false;
volatile int pedido = -1;       // -1 nada, 0 fechar, 1 abrir
volatile bool reanunciar = false;
volatile bool conectou = false;
volatile int tentativasErradas = 0;
volatile unsigned long bloqueioAte = 0;

void reles(bool abrir, bool fechar) {
  digitalWrite(PINO_ABRIR, abrir ? HIGH : LOW);
  digitalWrite(PINO_FECHAR, fechar ? HIGH : LOW);
}

void publicarEstado() {
  uint8_t v = autenticado ? estado : BLOQUEADO;
  caracEstado->setValue(&v, 1);
  caracEstado->notify();
}

void definirEstado(Estado e) {
  estado = e;
  if (e == ABERTO || e == FECHADO) memoria.putUChar("estado", e);
  Serial.printf("Estado: %d\n", e);
  publicarEstado();
}

void comandar(bool abrir) {
  // Repetir o mesmo comando e permitido: corrige se a posicao salva nao bate com a real
  if (estado == (abrir ? ABRINDO : FECHANDO)) return;
  // Nunca liga os dois reles juntos: desliga tudo e espera antes de inverter
  reles(false, false);
  pendente = abrir ? 1 : 0;
  liberarEm = millis() + PAUSA_INVERSAO_MS;
}

class ServidorCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) override { conectou = true; }
  void onDisconnect(BLEServer*) override {
    autenticado = false;
    reanunciar = true;
  }
};

class SenhaCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    if (millis() < bloqueioAte) {
      autenticado = false;
    } else {
      size_t n = c->getLength();
      autenticado = n == strlen(SENHA) && memcmp(c->getData(), SENHA, n) == 0;
      if (autenticado) {
        tentativasErradas = 0;
      } else if (++tentativasErradas >= 5) {
        tentativasErradas = 0;
        bloqueioAte = millis() + 30000;  // 5 erros: bloqueia 30 s
      }
    }
    uint8_t v = autenticado ? estado : BLOQUEADO;
    caracEstado->setValue(&v, 1);
    caracEstado->notify();
  }
};

class EstadoCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    if (autenticado && c->getLength() >= 1) pedido = c->getData()[0];
  }
};

void lerBotao() {
  static bool ultimo = HIGH;
  static unsigned long mudouEm = 0;
  static bool estavel = HIGH;
  bool agora = digitalRead(PINO_BOTAO);
  if (agora != ultimo) {
    ultimo = agora;
    mudouEm = millis();
  }
  if (millis() - mudouEm > 50 && agora != estavel) {
    estavel = agora;
    if (estavel == LOW) comandar(!(estado == ABERTO || estado == ABRINDO));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_ABRIR, OUTPUT);
  pinMode(PINO_FECHAR, OUTPUT);
  reles(false, false);
  pinMode(PINO_BOTAO, INPUT_PULLUP);

  memoria.begin("difusor", false);
  estado = (Estado)memoria.getUChar("estado", FECHADO);
  if (estado != ABERTO) estado = FECHADO;

  BLEDevice::init(NOME_BLUETOOTH);
  BLEServer* servidor = BLEDevice::createServer();
  servidor->setCallbacks(new ServidorCallbacks());

  BLEService* servico = servidor->createService(UUID_SERVICO);
  caracEstado = servico->createCharacteristic(
      UUID_ESTADO,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY);
  caracEstado->addDescriptor(new BLE2902());
  caracEstado->setCallbacks(new EstadoCallbacks());
  BLECharacteristic* caracSenha = servico->createCharacteristic(
      UUID_SENHA, BLECharacteristic::PROPERTY_WRITE);
  caracSenha->setCallbacks(new SenhaCallbacks());
  uint8_t v = BLOQUEADO;
  caracEstado->setValue(&v, 1);
  servico->start();

  BLEAdvertising* anuncio = BLEDevice::getAdvertising();
  anuncio->addServiceUUID(UUID_SERVICO);
  anuncio->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.printf("Bluetooth pronto: procure por \"%s\". Posicao salva: %s\n",
                NOME_BLUETOOTH, estado == ABERTO ? "ABERTO" : "FECHADO");
}

void loop() {
  if (conectou) {
    conectou = false;
    publicarEstado();  // nova conexao comeca bloqueada
  }

  int p = pedido;
  if (p == 0 || p == 1) comandar(p == 1);
  pedido = -1;

  lerBotao();

  // Liga o rele do comando pendente depois da pausa
  if (pendente >= 0 && millis() >= liberarEm) {
    bool abrir = pendente == 1;
    pendente = -1;
    reles(abrir, !abrir);
    inicioMovimento = millis();
    definirEstado(abrir ? ABRINDO : FECHANDO);
  }

  // Desliga o rele quando o tempo acaba
  if (estado == ABRINDO && millis() - inicioMovimento >= TEMPO_ABRIR_MS) {
    reles(false, false);
    definirEstado(ABERTO);
  } else if (estado == FECHANDO && millis() - inicioMovimento >= TEMPO_FECHAR_MS) {
    reles(false, false);
    definirEstado(FECHADO);
  }

  if (reanunciar) {
    reanunciar = false;
    delay(200);
    BLEDevice::startAdvertising();
  }
  delay(5);
}
