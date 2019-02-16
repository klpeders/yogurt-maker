/*
 * This file is part of the W1209 firmware replacement project
 * (https://github.com/mister-grumbler/w1209-firmware).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Control functions for analog-to-digital converter (ADC).
 * The ADC1 interrupt (22) is used to get signal on end of convertion event.
 * The port D6 (pin 3) is used as analog input (AIN6).
 */

#include "stm8s003/adc.h"
#include "adc.h"
#include "params.h"

// Filter timeconstant. TIMECONSTANT = log2(1/tc) in
//     z' = (1-tc)*z + tc*s  =  z + tc(s-z)
#define ADC_FILTER_TIMECONSTANT      4

/* The lookup table contains raw ADC and temperature values
 * between 125C and -25C
 * B = 3125, T0=25C, Rntc=10K, Rs=20K
 */
#define RAWTEMP_PWLSEGGROUPS \
    PWLGROUP(   0,  0,  142,  0, 15, 125.038),\
    PWLGROUP( 142,  0,  192,  6,  5, 125.038),\
    PWLGROUP( 334,  6,  320,  5,  6, 83.864),\
    PWLGROUP( 654, 11,  896,  7,  7, 55.312),\
    PWLGROUP(1550, 18, 2048,  8,  8, 19.492),\
    PWLGROUP(3598, 26,  498,  0, 15, -35.580),\
    /* END */
#define RAWTEMP_PWLSEGS \
    PWLSEGM(142, 125.038),	/*   0: 32, (12.7 bits) */\
    PWLSEGM(174, 114.596),	/*   1: 32, (12.4 bits) */\
    PWLSEGM(206, 106.263),	/*   2: 32, (12.1 bits) */\
    PWLSEGM(238, 99.358),	/*   3: 32, (11.9 bits) */\
    PWLSEGM(270, 93.481),	/*   4: 32, (11.7 bits) */\
    PWLSEGM(302, 88.374),	/*   5: 32, (11.5 bits) */\
    PWLSEGM(334, 83.864),	/*   6: 64, (13.3 bits) */\
    PWLSEGM(398, 76.183),	/*   7: 64, (13.0 bits) */\
    PWLSEGM(462, 69.797),	/*   8: 64, (12.8 bits) */\
    PWLSEGM(526, 64.334),	/*   9: 64, (12.6 bits) */\
    PWLSEGM(590, 59.558),	/*  10: 64, (12.4 bits) */\
    PWLSEGM(654, 55.312),	/*  11: 128, (14.2 bits) */\
    PWLSEGM(782, 47.999),	/*  12: 128, (14.0 bits) */\
    PWLSEGM(910, 41.817),	/*  13: 128, (13.8 bits) */\
    PWLSEGM(1038, 36.430),	/*  14: 128, (13.6 bits) */\
    PWLSEGM(1166, 31.628),	/*  15: 128, (13.4 bits) */\
    PWLSEGM(1294, 27.267),	/*  16: 128, (13.3 bits) */\
    PWLSEGM(1422, 23.246),	/*  17: 128, (13.2 bits) */\
    PWLSEGM(1550, 19.492),	/*  18: 256, (15.1 bits) */\
    PWLSEGM(1806, 12.566),	/*  19: 256, (15.0 bits) */\
    PWLSEGM(2062, 6.152),	/*  20: 256, (14.9 bits) */\
    PWLSEGM(2318, 0.009),	/*  21: 256, (14.9 bits) */\
    PWLSEGM(2574, -6.068),	/*  22: 256, (15.0 bits) */\
    PWLSEGM(2830, -12.291),	/*  23: 256, (15.1 bits) */\
    PWLSEGM(3086, -18.927),	/*  24: 256, (15.2 bits) */\
    PWLSEGM(3342, -26.401),	/*  25: 256, (15.5 bits) */\
    PWLSEGM(3598, -35.580),	/*  26: END */
