// For at teste dette, klik på mappen med en cirkel og prik foran i venstre hjørne. Ændre det til test-hastighed. Så render dette i stedet for.

#include "driver/mcpwm.h"
#include "driver/adc.h"
#include "Arduino.h"
// Register
#include "soc/sens_reg.h"
#include "soc/sens_struct.h"

#define PWM_GPIO 10
#define PIN_ADC1_A 1  
#define SAMPLES 500    

void setup() {
    Serial.begin(115200);
    delay(2000);

    // MCPWM Setup
    // MCPWM_UNIT_0 (1 PWM modul ud af 2)
    // MCPWM0A er en PWM udgang. MCPWM0B bruger samme puls, og kan da laves negativ hvis vi vil. Der ekssistere også MCPWM1A og MCPWM1B
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PWM_GPIO);
    mcpwm_config_t pwm_config = {
        .frequency = 100000,
        // Tændt 50% af tiden
        .cmpr_a = 50.0,
        // Tændt 0% af tiden på samme tid
        .cmpr_b = 0,
        .duty_mode = MCPWM_DUTY_MODE_0,
        // MCPWM_UP_DOWN_COUNTER -> I stedet for at gå op fra 0 til 100, vil den eks. gå fra 0 til 100 DOBBELT SÅ HURTIGT, og derefter tilbage til 0. 
        // Dette er nemmere at time midten af et signal, hvis man forestiller sig at PWM signalet er ON fra 90-100, og forbliver ON fra 100-90 igen, og OFF resten af tiden.
        .counter_mode = MCPWM_UP_DOWN_COUNTER
    };
    // Forbinder pin, frekvens timer og konfigurationen over
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    // ADC Setup
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
    
    // Gør ADC klar til at modtage input fra en kanal.
    // Registeret vi senere manipulere aktiveres her.
    adc1_get_raw(ADC1_CHANNEL_0); 

    Serial.println("\n--- REGISTER BENCHMARK ---");
}

void loop() {
    float total_registerADC = 0;
    float total_analogRead = 0;

    delay(400);
    Serial.println("Vi måler nu ADC hastigheden på en ESP32 - LILYGO T-Display S3");
    Serial.println("-------------------------------------");

    for (int i = 0; i < SAMPLES; i++) {
        // Test 1: Cycles tæller der måler analogRead hastighed
        uint32_t s1 = ESP.getCycleCount();
        analogRead(PIN_ADC1_A);
        total_analogRead += (ESP.getCycleCount() - s1);

        // Test 2: Register
        uint32_t s2 = ESP.getCycleCount();
        
        // 1. Start måling (SENS_SAR_MEAS1_CTRL2_REG)
        // Denne kommando bruges til at lave en bit til 1.
        // SENS_SAR_MEAS1_CTRL2_REG er adressen på registeret, alle 32 bits
        // SENS_MEAS1_START_SAR er den individuelle bit til at starte måling (Start SAR ADC)
        SET_PERI_REG_MASK(SENS_SAR_MEAS1_CTRL2_REG, SENS_MEAS1_START_SAR);
        
        // 2. Vent på done (SENS_MEAS1_DONE_SAR maske)
        // GET_PERI_REG_MASK tjekker, om noget er 0 eller 1
        // Den tjekker så det samme register som ovenover (SENS_SAR_MEAS_CTRL2_REG)
        // Den kigger på om en ny bit flipper til 1, altså at vores SAR ADC er done (SENS_MEAS1_DONE_SAR)
        while (!(GET_PERI_REG_MASK(SENS_SAR_MEAS1_CTRL2_REG, SENS_MEAS1_DONE_SAR)));
        
        // 3. Hent værdi (GET_PERI_REG_BITS2)
        // Vi kigger STADIG på det samme register (SENS_SAR_MEAS1_CTRL2_REG)
        // SENS_MEAS1_DATA_SAR_M -> M for Mask. Den siger hvor mange bits vi vil have (12 for ESP32 ADC)
        // SENS_MEAS1_DATA_SAR_S -> S for Shift. Hvor i de 32-bit de 12-bit rækken starter
        volatile int res = GET_PERI_REG_BITS2(SENS_SAR_MEAS1_CTRL2_REG, SENS_MEAS1_DATA_SAR_M, SENS_MEAS1_DATA_SAR_S);
        
        total_registerADC += (ESP.getCycleCount() - s2);
        
        delay(5);
    }

    // Cycles til mikrosekunder
    float avgAR = (total_analogRead / SAMPLES) / 240.0;
    float avgReg = (total_registerADC / SAMPLES) / 240.0;

    // %7.2f = float med 7 characters og 2 digits
    Serial.printf("AnalogRead: \t\t\t %7.2f µs\n", avgAR);
    Serial.printf("AnalogRead Cyklusser: \t\t %7.2f cycles\n", total_analogRead / SAMPLES);
    Serial.printf("Register Read: \t\t\t %7.2f µs\n", avgReg);
    Serial.printf("Register Read Cyklusser: \t %7.2f cycles\n", total_registerADC / SAMPLES);
    Serial.println("-------------------------------------");

    delay(5000);
}
