#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // Biblioteca para o LCD
#include <DHT.h>

// --- Configurações de Hardware ---
// Configuração do LCD (Endereço 0x27, 16 colunas, 2 linhas)
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Pinos dos Sensores
const int trigPin = 5;
const int echoPin = 18;
const int ldrPin = 34; // Pino analógico apenas entrada

// Pinos dos LEDs
const int ledRed = 4;    // Alerta (Temp alta ou Luz ruim)
const int ledGreen = 2;  // Ideal (Ambiente saudável)
const int ledBlue = 13;  // Standby (Usuário ausente)

// --- Configurações de Rede e MQTT ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com"; 
const char* mqtt_topic = "futuro_trabalho/workguard/dados";

WiFiClient espClient;
PubSubClient client(espClient);

// --- Variáveis Globais ---
long duration;
int distance;
unsigned long lastMsg = 0;
bool userPresent = false;

void setup_wifi() {
  delay(10);
  // Mensagem inicial no LCD
  lcd.setCursor(0,0);
  lcd.print("Conectando WiFi");
  
  Serial.println();
  Serial.print("Conectando em ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(0,1);
    lcd.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  lcd.clear();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect("ESP32WorkGuardClient")) {
      Serial.println("conectado");
      // Pisca o LED Azul para indicar conexão restabelecida
      digitalWrite(ledBlue, HIGH); delay(200); digitalWrite(ledBlue, LOW);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configura os Pinos
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ldrPin, INPUT);
  
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);

  // Inicializa Componentes
  dht.begin();
  lcd.init();      // Inicializa o LCD
  lcd.backlight(); // Liga a luz de fundo do LCD
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 1. Leitura de Sensores
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int lightLevel = analogRead(ldrPin); // ESP32: 0 (escuro) a 4095 (claro) - pode variar conforme fiação
  
  // Ultrassônico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // 2. Lógica de Decisão (WorkGuard AI)
  
  // Detectar Presença (< 50cm)
  userPresent = (distance < 50);

  // Detectar Problemas Ambientais
  // No Wokwi padrão, LDR baixo = escuro (se usar pull-down) ou alto = escuro (se usar pull-up).
  // Vamos assumir lógica padrão do módulo: < 500 é escuro demais para trabalhar.
  bool badLight = (lightLevel < 500); 
  bool badTemp = (temp > 30.0 || temp < 15.0);
  
  // Resetar LEDs
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledBlue, LOW);

  String lcdLine1 = "";
  String lcdLine2 = "";
  String statusMsg = "";

  if (!userPresent) {
    // --- MODO STANDBY (Economia) ---
    digitalWrite(ledBlue, HIGH); // Azul indica sistema rodando, mas sem usuário
    lcdLine1 = "Status: AUSENTE";
    lcdLine2 = "Modo Eco Ativo";
    statusMsg = "Ausente";
  } 
  else if (badTemp || badLight) {
    // --- MODO ALERTA (Risco à Saúde) ---
    digitalWrite(ledRed, HIGH);
    lcdLine1 = "ALERTA SAUDE!";
    statusMsg = "Risco:";
    
    if(badTemp) {
      lcdLine2 = "Temp Ruim: " + String((int)temp) + "C";
      statusMsg += " Temperatura";
    } else {
      lcdLine2 = "Luz Baixa!";
      statusMsg += " Iluminacao";
    }
  } 
  else {
    // --- MODO PRODUTIVO (Ideal) ---
    digitalWrite(ledGreen, HIGH);
    lcdLine1 = "Ambiente: BOM";
    lcdLine2 = "T:" + String((int)temp) + "C " + "LuzOK";
    statusMsg = "Produtivo";
  }

  // 3. Atualizar LCD (apenas se mudar para evitar flicker excessivo, ou a cada ciclo)
  // Como o loop é rápido, vamos atualizar sempre, o LCD I2C aguenta bem.
  lcd.setCursor(0, 0);
  lcd.print(lcdLine1 + "    "); // Espaços para limpar caracteres antigos
  lcd.setCursor(0, 1);
  lcd.print(lcdLine2 + "    ");

  // 4. Envio MQTT (A cada 3 segundos para não congestionar)
  unsigned long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;
    
    // JSON Payload
    String payload = "{";
    payload += "\"temperatura\":" + String(temp) + ",";
    payload += "\"usuario_presente\":" + String(userPresent ? "true" : "false") + ",";
    payload += "\"luminosidade\":" + String(lightLevel) + ",";
    payload += "\"status_trabalho\":\"" + statusMsg + "\"";
    payload += "}";

    Serial.print("Enviando MQTT: ");
    Serial.println(payload);
    client.publish(mqtt_topic, payload.c_str());
  }
  
  delay(200); // Pequeno delay para estabilidade
}