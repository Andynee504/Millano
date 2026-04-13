#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <BleGamepad.h>
#include <Preferences.h>
#include <NimBLEDevice.h>

Adafruit_MPU6050 mpu;
Preferences preferences;

// ===== enderecos BLE API =====
static const char* CONFIG_DEVICE_NAME = "Mellano Config";

static NimBLEServer* configServer = nullptr;
static NimBLEService* configService = nullptr;
static NimBLECharacteristic* configWriteChar = nullptr;
static NimBLECharacteristic* configNotifyChar = nullptr;

static const char* CONFIG_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* CONFIG_WRITE_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* CONFIG_NOTIFY_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

// ===== PINOS =====
const int SDA_PIN = 22;
const int SCL_PIN = 21;

const int PEDAL_1_PIN = 18;  // A
const int PEDAL_2_PIN = 19;  // B
const int PEDAL_3_PIN = 23;  // segurar no boot = modo config

// ===== CONFIGURACAO =====
float centerX = 0.0f;
float centerY = 0.0f;

float deadzone = 0.80f;
float sensX = 18.0f;
float sensY = 18.0f;

bool invertX = false;   // face pra cima olhando pra frente
bool invertY = false;  // talvez mude para jogo

// ===== GAMEPAD =====
BleGamepad bleGamepad("Mellano Proto", "PUCSP", 100);

// ===== FAIXA HID =====
const int HID_MIN = 0;
const int HID_MAX = 32767;

// ===== SAIDA LOGICA FINAL =====
int axisX = 0;
int axisY = 0;

bool btnA = false;
bool btnB = false;
bool btnX = false;

// ===== CONTROLE =====
unsigned long lastPrint = 0;
const unsigned long printInterval = 150;

// ===== MODO DE OPERACAO =====
enum DeviceMode {
  MODE_GAME,
  MODE_CONFIG
};

DeviceMode currentMode = MODE_GAME;
String commandBuffer = "";
const unsigned long configHoldMs = 1200;

// =========================================================
// Helpers
// =========================================================
int clampInt(int value, int minValue, int maxValue) {
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

float applyDeadzone(float value, float dz) {
  if (value > -dz && value < dz) return 0.0f;
  return value;
}

int mapAxisToHID(float value, float sensitivity, bool invertAxis) {
  float adjusted = applyDeadzone(value, deadzone);

  if (invertAxis) {
    adjusted *= -1.0f;
  }

  int signedValue = (int)(adjusted * sensitivity);
  signedValue = clampInt(signedValue, -127, 127);

  long hidValue = map(signedValue, -127, 127, HID_MIN, HID_MAX);
  return clampInt((int)hidValue, HID_MIN, HID_MAX);
}

// =========================================================
// Persistencia
// =========================================================
bool loadConfig() {
  preferences.begin("mellano", true);

  uint8_t saved = preferences.getUChar("saved", 0);
  if (saved == 0) {
    preferences.end();
    return false;
  }

  centerX = preferences.getFloat("centerX", 0.0f);
  centerY = preferences.getFloat("centerY", 0.0f);
  deadzone = preferences.getFloat("deadzone", 0.80f);
  sensX = preferences.getFloat("sensX", 18.0f);
  sensY = preferences.getFloat("sensY", 18.0f);
  invertX = preferences.getUChar("invertX", 1) != 0;
  invertY = preferences.getUChar("invertY", 0) != 0;

  preferences.end();
  return true;
}

void saveConfig() {
  preferences.begin("mellano", false);

  preferences.putUChar("saved", 1);
  preferences.putFloat("centerX", centerX);
  preferences.putFloat("centerY", centerY);
  preferences.putFloat("deadzone", deadzone);
  preferences.putFloat("sensX", sensX);
  preferences.putFloat("sensY", sensY);
  preferences.putUChar("invertX", invertX ? 1 : 0);
  preferences.putUChar("invertY", invertY ? 1 : 0);

  preferences.end();

  Serial.println("CONFIG SALVA.");
}

// =========================================================
// Sensor / botoes
// =========================================================
void calibrateCenter() {
  sensors_event_t a, g, temp;

  float sumX = 0.0f;
  float sumY = 0.0f;
  const int samples = 30;

  Serial.println("=== CALIBRANDO ===");

  for (int i = 0; i < samples; i++) {
    mpu.getEvent(&a, &g, &temp);
    sumX += a.acceleration.x;
    sumY += a.acceleration.y;
    delay(20);
  }

  centerX = sumX / samples;
  centerY = sumY / samples;

  Serial.println("=== RESULTADO ===");
  Serial.print("centerX: ");
  Serial.println(centerX, 3);
  Serial.print("centerY: ");
  Serial.println(centerY, 3);
  Serial.println("=================");
}

void readInputs() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float relativeX = a.acceleration.x - centerX;
  float relativeY = a.acceleration.y - centerY;

  axisX = mapAxisToHID(relativeX, sensX, invertX);
  axisY = mapAxisToHID(relativeY, sensY, invertY);

  btnA = (digitalRead(PEDAL_1_PIN) == LOW);
  btnB = (digitalRead(PEDAL_2_PIN) == LOW);
  btnX = (digitalRead(PEDAL_3_PIN) == LOW);
}

