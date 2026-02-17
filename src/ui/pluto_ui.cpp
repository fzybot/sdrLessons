#include <vector>
#include <iostream>
#include <numeric>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"
#include "imgui_internal.h"

#include "bit_generation.h"
#include "pulse_shaping.h"
#include "modulation.h"
#include "math_operations.h"
#include "pluto_sdr_lib.h"
#include "pluto_ui.h"

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
        ImGuiID dockspace_id = ImGui::GetID("My Dockspace");
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Create settings
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
            ImGuiID dock_id_left = 0;
            ImGuiID dock_id_main = dockspace_id;
            ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, &dock_id_left, &dock_id_main);
            ImGuiID dock_id_left_top = 0;
            ImGuiID dock_id_left_bottom = 0;
            ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.50f, &dock_id_left_top, &dock_id_left_bottom);
            ImGui::DockBuilderDockWindow("Main", dock_id_main);
            ImGui::DockBuilderDockWindow("Properties", dock_id_left_top);
            ImGui::DockBuilderDockWindow("Scene", dock_id_left_bottom);
            ImGui::DockBuilderFinish(dockspace_id);
        }
        ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

        show_global_menu_bar(sdr);
        show_properties_window(sdr);
        show_main_window(sdr);
        

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


void show_global_menu_bar(sdr_global_t *sdr)
{
    if(ImGui::BeginMainMenuBar()){
        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


void show_properties_window(sdr_global_t *sdr)
{
    static int counter = 0;
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::Begin("Properties");
    if (ImGui::Button("Button"))
        counter++;
    ImGui::Text("counter = %d", counter);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Window size: %lfx%lf", ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
    ImGui::End();
}

void show_main_window(sdr_global_t *sdr)
{
    ImGui::Begin("Main");
    if (ImGui::BeginTabBar("Main")) {
        if (ImGui::BeginTabItem("Real Time I/Q samples")) {
            show_iq_scatter_plot(sdr, sdr->phy.raw_samples);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Tests")) {
            show_test_sdr_set(sdr);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void show_iq_scatter_plot(sdr_global_t *sdr, std::vector< std::complex<float> > &samples)
{
    // Отображаем на графике сэмплы
    {
        ImVec2 plotSize(500, 600);
        //ImGui::Begin("Test for I/Q samples update in Scatter plot");
        ImVec2 win_size = ImGui::GetWindowSize();
        win_size.y -= 50;
        win_size.x -= 50;
        if (!samples.empty()) {
            ImPlot::BeginPlot("##ConstellationPlot", win_size);
            ImPlot::SetupAxes("I","Q");
            ImPlot::SetupAxisLimits(ImAxis_X1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetupAxisLimits(ImAxis_Y1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 1.8); // Тип и размер точек
            ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec =
                        *static_cast<std::vector<std::complex<float>>*>(
                            data);
                    return ImPlotPoint(vec[idx].real(), vec[idx].imag());
                },
                &samples,
                samples.size());
            ImPlot::EndPlot();
        }
    
        //ImGui::End();
    }
}
enum class pulse_shaping_filter;

void show_test_sdr_set(sdr_global_t *sdr)
{
    // 1. Генерация случайного набора бит
    int N = 10;
    int symb_size = 10;
    std::vector<int> xAxis(N);                              // Значения оси Х для графиков
    std::vector<int> xAxis_upsampled(N * symb_size);        // Значения для оси Х для графиков после upsampling
    std::iota(xAxis.begin(), xAxis.end(), 0);
    std::iota(xAxis_upsampled.begin(), xAxis_upsampled.end(), 0);

    std::vector<int> bit_array = {0, 1, 1, 0, 0, 1, 1, 1, 0, 1};

    // 2. Модуляция\манипуляция BPSK\QPSK\N_QAM и др
    std::vector<std::complex<float>> modulated_array = modulate(bit_array, 1);

    // 3. Формирование Символов (upsampling)
    std::vector<std::complex<float>> upsampled_bit_array = upsample(modulated_array, symb_size);

    // 4. Pulse shaping (формирующий фильтр matched filter)
    std::vector<std::complex<float>> pulse_shaped = pulse_shaping(upsampled_bit_array, 0, symb_size);

    // 5. Channel
    std::vector<std::complex<float>> channel_samples;

    // 5. Matched filter
    std::vector<std::complex<float>> matched_samples = pulse_shaping(pulse_shaped, 0, symb_size);

    std::vector<float> ted_samples = ted(matched_samples, symb_size);
    for (int i = 0; i < ted_samples.size(); i++){
        std::cout << ted_samples[i] << " ";
    }
    std::cout << " " << std::endl;

    static int rows = 4;
    static int cols = 1;
    static int plot_size = 10;
    static float rratios[] = {10, 10, 10, 10, 10, 1};
    static float cratios[] = {10, 10, 10, 10,10,1};
    static ImPlotSubplotFlags flags = ImPlotSubplotFlags_ShareItems|ImPlotSubplotFlags_NoLegend;
    if (ImPlot::BeginSubplots("My Subplots", rows, cols, ImVec2(-1,400), flags, rratios, cratios)) {

        ImPlot::BeginPlot("1. Generate bit array", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::PlotScatter("Mouse Y", xAxis.data(), bit_array.data(), N, 0);
        ImPlot::EndPlot();

        ImPlot::BeginPlot("2. Modulated Samples", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<float>>*>(data);
                    return ImPlotPoint(idx, vec[idx].real());
                },
                &modulated_array,
                modulated_array.size());
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<float>>*>(data);
                    return ImPlotPoint(idx, vec[idx].imag());
                },
                &modulated_array,
                modulated_array.size());
        ImPlot::EndPlot();

        ImPlot::BeginPlot("3. Upsampling", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<float>>*>(data);
                    return ImPlotPoint(idx, vec[idx].real());
                },
                &upsampled_bit_array,
                upsampled_bit_array.size());
        ImPlot::EndPlot();

        ImPlot::BeginPlot("4. Pulse shaping", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<float>>*>(data);
                    return ImPlotPoint(idx, vec[idx].real());
                },
                &pulse_shaped,
                pulse_shaped.size());
        ImPlot::EndPlot();

        ImPlot::BeginPlot("6. TED", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<float>*>(data);
                    return ImPlotPoint(idx, vec[idx]);
                },
                &ted_samples,
                ted_samples.size());
        ImPlot::EndPlot();

        ImPlot::EndSubplots();
    }

}