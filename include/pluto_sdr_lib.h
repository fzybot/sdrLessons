#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>


// Инициализация
typedef struct sdr_config_s{
    char *name;
    int buffer_size;
    int tx_sample_rate;
    int tx_carrier_freq;
    float tx_gain;
    int rx_sample_rate;
    int rx_carrier_freq;
    float rx_gain;
    size_t channels[1] = {0} ;
} sdr_config_t;

struct SoapySDRDevice *setup_pluto_sdr(sdr_config_t *config);
struct SoapySDRStream *setup_stream(struct SoapySDRDevice *sdr, sdr_config_t *config, bool isRx);

void close_pluto_sdr(struct SoapySDRDevice *sdr, SoapySDRStream *rxStream, SoapySDRStream *txStream);



// Работа с буфферами
void fill_test_tx_buffer(int16_t *buffer, int size);

void start_rx_tx(  struct SoapySDRDevice *sdr, SoapySDRStream *rxStream, SoapySDRStream *txStream, 
                                int16_t *tx_buffer, int16_t *rx_buffer, int size, int num_iteration);


// Преобразование сэмплов из двух массивов (I[N], Q[N]) в вид Pluto (buff[N*2] = {I, Q, I, Q, ..., I, Q})
void transform_to_pluto_type_smples(std::vector<float> &I_part, std::vector<float> &Q_part, int16_t *buffer);
void transform_from_pluto_type_samples(std::vector<float> &I_part, std::vector<float> &Q_part, int16_t *buffer);
