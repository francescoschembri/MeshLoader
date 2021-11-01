#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <FileBrowser/ImFileDialog.h>
#include <reskinner/StatusManager.h>

#include <algorithm>
#include <vector>

constexpr int MODEL = 0;
constexpr int CAMERA = 1;
constexpr int ANIMATOR = 2;
constexpr int ANIMATIONS = 3;
constexpr int STATUS = 4;

static bool showModel = true;
static bool showCamera = true;
static bool showAnimator = true;
static bool showRenderInfo = true;

static std::vector<int> panelTex;

void SetupImGui(GLFWwindow* window);
void CloseImGui();
void SetupFileDialog();
void NewGUIFrame();
void RenderGUI(StatusManager& status);
void OpenMeshDialog(StatusManager& status);
void OpenAnimationDialog(StatusManager& status);
void RenderMenuBar(StatusManager& status);
void RenderFileMenuSection(StatusManager& status);
void RenderEditMenuSection(StatusManager& status);
void RenderWindowMenuSection(StatusManager& status);
void RenderModelInfo(StatusManager& status);
void RenderMeshesInfo(StatusManager& status);
void RenderCameraInfo(StatusManager& status);
void RenderMeshTextureInfo(Mesh& mesh, int meshIndex, int textureIndex, TextureManager& texMan);
void ShowTextureInPanel(int m, int textureIndex, ImTextureID id, int width, int height);
void RenderAnimatorInfo(StatusManager& status);
void ShowAnimationNInfo(Animator& animator, int n);
void RenderRenderInfo(StatusManager& status);
//void RenderScenePanel(StatusManager& stauts);