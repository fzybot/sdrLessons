#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"

#include "pluto_sdr_lib.h"
#include "ui/pluto_ui.h"



int main(int argc, char* argv[])
{
    // Наши параметры SDR в одной структуре.
    sdr_global_t my_sdr;
    my_sdr.running = true;
    my_sdr.sdr_config.name = argv[1];
    my_sdr.sdr_config.buffer_size = BUFFER_SIZE;
    my_sdr.sdr_config.rx_carrier_freq = 107e6;
    my_sdr.sdr_config.tx_carrier_freq = 800e6;
    my_sdr.sdr_config.rx_sample_rate = 1e6;
    my_sdr.sdr_config.tx_sample_rate = 1e6;
    my_sdr.sdr_config.rx_gain = 60.0;
    my_sdr.sdr_config.tx_gain = -40.0;

    // size_t channels[] = {0} ; // {0} or {0, 1}
    // config.channels = &channels;

    my_sdr.sdr = setup_pluto_sdr(&my_sdr.sdr_config);
    my_sdr.rxStream = setup_stream(my_sdr.sdr, &my_sdr.sdr_config, 1);
    my_sdr.txStream = setup_stream(my_sdr.sdr, &my_sdr.sdr_config, 0);


    std::thread gui_thread(run_gui, &my_sdr);
    std::thread sdr_thread(run_sdr, &my_sdr);

    gui_thread.join();
    sdr_thread.join();

    
    // close_pluto_sdr(sdr, my_sdr->rxStream, my_sdr->txStream);

    return EXIT_SUCCESS;
}
