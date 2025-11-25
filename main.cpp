#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>
#include <unistd.h>
#include <vector>

#include "matplotlibcpp.h"

#include "pluto_sdr_lib.h"

namespace plt = matplotlibcpp;

int main(void)
{
    // Наши параметры SDR в одной структуре.
    sdr_config_t config;
    config.buffer_size = 1920;
    config.rx_carrier_freq = 800e6;
    config.tx_carrier_freq = 800e6;
    config.rx_sample_rate = 1e6;
    config.tx_sample_rate = 1e6;
    config.rx_gain = 30.0;
    config.tx_gain = -40.0;

    // size_t channels[] = {0} ; // {0} or {0, 1}
    // config.channels = &channels;

    struct SoapySDRDevice *sdr = setup_pluto_sdr(&config);
    struct SoapySDRStream *rxStream = setup_stream(sdr, &config, 1);
    struct SoapySDRStream *txStream = setup_stream(sdr, &config, 0);

    int16_t tx_buffer[2*config.buffer_size];
    int16_t rx_buffer[2*config.buffer_size];

    // TODO: здесь нужно отправлять и принимать сэмплы.
    fill_test_tx_buffer(tx_buffer, config.buffer_size);

    receive_and_send_samples(sdr, rxStream, txStream,
                             tx_buffer, rx_buffer, config.buffer_size,
                             10);

    // plt::plot({1,3,2,4});
    // plt::show();
    
    close_pluto_sdr(sdr, rxStream, txStream);


    return EXIT_SUCCESS;
}
