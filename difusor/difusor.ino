// Difusor Bluetooth — ESP32
// O iPhone conecta direto por Bluetooth (BLE), sem Wi-Fi.
// No iPhone: abra o link do app no Bluefy (gratis na App Store) e toque em Conectar.

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

// ======== CONFIGURE AQUI ========
const char* NOME_BLUETOOTH = "Difusor";  // nome que aparece na hora de conectar
const int PINO_DIFUSOR = 26;             // GPIO ligado ao rele / transistor
const bool ATIVO_EM_ALTO = true;         // rele que liga com LOW? coloque false

// Se a ESP32 "aperta" o botao original do difusor em vez de ligar a energia,
// coloque true: cada toque no app gera um pulso no pino.
const bool MODO_PULSO = false;
const int PULSO_MS = 300;
// ================================

// Precisam ser iguais aos do app (index.html)
#define UUID_SERVICO "8f1d0001-6c2e-4b7a-9d3f-2a5b1c0e7d41"
#define UUID_ESTADO  "8f1d0002-6c2e-4b7a-9d3f-2a5b1c0e7d41"

BLECharacteristic* caracEstado;
bool ligado = false;
volatile int pedido = -1;  // -1 nada, 0 desligar, 1 ligar, 2 alternar
volatile bool reanunciar = false;

void aplicarSaida(bool ativo) {
  digitalWrite(PINO_DIFUSOR, ativo == ATIVO_EM_ALTO ? HIGH : LOW);
}

void publicarEstado() {
  uint8_t v = ligado ? 1 : 0;
  caracEstado->setValue(&v, 1);
  caracEstado->notify();
}

void definirDifusor(bool novoEstado) {
  if (novoEstado != ligado) {
    if (MODO_PULSO) {
      aplicarSaida(true);
      delay(PULSO_MS);
      aplicarSaida(false);
    } else {
      aplicarSaida(novoEstado);
    }
    ligado = novoEstado;
  }
  publicarEstado();
}

class ServidorCallbacks : public BLEServerCallbacks {
  void onDisconnect(BLEServer*) override { reanunciar = true; }
};

class EstadoCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    if (c->getLength() >= 1) pedido = c->getData()[0];
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(PINO_DIFUSOR, OUTPUT);
  aplicarSaida(false);

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
  uint8_t v = 0;
  caracEstado->setValue(&v, 1);
  servico->start();

  BLEAdvertising* anuncio = BLEDevice::getAdvertising();
  anuncio->addServiceUUID(UUID_SERVICO);
  anuncio->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("Bluetooth pronto: procure por \"Difusor\"");
}

void loop() {
  int p = pedido;
  if (p >= 0) {
    pedido = -1;
    definirDifusor(p == 2 ? !ligado : p == 1);
  }
  if (reanunciar) {
    reanunciar = false;
    delay(200);
    BLEDevice::startAdvertising();
  }
  delay(10);
}
