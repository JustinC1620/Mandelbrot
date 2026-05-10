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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);                         // Set OpenGL version to 4.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);                         // Set OpenGL version to 4.6
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);         // Set OpenGl profile to core
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Initialize Window
    window = glfwCreateWindow(_DEFAULT_WIDTH, _DEFAULT_HEIGHT, name.c_str(), NULL, NULL);  // Create window
    if (!window)                                                                           // Check if window creation was successful
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);                                        // Set current context
    glfwSetFramebufferSizeCallback(window, resizeScreen);                  // Set Gl context framebuffer_size_callback function
    glfwSwapInterval(0);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F,    width * _AA_SCALE, height * _AA_SCALE, 0, GL_RED,  GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, X_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width * _AA_SCALE, height * _AA_SCALE, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, Y_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width * _AA_SCALE, height * _AA_SCALE, 0, GL_RGBA, GL_FLOAT, NULL);
}

// buildShader
// Compiles and links shaders, handling any errors
bool buildShader(Shader& shdr, const std::string path1, const std::string path2) {
    try {
        shdr = path2 == "" ? Shader(path1) : Shader(path1, path2);
    }
    catch (ShaderERROR& e) {
        std::cerr << e.what() << std::endl;
        glfwTerminate();
        return false;
    }
    return true;
}

// processInput()
// Closes window upon 'esc' key press.
void processInput(GLFWwindow* window, Real32& zoom, Real32& cntr_x, Real32& cntr_y,
    bool& smooth_color, bool& precision, int& render_mode, float delta_time, bool& ok_lch)
{
    const Real32 _MOVE_SPEED(0.5);
    const float  _IN_SPEED = 2.0;
    const float  _OUT_SPEED = 2.0;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cntr_y += _MOVE_SPEED * zoom * delta_time;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cntr_y -= _MOVE_SPEED * zoom * delta_time;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cntr_x -= _MOVE_SPEED * zoom * delta_time;

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cntr_x += _MOVE_SPEED * zoom * delta_time;

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        zoom *= (1.0 + _OUT_SPEED * delta_time);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        zoom *= (1.0 - _IN_SPEED * delta_time);

    static int prev_s   = GLFW_RELEASE;
    static int prev_p   = GLFW_RELEASE;
    static int prev_l   = GLFW_RELEASE;
    static int prev_1   = GLFW_RELEASE;
    static int prev_2   = GLFW_RELEASE;
    static int prev_3   = GLFW_RELEASE;
    static int prev_4   = GLFW_RELEASE;
    static int prev_5   = GLFW_RELEASE;
    static int prev_esc = GLFW_RELEASE;

    int cur_s   = glfwGetKey(window, GLFW_KEY_S);
    int cur_p   = glfwGetKey(window, GLFW_KEY_P);
    int cur_l   = glfwGetKey(window, GLFW_KEY_L);
    int cur_1   = glfwGetKey(window, GLFW_KEY_1);
    int cur_2   = glfwGetKey(window, GLFW_KEY_2);
    int cur_3   = glfwGetKey(window, GLFW_KEY_3);
    int cur_4   = glfwGetKey(window, GLFW_KEY_4);
    int cur_5   = glfwGetKey(window, GLFW_KEY_5);
    int cur_esc = glfwGetKey(window, GLFW_KEY_ESCAPE);

    if (cur_s   == GLFW_PRESS && prev_s   == GLFW_RELEASE) smooth_color = !smooth_color;
    if (cur_p   == GLFW_PRESS && prev_p   == GLFW_RELEASE) precision    = !precision;
    if (cur_l   == GLFW_PRESS && prev_l   == GLFW_RELEASE) ok_lch       = !ok_lch;
    if (cur_1   == GLFW_PRESS && prev_1   == GLFW_RELEASE) render_mode  = 0;  // shade
    if (cur_2   == GLFW_PRESS && prev_2   == GLFW_RELEASE) render_mode  = 1;  // sobel_mu
    if (cur_3   == GLFW_PRESS && prev_3   == GLFW_RELEASE) render_mode  = 2;  // laplace_mu
    if (cur_4   == GLFW_PRESS && prev_4   == GLFW_RELEASE) render_mode  = 3;  // sobel_rgb
    if (cur_5   == GLFW_PRESS && prev_5   == GLFW_RELEASE) render_mode  = 4;  // laplace_rgb
    if (cur_esc == GLFW_PRESS && prev_esc == GLFW_RELEASE) glfwSetWindowShouldClose(window, true);

    prev_s   = cur_s;
    prev_p   = cur_p;
    prev_l   = cur_l;
    prev_1   = cur_1;
    prev_2   = cur_2;
    prev_3   = cur_3;
    prev_4   = cur_4;
    prev_5   = cur_5;
    prev_esc = cur_esc;
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

void debug(GLFWwindow* window, int& num_frames, float& last_fps_time, const Real32& zoom)
{
    float current_time = static_cast<float>(glfwGetTime());
    num_frames++;

    float delta = current_time - last_fps_time;
    if (delta >= 0.25)
    {
        float fps = num_frames / delta;
        float mspf = delta * 1000.0 / num_frames;

        std::cout << "\rFPS: " << std::setw(6) << (int)fps
            << " | ms/frame: " << std::setw(6) << std::fixed << std::setprecision(2) << mspf
            << " | Zoom: " << std::setw(10) << std::fixed << std::setprecision(4) << (3.0 / zoom.getFloat())
            << "x    "
            << std::flush;

        num_frames = 0;
        last_fps_time = current_time;
    }
}