void printState() {
  Serial.println("--- INPUT STATE ---");
  Serial.print("axisX: ");
  Serial.println(axisX);
  Serial.print("axisY: ");
  Serial.println(axisY);
  Serial.print("btnA: ");
  Serial.println(btnA ? 1 : 0);
  Serial.print("btnB: ");
  Serial.println(btnB ? 1 : 0);
  Serial.print("btnX: ");
  Serial.println(btnX ? 1 : 0);
  Serial.println("-------------------");
}

void printConfig() {
  Serial.println("=== CONFIG ===");
  Serial.print("centerX: ");
  Serial.println(centerX, 3);
  Serial.print("centerY: ");
  Serial.println(centerY, 3);
  Serial.print("deadzone: ");
  Serial.println(deadzone, 3);
  Serial.print("sensX: ");
  Serial.println(sensX, 3);
  Serial.print("sensY: ");
  Serial.println(sensY, 3);
  Serial.print("invertX: ");
  Serial.println(invertX ? 1 : 0);
  Serial.print("invertY: ");
  Serial.println(invertY ? 1 : 0);
  Serial.println("==============");
}

// =========================================================
// Modo boot
// =========================================================
void detectBootMode() {
  unsigned long start = millis();

  while (millis() - start < configHoldMs) {
    if (digitalRead(PEDAL_3_PIN) != LOW) {
      currentMode = MODE_GAME;
      return;
    }
    delay(10);
  }

  currentMode = MODE_CONFIG;
}

// =========================================================
// Gamepad
// =========================================================
void setupGamepad() {
  BleGamepadConfiguration config;

  config.setAutoReport(false);
  config.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  config.setButtonCount(3);
  config.setHatSwitchCount(0);

  config.setWhichAxes(true, true, false, false, false, false, false, false);

  config.setAxesMin(HID_MIN);
  config.setAxesMax(HID_MAX);

  config.setIncludeStart(false);
  config.setIncludeSelect(false);
  config.setIncludeMenu(false);
  config.setIncludeHome(false);
  config.setIncludeBack(false);
  config.setIncludeVolumeInc(false);
  config.setIncludeVolumeDec(false);
  config.setIncludeVolumeMute(false);

  bleGamepad.begin(&config);
}

void sendGamepadReport() {
  if (!bleGamepad.isConnected()) return;

  bleGamepad.setX(axisX);
  bleGamepad.setY(axisY);

  if (btnA) bleGamepad.press(BUTTON_1);
  else bleGamepad.release(BUTTON_1);

  if (btnB) bleGamepad.press(BUTTON_2);
  else bleGamepad.release(BUTTON_2);

  if (btnX) bleGamepad.press(BUTTON_3);
  else bleGamepad.release(BUTTON_3);

  bleGamepad.sendReport();
}

// =========================================================
// Parser de comandos de configuracao
// =========================================================
void printHelp() {
  Serial.println("=== COMANDOS ===");
  Serial.println("HELP");
  Serial.println("PRINT");
  Serial.println("CAL");
  Serial.println("SX=18.0");
  Serial.println("SY=18.0");
  Serial.println("DZ=0.80");
  Serial.println("IX=0 ou 1");
  Serial.println("IY=0 ou 1");
  Serial.println("SAVE");
  Serial.println("LOAD");
  Serial.println("REBOOT");
  Serial.println("GAME");
  Serial.println("================");
}

