#define GLEW_STATIC
#include <GL/glew.h>
#include "Utilities.hpp"
#include "Graphics/Renderer/Texture.h"
#include "Graphics/Renderer/Shader.h"
#include "Graphics/Renderer/VertexArray.h"
#include "Graphics/Renderer/IndexBuffer.h"
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>
#include "stb_image.h"


void StartUp(){
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    GLFWwindow* logo_window = glfwCreateWindow(595, 61*3, "Logo", NULL, NULL);
    if (!logo_window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }

    // Set the logo window as the current context
    glfwMakeContextCurrent(logo_window);
    if(glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;

    // Get the size of the primary monitor
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidmode = glfwGetVideoMode(primary_monitor);
    int monitor_width = vidmode->width;
    int monitor_height = vidmode->height;

    // Get the size of the window
    int window_width, window_height;
    glfwGetWindowSize(logo_window, &window_width, &window_height);

    // Calculate the center position of the screen
    int center_x = (monitor_width - window_width) / 2;
    int center_y = (monitor_height - window_height) / 2;

    // Set the window position to the center position
    glfwSetWindowPos(logo_window, center_x, center_y);

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GLCall(glClearColor(0.3f, 0.5f, 0.4f, 0.0f));

    SapphireRenderer::Shader shader("Shaders/Texture.glsl");
    float Vertices[]{
        -595.0f/2, -61.0f, 0.0f, 0.0f,
        595.0f/2, -61.0f, 1.0f, 0.0f,
        595.0f/2, 61.0f, 1.0f, 1.0f,
        -595.0f/2, 61.0f, 0.0f, 1.0f
    };
    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };
    SapphireRenderer::VertexArray VertexArray;
    SapphireRenderer::VertexBuffer VertexBuffer(4 * 4 * sizeof(float), (GLbyte*)Vertices, GL_STATIC_DRAW);
    SapphireRenderer::IndexBuffer IndexBuffer(6 * sizeof(unsigned int), (GLbyte*)Indices, GL_STATIC_DRAW);
    shader.Bind();
    VertexArray.Bind();
    VertexBuffer.Bind();
    IndexBuffer.Bind();
    SapphireRenderer::VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 2);
    layout.Push(GL_FLOAT, 2);
    VertexArray.AddBuffer(VertexBuffer, layout);

    stbi_set_flip_vertically_on_load(true);

    SapphireRenderer::Texture Texture("Assets/Logo.png");
    VertexArray.Bind();
    GLCall(glActiveTexture(GL_TEXTURE0));
    Texture.Bind();
    shader.SetUniform("u_Color", glm::vec4(0.5f, 0.3f, 0.7f, 1.0f));
    shader.SetUniform("u_Texture", (int)0);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(800/2,600/2,0));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0));
    glm::mat4 proj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    glm::mat4 mvp = proj * view * model;
    shader.SetUniform("u_MVP", 1,GL_FALSE, glm::value_ptr(mvp));

    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    VertexBuffer.Unbind();
    IndexBuffer.Unbind();
    Texture.Unbind();
    VertexArray.Unbind();
    shader.Unbind();

    // Swap buffers to show the logo
    glfwSwapBuffers(logo_window);

    // Wait for some time
    glfwWaitEventsTimeout(2.0);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Close the logo window
    glfwDestroyWindow(logo_window);
}