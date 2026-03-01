#include <fftw3.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>
#include <vector>
#include <iostream>
#include <cstdint>
#include <numeric>
#include <algorithm>

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

#include "bit_generation.h"
#include "pulse_shaping.h"
#include "modulation.h"
#include "math_operations.h"
#include "freq_sync.h"
#include "time_sync.h"
#include "test_samples.h"
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
    if (SoapySDRDevice_setBandwidth(sdr, SOAPY_SDR_RX, 0, config->rx_bandwidth) != 0)
    {
        printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, config->rx_sample_rate) != 0)
    {
        printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, config->rx_carrier_freq, NULL) != 0)
    {
        printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setBandwidth(sdr, SOAPY_SDR_TX, 0, config->tx_bandwidth) != 0)
    {
        printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
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
    FILE *file;

    if (sdr->sdr)
    {
        const long  timeoutUs = 400000; // arbitrarily chosen (взяли из srsRAN)
        long long last_time = 0;

        if(!sdr->sdr_config.is_tx){
            file = fopen("./txdata.pcm", "w");
        }
            
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
            if(!sdr->sdr_config.is_tx){
                fwrite(sdr->phy.pluto_rx_buffer, 2* sdr->sdr_config.buffer_size * sizeof(int16_t), 1, file);
                for (int i = 0; i < BUFFER_SIZE; i++){
                    sdr->phy.raw_samples[i] = std::complex<double>(sdr->phy.pluto_rx_buffer[i * 2], sdr->phy.pluto_rx_buffer[i * 2 + 1] );
                }
            }

            // Dump info
            // printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", 0, sr, flags, timeNs, timeNs - last_time);
            // for (int i = 0; i < BUFFER_SIZE; i++){
            //     sdr->phy.raw_samples[i] = std::complex<double>(sdr->phy.pluto_rx_buffer[i * 2], sdr->phy.pluto_rx_buffer[i * 2 + 1] );
            // }
            // sdr->phy.matched_samples =  pulse_shaping(sdr->phy.raw_samples, 0, sdr->phy.Nsps);
            // sdr->phy.symb_sync_samples = symbol_sync(sdr->phy.matched_samples, sdr->phy.Nsps);
            // sdr->phy.costas_sync_samples = costas_loop(sdr->phy.symb_sync_samples);
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
            if(sdr->sdr_config.is_tx == true){
                if( (buffers_read % 2 == 0) ){
                    //printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", 0, sr, flags, timeNs, timeNs - last_time);
                    flags = SOAPY_SDR_HAS_TIME;
                    int st = SoapySDRDevice_writeStream(sdr->sdr, sdr->txStream, (const void * const*)tx_buffs, sdr->sdr_config.buffer_size, &flags, tx_time, timeoutUs);
                    if ((size_t)st != sdr->sdr_config.buffer_size)
                    {
                        printf("TX Failed: %i\n", st);
                    }
                }
                buffers_read++;
            }
        }
        fclose(file);
        close_pluto_sdr(sdr);
    }
}

