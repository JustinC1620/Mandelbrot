#include <utils.h>

unsigned int X_FBO;
unsigned int Y_FBO;
unsigned int MAIN_TEX;
unsigned int X_TEX;
unsigned int Y_TEX;
int width = _DEFAULT_WIDTH;
int height = _DEFAULT_HEIGHT;

// init()
// Creates a window and sets the current context
bool init(std::string name, GLFWwindow*& window)
{

    // Initialize OpenGL
    glfwInit();                                                            // Init GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);                         // Set OpenGL version to 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);                         // Set OpenGL version to 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);         // Set OpenGl profile to core
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Initialize Window
    window = glfwCreateWindow(_DEFAULT_WIDTH, _DEFAULT_HEIGHT, name.c_str(), NULL, NULL);  // Create window
    if (!window)                                                           // Check if window creattion was successful
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);                                        // Set current context
    glfwSetFramebufferSizeCallback(window, resizeScreen);                  // Set Gl context framebuffer_size_callback function
    //glfwSwapInterval(0);

    // Load function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))               // Check if glad loaded successfully
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return false;
    }
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    return true;
}

// framebuffer_size_callback()
// Resizes the viewport when the window is resized
void resizeScreen(GLFWwindow* window, int new_width, int new_height)
{
    width = new_width;
    height = new_height;
    glBindTexture(GL_TEXTURE_2D, MAIN_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width * _AA_SCALE, height * _AA_SCALE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, X_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width * _AA_SCALE, height * _AA_SCALE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

bool linkShaderProgram(unsigned int vert_shad, unsigned int frag_shad, unsigned int& shad_prog)
{
    int success;
    char err_msg[512];

    shad_prog = glCreateProgram();                           // Create shader program
    glAttachShader(shad_prog, vert_shad);                    // Attach vertex shader
    glAttachShader(shad_prog, frag_shad);                    // Attach fragment shader
    glLinkProgram(shad_prog);                                // Link shader program

    glGetProgramiv(shad_prog, GL_LINK_STATUS, &success);     // Checks if linking was successful
    if (!success)                                            // If linking failed
    {
        glGetShaderInfoLog(shad_prog, 512, NULL, err_msg);   // Gets error message
        std::cout << "ERROR::SHADER:PROGRAM" << shad_prog
            << ":LINKING_FAILED\n" << err_msg << std::endl;
        return false;
    }
    return true;
}

// processInput()
// Closes window upon 'esc' key press.
void processInput(GLFWwindow* window, Real32& zoom, Real32& cntr_x, Real32& cntr_y)
{
    const Real32 _MOVE_SPEED(0.01), _IN_SPEED(1.01), _OUT_SPEED(0.99);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cntr_y += _MOVE_SPEED * zoom;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cntr_y -= _MOVE_SPEED * zoom;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cntr_x -= _MOVE_SPEED * zoom;

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cntr_x += _MOVE_SPEED * zoom;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        zoom *= _OUT_SPEED;

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        zoom *= _IN_SPEED;
}

void renderTexture(unsigned int& vao, unsigned int& vbo)
{
    if (vao == 0)
    {
        float vertices[] = 
        {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void queryWorkGroup(int max_cwg_count[3], int max_cwg_size[3], int& max_cwg_invo) {
    for (int idx = 0; idx < 3; idx++) {
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_cwg_count[idx]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_cwg_size[idx]);
    }
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_cwg_invo);

    std::cout << "OpenGL Limitations: " << std::endl;
    std::cout << "maximum number of work groups in X dimension " << max_cwg_count[0] << std::endl;
    std::cout << "maximum number of work groups in Y dimension " << max_cwg_count[1] << std::endl;
    std::cout << "maximum number of work groups in Z dimension " << max_cwg_count[2] << std::endl;

    std::cout << "maximum size of a work group in X dimension " << max_cwg_size[0] << std::endl;
    std::cout << "maximum size of a work group in Y dimension " << max_cwg_size[1] << std::endl;
    std::cout << "maximum size of a work group in Z dimension " << max_cwg_size[2] << std::endl;

    std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_cwg_invo << std::endl;
}

void debug(int& num_frames, float& last_time, const Real32& zoom)
{
    const float INTERVAL = 5.0f;

    float current_time = static_cast<float>(glfwGetTime());

    num_frames++;
    if (current_time - last_time >= INTERVAL)
    {
        float fps = num_frames / INTERVAL;
        float mspf = INTERVAL * 1000 / num_frames;
        std::cout << "\n"
            << "FPS: " << fps << " with " <<  mspf << " ms / frame\n"
            << "Zoom: " << 1 / zoom.getFloat() << "x" << std::endl;;
        num_frames = 0;
        last_time = current_time;
    }
}