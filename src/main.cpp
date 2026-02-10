#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>


#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"

#include "pluto_sdr_lib.h"

void run_gui(sdr_global_t *sdr)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_Window* window = SDL_CreateWindow(
        "Backend start", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Включить Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Включить Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Включить Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Включить Multi-Viewport / Platform Windows. Позволяет работать "окнам" вне основного окна. 

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // auto last_frame_time = std::chrono::steady_clock::now();
    while (sdr->running) {

        // Обработка event'ов (inputs, window resize, mouse moving, etc.)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // std::cout << "Processing some event: "<< event.type << " timestamp: " << event.motion.timestamp << std::endl;
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                sdr->running = false;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);


        // Отображаем на графике сэмплы
        {
            ImVec2 plotSize(500, 600);
            ImGui::Begin("Test for I/Q samples update in Scatter plot");
            ImVec2 win_size = ImGui::GetWindowSize();
            win_size.y -= 100;
            ImGui::Text("Window size equal to: %f x %f", win_size.x, win_size.y);
            if(ImPlot::BeginPlot("I/Q", win_size))
            {
                ImPlot::PlotScatter("Samples", sdr->phy.raw_real, sdr->phy.raw_imag, BUFFER_SIZE);
                ImPlot::EndPlot();
            }
        
            ImGui::End();
        }


        ImGui::Render();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* argv[])
{
    // Наши параметры SDR в одной структуре.
    sdr_global_t my_sdr;
    my_sdr.running = true;
    my_sdr.sdr_config.name = argv[1];
    my_sdr.sdr_config.buffer_size = BUFFER_SIZE;
    my_sdr.sdr_config.rx_carrier_freq = 2400e6;
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

    int16_t tx_buffer[2*my_sdr.sdr_config.buffer_size];
    int16_t rx_buffer[2*my_sdr.sdr_config.buffer_size];

    // TODO: здесь нужно отправлять и принимать сэмплы.
    //fill_test_tx_buffer(tx_buffer, my_sdr.sdr_config.buffer_size);

    std::thread gui_thread(run_gui, &my_sdr);
    std::thread sdr_thread(run_sdr, &my_sdr);

    gui_thread.join();
    sdr_thread.join();

    
    // close_pluto_sdr(sdr, my_sdr->rxStream, my_sdr->txStream);

    return EXIT_SUCCESS;
}
