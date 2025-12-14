#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>
#include <vector>
#include <cstdint>

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
    size_t channel_count = sizeof(config->channels) / sizeof(config->channels[0]);
    printf("Channel count = %d\n", channel_count);
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
}

void fill_test_tx_buffer(int16_t *buffer, int size)
{
    printf("Filling TX buffer.\n");
    //заполнение tx_buff значениями сэмплов первые 16 бит - I, вторые 16 бит - Q.
    for (int i = 0; i < 2 * size; i+=2)
    {
        buffer[i] = 2000 << 4;
        buffer[i + 1] = 2000 << 4;
    }

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

void start_rx_tx(  struct SoapySDRDevice *sdr, SoapySDRStream *rxStream, SoapySDRStream *txStream, 
                                int16_t *tx_buffer, int16_t *rx_buffer, int buff_size, int num_iteration)
{
    printf("Starting recv and send samples.\n");
    const long  timeoutUs = 400000; // arbitrarily chosen (взяли из srsRAN)
    long long last_time = 0;

    FILE *file = fopen("txdata.pcm", "w");

    // Начинается работа с получением и отправкой сэмплов
    for (size_t buffers_read = 0; buffers_read < num_iteration; buffers_read++)
    {
        printf("1.\n");
        void *rx_buffs[] = {rx_buffer};
        int flags;        // flags set by receive operation
        long long timeNs; //timestamp for receive buffer

        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, buff_size, &flags, &timeNs, timeoutUs);
        if (sr < 0)
        {
            printf("ERROR. SoapySDRDevice_readStream.\n");
            continue;
        }
        

        // Dump info
        printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", buffers_read, sr, flags, timeNs, timeNs - last_time);
        fwrite(rx_buffer, 2* buff_size * sizeof(int16_t), 1, file);
        last_time = timeNs;

        // Переменная для времени отправки сэмплов относительно текущего приема
        long long tx_time = timeNs + (4 * 1000 * 1000); // на 4 [мс] в будущее

        // Добавляем время, когда нужно передать блок tx_buffer, через N -наносекунд
        for(size_t i = 0; i < 8; i++)
        {
            // Extract byte from tx time
            uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;

            // Add byte to buffer
            tx_buffer[2 + i] = tx_time_byte << 4;
        }

        // Send buffer
        void *tx_buffs[] = {tx_buffer};
        if( (buffers_read == 2) ){
            printf("buffers_read: %d\n", buffers_read);
            flags = SOAPY_SDR_HAS_TIME;
            int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, buff_size, &flags, tx_time, timeoutUs);
            if ((size_t)st != buff_size)
            {
                printf("TX Failed: %i\n", st);
            }
        }
        
    }
    fclose(file);

}

void close_pluto_sdr(SoapySDRDevice *sdr, SoapySDRStream *rxStream, SoapySDRStream *txStream)
{
    printf("Trying to close Pluto SDR\n");
        //stop streaming
    if(sdr || rxStream || txStream){
        SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
        SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);

        //shutdown the stream
        SoapySDRDevice_closeStream(sdr, rxStream);
        SoapySDRDevice_closeStream(sdr, txStream);

        //cleanup device handle
        SoapySDRDevice_unmake(sdr);
    }

    printf("Pluto SDR is closed. Streams are deactivated.\n");
}