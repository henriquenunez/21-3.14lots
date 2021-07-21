#include "song.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <ImGuiFileBrowser.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

#include <iostream>
#include <thread>

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

int SCREEN_HEIGHT = 400;
int SCREEN_WIDTH = 600;
bool viewport_changed;

ImVec4 clear_color = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);

struct ProgState
{
};

bool show_demo_window, rotate_grid, show_open_file_window;

void runUI()
{
        {
            static int counter = 0;

            ImGui::Begin("HeatMAP3D Menu");
            //ImGui::Text("This is some useful text.");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            //ImGui::Checkbox("Another window", &show_another_window);
            ImGui::Checkbox("Rotate Grid", &rotate_grid);
            if (ImGui::Button("Open File")) show_open_file_window = true;
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
	    ImGui::End();
        }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    viewport_changed = true;
    SCREEN_HEIGHT = height; SCREEN_WIDTH = width;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    /*
    fov -= (float)yoffset;
    if (fov < 1.0f)
	fov = 1.0f;
    if (fov > 800.0f)
	fov = 800.0f; */

    //cameraPos += (float)yoffset * 20.0f * cameraFront;
}


void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

GLFWwindow* initGL()
{
    /******************* GL INITIALIZATION  **************/
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    //WINDOW CREATION
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HeatMAP3D", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSwapInterval(0); // 60fps or so i guess

    const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
    const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model

    printf("%s\n", vendor);
    printf("%s\n", renderer);
    //So triagles do not overlap improperly.

    glEnable(GL_DEPTH_TEST);
    /***************  END GL INITIALIZATION  *************/

    //Setting up imgui
    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO(); (void)io;
    // ImGui::StyleColorsDark();
    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    // ImGui_ImplOpenGL3_Init("#version 330 core");

    return window;
}

void run_GA(Population *ref)
{
    std::cout << "Ga thread started\n";

    ref->evaluate();
    ref->elitism();
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_AUDIO);
    GLFWwindow* window = initGL();

    Population a_pop(5);
    //a_pop.playPopulation();
    printf("kappa\n");
    std::thread ga_thread (run_GA, &a_pop);

    // Shader to be used
    shader_err err;
    shader_t* display_shader = load_shader("shaders/vertex.glsl", "shaders/fragment.glsl", &err);

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        use_shader(display_shader);
        a_pop.draw_last_played();
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ga_thread.join();

    return 0;
}

