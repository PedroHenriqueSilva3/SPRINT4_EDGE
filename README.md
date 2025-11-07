# 🌐 Projeto IoT – Monitoramento de Dados Biométricos e Ambientais com ESP32

## 👥 Integrantes
- Gabriel C. S. da Silva RM: 563169
- João Gabriel C. M. Santos RM: 563953
- Leonardo V. de Souza RM: 562299
- Pedro H. da Silva RM: 566235

---

## 📘 Descrição do Projeto

Este projeto tem como objetivo o desenvolvimento de uma **Prova de Conceito (PoC)** de um sistema IoT capaz de **coletar, processar e enviar dados fisiológicos e de movimento** de um atleta em tempo real.

A implementação utiliza o **ESP32** no simulador **Wokwi**, equipado com sensores:
- **MPU6050** – aceleração, giroscópio e temperatura;
- **Sensor de pulso** (simulado via entrada analógica);
- **Sensor de oxigenação (SpO₂)** (simulado via entrada analógica).

Os dados coletados são enviados simultaneamente para:
1. **ThingSpeak (via HTTP GET)** – armazenamento e visualização em gráficos.
2. **Broker MQTT público (`test.mosquitto.org`)** – envio em tempo real para visualização via **aplicativo MQTT Dashboard**.

---


## ⚙️ Detalhes da Implementação

### 🧠 Tecnologias Utilizadas
- **Plataforma:** Wokwi (simulador ESP32)
- **Linguagem:** C++ (Arduino Framework)
- **Protocolo de comunicação:** HTTP e MQTT
- **Serviços utilizados:**
  - ThingSpeak (para armazenamento em nuvem)
  - test.mosquitto.org (broker MQTT público)
- **Aplicativo de visualização:** MQTT Dashboard (Android)

### 📡 Fluxo de Comunicação

```text
[MPU6050 + Sensores] 
      ↓
     ESP32
      ↓
 ┌───────────────┬─────────────────────┐
 │ ThingSpeak    │ Broker MQTT         │
 │ (HTTP GET)    │ (test.mosquitto.org)│
 └───────┬────────┴───────────┬────────┘
         ↓                    ↓
     Gráficos Web        MQTT Dashboard (App)


