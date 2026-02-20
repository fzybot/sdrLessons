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
        1920, 1080, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
void DemoHeader(const char* label, void(*demo)()) {
    if (ImGui::TreeNodeEx(label)) {
        demo();
        ImGui::TreePop();
    }
}

void show_main_window(sdr_global_t *sdr)
{
    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginTabBar("Main")) {
        if (ImGui::BeginTabItem("Real Time I/Q samples")) {
            ImVec2 win_size = ImGui::GetWindowSize();
            win_size.y -= 50;
            win_size.x -= 50;
            show_iq_scatter_plot(sdr, sdr->phy.raw_samples);
            //show_iq_scatter_plot(sdr, sdr->phy.matched_samples);

            ImPlot::BeginPlot("Pulse Shaping ConstellationPlot");
            ImPlot::SetupAxes("I","Q");
            ImPlot::SetupAxisLimits(ImAxis_X1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetupAxisLimits(ImAxis_Y1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 1.8); // Тип и размер точек
            ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec =
                        *static_cast<std::vector<std::complex<double>>*>(
                            data);
                    return ImPlotPoint(vec[idx].real(), vec[idx].imag());
                },
                &sdr->phy.matched_samples,
                sdr->phy.matched_samples.size());
            ImPlot::EndPlot();

            ImPlot::BeginPlot("Symb sync ConstellationPlot");
            ImPlot::SetupAxes("I","Q");
            ImPlot::SetupAxisLimits(ImAxis_X1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetupAxisLimits(ImAxis_Y1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 1.8); // Тип и размер точек
            ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec =
                        *static_cast<std::vector<std::complex<double>>*>(
                            data);
                    return ImPlotPoint(vec[idx].real(), vec[idx].imag());
                },
                &sdr->phy.symb_sync_samples,
                sdr->phy.symb_sync_samples.size());
            ImPlot::EndPlot();

            ImPlot::BeginPlot("Costas Loop ConstellationPlot");
            ImPlot::SetupAxes("I","Q");
            ImPlot::SetupAxisLimits(ImAxis_X1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetupAxisLimits(ImAxis_Y1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 1.8); // Тип и размер точек
            ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec =
                        *static_cast<std::vector<std::complex<double>>*>(
                            data);
                    return ImPlotPoint(vec[idx].real(), vec[idx].imag());
                },
                &sdr->phy.costas_sync_samples,
                sdr->phy.costas_sync_samples.size());
            ImPlot::EndPlot();


            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Tests")) {
            show_test_sdr_set(sdr);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Pulse Shaping")){
            test_pulse_shaping_srrc(sdr);
            test_pulse_shaping_hamming(sdr);
            // srrc(int syms, double beta, int P, double t_off = 0)
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void test_pulse_shaping_srrc(sdr_global_t *sdr)
{

    if(ImPlot::BeginPlot("1. SRRC", ImVec2())){
        ImPlot::SetupAxisLimits(ImAxis_Y1, -1.0, 1.0);
        ImPlot::SetupAxisLimits(ImAxis_X1, 30.0, 70.0);
        //ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_Reverse);
        int over_samp_rate = 10;
        int syms = 5;
        double beta[5] = {0, 0.25, 0.5, 0.75, 1};
        std::vector<double> s1 = srrc(syms, beta[0], over_samp_rate, 0);
        std::vector<double> s2 = srrc(syms, beta[1], over_samp_rate, 0);
        std::vector<double> s3 = srrc(syms, beta[2], over_samp_rate, 0);
        std::vector<double> s4 = srrc(syms, beta[3], over_samp_rate, 0);
        std::vector<double> s5 = srrc(syms, beta[4], over_samp_rate, 0);
        ImPlot::PlotLine("beta = 0",s1.data(),s1.size());
        ImPlot::PlotLine("beta = 0.25",s2.data(),s2.size());
        ImPlot::PlotLine("beta = 0.50",s3.data(),s3.size());
        ImPlot::PlotLine("beta = 0.75",s4.data(),s4.size());
        ImPlot::PlotLine("beta = 1",s5.data(),s5.size());
        ImPlot::EndPlot();
    }
}

void test_pulse_shaping_hamming(sdr_global_t *sdr)
{
    if (ImPlot::BeginPlot("Hamming", ImVec2()))
    {
        ImPlot::SetupAxisLimits(ImAxis_Y1, -1.0, 1.0);
        ImPlot::SetupAxisLimits(ImAxis_X1, 30.0, 70.0);
        int M = 100;
        std::vector<double> s10 = hamming(M);
        ImPlot::PlotLine("Hamming",s10.data(),s10.size());
        ImPlot::EndPlot();
    }
}


