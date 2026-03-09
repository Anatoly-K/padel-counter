/*
 * Padel Clicker — ESP32-WROOM-32
 * BLE GATT + OLED дисплей + 2 кнопки
 *
 * Подключение:
 *   Синяя кнопка  (моя команда): GPIO16 + GND
 *   Красная кнопка (противник) : GPIO17 + GND
 *   Дисплей SDA: GPIO21
 *   Дисплей SCL: GPIO22
 *   Дисплей VCC: 3.3V
 *   Дисплей GND: GND
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- Пины ---
#define PIN_MY_TEAM  16
#define PIN_OPPONENT 17

// --- Дисплей ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- BLE UUID ---
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-ab12-ab12-ab12-abcdef123456"

// --- BLE переменные ---
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// --- Счёт ---
int scoreMe  = 0;
int scoreOpp = 0;

// --- Forward declaration ---
void showScore();

// --- Колбэк подключения/отключения ---
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    showScore();
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    showScore();
    pServer->getAdvertising()->start();
  }
};

// --- Отображение счёта на дисплее ---
void showScore() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(deviceConnected ? "BLE: connected" : "BLE: waiting...");

  display.setTextSize(3);
  display.setCursor(15, 25);
  display.print(scoreMe);
  display.print(" : ");
  display.print(scoreOpp);

  display.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_MY_TEAM,  INPUT_PULLUP);
  pinMode(PIN_OPPONENT, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  showScore();

  BLEDevice::init("Padel Clicker");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  pServer->getAdvertising()->start();

  Serial.println("BLE started, waiting for connection...");
}

void loop() {
  if (digitalRead(PIN_MY_TEAM) == LOW) {
    scoreMe = (scoreMe + 1) % 100;
    if (deviceConnected) {
      pCharacteristic->setValue("1");
      pCharacteristic->notify();
    }
    showScore();
    delay(300);
  }

  if (digitalRead(PIN_OPPONENT) == LOW) {
    scoreOpp = (scoreOpp + 1) % 100;
    if (deviceConnected) {
      pCharacteristic->setValue("2");
      pCharacteristic->notify();
    }
    showScore();
    delay(300);
  }
}
