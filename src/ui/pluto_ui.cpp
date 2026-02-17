#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"
#include "imgui_internal.h"

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
            // DemoHeader("Sizing", Demo_SubplotsSizing);
            // DemoHeader("Item Sharing", Demo_SubplotItemSharing);
            // DemoHeader("Axis Linking", Demo_SubplotAxisLinking);
            // DemoHeader("Tables", Demo_Tables);
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