void prepare_test_tx_buffer(sdr_global_t *sdr)
{
    // 1. Код Баркера
    std::cout << "Формируем код Баркера (13 бит)" << std::endl;
    std::vector<int> barker_real = {1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1};
    std::vector<int> barker_imag = {1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1};
    std::vector<std::complex<double>> barker_complex;
    for(int i = 0; i < barker_real.size(); i++){
        barker_complex.push_back(std::complex(barker_real[i] * 1.1f, barker_imag[i] * 1.1f));
        //std::cout << barker_complex[i] << " ";
    }
    // std::cout << std::endl;

    // 2. "0X00Hello from user10X00" в ASCII\UTF-8 - 104 бита
    std::vector<int> hello_sibguti = {0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 
        1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 
        0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 
        1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 
        1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 
        0, 0, 0, 1, 1, 0, 0, 0, 0};
    std::cout << "text len = " << hello_sibguti.size() << std::endl;

    // 3. Модуляция 4_2_QAM
    // std::vector<int> pam_4_data;
    // pam_4(hello_sibguti, pam_4_data);
    // std::cout << "pam_4_data = " << pam_4_data.size() << std::endl;

    // std::vector<std::complex<double>> modulated_data;
    // pam_4_to_qam_4_2(pam_4_data, modulated_data);
    // 3. Модуляция QPSK
    std::vector<std::complex<double>> modulated_data = modulate(hello_sibguti, 1);

    // 4. Объединяем Код Баркера с текстом (после модуляции)
    std::vector<std::complex<double>> frame_data;
    frame_data.reserve(barker_complex.size() + modulated_data.size());
    frame_data.insert(frame_data.end(), barker_complex.begin(), barker_complex.end());
    frame_data.insert(frame_data.end(), modulated_data.begin(), modulated_data.end());
    
    std::vector<std::complex<double>> summ_vector;
    for (int i = 0; i < 1; i++)
    {
        summ_vector.insert(summ_vector.end(), frame_data.begin(), frame_data.end());
    }
    // std::cout << "frame_data = " << summ_vector.size() << std::endl;
    // for (int i = 0; i < frame_data.size();i++){
    //     std::cout << frame_data[i] << " ";
    // }
    // std::cout << std::endl;
    std::cout << "summ_vector len = " << summ_vector.size() << std::endl;
    // 5. Апсемплинг и Формирующий фильтр
    int nsps = sdr->phy.Nsps;
    int syms = 5;
    double beta = 0.75;
    std::vector<std::complex<double>> upsampled = upsample(summ_vector, nsps);
    std::vector<double> filter = srrc(syms, beta, nsps, 0.0f);
    sdr->test_rx_sdr.pulse_shaped = convolve(upsampled, filter);
    for (int i = 0; i < sdr->test_rx_sdr.pulse_shaped.size(); i++)
    {
        sdr->test_rx_sdr.samples_to_tx.push_back({  int(sdr->test_rx_sdr.pulse_shaped[i].real() * 2000) << 4, 
                                                    int(sdr->test_rx_sdr.pulse_shaped[i].imag() * 2000) << 4});
    }
    std::cout << "pulse size = " << sdr->test_rx_sdr.pulse_shaped.size() << std::endl;

    int j = 0;
    for (int i = 13; i < 2 * sdr->sdr_config.buffer_size; i += 2)
    {
        if(j < sdr->test_rx_sdr.pulse_shaped.size()){
            
            sdr->phy.pluto_tx_buffer[i] = sdr->test_rx_sdr.samples_to_tx[j].real();
            sdr->phy.pluto_tx_buffer[i + 1] = sdr->test_rx_sdr.samples_to_tx[j].imag();
            j++;
        }
        else
        {
            sdr->phy.pluto_tx_buffer[i] = int(3000);
            sdr->phy.pluto_tx_buffer[i + 1] = int(3000);
        }
    }

    //  Подготовим Флаги для Timestamp поля.
    for(size_t i = 0; i < 2; i++)
    {
        sdr->phy.pluto_tx_buffer[0 + i] = 0xffff;
        // 8 x timestamp words
        sdr->phy.pluto_tx_buffer[10 + i] = 0xffff;
    }


}

// Function to perform a 1D fftshift in-place on a complex array
// void fftshift_1d(fftw_complex* data, int N) {
//     // Calculate the midpoint for the shift
//     int shift_len = (N + 1) / 2; // Correctly handles both even and odd N

//     // Allocate a temporary array to hold the shifted data
//     fftw_complex* temp = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
//     if (temp == NULL) {
//         fprintf(stderr, "Memory allocation failed for fftshift temp array\n");
//         exit(EXIT_FAILURE);
//     }

//     // Perform the circular shift
//     for (int i = 0; i < N; i++) {
//         // The destination index using modulo arithmetic for circularity
//         int dst = (i + shift_len) % N;
//         temp[dst][0] = data[i][0]; // Real part
//         temp[dst][1] = data[i][1]; // Imaginary part
//     }

