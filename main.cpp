#pragma execution_character_set("utf-8")

// Dear ImGui: standalone example application for DirectX 9

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp


#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_dx9.h"
#include "vendor/imgui/imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include "src/WeatherApp.h"
#include <chrono>
#include <imgui_extension.h>
#include "implot.h"
#include <ctime>
#include <cstdio>

const wchar_t CLASS_NAME[] = L"WeatherAppWindowClass";
const wchar_t MAIN_WINDOW_TITLE[] = L"Weather App";

// Data
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//WEATHER APP GUI ELEMENTS
int selectedComboStationIndex = 0;
vector<Station> comboStations;
vector<const char*> comboStationEntries;
bool isFetching = false;
int fromBack = 3;
int toBack = -1;
std::string startDate, endDate;

unordered_map<INT64, bool> cachedDisplaySensorOnPlot;
unordered_map<INT64, SensorReading> cachedMostRecentSensorReadings;
unordered_map<INT64, SensorPlotContainer> cachedSensorPlotPoints;

bool refreshPlot = false;

int DateTimeFormatter(double value, char* buff, int size, void* /*data*/) {
    // Konwertuj timestamp do czasu
    std::time_t t = static_cast<std::time_t>(value);
    std::tm tm;
    localtime_s(&tm, &t);  // wątko‑bezpieczna wersja localtime

    // Jeśli godzina to 00:00, wyświetl tylko datę (dzień.miesiąc.rok)
    if (tm.tm_hour == 0 && tm.tm_min == 0) {
        return std::strftime(buff, size, "%d.%m.%Y", &tm);
    }
    // W przeciwnym razie, wyświetl godzinę i minutę (godzina:minuta)
    return std::strftime(buff, size, "%H:%M", &tm);
}

