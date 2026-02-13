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
            win_size.y -= 50;
            win_size.x -= 50;
            if (!sdr->phy.raw_samples.empty()) {
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
                    &sdr->phy.raw_samples,
                    sdr->phy.raw_samples.size());
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