//     // Copy the shifted data back to the original array
//     for (int i = 0; i < N; i++) {
//         data[i][0] = temp[i][0];
//         data[i][1] = temp[i][1];
//     }

//     // Free the temporary array
//     fftw_free(temp);
// }

void fftshift_1d(std::vector<double> &data, int N) {
    // Calculate the midpoint for the shift
    int shift_len = (N + 1) / 2; // Correctly handles both even and odd N

    // Allocate a temporary array to hold the shifted data
    std::vector<double> temp;
    temp.resize(N);

    // Perform the circular shift
    for (int i = 0; i < N; i++) {
        // The destination index using modulo arithmetic for circularity
        int dst = (i + shift_len) % N;
        temp[dst] = data[i]; // Real part
    }

    // Copy the shifted data back to the original array
    for (int i = 0; i < N; i++) {
        data[i] = temp[i];
    }
}

// Note: The following helper functions are needed to replace Python-specific functions
// such as remez, freqz, and fliplr. These are placeholders and need proper DSP implementations.

std::vector<double> remez(int fl, const std::vector<double>& ff, const std::vector<double>& fa, double fs) {
    // Placeholder: Implement Parks-McClellan algorithm or use a DSP library
    return std::vector<double>(fl, 0.0);
}

std::pair<std::vector<double>, std::vector<std::complex<double>>> freqz(
    const std::vector<double>& taps, const std::vector<double>& den, int worN, double fs) {
    // Placeholder: Implement frequency response calculation or use a DSP library
    std::vector<double> w(worN);
    std::vector<std::complex<double>> h(worN, std::complex<double>(1.0, 0.0));
    for (int i = 0; i < worN; ++i) {
        w[i] = fs * i / worN;
    }
    return {w, h};
}

template<typename T>
std::vector<T> fliplr(const std::vector<T>& v) {
    std::vector<T> flipped(v.rbegin(), v.rend());
    return flipped;
}

template<typename T>
std::vector<T> append(const std::vector<T>& v, T val) {
    std::vector<T> result(v.begin() + 1, v.end());
    result.push_back(val);
    return result;
}

std::vector<double> arange(double start, double stop, double step) {
    std::vector<double> result;
    for (double val = start; val < stop; val += step) {
        result.push_back(val);
    }
    return result;
}

std::complex<double> cexp(std::complex<double> x) {
    return std::exp(x);
}

std::vector<std::complex<double>> costas_loop_QAM(
    const std::vector<double>& rx, double fs, double mu, double estimated_frequency, double theta_init,
    bool display_output) {

    const std::vector<double>& r = rx;
    double f0 = estimated_frequency;
    double Ts = 1.0 / fs;
    double nyquist_freq = fs / 2.0;
    int N = static_cast<int>(rx.size());
    double time = N * Ts;
    std::vector<double> t = arange(0.0, time, Ts);
    int fl = 201;
    double cut_off_freq = 0.4 * nyquist_freq;
    double start_freq = 0.3 * nyquist_freq;
    std::vector<double> ff = {0.0, start_freq, cut_off_freq, nyquist_freq};
    std::vector<double> fa = {1.0, 0.0};
    std::vector<double> taps = remez(fl, ff, fa, fs);

    auto freqz_result = freqz(taps, {1.0}, 1048, fs);
    std::vector<double> wBP = freqz_result.first;
    std::vector<std::complex<double>> hBP = freqz_result.second;

    // reshape taps to 1 x len(taps) is not needed in C++
    std::vector<double> theta(t.size(), 0.0);
    theta[0] = theta_init;
    int q = fl;
    std::vector<double> z1(q, 0.0);
    std::vector<double> z2(q, 0.0);
    std::vector<double> z3(q, 0.0);
    std::vector<double> z4(q, 0.0);
    std::vector<double> carrier_estimation(N, 0.0);
    std::vector<std::complex<double>> complex_exp_estimation(N, std::complex<double>(0.0, 0.0));

    for (size_t k = 0; k < t.size() - 1; ++k) {
        double s = 2.0 * r[k];
        z1 = append(z1, s * std::cos(2.0 * M_PI * f0 * t[k] + theta[k]));
        z2 = append(z2, s * std::cos(2.0 * M_PI * f0 * t[k] + M_PI / 4.0 + theta[k]));
        z3 = append(z3, s * std::cos(2.0 * M_PI * f0 * t[k] + M_PI / 2.0 + theta[k]));
        z4 = append(z4, s * std::cos(2.0 * M_PI * f0 * t[k] + 3.0 * M_PI / 4.0 + theta[k]));

        std::vector<double> taps_flipped = fliplr(taps);

        // Compute dot product (equivalent to matmul of 1xfl and flx1)
        double lpf1 = std::inner_product(taps_flipped.begin(), taps_flipped.end(), z1.begin(), 0.0);
        double lpf2 = std::inner_product(taps_flipped.begin(), taps_flipped.end(), z2.begin(), 0.0);
        double lpf3 = std::inner_product(taps_flipped.begin(), taps_flipped.end(), z3.begin(), 0.0);
        double lpf4 = std::inner_product(taps_flipped.begin(), taps_flipped.end(), z4.begin(), 0.0);

        theta[k + 1] = theta[k] + mu * lpf1 * lpf2 * lpf3 * lpf4;

        carrier_estimation[k] = std::cos(2.0 * M_PI * f0 * t[k] + theta[k]);
        complex_exp_estimation[k] = std::exp(std::complex<double>(0.0, -1.0 * (2.0 * M_PI * estimated_frequency * t[k] + theta[k])));
    }

    return complex_exp_estimation;
}