void applyConfigCommand(String rawCmd) {
  rawCmd.trim();
  if (rawCmd.length() == 0) return;

  String upperCmd = rawCmd;
  upperCmd.toUpperCase();

  if (upperCmd == "HELP") {
    printHelp();
    return;
  }

  if (upperCmd == "PRINT") {
    printConfig();
    return;
  }

  if (upperCmd == "CAL") {
    calibrateCenter();
    return;
  }

  if (upperCmd == "SAVE") {
    saveConfig();
    return;
  }

  if (upperCmd == "LOAD") {
    if (loadConfig()) {
      Serial.println("CONFIG CARREGADA.");
      printConfig();
    } else {
      Serial.println("Nenhuma config salva encontrada.");
    }
    return;
  }

  if (upperCmd == "REBOOT" || upperCmd == "GAME") {
    Serial.println("Reiniciando...");
    delay(150);
    ESP.restart();
    return;
  }

  if (upperCmd.startsWith("SX=")) {
    sensX = rawCmd.substring(3).toFloat();
    Serial.print("sensX = ");
    Serial.println(sensX, 3);
    return;
  }

  if (upperCmd.startsWith("SY=")) {
    sensY = rawCmd.substring(3).toFloat();
    Serial.print("sensY = ");
    Serial.println(sensY, 3);
    return;
  }

  if (upperCmd.startsWith("DZ=")) {
    deadzone = rawCmd.substring(3).toFloat();
    Serial.print("deadzone = ");
    Serial.println(deadzone, 3);
    return;
  }

  if (upperCmd.startsWith("IX=")) {
    invertX = (rawCmd.substring(3).toInt() != 0);
    Serial.print("invertX = ");
    Serial.println(invertX ? 1 : 0);
    return;
  }

  if (upperCmd.startsWith("IY=")) {
    invertY = (rawCmd.substring(3).toInt() != 0);
    Serial.print("invertY = ");
    Serial.println(invertY ? 1 : 0);
    return;
  }

  Serial.print("Comando desconhecido: ");
  Serial.println(rawCmd);
}

void handleConfigSerial() {
  while (Serial.available()) {
    char ch = (char)Serial.read();

    if (ch == '\n' || ch == '\r') {
      if (commandBuffer.length() > 0) {
        applyConfigCommand(commandBuffer);
        commandBuffer = "";
      }
    } else {
      commandBuffer += ch;
    }
  }
}

// =========================================================
// BLE HID API
// =========================================================
class ConfigServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    Serial.println("BLE CONFIG: cliente conectado.");
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    Serial.println("BLE CONFIG: cliente desconectado.");
    NimBLEDevice::startAdvertising();
  }
};

class ConfigWriteCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    std::string value = pCharacteristic->getValue();
    if (value.empty()) return;

    String cmd = String(value.c_str());
    cmd.trim();

    Serial.print("BLE CMD: ");
    Serial.println(cmd);

    applyConfigCommand(cmd);

    if (configNotifyChar != nullptr) {
      String ack = "OK:" + cmd;
      configNotifyChar->setValue(ack.c_str());
      configNotifyChar->notify();
    }
  }
};

void setupConfigBle() {
  NimBLEDevice::init(CONFIG_DEVICE_NAME);

  configServer = NimBLEDevice::createServer();
  configServer->setCallbacks(new ConfigServerCallbacks());

  configService = configServer->createService(CONFIG_SERVICE_UUID);

  configWriteChar = configService->createCharacteristic(
    CONFIG_WRITE_UUID,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );

  configNotifyChar = configService->createCharacteristic(
    CONFIG_NOTIFY_UUID,
    NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
  );

  configWriteChar->setCallbacks(new ConfigWriteCallbacks());
  configNotifyChar->setValue("READY");

  configService->start();

  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();

  NimBLEAdvertisementData advData;
  advData.setFlags(0x06);
  advData.addServiceUUID(CONFIG_SERVICE_UUID);

  NimBLEAdvertisementData scanResp;
  scanResp.setName(CONFIG_DEVICE_NAME);

  advertising->setAdvertisementData(advData);
  advertising->setScanResponseData(scanResp);
  advertising->start();

  Serial.println("BLE CONFIG pronto.");
  Serial.println("Dispositivo anunciando como 'Mellano Config'.");
}

// =========================================================
// Setup / Loop
// =========================================================
void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(PEDAL_1_PIN, INPUT_PULLUP);
  pinMode(PEDAL_2_PIN, INPUT_PULLUP);
  pinMode(PEDAL_3_PIN, INPUT_PULLUP);

  detectBootMode();

  Wire.begin(SDA_PIN, SCL_PIN);
  delay(300);

  if (!mpu.begin()) {
    Serial.println("ERRO: MPU6050 nao encontrado.");
    while (true) {
      delay(50);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 iniciado.");

  bool loaded = loadConfig();

  if (loaded) {
    Serial.println("CONFIG SALVA CARREGADA.");
    printConfig();
  } else {
    Serial.println("Nenhuma config salva. Fazendo calibracao inicial.");
    calibrateCenter();
  }

  if (currentMode == MODE_GAME) {
    setupGamepad();
    Serial.println("MODO: GAME");
    Serial.println("BLE gamepad pronto.");
    Serial.println("Pareie o dispositivo 'Mellano Proto' no celular.");
  } else {
    Serial.println("MODO: CONFIG");
    printHelp();
    setupConfigBle();
  }
}

void loop() {
  if (currentMode == MODE_GAME) {
    readInputs();
    sendGamepadReport();
  } else {
    if (millis() - lastPrint >= printInterval) {
      lastPrint = millis();
      readInputs();
      // printState();
    }
  }
}