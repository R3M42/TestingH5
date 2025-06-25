#include <ArduinoUnit.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

// --- Sensor pH ---
float calibration_value = 21.34;
int buffer_arr[10], temp;
unsigned long int avgval;

// --- Sensor Temperatura ---
OneWire ourWire(4);
DallasTemperature DS18B20(&ourWire);

// --- Sensor Turbidez ---
const int turbidezPin = 35;

// --- Sensor de Flujo ---
const int flujoPin = 23;
volatile int pulsos = 0;
unsigned long tiempoFlujo = 0;

// --- Sensor de Nivel ---
const int nivelPin = 33;

// --- WiFi & MQTT ---
const char* ssid = "TIGO_EXCLUSIVE";
const char* password = "motivate";
const char* mqtt_server = "192.168.0.149";
const char* serverName = "http://127.0.0.1:5000/clasificar";

WiFiClient espClient;
PubSubClient client(espClient);

void IRAM_ATTR contarPulsos() {
  pulsos++;
}

void setup_wifi() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado! IP: " + WiFi.localIP().toString());
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("✅ Conectado a MQTT");
    } else {
      Serial.print("❌ Falló, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  DS18B20.begin();

  pinMode(nivelPin, INPUT);
  pinMode(flujoPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flujoPin), contarPulsos, RISING);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // --- Leer pH ---
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(35);
    delay(30);
  }
  for (int i = 0; i < 9; i++)
    for (int j = i + 1; j < 10; j++)
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
  avgval = 0;
  for (int i = 2; i < 8; i++) avgval += buffer_arr[i];
  float volt = (float)avgval * 3.3 / 4095 / 6;
  float ph_act = -5.70 * volt + calibration_value;

  // --- Temperatura ---
  DS18B20.requestTemperatures();
  float tem = DS18B20.getTempCByIndex(0);

  // --- Turbidez ---
  int turbidezRaw = analogRead(turbidezPin);
  float turbidezNTU = turbidezRaw / 1000.0;

  // --- Flujo ---
  float caudalLmin = pulsos / 7.5;
  pulsos = 0;

  // --- Nivel de agua ---
  bool hayAgua = digitalRead(nivelPin);

  // --- Serial Monitor ---
  Serial.println("------------- Datos de Sensores -------------");
  Serial.printf("📌 pH: %.2f\n", ph_act);
  Serial.printf("🌡️ Temperatura: %.2f °C\n", tem);
  Serial.printf("🌊 Turbidez: %.2f NTU\n", turbidezNTU);
  Serial.printf("🚰 Flujo: %.2f L/min\n", caudalLmin);
  Serial.printf("🧪 Nivel: %s\n", hayAgua ? "Con agua" : "Sin agua");

  // --- MQTT JSON ---
  String payloadMQTT = String("{\"ph\":") + ph_act +
                        ",\"temperatura\":" + tem +
                        ",\"turbidez\":" + turbidezNTU +
                        ",\"flujo\":" + caudalLmin +
                        ",\"nivel\":" + (hayAgua ? "true" : "false") + "}";

  client.publish("sensores/datos", payloadMQTT.c_str());

  // --- HTTP POST a Flask API ---
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(payloadMQTT);
    if (code > 0) {
      Serial.print("✅ Flask API: "); Serial.println(http.getString());
    } else {
      Serial.print("❌ Error POST: "); Serial.println(code);
    }
    http.end();
  }

  delay(5000); // espera 5 segundos
}
