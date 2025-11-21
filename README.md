# WorkGuard

## 📖 Descrição do Problema

Com o avanço do trabalho remoto e híbrido, profissionais enfrentam novos desafios invisíveis:

* **Saúde:** Ambientes inadequados (má iluminação, temperatura desconfortável) causam fadiga e problemas ergonômicos.
* **Gestão:** Dificuldade em monitorar a disponibilidade e o bem-estar real dos colaboradores sem invasão de privacidade.
* **Desperdício:** Equipamentos e luzes ligados em estações de trabalho vazias.

## 💡 A Solução

O **WorkGuard** é um dispositivo de monitoramento ambiental e de presença. Ele atua como um "assistente de saúde", validando se o ambiente está propício para a produtividade e reportando dados para um dashboard na nuvem.

### Funcionalidades Principais:

1. **Detecção de Presença:** Identifica se o posto de trabalho está ocupado (Sensor Ultrassônico).
2. **Qualidade Ambiental:** Monitora Temperatura (DHT22) e Luminosidade (LDR).
3. **Feedback Visual Local:**
   * 🔵 **LED Azul:** Standby/Ausente (Economia).
   * 🟢 **LED Verde:** Ambiente Produtivo e Saudável.
   * 🔴 **LED Vermelho:** Alerta de Risco (Calor excessivo ou falta de luz).
4. **Dashboard Remoto:** Visualização de dados em tempo real via Node-RED.

---

## 💻 Instruções de Uso e Reprodução

### 1. Simulação no Wokwi

O código fonte completo (`sketch.ino` e `diagram.json`) está disponível para execução direta no navegador.

👉 **[https://wokwi.com/projects/448089858928017409]**  

### 2. Dashboard no Node-RED

Para visualizar os dados:

1. Instale o **Node-RED**.
2. Instale a paleta de Dashboard:  
   `Menu -> Manage Palette -> Install -> node-red-dashboard`
3. Importe o fluxo (**flow.json**) disponibilizado neste repositório.
4. Acesse **http://localhost:1880/ui** para ver o painel.

---

## 📚 Dependências (Bibliotecas Arduino)

Para compilar este projeto (no Wokwi ou na IDE Arduino), instale:

- `WiFi.h` (Nativa do ESP32)  
- `PubSubClient` (Nick O'Leary) — Comunicação MQTT  
- `DHT sensor library` (Adafruit) — Sensor DHT22  
- `LiquidCrystal I2C` — Display LCD  
- `Wire.h` (Nativa) — Comunicação I2C  

---

### Circuito no Wokwi

<img src="https://github.com/user-attachments/assets/1ef21a8a-0b88-4600-8d6c-ef99a6f5b6c4" alt="Circuito no Wokwi" width="400px">

### Dashboard Node-RED

<img src="https://github.com/user-attachments/assets/615840bf-c09d-47e9-9f72-d63d0f09ee15" alt="Dashboard Node-RED" width="400px">

---

## 📡 Comunicação e MQTT

O sistema utiliza o protocolo MQTT para enviar dados leves e rápidos para a nuvem.

* **Broker Público:** `broker.hivemq.com`
* **Porta:** `1883`
* **Tópico de Publicação:** `futuro_trabalho/workguard/dados`

### Estrutura do Payload (JSON)

A cada 3 segundos, o ESP32 envia uma mensagem neste formato:

```json
{
  "temperatura": 24.50,
  "usuario_presente": true,
  "luminosidade": 2048,
  "status_trabalho": "Produtivo"
}
