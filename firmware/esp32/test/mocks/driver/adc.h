#pragma once
// Mock driver/adc.h for native unit testing

typedef enum { ADC_0db, ADC_2_5db, ADC_6db, ADC_11db } adc_atten_t;

inline void analogSetPinAttenuation(int /*pin*/, adc_atten_t /*atten*/) {}
inline void analogSetAttenuation(adc_atten_t /*atten*/) {}
