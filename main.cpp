
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// ==== Vertex structure ====
struct Vertex {
    float x, y, z;
};

// ==== Load STL with Assimp ====
std::vector<Vertex> loadSTL(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

    std::vector<Vertex> vertices;
    if (!scene || !scene->HasMeshes()) {
        std::cerr << "Failed to load STL file\n";
        return vertices;
    }

    const aiMesh* mesh = scene->mMeshes[0];
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        aiVector3D v = mesh->mVertices[i];
        vertices.push_back({ v.x, v.y, v.z });
    }

    return vertices;
}

// ==== Rendering Mode ====
enum RenderMode { SHADED, WIREFRAME, BOTH };

// ==== Draw Model ====
void drawModel(const std::vector<Vertex>& vertices, RenderMode mode) {
    if (mode == SHADED || mode == BOTH) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
        glColor3f(0.8f, 0.8f, 0.8f);
        glBegin(GL_TRIANGLES);
        for (const auto& v : vertices) {
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }

    if (mode == WIREFRAME || mode == BOTH) {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0.1f, 1.0f, 0.1f); // green lines
        glBegin(GL_TRIANGLES);
        for (const auto& v : vertices) {
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // reset
    }
}

// ==== Camera Globals ====
float cameraDistance = 5.0f;
float cameraYaw = 0.0f, cameraPitch = 0.0f;
float lastX = 0.0f, lastY = 0.0f;
bool rotating = false;
glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);

// ==== Input Callbacks ====
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            rotating = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastX = (float)xpos;
            lastY = (float)ypos;
        }
        else if (action == GLFW_RELEASE) {
            rotating = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (rotating) {
        float xoffset = (float)xpos - lastX;
        float yoffset = (float)ypos - lastY;
        lastX = (float)xpos;
        lastY = (float)ypos;

        cameraYaw += xoffset * 0.3f;
        cameraPitch += yoffset * 0.3f;
        cameraPitch = std::max(-89.0f, std::min(89.0f, cameraPitch));
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraDistance -= (float)yoffset * 0.5f;
    cameraDistance = std::max(1.0f, cameraDistance);
}

// ==== Main ====
int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(1280, 960, "STL Viewer", NULL, NULL);
    if (!window) return -1;

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light_pos[] = { 0.0f, 5.0f, 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    std::vector<Vertex> model;
    bool loaded = false;
    RenderMode renderMode = SHADED;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Pan target using WASD
        const float panSpeed = 0.05f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraTarget.z -= panSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraTarget.z += panSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraTarget.x -= panSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraTarget.x += panSpeed;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ==== Menu ====
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load STL")) {
                    model = loadSTL("C:\\Avinash Kale\\M2MCAD\\UnitCollet.stl");
                    loaded = true;
                }
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // ==== UI ====
        ImGui::Begin("STL Viewer");
        if (loaded) {
            ImGui::Text("Model loaded: %zu vertices", model.size());
        }
        else {
            ImGui::Text("No model loaded.");
        }
        ImGui::Combo("Render Mode", (int*)&renderMode, "Shaded\0Wireframe\0Both\0");
        ImGui::Text("Camera Target: (%.2f, %.2f, %.2f)", cameraTarget.x, cameraTarget.y, cameraTarget.z);
        ImGui::End();

        // ==== Clear and Setup ====
        ImGui::Render();
      // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClearColor(0.1f, 0.3f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        float aspect = static_cast<float>(width) / static_cast<float>(height);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&projection[0][0]);

        float yawRad = glm::radians(cameraYaw);
        float pitchRad = glm::radians(cameraPitch);
        glm::vec3 direction = {
            cos(yawRad) * cos(pitchRad),
            sin(pitchRad),
            sin(yawRad) * cos(pitchRad)
        };
        glm::vec3 cameraPos = cameraTarget - direction * cameraDistance;
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&view[0][0]);

        if (loaded) {
            drawModel(model, renderMode);
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
