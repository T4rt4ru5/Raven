#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <unordered_map>

#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_glfw.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"

#include "source/Shader.h"
#include "source/Image.h"
#include "source/Texture.h"
#include "source/Controls/Mouse.h"
#include "source/Window.h"
#include "source/Callbacks.h"
#include "source/Callback.h"
#include "source/TypeID.h"

void framebuffer_size_callback1(GLFWwindow* window, int width, int height) {
    printf("Callback function 1: %i, %i\n", width, height);
}
void framebuffer_size_callback1(GLFWwindow* window, int width) {
    printf("Callback function 1: %i\n", width);
}
void framebuffer_size_callback2(GLFWwindow* window, int width, int height) {
    printf("Callback function 2: %i, %i\n", width, height);
}

#define LOG_KEYBOARD
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//  #define LOG_MOUSE_SCROLL
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

#define LOG_MOUSE_BUTTON
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_enter_callback(GLFWwindow* window, int entered);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

struct Vertex {
    glm::vec3 pos;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const {
        return pos == other.pos && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return (hash<glm::vec3>()(vertex.pos) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

void myFunc(int i) {
    printf("my: %i\n", i);
}

int main()
{
    Raven::Window myWindow = {"Engine", 800, 600};
    myWindow.setHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    myWindow.create();
    myWindow.setContextCurrent();

    int xxx = 0;

    Raven::Callbacks callbacks = {};

    callbacks.append<GLFWframebuffersizefun>(Raven::Callbacks::Types::FramebufferSize, [&](GLFWwindow *window, int width, int height){
        printf("XXX: %i, W: %i, H: %i\n", xxx++, width, height);
    });

    callbacks.bind(myWindow.ptr());

//    // Setup callbacks
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//    glfwSetKeyCallback(window, key_callback);
//    glfwSetCursorEnterCallback(window, cursor_enter_callback);
//    //glfwSetCursorPosCallback(window, cursor_position_callback);
//
//    Raven::Mouse mouse = {window};
//
//    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE); // PRESS-HOLD = 1 press, no refresh

    // Bind glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glDepthFunc(GL_LESS);

    // INIT IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(myWindow.ptr(), true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // Load Model
    const char* MODEL_PATH = "assets/objects/viking/viking_room.obj";

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH)) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos[0] = attrib.vertices[3 * index.vertex_index + 0];
            vertex.pos[1] = attrib.vertices[3 * index.vertex_index + 1];
            vertex.pos[2] = attrib.vertices[3 * index.vertex_index + 2];

            vertex.texCoord[0] = attrib.texcoords[2 * index.texcoord_index + 0];
            vertex.texCoord[1] = attrib.texcoords[2 * index.texcoord_index + 1];

//            if (uniqueVertices.count(vertex) == 0) {
//                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
//                vertices.push_back(vertex);
//            }

            indices.push_back(vertices.size());
            vertices.push_back(vertex);
//            indices.push_back(uniqueVertices[vertex]);
        }
    }

    std::cout << indices.size() << std::endl;
    std::cout << vertices.size() << std::endl;


    unsigned int VBO; // Vertex buffer object => rawVertices locations
    unsigned int VAO; // Vertex array object => Relations between GPU buffers
    unsigned int EBO; // Element array buffer => Indexation order of vertex buffer

    // generate vertex array
    glGenVertexArrays(1, &VAO);
    // bind vertex array buffer
    glBindVertexArray(VAO);
    // generate any buffer
    glGenBuffers(1, &VBO);
    // generate any buffer
    glGenBuffers(1, &EBO);


    // bind vertex buffer object and upload data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices.front()), &vertices.data()[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);


    glBindVertexArray(0);

    Raven::Shader s1 = Raven::Shader();
    s1.add("assets/shaders/sources/simple/shader.vert");
    s1.add("assets/shaders/sources/simple/shader.frag");
    s1.build();

    Raven::Shader s2("assets/shaders/sources/simple/shader.vert", "assets/shaders/sources/simple/shader.frag");

    Raven::Texture tex("assets/objects/viking/viking_room.png");

    glActiveTexture(GL_TEXTURE0);
    tex.use();

    // Model
    glm::vec3 translate(0.0f, 0.0f, 0.0f);
    float rotate = 0.0f;
    glm::vec3 scale(1.0f, 1.0f, 1.0f);

    // View
    glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Projection
    float fov = 45;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool isRotating = true;
    bool a = true;
    while (!glfwWindowShouldClose(myWindow.ptr()))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {

            ImGui::Begin("Master Debugger!");

            ImGui::Text("Model Matrix");
            ImGui::InputFloat3("Scale", glm::value_ptr(scale), "%.2f", 0);
            ImGui::InputFloat("Rotate", &rotate, 0.0f, 0.0f, "%.2f", 0);
            ImGui::SameLine();
            ImGui::Checkbox("Auto Rotate", &isRotating);
            ImGui::InputFloat3("Translate", glm::value_ptr(translate), "%.2f", 0);

            ImGui::Text("View Matrix");
            ImGui::InputFloat3("Camera Position", glm::value_ptr(cameraPosition), "%.2f", 0);
            ImGui::InputFloat3("Camera Target", glm::value_ptr(cameraTarget), "%.2f", 0);

            ImGui::Text("Projection Matrix");
            ImGui::DragFloat("FOV", &fov, 1.0f, 0.0f, 180.0f, "%.3f", 0);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        //##############################################################################################################
        glClearColor(0.1f, 0.1f, 0.1f, 0.25f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //##############################################################################################################

        if(isRotating) {
            rotate += 1;
            rotate = static_cast<float>(static_cast<int>(rotate) % 360);
        }

        glm::mat4 model(1.0f);
        model = glm::translate(model, translate);
        model = glm::rotate(model, glm::radians(rotate), up);
        model = glm::scale(model, scale);

        glm::mat4 view = glm::lookAt(cameraPosition, cameraTarget, up);

        glm::mat4 projection = glm::perspective(glm::radians(fov), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);

        glm::mat4 mvp = projection * view * model;

        s2.use();
        s2.set(mvp);
//        s2.set(glm::mat4(1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        //##############################################################################################################
        glfwSwapBuffers(myWindow.ptr());
    }

    // Shutdown system
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    SCR_HEIGHT = height;
    SCR_WIDTH = width;
}

//  Mods:
//      Shift   : 1 -> 0000 0001
//      Ctrl    : 2 -> 0000 0010
//      Alt     : 4 -> 0000 0100
//  Actions:
//      Released    : 0
//      Pressed     : 1
//      Held        : 2
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
#ifdef LOG_KEYBOARD
    printf("KEYBOARD::KEY\n\tKey: %i, Scancode: %i, Action: %i, Mods: %i\n", key, scancode, action, mods);
#endif

    if(key == GLFW_KEY_ESCAPE && action == GLFW_TRUE)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint);

//  UPPER-LEFT = 0,0
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    printf("CURSOR::POSITION\n\tX: %.3f, Y: %.3f\n", xpos, ypos);
}

void cursor_enter_callback(GLFWwindow* window, int entered)
{
    printf("CURSOR::ENTER\n\tEntered: %i\n", entered);
}

void joystick_callback(int jid, int event);
void drop_callback(GLFWwindow* window, int count, const char** paths);