std::vector<std::complex<double>> quantalph_distance(const std::vector<std::complex<double>>& symbol_array, const std::vector<double>& alphabet) {
    // symbolArray = downsampled symbols
    // alphabet = coded alphabet, it should be pass with symbols values which are greater than zero
    // distance = the distance on I and Q axis between the max and min QAM4_2 symbols
    // threshold = this is equal to the 1/4 of the total distance. This distance in the IQ axis is considered as a circle whose diameter is equal to the distance
    // the radius of that circle is equal to distance/4. this radius is divided into 6 equal points and the distance of the third point to the origin is set as threshold

    auto min_alphabet = *min_element(alphabet.begin(), alphabet.end());
    auto max_alphabet = *max_element(alphabet.begin(), alphabet.end());

    auto min_point = *min_element(symbol_array.begin(), symbol_array.end(), [](const std::complex<double>& a, const std::complex<double>& b) {
        return abs(a) < abs(b);
    });
    auto max_point = *max_element(symbol_array.begin(), symbol_array.end(), [](const std::complex<double>& a, const std::complex<double>& b) {
        return abs(a) < abs(b);
    });

    double distance = abs(min_point) + abs(max_point);
    double threshold = distance / 4.0;

    std::vector<std::complex<double>> quantized_symbols(symbol_array.size());

    auto sign = [](double val) -> double {
        if (val > 0) return 1.0;
        else if (val < 0) return -1.0;
        else return 0.0;
    };

    for (size_t i = 0; i < symbol_array.size(); ++i) {
        if (std::abs(symbol_array[i]) < threshold) {
            quantized_symbols[i] = std::complex<double>(
                sign(symbol_array[i].real()) * min_alphabet,
                sign(symbol_array[i].imag()) * min_alphabet
            );
        } else {
            quantized_symbols[i] = std::complex<double>(
                sign(symbol_array[i].real()) * max_alphabet,
                sign(symbol_array[i].imag()) * max_alphabet
            );
        }
    }

    return quantized_symbols;
}

