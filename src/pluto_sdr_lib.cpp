#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>
#include <vector>
#include <iostream>
#include <cstdint>
#include <numeric>


#include "bit_generation.h"
#include "pulse_shaping.h"
#include "modulation.h"
#include "math_operations.h"
#include "pluto_sdr_lib.h"

struct SoapySDRDevice *setup_pluto_sdr(sdr_config_t *config)
{
    // Создание сущности устройства
    // args can be user defined or from the enumeration result
    SoapySDRKwargs args = {};
    char buffer_size[10]; // Allocate enough space
    sprintf(buffer_size, "%d", config->buffer_size);
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    if (1) {
        SoapySDRKwargs_set(&args, "uri", config->name);
    } else {
        SoapySDRKwargs_set(&args, "uri", "ip:192.168.2.1");
    }
    SoapySDRKwargs_set(&args, "direct", "1");
    SoapySDRKwargs_set(&args, "timestamp_every", buffer_size);
    SoapySDRKwargs_set(&args, "loopback", "0");
    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL)
    {
        printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
        return NULL;
    }

    // Настраиваем устройства RX\TX
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, config->rx_sample_rate) != 0)
    {
        printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, config->rx_carrier_freq, NULL) != 0)
    {
        printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, config->tx_sample_rate) != 0)
    {
        printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, config->tx_carrier_freq, NULL) != 0)
    {
        printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());
    }
    printf("SoapySDRDevice_getFrequency tx: %lf\n", SoapySDRDevice_getFrequency(sdr, SOAPY_SDR_TX, 0));

    return sdr;
}

struct SoapySDRStream *setup_stream(struct SoapySDRDevice *sdr, sdr_config_t *config, bool isRx)
{
    if(sdr){
        size_t channel_count = sizeof(config->channels) / sizeof(config->channels[0]);
        SoapySDRStream *stream;

        if(isRx){
            stream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, config->channels, channel_count, NULL);
            if (stream == NULL)
            {
                printf("setupStream rx fail: %s\n", SoapySDRDevice_lastError());
                SoapySDRDevice_unmake(sdr);
                return NULL;
            }
            if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, *config->channels, config->rx_gain) !=0 ){
                printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
            }
            
            // Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
            size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, stream);
            printf("MTU - RX: %lu\n",rx_mtu);

            //activate streams
            SoapySDRDevice_activateStream(sdr, stream, 0, 0, 0); //start streaming
            
            printf("Streams are activated\n");
        } else {
            stream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, config->channels, channel_count, NULL);
            if (stream == NULL)
            {
                printf("setupStream tx fail: %s\n", SoapySDRDevice_lastError());
                SoapySDRDevice_unmake(sdr);
                return NULL;
            }
            if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, *config->channels, config->tx_gain) !=0 ){
                printf("setGain tx fail: %s\n", SoapySDRDevice_lastError());
            }
            size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, stream);
            printf("MTU - TX: %lu\n", tx_mtu);

            SoapySDRDevice_activateStream(sdr, stream, 0, 0, 0); //start streaming
        }
        return stream;
    } else {
        return NULL;
    }
}

