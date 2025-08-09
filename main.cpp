#include <utils.h>

int main(void)
{
    // GL Variable Declaration
    GLFWwindow* window;
    Shader on_screen, off_screen, comp;
    float last_time = 0.0f;
    int num_frames = 0;
    unsigned int vao = 0, vbo = 0;
    Real32 zoom(4), cntr_x, cntr_y;

    if (!init(_WINDOW_NAME, window)) { return -1; }
    
    std::cout << "Compiling On-Screen Vertex & Fragment Shader" << std::endl;
    try {
        on_screen = Shader(_ON_SCREEN_VERTEX, _ON_SCREEN_FRAGMENT);
    }
    catch (ShaderERROR& e) {
        std::cerr << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "Compiling Off-Screen Vertex & Fragment Shader" << std::endl;
    try {
        off_screen = Shader(_OFF_SCREEN_VERTEX, _OFF_SCREEN_FRAGMENT);
    }
    catch (ShaderERROR& e) {
        std::cerr << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "Compiling Compute Shader" << std::endl;
    try {
        comp = Shader(_COMPUTE_SHADER);
    }
    catch (ShaderERROR& e) {
        std::cerr << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    glGenTextures(1, &MAIN_TEX);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, MAIN_TEX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width * _AA_SCALE, height * _AA_SCALE, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, MAIN_TEX, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glGenTextures(1, &X_TEX);
    glBindTexture(GL_TEXTURE_2D, X_TEX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width * _AA_SCALE, height * _AA_SCALE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);


    glGenFramebuffers(1, &X_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, X_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, X_TEX, 0);

    last_time = static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        processInput(window, zoom, cntr_x, cntr_y);
        
        comp.use();
        glBindTexture(GL_TEXTURE_2D, MAIN_TEX);
        comp.setUIntArr("cntr_x", cntr_x.getReal(), cntr_x.getSize());
        comp.setUIntArr("cntr_y", cntr_y.getReal(), cntr_y.getSize());
        comp.setUIntArr("zoom", zoom.getReal(), zoom.getSize());
        comp.setFloat("time", static_cast<float>(glfwGetTime()));
        glDispatchCompute((unsigned int)(((width * _AA_SCALE) + _GROUP_SIZE - 1) / _GROUP_SIZE), 
            (unsigned int)(((height * _AA_SCALE) + _GROUP_SIZE - 1) / _GROUP_SIZE), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glViewport(0, 0, width * _AA_SCALE, height * _AA_SCALE);
        glBindFramebuffer(GL_FRAMEBUFFER, X_FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        off_screen.use();
        renderTexture(vao, vbo);
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        on_screen.use();
        glBindTexture(GL_TEXTURE_2D, X_TEX);
        renderTexture(vao, vbo);

        glfwSwapBuffers(window);
        debug(num_frames, last_time, zoom);
    }

    glfwTerminate();
    return 0;
}

