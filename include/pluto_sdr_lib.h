#pragma ones

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>
#include <vector>

#define BUFFER_SIZE 1920 * 10

typedef struct sdr_config_s{
    char *name; // usb or ip adress
    int buffer_size;
    double tx_sample_rate;
    double tx_carrier_freq;
    float tx_gain;

    double rx_sample_rate;
    double rx_carrier_freq;
    float rx_gain;
    size_t channels[1] = {0} ;
} sdr_config_t;

typedef struct sdr_phy_s{
    long long rx_timeNs;
    int16_t pluto_rx_buffer[2 * BUFFER_SIZE];
    int16_t pluto_tx_buffer[2 * BUFFER_SIZE];
    std::vector< std::complex<float> > raw_samples;
} sdr_phy_t;

// Инициализация
typedef struct sdr_global_s{
    bool running;
    sdr_config_t sdr_config;
    sdr_phy_t phy;
    SoapySDRDevice *sdr;
    SoapySDRStream *rxStream;
    SoapySDRStream *txStream;
} sdr_global_t;



struct SoapySDRDevice *setup_pluto_sdr(sdr_config_t *config);
struct SoapySDRStream *setup_stream(struct SoapySDRDevice *sdr, sdr_config_t *config, bool isRx);

void close_pluto_sdr(sdr_global_t *sdr);

// Работа с буфферами
void fill_test_tx_buffer(int16_t *buffer, int size);

// Преобразование сэмплов из двух массивов (I[N], Q[N]) в вид Pluto (buff[N*2] = {I, Q, I, Q, ..., I, Q})
void transform_to_pluto_type_smples(std::vector<float> &I_part, std::vector<float> &Q_part, int16_t *buffer);
void transform_from_pluto_type_samples(std::vector<float> &I_part, std::vector<float> &Q_part, int16_t *buffer);

void run_sdr(sdr_global_t *sdr);