void fill_test_tx_buffer(int16_t *buffer, int size)
{
    int Nsps = 10;
    std::vector<int> bit_array = generate_bit_array(size/Nsps*2);
    std::vector<std::complex<double>> modulated_array = modulate(bit_array, 2);
    std::vector<std::complex<double>> upsampled_bit_array = upsample(modulated_array, Nsps);
    std::vector<std::complex<double>> pulse_shaped = pulse_shaping(upsampled_bit_array, 0, Nsps);
    std::cout << "bit_array size = " << bit_array.size() << std::endl;
    std::cout << "modulated_array size = " << modulated_array.size() << std::endl;
    std::cout << "upsampled_bit_array size = " << upsampled_bit_array.size() << std::endl;
    std::cout << "pulse_shaped size = " << pulse_shaped.size() << std::endl;
    // заполнение tx_buff значениями сэмплов первые 16 бит - I, вторые 16 бит - Q.
    // for (int i = 0; i < pulse_shaped.size(); i++)
    // {
    //     std::cout << pulse_shaped[i] << " ";
    // }
    // std::cout << std::endl;
    for (int i = 0; i < 2 * size; i+=2)
    {
        buffer[i] = int(pulse_shaped[i].real() * 2000) << 4;
        buffer[i + 1] = int(pulse_shaped[i].imag() * 2000) << 4;
        //std::cout << buffer[i] << " " << buffer[i+1] << " ";
    }
    std::cout << std::endl;

    //prepare fixed bytes in transmit buffer
    //we transmit a pattern of FFFF FFFF [TS_0]00 [TS_1]00 [TS_2]00 [TS_3]00 [TS_4]00 [TS_5]00 [TS_6]00 [TS_7]00 FFFF FFFF
    //that is a flag (FFFF FFFF) followed by the 64 bit timestamp, split into 8 bytes and packed into the lsb of each of the DAC words.
    //DAC samples are left aligned 12-bits, so each byte is left shifted into place
    for(size_t i = 0; i < 2; i++)
    {
        buffer[0 + i] = 0xffff;
        // 8 x timestamp words
        buffer[10 + i] = 0xffff;
    }
}

void run_sdr(sdr_global_t *sdr)
{
    if(sdr->sdr){
        const long  timeoutUs = 400000; // arbitrarily chosen (взяли из srsRAN)
        long long last_time = 0;

        //FILE *file = fopen("txdata.pcm", "w");

        // Начинается работа с получением и отправкой сэмплов
        int buffers_read = 0;
        while (sdr->running)
        {
            void *rx_buffs[] = {sdr->phy.pluto_rx_buffer};
            int flags;        // flags set by receive operation
            long long timeNs; //timestamp for receive buffer

            int sr = SoapySDRDevice_readStream(sdr->sdr, sdr->rxStream, rx_buffs, sdr->sdr_config.buffer_size, &flags, &timeNs, timeoutUs);
            if (sr < 0)
            {
                printf("ERROR. SoapySDRDevice_readStream.\n");
                continue;
            }
            
            // Dump info
            //printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", 0, sr, flags, timeNs, timeNs - last_time);
            for (int i = 0; i < BUFFER_SIZE; i++){
                sdr->phy.raw_samples[i] = std::complex<double>(sdr->phy.pluto_rx_buffer[i * 2], sdr->phy.pluto_rx_buffer[i * 2 + 1] );
            }
            sdr->phy.matched_samples =  pulse_shaping(sdr->phy.raw_samples, 0, sdr->phy.Nsps);
            sdr->phy.symb_sync_samples = symbol_sync(sdr->phy.matched_samples, sdr->phy.Nsps);
            sdr->phy.costas_sync_samples = costas_loop(sdr->phy.symb_sync_samples);
            sdr->phy.rx_timeNs = timeNs;
            last_time = timeNs;

            // Переменная для времени отправки сэмплов относительно текущего приема
            long long tx_time = timeNs + (4 * 1000 * 1000); // на 4 [мс] в будущее


            void *tx_buffs[] = {sdr->phy.pluto_tx_buffer};
            // Добавляем время, когда нужно передать блок tx_buffer, через N -наносекунд
            for(size_t i = 0; i < 8; i++)
            {
                // Extract byte from tx time
                uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;

                // Add byte to buffer
                sdr->phy.pluto_tx_buffer[2 + i] = tx_time_byte << 4;
            }

            if( (buffers_read % 10 == 0) ){
                //printf("buffers_read: %d\n", buffers_read);
                flags = SOAPY_SDR_HAS_TIME;
                int st = SoapySDRDevice_writeStream(sdr->sdr, sdr->txStream, (const void * const*)tx_buffs, sdr->sdr_config.buffer_size, &flags, tx_time, timeoutUs);
                if ((size_t)st != sdr->sdr_config.buffer_size)
                {
                    printf("TX Failed: %i\n", st);
                }
            }
            buffers_read++;
        }
        close_pluto_sdr(sdr);
    }
}

