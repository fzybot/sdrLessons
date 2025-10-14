#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>

typedef struct sdr_config_s{
    int buffer_size;
    int tx_sample_rate;
    int tx_carrier_freq;
    float tx_gain;
    int rx_sample_rate;
    int rx_carrier_freq;
    float rx_gain;
    size_t *channels;
} sdr_config_t;

struct SoapySDRDevice *setup_pluto_sdr(sdr_config_t *config);
struct SoapySDRStream *setup_stream(struct SoapySDRDevice *sdr, sdr_config_t *config, bool isRx);

void close_pluto_sdr(struct SoapySDRDevice *sdr, SoapySDRStream *rxStream, SoapySDRStream *txStream);

void fill_txZ_buffer(int16_t *buffer, int size);

void receive_and_send_samples(  struct SoapySDRDevice *sdr, SoapySDRStream *rxStream, SoapySDRStream *txStream, 
                                int16_t *tx_buffer, int16_t *rx_buffer, int size, int num_iteration);
