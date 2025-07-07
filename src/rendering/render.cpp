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

static void glfw_error_callback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

std::string gen_random(const int len) {
  static const char alphanum[] =
    "0123456789"
    //"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    //"abcdefghijklmnopqrstuvwxyz";
    ;
  std::string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

bool render::create()
{
  srand((unsigned)time(NULL));

  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
  window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "BinaryHammer", nullptr, nullptr);
  if (window == nullptr)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
  style.FontScaleDpi = main_scale;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

void render::destroy()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

bool render::render()
{
  if (glfwWindowShouldClose(window))
    return false;
   
  glfwPollEvents();
  if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
  {
    ImGui_ImplGlfw_Sleep(10);
    return true;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  static bool docking_enabled = false;
  if (!docking_enabled) {
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    docking_enabled = true;
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
    ImGui::DockBuilderRemoveNode(dockspace_id); // Clear any existing layout
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    ImGuiID dock_left, dock_right;
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, &dock_left, &dock_right);

    ImGui::DockBuilderDockWindow("Function Explorer", dock_left);
    ImGui::DockBuilderDockWindow("Disassembly", dock_right);
    ImGui::DockBuilderFinish(dockspace_id);
  }

  ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_None);
  ImGui::End();

  ImGui::PopStyleVar(2);

  ImGui::Begin("Function Explorer");
  static bool gen_once = false;
  static std::vector<std::string> functions;
  static int index = 0;
  if (!gen_once) {
    for (int i = 0; i < 25; ++i)
      functions.push_back("func_" + gen_random(4));
    gen_once = true;
  }

  for (int idx = 0; idx < functions.size(); ++idx) {
    if (ImGui::Selectable(functions[idx].c_str(), index == idx))
      index = idx;
  }
  ImGui::End();

  ImGui::Begin("Disassembly");
  if (index >= 0 && index < functions.size())
    ImGui::Text("Disassembling Function: %s", functions[index].c_str());
  ImGui::End();

  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(1.f, 1.f, 1.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window);

  return true;
}