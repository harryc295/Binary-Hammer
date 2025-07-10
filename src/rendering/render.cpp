#include "render.h"
#include "../../incl/imgui.h"
#include "../../incl/imgui_internal.h"
#include "../../incl/imgui_impl_glfw.h"
#include "../../incl/imgui_impl_opengl3.h"
#include "../../incl/ImGuiFileDialog/ImGuiFileDialog.h"

#include "../binary/binary.h"
#include "../console_handler.h"

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

    ImGuiStyle& style = ImGui::GetStyle();

    static bool openfile_dialog = false;
    float menubar_height = 0.0f;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem("Open", "Ctrl+O")) { openfile_dialog = true; }
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
        menubar_height = ImGui::GetFrameHeight();
    }

    static bool first_run = true;
    if (first_run) {
        ImGui::OpenPopup("Welcome");
        first_run = false;
    }

    // Prevention of late calls to OpenPopup which result in no windows being open
    if (openfile_dialog) {
      ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".exe,.bin,.dll", ".");
      openfile_dialog = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Welcome", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Welcome to BinaryHammer");
        ImGui::Text("Load a binary file to begin analysis");
        ImGui::Separator();

        if (ImGui::Button("Load File", ImVec2(120, 0))) {
          ImGui::CloseCurrentPopup();
          openfile_dialog = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Exit", ImVec2(120, 0))) {
            glfwSetWindowShouldClose(window, true);
        }
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", 0, ImVec2(600, 350), ImVec2(800, 600))) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        open_binary = Binary(ImGuiFileDialog::Instance()->GetFilePathName());
      }
      ImGuiFileDialog::Instance()->Close();
    }
    
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menubar_height));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menubar_height));
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
        ImGuiWindowFlags_NoDocking
    );

    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(viewport->Size.x, viewport->Size.y - menubar_height));

        ImGuiID dock_left, dock_right;
        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, &dock_left, &dock_right);

        ImGuiID dock_left_top, dock_left_bottom;
        ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Up, 0.6f, &dock_left_top, &dock_left_bottom);

        ImGuiID dock_right_top, dock_right_bottom;
        ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Up, 0.7f, &dock_right_top, &dock_right_bottom);

        ImGui::DockBuilderDockWindow("Function Explorer", dock_left_top);
        ImGui::DockBuilderDockWindow("Exports", dock_left_bottom);
        ImGui::DockBuilderDockWindow("Imports", dock_left_bottom);
        ImGui::DockBuilderDockWindow("Disassembly", dock_right_top);
        ImGui::DockBuilderDockWindow("Pseudo Code", dock_right_top);
        ImGui::DockBuilderDockWindow("Hex View", dock_right_top);
        ImGui::DockBuilderDockWindow("Console", dock_right_bottom);
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
    
    std::vector<char> bin = open_binary.get_binary();
    if (!bin.empty()) {
      /*
      * Total 5 spaces
      * Offset = 6 characters + 2 characters because the actual offset is displayed as 8 characters + 3 characters for spacing
      */
      ImGui::Text("Offset     ");
      for (int col = 0x0; col <= 0x0F; ++col) {
        if (col == 8) {
          ImGui::SameLine();
          ImGui::Text("  ");
        }

        ImGui::SameLine();
        ImGui::Text("%02X", col);
      }

      ImGui::SameLine();
      ImGui::Text("   "); // 3 characters for spacing
      ImGui::SameLine();
      ImGui::Text("Text");

      ImGui::Separator();

      int extra_row = 0;
      if (bin.size() % 16)
        extra_row = 1;
      int max_rows = bin.size() / 16 + extra_row;
      for (int row = 0; row < max_rows; ++row) {
        /*
        * Total 3 spaces
        * Offset is always 8 digits + 3 characters for spacing
        */
        ImGui::Text("%08X   ", row * 16);
        std::string resolved = "";
        int empty_columns = 0;
        for (int col = 0; col < 16; ++col) {
          int idx = row * 16 + col;
          if (idx < bin.size()) {
            if (idx == 8) {
              ImGui::SameLine();
              ImGui::Text("  ");
            }

            if (bin[idx] >= 0x20 && bin[idx] <= 0x7E)
              resolved += bin[idx];
            else
              resolved += ".";

            ImGui::SameLine();
            ImGui::Text("%02X", bin[idx]);
            if (col == 7) ImGui::SameLine();
          }
          else {
            auto bak = style.Colors[ImGuiCol_Text];
            style.Colors[ImGuiCol_Text] = ImColor(0.5f, 0.5f, 0.5f);
            
            for (int _ = 0; _ < 16 - col; ++_) {
              ImGui::SameLine();
              ImGui::Text("00");
              resolved += '.';
            }
            style.Colors[ImGuiCol_Text] = bak;
            break;
          }
        }
        ImGui::SameLine();
        ImGui::Text("   "); // 3 characters for spacing
        ImGui::SameLine();
        
        auto bak = style.Colors[ImGuiCol_Text];
        style.Colors[ImGuiCol_Text] = ImColor(0.5f, 0.5f, 0.5f);
        ImGui::Text(resolved.c_str());
        style.Colors[ImGuiCol_Text] = bak;
      }
    } else
      ImGui::Text("The binary is empty.");

    ImGui::End();

    ImGui::Begin("Pseudo Code");
    if (index >= 0 && index < functions.size()) {
        ImGui::Text("int %s(int param) {", functions[index].c_str());
        ImGui::Text("    int result = param + %i;", index);
        ImGui::Text("    return result;");
        ImGui::Text("}");
    }
    ImGui::End();

    ImGui::Begin("Console", 0);
   
    static char buffer[1024]{};
    if (ImGui::InputText("Input (prefix: \".\")", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
      handle_console_command(buffer);
      for (char& c : buffer)
        c = '\0';
    }

    ImGui::Separator();
    for (std::pair<std::string, std::string> message : Logger::get()->get_logs()) {
      ImGui::Text("[%s]: %s", message.first.c_str(), message.second.c_str());
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