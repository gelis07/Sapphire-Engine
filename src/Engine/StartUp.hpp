#define GLEW_STATIC
#include <GL/glew.h>
#include "Utilities.hpp"
#include "Graphics/ShaderFunc.h"
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

    unsigned int shader;
    LoadShader(shader, "Shaders/Texture.glsl");
    unsigned int VertexBuffer = 1;
    unsigned int VertexArray = 1;
    unsigned int IndexBuffer = 1;
    GLCall(glUseProgram(shader));
    GLCall(glGenVertexArrays(1, &VertexArray));
    GLCall(glGenBuffers(1, &VertexBuffer));
    GLCall(glGenBuffers(1, &IndexBuffer));

    GLCall(glBindVertexArray(VertexArray));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer));
    float Vertices[]{
        -595.0f/2, -61.0f, 0.0f, 0.0f,
        595.0f/2, -61.0f, 1.0f, 0.0f,
        595.0f/2, 61.0f, 1.0f, 1.0f,
        -595.0f/2, 61.0f, 0.0f, 1.0f
    };
    GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), Vertices, GL_STATIC_DRAW));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (const void*)0));
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (const void*)(sizeof(float) * 2)));

    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };

    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), Indices, GL_STATIC_DRAW));
    unsigned int textureID;
    std::string Filepath = "Assets/Logo.png";
    unsigned char* LocalBuffer;
    int Width, Height, BPP;

    stbi_set_flip_vertically_on_load(1);
    LocalBuffer = stbi_load(Filepath.c_str(), &Width, &Height, &BPP, 4);

    GLCall(glGenTextures(1, &textureID));
    GLCall(glBindTexture(GL_TEXTURE_2D, textureID));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, LocalBuffer));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));


    GLCall(glActiveTexture(GL_TEXTURE0));
    GLCall(glBindTexture(GL_TEXTURE_2D, textureID));

    GLCall(glUniform4f(glGetUniformLocation(shader, "u_Color"), 0.5f, 0.3f, 0.7f, 1.0f));
    GLCall(glUniform1i(glGetUniformLocation(shader, "u_Texture"), 0));
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(800/2,600/2,0));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0));
    glm::mat4 proj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    glm::mat4 mvp = proj * view * model;
    GLCall(glUniformMatrix4fv(glGetUniformLocation(shader, "u_MVP"), 1,GL_FALSE, &mvp[0][0]));

    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    GLCall(glBindVertexArray(0));
    GLCall(glUseProgram(0));

    // Swap buffers to show the logo
    glfwSwapBuffers(logo_window);

    // Wait for some time
    glfwWaitEventsTimeout(2.0);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Close the logo window
    glfwDestroyWindow(logo_window);
    stbi_image_free(LocalBuffer);
}