#define RAWTEMP_TABLEBITS     12 // Table optimized for 12 bit input
#define RAWTEMP_SCALE         20 // 1/2*C (centigrade, rounding *2)
#define RAWTEMP_COUNT_MAX   3598 // Tmax=-35.580
#define RAWTEMP_COUNT_MIN    142 // Tmin=125.038
#define RAWTEMP_T_MAX    125.038
#define RAWTEMP_T_MIN    -35.580


#define PWLSEGM(adccount,temp) ((int16_t)(RAWTEMP_SCALE*temp))
#define PWLGROUP(adccount, index, size, entries, logsize, temp) {size, index, logsize}

static const struct {
    uint16_t size;
    uint8_t  index;
    uint8_t  log2_segsize;
} pwl_seg[] = {
    RAWTEMP_PWLSEGGROUPS
};

static const int16_t pwl_interp[] = {
    RAWTEMP_PWLSEGS
};

static uint16_t filtered;


static int16_t getTemp(uint16_t adccount)
{
    uint8_t offset;
    uint8_t i = 0, index;
    int16_t a, b, temperature;

    if (adccount >= RAWTEMP_COUNT_MAX) adccount = RAWTEMP_COUNT_MAX;

    while (adccount >= pwl_seg[i].size) {
        adccount -= pwl_seg[i].size;
        i++;
    }
    index = pwl_seg[i].index + (adccount >> pwl_seg[i].log2_segsize);
    offset = adccount & ((1 << pwl_seg[i].log2_segsize) - 1);

    a = pwl_interp[index];
    b = pwl_interp[index+1];
    temperature = a - (((uint16_t)(a - b)*offset) >> pwl_seg[i].log2_segsize);

    // Round:
    temperature = (temperature + 1) >> 1;

    return temperature;
}


/**
 * @brief Initialize ADC's configuration registers.
 */
void initADC()
{
    ADC_CR1 |= 0x70;    // Prescaler f/18 (SPSEL)
    ADC_CSR |= 0x06;    // select AIN6
    ADC_CSR |= 0x20;    // Interrupt enable (EOCIE)
    ADC_CR1 |= 0x01;    // Power up ADC
    filtered = 0;
}

/**
 * @brief Sets bit in ADC control register to start data convertion.
 */
void startADC()
{
    ADC_CR1 |= 0x01;
}

/**
 * @brief Gets filtered ADC value
 * @return Filtered result.
 */
uint16_t getAdcFiltered()
{
    return filtered;
}

/**
 * @brief Calculation of real temperature using averaged result of
 *  AnalogToDigital conversion and the lookup table.
 * @return temperature in tenth of degrees of Celsius.
 */
int getTemperature()
{
    // Need 12 bits
    uint16_t val = getAdcFiltered() >> (16-RAWTEMP_TABLEBITS);

    // Final calculation and correction
    return getTemp(val) + getParamById (PARAM_TEMPERATURE_CORRECTION);
}

/**
 * @brief This function is ADC's interrupt request handler
 */
void ADC1_EOC_handler() __interrupt (22)
{
    static bool init = false; // init once
    uint16_t h = ADC_DRH;
    uint8_t  l = ADC_DRL;

    // Create 16 bit result
    uint16_t adc_v = ((h << 2) | l) << 6;

    ADC_CSR &= ~0x80;   // reset EOC

    // init if needed
    if (!init) filtered = adc_v, init = true;

//  Calculate filter:
//     z' = (1-tc)*z + tc*s  =  z + tc(s-z)
//  Using unsigned integer arith
//     z' = z - tc*z + tc*s
//  Restricting timeconstant, TC to power of 2, tc' = log2(1/tc)
//     z' = z - z >> tc' + s >> tc'
    filtered = filtered - (filtered >> ADC_FILTER_TIMECONSTANT) + (adc_v >> ADC_FILTER_TIMECONSTANT);
}
