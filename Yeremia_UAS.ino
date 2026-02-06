#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Preferences.h>

// ================= WIFI =================
const char* ssid     = "Simarmata2";
const char* password = "simata321";

// ================= MQTT =================
const char* mqtt_server = "5a34d4e722014e83a7a7398846c7de51.s1.eu.hivemq.cloud";
const int   mqtt_port   = 8883;
const char* mqtt_user   = "hivemq.webclient.1770340017196";
const char* mqtt_pass   = "Od.jBP2M>c0Ap1,f6;Wb";

// ================= TOPIC =================
const char* topic_pwm = "led/pwm";
const char* topic_status = "led/status";

// ================= PIN =================
#define LED_PIN 25
#define BTN_PIN 13

// ================= OBJECT =================
WiFiClientSecure espClient;
PubSubClient client(espClient);
Preferences prefs;

// ================= VAR =================
volatile bool buttonFlag = false;
int pwmValue = 0;

// ================= INTERRUPT =================
void IRAM_ATTR handleButton() {
  buttonFlag = true;
}

// ================= FUNGSI UPDATE PWM =================
void updatePWM(int newValue, const char* source) {
  pwmValue = constrain(newValue, 0, 255);
  ledcWrite(LED_PIN, pwmValue);
  prefs.putInt("pwm", pwmValue);
  
  Serial.print("PWM dari ");
  Serial.print(source);
  Serial.print(": ");
  Serial.println(pwmValue);
  
  // Publish status ke MQTT
  if (client.connected()) {
    char msg[10];
    itoa(pwmValue, msg, 10);
    client.publish(topic_status, msg);
  }
}

// ================= MQTT CALLBACK =================
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  updatePWM(msg.toInt(), "MQTT");
}

// ================= TASK MQTT =================
void taskMQTT(void *pvParameters) {
  for (;;) {
    if (!client.connected()) {
      Serial.println("MQTT reconnect...");
      if (client.connect("ESP32_FREERTOS", mqtt_user, mqtt_pass)) {
        Serial.println("MQTT terhubung!");
        client.subscribe(topic_pwm);
        
        // Publish nilai PWM saat ini setelah reconnect
        char msg[10];
        itoa(pwmValue, msg, 10);
        client.publish(topic_status, msg);
      } else {
        Serial.print("MQTT gagal, rc=");
        Serial.println(client.state());
      }
    }
    client.loop();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ================= TASK CONTROL =================
void taskControl(void *pvParameters) {
  for (;;) {
    if (buttonFlag) {
      buttonFlag = false;
      int newValue = (pwmValue == 0) ? 255 : 0;
      updatePWM(newValue, "TOMBOL");
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 PWM with Memory ===");
  
  // ===== Preferences =====
  prefs.begin("config", false);
  
  // ===== BACA NILAI TERAKHIR DARI MEMORI =====
  pwmValue = prefs.getInt("pwm", 0);
  Serial.print("PWM terakhir dari memori: ");
  Serial.println(pwmValue);
  
  // ===== SETUP PWM =====
  // Attach PWM ke pin dengan frekuensi 5000Hz dan resolusi 8-bit
  ledcAttach(LED_PIN, 5000, 8);
  
  // PENTING: Tunggu sebentar agar PWM siap
  delay(200);
  
  // ===== TERAPKAN PWM DARI MEMORI =====
  ledcWrite(LED_PIN, pwmValue);
  
  Serial.print("LED di-set ke PWM: ");
  Serial.println(pwmValue);
 
 
  
  // ===== Button Interrupt =====
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(BTN_PIN, handleButton, FALLING);
  
  // ===== WiFi =====
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // ===== MQTT =====
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  // ===== FreeRTOS TASK =====
  xTaskCreate(taskMQTT, "MQTT Task", 4096, NULL, 1, NULL);
  xTaskCreate(taskControl, "Control Task", 2048, NULL, 1, NULL);
  
  Serial.println("System ready!");
}

// ================= LOOP =================
void loop() {
  // kosong — semua ditangani FreeRTOS
}
