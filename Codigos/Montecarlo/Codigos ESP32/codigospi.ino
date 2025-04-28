#include <Arduino.h>
#include <SPI.h>
#include <BluetoothSerial.h>  // Biblioteca para el Bluetooth Serial

// Definición de pines
#define ATN_MA  25  // Entrada digital para seleccionar micro/miliamperios
#define MODE_AUTO 27 // Entrada digital para seleccionar modo automático/manual
#define DRDY    32  // Pin de datos listos del ADS1118
#define CS      5   // Chip Select del ADC (se asigna a GPIO5)
#define SCLK    18  // Reloj SPI (se asigna a GPIO18)
#define MISO    19  // Datos SPI desde ADC (se asigna a GPIO19)
#define MOSI    23  // Datos SPI hacia ADC (se asigna a GPIO23)

// Umbrales para cambio de rango en modo automático
#define UMBRAL_1  1  // Cambia a miliamperios si Vout > UMBRAL_1
#define UMBRAL_2  0.001  // Cambia a microamperios si Vout < UMBRAL_2

// Configuración del Bluetooth
BluetoothSerial SerialBT;

// Variables para almacenar la corriente y el modo
float corriente = 0.0;
bool modo_mA = true;  // Inicialmente en miliamperios

void setup() {
  // Inicializar comunicación serial
  Serial.begin(115200);
  SerialBT.begin("ESP32_Modo_Operativo");  // Nombre del dispositivo Bluetooth

  // Configurar pines
  pinMode(ATN_MA, INPUT);
  pinMode(MODE_AUTO, INPUT);
  pinMode(DRDY, INPUT);
  pinMode(CS, OUTPUT);
  
  digitalWrite(CS, HIGH);  // Deshabilitar SPI por defecto
  SPI.begin(SCLK, MISO, MOSI, CS); // Inicializar comunicación SPI con pines reasignados

  Serial.println("Esperando conexión Bluetooth...");
}

void loop() {
  if (digitalRead(DRDY) == LOW) { // Verificar si los datos están listos
    float vout = leerADC();  // Leer el valor del ADC
    bool autoMode = digitalRead(MODE_AUTO); // Leer estado del modo automático

    if (autoMode) {
      // Selección de rango en modo automático según los umbrales
      if (vout > UMBRAL_1) {
        modo_mA = true;
      } else if (vout < UMBRAL_2) {
        modo_mA = false;
      }
    } else {
      modo_mA = digitalRead(ATN_MA); // Leer selección manual
    }

    // Calcular corriente según el modo
    corriente = modo_mA ? (vout / (100 * 0.01)) : (vout / (100 * 10));
    
    // Enviar datos por Bluetooth
    enviarDatos(corriente, modo_mA, autoMode);
  }
}

// Función para leer datos desde el ADC mediante SPI (16 bits)
float leerADC() {
  digitalWrite(CS, LOW); // Seleccionar ADC
  delayMicroseconds(10);
  
  // Leer los 2 bytes de datos del ADC (16 bits)
  uint8_t msb = SPI.transfer(0x00);  // Lee el byte más significativo
  uint8_t lsb = SPI.transfer(0x00);  // Lee el byte menos significativo

  digitalWrite(CS, HIGH); // Liberar ADC

  // Combinar los 2 bytes en un valor de 16 bits
  int16_t raw = (msb << 8) | lsb;

  // Convertir el valor ADC a voltaje basado en referencia de 3.3V
  return raw * (3.3 / 32768.0);  // 32768.0 es el rango de un ADC de 16 bits (2^15)
}

// Función para enviar datos por Bluetooth
void enviarDatos(float corriente, bool modo_mA, bool autoMode) {
  String mensaje = "{";
  mensaje += "\"current\":" + String(corriente, 3) + ",";
  mensaje += "\"unit\":\"" + String(modo_mA ? "mA" : "uA") + "\",";
  mensaje += "\"mode\":\"" + String(autoMode ? "Automatic" : "Manual") + "\"}";
  
  // Enviar mensaje al dispositivo conectado por Bluetooth
  SerialBT.println(mensaje);
}