void calculate_test_set(sdr_global_t *sdr)
{
    sdr->test_set.N = sdr->test_set.Nbit / sdr->test_set.MO;
    std::cout << "sdr->test_set.N = " << sdr->test_set.N << std::endl;
    sdr->test_set.xAxis.resize(sdr->test_set.N);
    sdr->test_set.xAxis_upsampled.resize(sdr->test_set.N * sdr->test_set.symb_size); 
    std::iota(sdr->test_set.xAxis.begin(), sdr->test_set.xAxis.end(), 0);
    std::iota(sdr->test_set.xAxis_upsampled.begin(), sdr->test_set.xAxis_upsampled.end(), 0);
    // 2. Модуляция\манипуляция BPSK\QPSK\N_QAM и др
    sdr->test_set.modulated_array = modulate(sdr->test_set.bit_array, sdr->test_set.MO);
    for (int i = 0; i < sdr->test_set.modulated_array.size(); i++){
        std::cout << sdr->test_set.modulated_array[i];
    }
    std::cout << std::endl;

    // 3. Формирование Символов (upsampling)
    sdr->test_set.upsampled_bit_array = upsample(sdr->test_set.modulated_array, sdr->test_set.symb_size);

    // 4. Pulse shaping (формирующий фильтр matched filter)
    sdr->test_set.pulse_shaped = pulse_shaping(sdr->test_set.upsampled_bit_array, 0, sdr->test_set.symb_size);
    for (int i = 0; i < sdr->test_set.pulse_shaped.size(); i++){
        std::cout << sdr->test_set.pulse_shaped[i] << " "; 
    }
    std::cout << "" << std::endl;

    // 5. Channel
    // sdr->test_set.channel_samples;

    // 5. Matched filter
    std::cout << "5. Matched filter" << std::endl;
    sdr->test_set.matched_samples = pulse_shaping(sdr->test_set.pulse_shaped, 0, sdr->test_set.symb_size);
    for (int i = 0; i < sdr->test_set.matched_samples.size(); i++){
        std::cout << sdr->test_set.matched_samples[i] << " "; 
    }
    std::cout << "" << std::endl;

    std::cout << "6. TED" << std::endl;
    // sdr->test_set.ted_err_idx = ted(sdr->test_set.matched_samples, sdr->test_set.symb_size);
    sdr->test_set.ted_samples = symbol_sync(sdr->test_set.matched_samples, sdr->test_set.symb_size);
    sdr->test_set.ted_indexes.resize(sdr->test_set.N * sdr->test_set.symb_size);
    for (int i = 0; i < sdr->test_set.ted_err_idx.size(); i++){
        sdr->test_set.ted_indexes[sdr->test_set.ted_err_idx[i]] = 10;
        std::cout << sdr->test_set.ted_err_idx[i] << " ";
    }
    std::cout << "" << std::endl;
    for (int i = 0; i < sdr->test_set.ted_samples.size(); i++){
        std::cout << sdr->test_set.ted_samples[i] << " ";
    }
    std::cout << "" << std::endl;

    std::cout << "6. Costas Loop" << std::endl;
    sdr->test_set.costas_samples = costas_loop(sdr->test_set.ted_samples);
    for (int i = 0; i < sdr->test_set.costas_samples.size(); i++){
        std::cout << sdr->test_set.costas_samples[i] << " ";
    }
    std::cout << "" << std::endl;
}

void close_pluto_sdr(sdr_global_t *sdr)
{
    printf("Trying to close Pluto SDR\n");
        //stop streaming
    if(sdr->sdr || sdr->rxStream || sdr->txStream){
        SoapySDRDevice_deactivateStream(sdr->sdr, sdr->rxStream, 0, 0);
        SoapySDRDevice_deactivateStream(sdr->sdr, sdr->txStream, 0, 0);

        //shutdown the stream
        SoapySDRDevice_closeStream(sdr->sdr, sdr->rxStream);
        SoapySDRDevice_closeStream(sdr->sdr, sdr->txStream);

        //cleanup device handle
        SoapySDRDevice_unmake(sdr->sdr);
    }

    printf("Pluto SDR is closed. Streams are deactivated.\n");
}