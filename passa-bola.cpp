#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <cmath>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

String apiKey = "5XLAZAE6ON3FKP04";
const char* server = "http://api.thingspeak.com/update";

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

Adafruit_MPU6050 mpu;
WiFiClient espClient;
PubSubClient client(espClient);

float x, y, z, vel, vel_max = 0, distancia = 0;
int pulsePin = 34;
int oxigenPin = 32;
int valor_ox, valor_bpm, bpm, ox;

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect("ESP32_Atleta_Wokwi")) {
      Serial.println("Conectado ao MQTT!");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup(void) {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  Serial.println("Monitor serial iniciado!");

  WiFi.begin(ssid, password);
  Serial.println("Conectando ao wifi!");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  if (!mpu.begin()){
    Serial.println("Falha ao encontrar o chip MPU6050");
    while (1){
      delay(10);
    }
  }

  Serial.println("MPU6050 Encontrado");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.println("Acelerometro range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.println("Gyro range set to: ");
  switch(mpu.getGyroRange()){
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG: 
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  client.setServer(mqtt_server, mqtt_port);
  Serial.println("MQTT configurado.");

  Serial.println("");
  delay(100);
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    bpm = analogRead(pulsePin);
    ox = analogRead(oxigenPin);
    valor_bpm = map(bpm, 0, 4095, 40, 240);
    Serial.print("Valor do pulso: ");
    Serial.println(valor_bpm);
    if (valor_bpm > 200){
      Serial.println("ATENÇÃO BPM MUITO ELEVADO!!");
    }
    valor_ox = map(ox, 0, 4095, 80, 100);
    Serial.print("Valor da oxigenação: ");
    Serial.println(valor_ox);
    if(valor_ox < 90){
      Serial.println("ATENÇÃO OXIGENAÇÃO BAIXA!!!");
    }
    delay(100);

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    x = pow(a.acceleration.x,2);
    y = pow(a.acceleration.y,2);
    z = pow(a.acceleration.z,2);
    vel = sqrt(x+y+z) * 3.6;
    Serial.print("Velocidade media de: ");
    Serial.print(vel);
    if (vel > vel_max) {
      vel_max = vel;
    }
    Serial.println("KM/H");
    Serial.print("Maior velocidade atingida: ");
    Serial.print(vel_max);
    Serial.println("KM/H");

    distancia = distancia + (vel / 3.6 * 13);
    Serial.print("Distancia percorrida em metros: ");
    Serial.print(distancia);
    Serial.println("M");

    Serial.print("Temperatura da atleta: ");
    Serial.print(temp.temperature);
    Serial.println("°C");

    Serial.println("");
    delay(100);

    String url = String(server) + "?api_key=" + apiKey +
                    "&field1=" + valor_bpm +  // BPM simulado
                    "&field2=" + valor_ox + 
                    "&field3=" + vel +
                    "&field4=" + distancia +
                    "&field5=" + temp.temperature +
                    "&field6=" + vel_max;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Resposta ThingSpeak: " + payload);
    } else {
      Serial.println("Eroo http");
    }
    http.end();

    if (!client.connected()) reconnectMQTT();
    client.loop();

    client.publish("wokwi/esp32/bpm", String(valor_bpm).c_str());
    client.publish("wokwi/esp32/oxigenacao", String(valor_ox).c_str());
    client.publish("wokwi/esp32/velocidade", String(vel).c_str());
    client.publish("wokwi/esp32/distancia", String(distancia).c_str());
    client.publish("wokwi/esp32/temperatura", String(temp.temperature).c_str());
    client.publish("wokwi/esp32/velocidade_max", String(vel_max).c_str());
    Serial.println("Enviado");
  }
  delay(15000);

    
}
