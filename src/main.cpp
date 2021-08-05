#include "song.hpp"

#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <ImGuiFileBrowser.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

#include <iostream>
#include <string>
#include <thread>

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

int SCREEN_HEIGHT = 640;
int SCREEN_WIDTH = 800;
bool viewport_changed;

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

struct TOP
{
    ImVec4 clear_color = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);

    bool run_ga = true;
    bool show_demo_window, rotate_grid, show_open_file_window;
    bool start_running_ga = false, play_stuff = false;
    bool init_population = false;
    bool can_plot = false;
    bool should_plot = false;
    bool plot_only_best = false;

    int generation_num = 1500;

    // Plot data
    float *x_data = NULL;
    float *y_data = NULL;

    void runUI()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            static int counter = 0;

            ImGui::Begin("Menu");
	    //ImGui::ShowDemoWindow();

            ImGui::InputInt("Generation number", &generation_num);
            ImGui::InputInt("Mutation rate (notes)", &note_mut_span);

	    //ImGui::InputInt("Mutation rate (duration)", &note_mut_span);
            //ImGui::SameLine();

            //ImGui::Text("This is some useful text.");
            //ImGui::Checkbox("Demo Window", &show_demo_window);
            //ImGui::Checkbox("Another window", &show_another_window);
            //if (ImGui::Button("Open File")) show_open_file_window = true;
            if (ImGui::Button("Start Running GA")) start_running_ga = true;
            if (ImGui::Button("Init Population")) init_population = true;
            if (ImGui::Button("Play stuff")) play_stuff = true;

            ImGui::End();
        }

        if (can_plot && should_plot)
        {
	    ImGui::Begin("GA");
            ImGui::Checkbox("Only best", &plot_only_best);

	    if (ImPlot::BeginPlot("Fitness Evolution"))
	    {
		ImPlot::PlotLine("Best Fitness", x_data, y_data, generation_num);
		if (!plot_only_best)
		    for (Song &a : a_pop->songs)
		    {
			std::string b = std::string("Individual: ") + std::to_string(a.id);
			//std::cout << b << "\n";
			ImPlot::PlotLine(b.c_str(), x_data, &a.scores[0], generation_num);
		    }
		ImPlot::EndPlot();
	    }
	    ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
        GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "21-3.14lots", NULL, NULL);

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
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
    
        return window;
    }

    void run_GA(Population *ref, int max_iter)
    {
        int ga_iter_n = 0;
    
        while (ga_iter_n < max_iter)
        {
            std::cout << "GA iter: " << ga_iter_n << "\n";
            ref->evaluate();
            ref->elitism();
            ga_iter_n++;
        }
    }
    
    static void playsong(Population *ref)
    {
        ref->playAll();
    }

    TOP()
    {
    
    }

    Population *a_pop = NULL;

    void run()
    {
	SDL_Init(SDL_INIT_AUDIO);
    	GLFWwindow* window = initGL();

    	int ga_iter_n;

    	//if (argc < 2) ga_iter_n = 50;
    	//else ga_iter_n = atoi(argv[1]);

    	// Shader to be used
    	shader_err err;
    	shader_t* display_shader = load_shader("shaders/vertex.glsl", "shaders/fragment.glsl", &err);

    	srand(time(NULL));
    	Song mysong = Song::read_song("samples/twinkle_twinkle.top");
    	mysong.initGL();

    	std::thread playthread;

	size_t plot_data_aloc_size = 0;

	if (a_pop != NULL) delete [] a_pop;

	a_pop = new Population(20, &mysong);
	a_pop->_shader = display_shader;

    	while(!glfwWindowShouldClose(window))
    	{
    	    processInput(window);

    	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    	    //if (init_population)
    	    //{
    	    //    a_pop = Population(20, mysong);
    	    //    init_population = false;
    	    //}

    	    if (start_running_ga)
    	    {
		if (plot_data_aloc_size != generation_num)
		{
		    if (x_data != NULL && y_data != NULL)
		    {
			delete[] x_data;
			delete[] y_data;
		    }
		    x_data = new float[generation_num];
		    y_data = new float[generation_num];
		    plot_data_aloc_size = generation_num;
		}

		a_pop->reset();
		run_GA(a_pop, generation_num);

    	        // Now, copy data from the population
    	        memcpy(x_data, &a_pop->generation_num[0], sizeof(float) * generation_num);
    	        memcpy(y_data, &a_pop->generation_fitness[0], sizeof(float) * generation_num);

    	        should_plot = can_plot = true;
    	        start_running_ga = false;
    	    }

    	    if (play_stuff)
    	        playthread = std::thread(TOP::playsong, a_pop);
    	    {
    	        play_stuff = false;
    	    }

    	    use_shader(display_shader);
    	    set_uniform_float3(display_shader, 1.0, 1.0, 1.0, "color"); // Just to init

    	    a_pop->draw_last_played();

    	    runUI();
    	    glfwSwapBuffers(window);
    	    glfwPollEvents();
    	}

    	run_ga = false;

    	playthread.join();
    	unload_shader(display_shader);

    	ImGui_ImplOpenGL3_Shutdown();
    	ImGui_ImplGlfw_Shutdown();
    	ImPlot::CreateContext();
    	ImGui::DestroyContext();
    }
};

int main(int argc, char* argv[])
{
    TOP main_logic;

    main_logic.run();

    return 0;
}

