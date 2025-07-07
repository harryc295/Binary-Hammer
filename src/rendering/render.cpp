#include "render.h"
#include "../../incl/imgui.h"
#include "../../incl/imgui_internal.h"
#include "../../incl/imgui_impl_glfw.h"
#include "../../incl/imgui_impl_opengl3.h"
#include <cstdio>
#include <string>
#include <ctime>
#include <vector>
#include <GLFW/glfw3.h>

GLFWwindow* window;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

std::string gen_random(const int len) {
    static const char alphanum[] = "0123456789";
    std::string tmp_s;
    tmp_s.reserve(len);
    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return tmp_s;
}

bool render::create() {
    srand((unsigned)time(NULL));
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return false;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "BinaryHammer", nullptr, nullptr);
    if (!window) return false;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    return true;
}

void render::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool render::render() {
    if (glfwWindowShouldClose(window)) return false;

    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
        ImGui_ImplGlfw_Sleep(10);
        return true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) glfwSetWindowShouldClose(window, true);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Function Explorer", nullptr, nullptr);
            ImGui::MenuItem("Disassembly", nullptr, nullptr);
            ImGui::MenuItem("Exports", nullptr, nullptr);
            ImGui::MenuItem("Imports", nullptr, nullptr);
            ImGui::MenuItem("Hex View", nullptr, nullptr);
            ImGui::MenuItem("Pseudo Code", nullptr, nullptr);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    static bool first_run = true;
    if (first_run) {
        ImGui::OpenPopup("Welcome");
        first_run = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Welcome", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Welcome to BinaryHammer");
        ImGui::Text("Load a binary file to begin analysis");
        ImGui::Separator();

        if (ImGui::Button("Load File", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Exit", ImVec2(120, 0))) {
            glfwSetWindowShouldClose(window, true);
        }
        ImGui::EndPopup();
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("MainDockspace", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoDocking);

    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        ImGuiID dock_left, dock_right;
        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, &dock_left, &dock_right);

        ImGuiID dock_left_top, dock_left_bottom;
        ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Up, 0.6f, &dock_left_top, &dock_left_bottom);

        ImGuiID dock_right_top, dock_right_bottom;
        ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Up, 0.5f, &dock_right_top, &dock_right_bottom);

        ImGui::DockBuilderDockWindow("Function Explorer", dock_left_top);
        ImGui::DockBuilderDockWindow("Exports", dock_left_bottom);
        ImGui::DockBuilderDockWindow("Imports", dock_left_bottom);
        ImGui::DockBuilderDockWindow("Disassembly", dock_right_top);
        ImGui::DockBuilderDockWindow("Hex View", dock_right_bottom);
        ImGui::DockBuilderDockWindow("Pseudo Code", dock_right_bottom);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_None);
    ImGui::End();
    ImGui::PopStyleVar(2);

    static bool gen_once = false;
    static std::vector<std::string> functions;
    static int index = 0;
    if (!gen_once) {
        for (int i = 0; i < 25; ++i)
            functions.push_back("func_" + gen_random(4));
        gen_once = true;
    }

    ImGui::Begin("Function Explorer");
    for (int idx = 0; idx < functions.size(); ++idx) {
        if (ImGui::Selectable(functions[idx].c_str(), index == idx))
            index = idx;
    }
    ImGui::End();

    ImGui::Begin("Exports");
    static std::vector<std::string> exports = {
        "ExitProcess", "MessageBoxA", "GetModuleHandleA",
        "LoadLibraryA", "GetProcAddress", "FreeLibrary"
    };
    for (const auto& exp : exports) {
        ImGui::Text("%s", exp.c_str());
    }
    ImGui::End();

    ImGui::Begin("Imports");
    static std::vector<std::string> imports = {
        "kernel32.dll", "user32.dll", "gdi32.dll",
        "advapi32.dll", "comdlg32.dll", "shell32.dll"
    };
    for (const auto& imp : imports) {
        ImGui::Text("%s", imp.c_str());
    }
    ImGui::End();

    ImGui::Begin("Disassembly");
    if (index >= 0 && index < functions.size()) {
        ImGui::Text("Disassembling: %s", functions[index].c_str());
        ImGui::Separator();
        ImGui::Text("0x00000000: push ebp");
        ImGui::Text("0x00000001: mov ebp, esp");
        ImGui::Text("0x00000003: sub esp, 0x10");
        ImGui::Text("0x00000006: mov eax, [ebp+8]");
        ImGui::Text("0x00000009: add eax, 5");
    }
    ImGui::End();

    ImGui::Begin("Hex View");
    static const unsigned char sample_data[64] = {
        0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x10, 0x8B, 0x45,
        0x08, 0x83, 0xC0, 0x05, 0x8B, 0xE5, 0x5D, 0xC3,
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
        0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48,
        0x8B, 0xDA, 0x48, 0x8B, 0xF9, 0x48, 0x8B, 0xD1,
        0x48, 0x8B, 0xCB, 0xE8, 0x00, 0x00, 0x00, 0x00,
        0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x74
    };

    ImGui::Text("Offset     00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F");
    ImGui::Separator();

    for (int row = 0; row < 4; ++row) {
        ImGui::Text("%08X   ", row * 16);
        for (int col = 0; col < 16; ++col) {
            int idx = row * 16 + col;
            if (idx < sizeof(sample_data)) {
                ImGui::SameLine();
                ImGui::Text("%02X ", sample_data[idx]);
                if (col == 7) ImGui::SameLine();
            }
        }
    }
    ImGui::End();

    ImGui::Begin("Pseudo Code");
    if (index >= 0 && index < functions.size()) {
        ImGui::Text("int %s(int param) {", functions[index].c_str());
        ImGui::Text("    int result = param + 5;");
        ImGui::Text("    return result;");
        ImGui::Text("}");
    }
    ImGui::End();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.08f, 0.08f, 0.09f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);

    return true;
}