void test_rx_sdr(sdr_global_t *sdr)
{
    int buffer_size = sdr->sdr_config.buffer_size;
    int target_idx = 3700;
    double scalar_val = 40.0;

    for (int i = target_idx; i < target_idx + buffer_size; i++){
        sdr->test_rx_sdr.channel_samples.push_back(test_rx_samples[i] / std::pow(2, 11));
    }

    // sdr->test_rx_sdr.matched_samples  = pulse_shaping(sdr->test_rx_sdr.channel_samples, 0, sdr->test_rx_sdr.symb_size);

    // Define the frequency and amplitude of the sine wave
    // const double frequency = 10.0;
    // const double amplitude = 1.0;
    // const double PI = std::acos(-1.0);
    //     for (int i = 0; i < buffer_size; ++i)
    // {
    //     double angle = (static_cast<double>(i) / buffer_size) * (2.0 * PI * frequency);
    //     sdr->test_rx_sdr.fft_in_samples[i] = amplitude * std::sin(angle);
    // }

    sdr->test_rx_sdr.fft_out_samples.resize(buffer_size);
    sdr->test_rx_sdr.fft_in_samples.resize(buffer_size);
    sdr->test_rx_sdr.fft_out_abs.resize(buffer_size);
    sdr->test_rx_sdr.matched_squared_samples.resize(buffer_size);
    sdr->test_rx_sdr.matched_samples.resize(buffer_size);
    sdr->test_rx_sdr.coarsed_samples.resize(buffer_size);
    sdr->test_rx_sdr.shifted_before_cl.resize(buffer_size);
    

    // 1. Coarse Freq Shift
    double fs = 4e6;
    

    
    std::transform(sdr->test_rx_sdr.channel_samples.begin(),
                   sdr->test_rx_sdr.channel_samples.end(),
                   sdr->test_rx_sdr.matched_squared_samples.begin(),
                   [](const std::complex<double>& z){ return std::pow(z, 4); });

    fftw_plan p = fftw_plan_dft_1d(buffer_size, 
                                   reinterpret_cast<fftw_complex*>(sdr->test_rx_sdr.matched_squared_samples.data()), 
                                   reinterpret_cast<fftw_complex*>(sdr->test_rx_sdr.fft_out_samples.data()), 
                                   FFTW_FORWARD, 
                                   FFTW_ESTIMATE);
    fftw_execute(p);
    std::transform(sdr->test_rx_sdr.fft_out_samples.begin(),
                   sdr->test_rx_sdr.fft_out_samples.end(),
                   sdr->test_rx_sdr.fft_out_abs.begin(),
                   [](const std::complex<double>& z){ return std::abs(z); });

    fftshift_1d(sdr->test_rx_sdr.fft_out_abs, buffer_size);
    std::vector<double> freqs = arange( (-1) * fs/2.0f, (1) * fs/2.0f, fs / buffer_size);
    std::cout << "freqs size = " << freqs.size() << " ";
    std::cout << "(int)fs/buffer_size = " << (int)fs/buffer_size << " ";

    std::vector<double>::iterator max_freq_ind = std::max_element(sdr->test_rx_sdr.fft_out_abs.begin(), sdr->test_rx_sdr.fft_out_abs.end());
    int index = std::distance(sdr->test_rx_sdr.fft_out_abs.begin(), max_freq_ind);
    std::cout << "index max_freq = " << index << std::endl;
    std::cout << "max_freq = " << freqs[index] / 4.0 << std::endl;
    double Ts = 1 / fs;
    double fc = 1e6;
    std::vector<double> times = arange(0, Ts * buffer_size, Ts);
    for (int i = 0; i < buffer_size; i++)
    {
        std::complex<double> val = std::complex<double>(0.0, -1.0f * 2.0 * M_PI * (freqs[index] / 4.0) * times[i]);
        std::complex<double> val_2 = std::complex<double>(0.0, 1.0f * 2.0 * M_PI * fc * times[i]);
        sdr->test_rx_sdr.coarsed_samples[i] = sdr->test_rx_sdr.channel_samples[i] * std::exp(val);
        sdr->test_rx_sdr.shifted_before_cl[i] = sdr->test_rx_sdr.coarsed_samples[i] * std::exp(val_2);
    }
    std::cout << std::endl;

    //coarse_demodulated_baseband_signal * np.exp(1j * 2 * np.pi * fc * t)
    
    // sdr->test_rx_sdr.matched_samples  = pulse_shaping(sdr->test_rx_sdr.channel_samples, 0, sdr->test_rx_sdr.symb_size);
    std::vector<double> real_parts;
    std::vector<double> imag_parts;
    for (const auto& complex_num : sdr->test_rx_sdr.shifted_before_cl) {
        real_parts.push_back(complex_num.real());
        imag_parts.push_back(complex_num.imag());
    }
    sdr->test_rx_sdr.costas_samples.resize(buffer_size);
    std::vector<std::complex<double>> complex_exp_est = costas_loop_QAM(real_parts, fs, 0.2, fc, M_PI / 6, 0);
    for (int i = 0; i < sdr->test_rx_sdr.costas_samples.size(); i++)
    {
        sdr->test_rx_sdr.costas_samples[i] = sdr->test_rx_sdr.shifted_before_cl[i] * complex_exp_est[i];
    }
    std::cout << std::endl;
    
    // sdr->test_rx_sdr.costas_samples = costas_loop(sdr->test_rx_sdr.ted_samples);
    int nsps = sdr->phy.Nsps;
    int syms = 5;
    double beta = 0.75;
    std::vector<double> filter = srrc(syms, beta, nsps, 0.0f);
    // std::vector<double>::iterator max_it = std::max_element(filter.begin(), filter.end());
    // double max_value = *max_it;

    sdr->test_rx_sdr.matched_samples = convolve(sdr->test_rx_sdr.costas_samples, filter);
    std::cout << "costas_samples size = " << sdr->test_rx_sdr.costas_samples.size() << std::endl;

    // for (int i = 0; i < sdr->test_rx_sdr.matched_samples.size(); i++)
    // {
    //     sdr->test_rx_sdr.matched_samples[i] *= max_value;
    // }

    sdr->test_rx_sdr.ted_samples = symbol_sync(sdr->test_rx_sdr.matched_samples, sdr->test_rx_sdr.symb_size);

    
    std::vector<double> alphabet = {1, 3};
    sdr->test_rx_sdr.quantalph = quantalph_distance(sdr->test_rx_sdr.ted_samples, alphabet);
    std::vector<double> quant_real;
    std::vector<double> quant_imag;
    for (int i = 0; i < sdr->test_rx_sdr.quantalph.size(); i++){
        quant_real.push_back(sdr->test_rx_sdr.quantalph[i].real());
        quant_imag.push_back(sdr->test_rx_sdr.quantalph[i].imag());
    }
        sdr->test_rx_sdr.demod_bit_array = demodulate(sdr->test_rx_sdr.ted_samples, 1);
    for (int i = 0; i < sdr->test_rx_sdr.demod_bit_array.size(); i++){
        std::cout << sdr->test_rx_sdr.demod_bit_array[i] << " ";
    }
    std::cout << std::endl;
    std::vector<int> barker_real = {1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1};
    std::vector<int> barker_imag = {1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1};
    std::vector<double> barker_real_double;
    std::vector<double> barker_imag_double;
    
    std::vector<std::complex<double>> barker_complex;
    for(int i = 0; i < barker_real.size(); i++){
        barker_complex.push_back(std::complex(barker_real[i] * 3.0f, barker_imag[i] * 3.0f));
        barker_real_double.push_back(barker_real[i] * 3.0f);
        barker_imag_double.push_back(barker_imag[i] * 3.0f);
    }

    sdr->test_rx_sdr.barker_corr_real.resize(buffer_size);
    sdr->test_rx_sdr.barker_corr_real = correlate_manual(quant_real, barker_real_double, "same");
    sdr->test_rx_sdr.barker_corr_imag = correlate_manual(quant_imag, barker_imag_double, "same");
    int max = 80;
    int frame_size = 104;
    for (int i = max; i < max + frame_size; i++){
        std::cout << sdr->test_rx_sdr.demod_bit_array[i] << " ";
    }
    std::cout << std::endl;
    // correlate_valid(sdr->test_rx_sdr.demod_bit_array, barker_real);
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