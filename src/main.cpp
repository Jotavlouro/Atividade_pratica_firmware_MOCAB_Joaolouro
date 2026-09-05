#include <Arduino.h>

const uint8_t GPIO_DAC = 25; // Pino de saida analogica
const uint8_t GPIO_ADC = 34; // Pino de entrada analogica
uint16_t valorDAC = 0;
const int NUM_AMOSTRAS = 10;

// Parametros
float Vmin = 3.3;
float Vmax = 0.0;
float somaTensao = 0.0;
float somaQuadrados = 0.0;
int contadorPontos = 0;

void setup() {
  // Iniciando comunicacao serial
  Serial.begin(115200);

  // Configuracoes do ADC no ESP32
  analogReadResolution(12); // Resolucao de 12 bits
  // Para não saturar no valor maximo é necessario realizar uma atenuacao do sinal
  // Atenuacao(dB) do sinal de tensão : 20 x log10 (V_entrada/(V_interna)) = 20 log10(3,3/1,1) = 20 log10 (3) = 9,55dB
  // Para ter uma margem segura, será utilizada uma atenuacao de 11dB
  analogSetPinAttenuation(GPIO_ADC, ADC_11db);
}

void loop() {
  // Gerando uma tensão
  dacWrite(GPIO_DAC, valorDAC);

  // Filtrando leituras
  long somaLeituras = 0;
  for (int i = 0; i < NUM_AMOSTRAS; i++) {
    somaLeituras += analogRead(GPIO_ADC);
    delayMicroseconds(100);
  }

  // Valor bruto do ADC
  int valorBruto = somaLeituras / NUM_AMOSTRAS;

  // Conversao do valor lido bruto para Volts
  float tensaoVolts = (valorBruto / 4095.0) * 3.3;

  // Acumulando dados para os parametros do sinal
  if (tensaoVolts < Vmin) Vmin = tensaoVolts;
  if (tensaoVolts > Vmax) Vmax = tensaoVolts;
  somaTensao += tensaoVolts;
  somaQuadrados += (tensaoVolts * tensaoVolts);
  contadorPontos++;

  Serial.print("DAC Enviado: ");
  Serial.print(valorDAC);
  Serial.print(" | ADC Bruto: ");
  Serial.print(valorBruto);
  Serial.print(" | Tensao: ");
  Serial.print(tensaoVolts, 2);
  Serial.println(" V");

  valorDAC += 25;

  // Garante que o ultimo valor testado antes de zerar seja exatamente 255
  if (valorDAC > 250 && valorDAC < 276) {
    valorDAC = 255;
  } else if (valorDAC >= 276) {
    valorDAC = 0;

    // Calculo e impressao do relatorio de parametros ao fim de cada ciclo
    float Vmedia = somaTensao / contadorPontos;
    float Vvpp = Vmax - Vmin;
    float Vrms = sqrt(somaQuadrados / contadorPontos);

    Serial.println("\n======= RESUMO DO CICLO =======");
    Serial.print("Tensao Minima (Vmin):     "); Serial.print(Vmin, 2); Serial.println(" V");
    Serial.print("Tensao Maxima (Vmax):     "); Serial.print(Vmax, 2); Serial.println(" V");
    Serial.print("Pico a Pico (Vpp):        "); Serial.print(Vvpp, 2); Serial.println(" V");
    Serial.print("Tensao Media (Vmed):      "); Serial.print(Vmedia, 2); Serial.println(" V");
    Serial.print("Tensao Eficaz (Vrms):     "); Serial.print(Vrms, 2); Serial.println(" V");
    Serial.println("================================\n");

    // Reseta os acumuladores para o proximo ciclo
    Vmin = 3.3;
    Vmax = 0.0;
    somaTensao = 0.0;
    somaQuadrados = 0.0;
    contadorPontos = 0;

    Serial.println(" Reiniciando Ciclo ...");
  }

  delay(300);
}