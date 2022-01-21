#include "Debugger.h"

#include "../Renderer/Renderer.h"
#include "../Window/Window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

extern Window       g_Window;
extern Renderer     g_Renderer;

void Debugger::init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(g_Window.window().get(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

Debugger::~Debugger()
{
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void Debugger::update(const float renderTime)
{
    _renderTimes.push_back(renderTime);
    if (_renderTimes.size() >= 512)
    {
        _renderTimes.pop_front();
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    performanceWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Debugger::performanceWindow()
{
    ImGui::SetNextWindowSize({ 400, 150 });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    ImPlot::GetStyle().Colormap = ImPlotColormap_Cool;
    ImGui::SetNextWindowPos({ 1280-400, 720-150 });

    ImGui::Begin("Performances", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
    
    // Pretty ugly but for debugging
    std::vector<float> renderTimesVec;
    std::vector<float> indices;
    int i = 0;
    for (const auto& renderTime : _renderTimes)
    {
        renderTimesVec.push_back(renderTime*1000.f);
        indices.push_back(i++); 
    }
    ImPlot::StyleColorsDark();
    if (ImPlot::BeginPlot("Render Time", 0, 0, {-1, -1}, ImPlotFlags_NoInputs | ImPlotFlags_NoTitle | ImPlotFlags_AntiAliased | ImPlotFlags_NoLegend, ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Lock, ImPlotAxisFlags_Lock))
    {
        ImPlot::SetupAxesLimits(1, 512, 0.0, 16.0);
        ImPlot::PlotLine("ms", indices.data(), renderTimesVec.data(), renderTimesVec.size());
        ImPlot::EndPlot();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