// Main code
//int APIENTRY WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hInstPrev, _In_ PSTR cmdline, _In_ int cmdshow)
int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    WeatherApp::App weatherApp;
    comboStations = weatherApp.GetCachedStations();

	sort(comboStations.begin(), comboStations.end(), [](const Station& a, const Station& b) {
		return a.comboLabel < b.comboLabel;
		});

    int i = 0;
    for (const auto& station : comboStations) {
        if (station.id == 225) {
            selectedComboStationIndex = i;
        }
		comboStationEntries.push_back(station.comboLabel.c_str());
        i++;
    }

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, CLASS_NAME, nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, MAIN_WINDOW_TITLE, WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddText("ąćęłńóśźżĄĆĘŁŃÓŚŹŻ");
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.BuildRanges(&ranges);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f, nullptr, ranges.Data);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle lost D3D9 device
        if (g_DeviceLost)
        {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST)
            {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                ResetDevice();
            g_DeviceLost = false;
        }

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        // HERE WINDOW CODE

        ImGuiWindowFlags fullscreenWindowFlags =    ImGuiWindowFlags_NoTitleBar |
                                                    ImGuiWindowFlags_NoResize |
                                                    ImGuiWindowFlags_NoMove |
                                                    ImGuiWindowFlags_NoCollapse |
                                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                                    ImGuiWindowFlags_NoNavFocus |
                                                    ImGuiWindowFlags_NoDocking;

		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

        bool open = true;
        if (ImGui::Begin("FullScreenWindow", &open, fullscreenWindowFlags))
        {
            if (!comboStations.empty()) {

				ImGui::BeginDisabled(isFetching);
                if (ImGui::Combo("Kody dostępnych stacji", &selectedComboStationIndex, comboStationEntries.data(), comboStationEntries.size())) {
                    cachedMostRecentSensorReadings.clear();
                    refreshPlot = true;
                }

                Station selectedStation = comboStations[selectedComboStationIndex];
                ImGui::Text("Wybrana stacja: %s (%d)", selectedStation.kodStacji.c_str(), selectedStation.id);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Text("Sensory:");
                ImGui::Spacing();

                // Lista sensorów
                for (const auto& sensor : selectedStation.sensors) {

                    if (cachedMostRecentSensorReadings.size() != selectedStation.sensors.size()) {
                        cachedMostRecentSensorReadings.insert({ sensor.id, weatherApp.GetLastSensorReading(sensor.id) });
						if (cachedDisplaySensorOnPlot.find(sensor.id) == cachedDisplaySensorOnPlot.end()) {
							cachedDisplaySensorOnPlot.insert({ sensor.id, false });
						}
                    }

                    SensorReading lastCachedReading = cachedMostRecentSensorReadings[sensor.id];

                    string lastReadingMessage = "";

                    if (lastCachedReading.timestamp != 0) {
                        auto currentTimestamp = std::chrono::system_clock::now();
                        auto currentTimestampSec = std::chrono::duration_cast<std::chrono::seconds>(currentTimestamp.time_since_epoch()).count();

                        int hourDiff = (currentTimestampSec - lastCachedReading.timestamp) / 3600;
						lastReadingMessage = format("{} ({}h temu)", lastCachedReading.value, hourDiff);
                    }
                    else {
						lastReadingMessage = "Brak";
                    }

                    ImGui::Text(format("- {} - Ostatni odczyt: {}", sensor.meteredValue, lastReadingMessage).c_str());

                    ImGui::SameLine();
                    if (ImGui::Button(format("Pobierz dane##{}", sensor.id).c_str())) {
                        INT64 idCopy = sensor.id;
                        isFetching = true;
                        thread([&, idCopy]() {
                            json sensorData;
                            cout << selectedComboStationIndex << endl;
                            weatherApp.GetDataForSensorByTimeFrame(idCopy, sensorData, startDate, endDate);
                            cachedMostRecentSensorReadings[idCopy] = weatherApp.GetLastSensorReading(idCopy);
                            isFetching = false;
                            refreshPlot = true;
                            }).detach();
                    }

                    ImGui::SameLine();
                    if (ImGui::Checkbox(format("Pokaż na wykresie##{}", sensor.id).c_str(), &cachedDisplaySensorOnPlot[sensor.id])) {
                        refreshPlot = true;
                    }
                    
                    ImGui::Spacing();
                }

                ImGui::Text("Wybrany zakres dotyczy przedziału czasowego, z którego pobrane zostaną dane po naciśnięciu przycisku \"Pobierz dane\" oraz informacji wyświetlanych na wykresie.");
                if (DateRangeBackwardsSelector(&fromBack, &toBack, startDate, endDate)) {
                    refreshPlot = true;
                }

                
				if (refreshPlot) {
                    cout << "refreshPlot" << endl;
					for (const auto& sensor : selectedStation.sensors) {
						if (cachedDisplaySensorOnPlot[sensor.id]) {
                            cout << sensor.id << endl;
							cachedSensorPlotPoints[sensor.id] = weatherApp.GetPlotPointsForSensorInTimeFrame(sensor.id, startDate, endDate);
						}
					}
					refreshPlot = false;
				}



                ImPlot::BeginPlot("Wykres");
                ImPlot::SetupAxisLimits(ImAxis_X1, WeatherApp::ParseTimestamp(startDate), WeatherApp::ParseTimestamp(endDate), ImGuiCond_Always);
                //ImPlot::SetupAxisFormat(ImAxis_X1, DateTimeFormatter);

                for (const auto& sensor : selectedStation.sensors) {
                    if (cachedDisplaySensorOnPlot[sensor.id]) {
                        SensorPlotContainer container = cachedSensorPlotPoints[sensor.id];
                        if (container.xValues.size() > 0) {
                            ImPlot::PlotStems(sensor.meteredValue.c_str(), container.xValues.data(), container.yValues.data(), container.xValues.size());
                        }
                    }
                }

                ImPlot::EndPlot();

                ImGui::EndDisabled();
            }
        }
        ImGui::End();


        //ImGui::ShowDemoWindow();

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