void show_iq_scatter_plot(sdr_global_t *sdr, std::vector< std::complex<double> > &samples)
{
    // Отображаем на графике сэмплы
    {
        //ImVec2 plotSize(500, 600);
        //ImGui::Begin("Test for I/Q samples update in Scatter plot");
        ImVec2 win_size = ImGui::GetWindowSize();
        win_size.y -= 50;
        win_size.x -= 50;
        if (!samples.empty()) {
            ImPlot::BeginPlot("ConstellationPlot");
            ImPlot::SetupAxes("I","Q");
            ImPlot::SetupAxisLimits(ImAxis_X1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetupAxisLimits(ImAxis_Y1, -2500.0, 2500.0); // 12-bit АЦП
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 1.8); // Тип и размер точек
            ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec =
                        *static_cast<std::vector<std::complex<double>>*>(
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

void show_test_sdr_set(sdr_global_t *sdr)
{
    static int rows = 7;
    static int cols = 1;
    static int plot_size = 10;
    static float rratios[] = {5, 5, 5, 5, 5, 5, 5};
    static float cratios[] = {5, 5, 5, 5, 5, 5, 5};
    ImVec2 win_size = ImGui::GetWindowSize();
    win_size.y -= 50;
    win_size.x -= 50;
    static ImPlotSubplotFlags flags = ImPlotSubplotFlags_ShareItems | ImPlotSubplotFlags_NoLegend;
    // ImGui::CheckboxFlags("ImPlotSubplotFlags_LinkRows", (unsigned int*)&flags, ImPlotSubplotFlags_LinkRows);
    // ImGui::CheckboxFlags("ImPlotSubplotFlags_LinkCols", (unsigned int*)&flags, ImPlotSubplotFlags_LinkCols);
    if (ImPlot::BeginSubplots("My Subplots", rows, cols, win_size, flags, rratios, cratios)) {

        ImPlot::BeginPlot("1. Generate bit array", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
        ImPlot::PlotStems("Mouse Y", sdr->test_set.xAxis.data(), sdr->test_set.bit_array.data(), sdr->test_set.N, 0);
        ImPlot::EndPlot();

        ImPlot::BeginPlot("2. Modulated Samples", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<double>>*>(data);
                    return ImPlotPoint(idx, vec[idx].real());
                },
                &sdr->test_set.modulated_array,
                sdr->test_set.modulated_array.size());
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<double>>*>(data);
                    return ImPlotPoint(idx, vec[idx].imag());
                },
                &sdr->test_set.modulated_array,
                sdr->test_set.modulated_array.size());
        ImPlot::EndPlot();

        ImPlot::BeginPlot("2. Modulated Constellation", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::SetupAxisLimits(ImAxis_X1, -2.0, 2.0);
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<double>>*>(data);
                    return ImPlotPoint(vec[idx].real(), vec[idx].imag());
                },
                &sdr->test_set.modulated_array,
                sdr->test_set.modulated_array.size());
        ImPlot::EndPlot();
    
        ImPlot::BeginPlot("3. Upsampling", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<double>>*>(data);
                    return ImPlotPoint(idx, vec[idx].real());
                },
                &sdr->test_set.upsampled_bit_array,
                sdr->test_set.upsampled_bit_array.size());
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<double>>*>(data);
                    return ImPlotPoint(idx, vec[idx].imag());
                },
                &sdr->test_set.upsampled_bit_array,
                sdr->test_set.upsampled_bit_array.size());
        ImPlot::EndPlot();

        ImPlot::BeginPlot("4. Pulse shaping", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<double>>*>(data);
                    return ImPlotPoint(idx, vec[idx].real());
                },
                &sdr->test_set.pulse_shaped,
                sdr->test_set.pulse_shaped.size());
        ImPlot::EndPlot();

        ImPlot::BeginPlot("6. TED", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -11.0, 11.0);
        ImPlot::PlotLineG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<double>>*>(data);
                    return ImPlotPoint(idx, vec[idx].real());
                },
                &sdr->test_set.matched_samples,
                sdr->test_set.matched_samples.size());
        ImPlot::PlotLineG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<double>>*>(data);
                    return ImPlotPoint(idx, vec[idx].imag());
                },
                &sdr->test_set.matched_samples,
                sdr->test_set.matched_samples.size());
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
        ImPlot::PlotStems("Stems 2", sdr->test_set.xAxis_upsampled.data(), sdr->test_set.ted_indexes.data(),sdr->test_set.ted_indexes.size());
        ImPlot::EndPlot();

        ImPlot::BeginPlot("7. Costas Loop Constellation", ImVec2(), ImPlotFlags_NoLegend);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0);
        ImPlot::SetupAxisLimits(ImAxis_X1, -2.0, 2.0);
        ImPlot::PlotScatterG(
                "Signal",
                [](int idx, void* data) {
                    auto& vec = *static_cast<std::vector<std::complex<double>>*>(data);
                    return ImPlotPoint(vec[idx].real(), vec[idx].imag());
                },
                &sdr->test_set.costas_samples,
                sdr->test_set.costas_samples.size());
        ImPlot::EndPlot();

        ImPlot::EndSubplots();
    }

}