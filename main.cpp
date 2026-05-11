#include <utils.h>

extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1;
}

int main(void)
{
    // GL Variable Declaration
    GLFWwindow* window;
    Shader on_shdr, post_shdr, mu_shdr, float_shdr, arb_shdr;
    float last_time = static_cast<float>(glfwGetTime());
    float last_fps_time = static_cast<float>(glfwGetTime());
    unsigned int vao = 0, vbo = 0;
    Real32 zoom(3), cntr_x(-0.5), cntr_y(0.0);
    float delta_time = 0.0;
    int num_frames = 0;
    int render_mode = 0;
    bool smooth_color = false;
    bool precision = false;
    bool ok_lch = false;

    if (!init(_WINDOW_NAME, window)) { return -1; }

    std::cout << "Compiling On-Screen Shader" << std::endl;
    if (!buildShader(on_shdr, _ON_SCREEN_VERTEX, _ON_SCREEN_FRAGMENT)) {
        return -1;
    }

    std::cout << "Compiling Mu Shader" << std::endl;
    if (!buildShader(mu_shdr, _OFF_SCREEN_VERTEX, _MU_FRAGMENT)) {
        return -1;
    }

    std::cout << "Compiling Post Shader" << std::endl;
    if (!buildShader(post_shdr, _OFF_SCREEN_VERTEX, _POST_FRAGMENT)) {
        return -1;
    }

    std::cout << "Compiling Float Compute Shader" << std::endl;
    if (!buildShader(float_shdr, _FLOAT_SHADER)) {
        return -1;
    }

    std::cout << "Compiling Arb Compute Shader" << std::endl;
    if (!buildShader(arb_shdr, _ARB_SHADER)) {
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
    glBindImageTexture(0, MAIN_TEX, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glGenTextures(1, &Y_TEX);
    glBindTexture(GL_TEXTURE_2D, Y_TEX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width * _AA_SCALE, height * _AA_SCALE, 0, GL_RGBA, GL_FLOAT, NULL);

    glGenFramebuffers(1, &Y_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, Y_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Y_TEX, 0);

    glGenTextures(1, &X_TEX);
    glBindTexture(GL_TEXTURE_2D, X_TEX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width * _AA_SCALE, height * _AA_SCALE, 0, GL_RGBA, GL_FLOAT, NULL);

    glGenFramebuffers(1, &X_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, X_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, X_TEX, 0);

    while (!glfwWindowShouldClose(window))
    {
        float current_time = static_cast<float>(glfwGetTime());
        delta_time = current_time - last_time;
        last_time = current_time;

        glfwPollEvents();
        processInput(window, zoom, cntr_x, cntr_y, smooth_color, precision, render_mode, delta_time, ok_lch);

        if (precision) {
            arb_shdr.use();
            arb_shdr.setUIntArr("cntr_x", cntr_x.getReal(), cntr_x.getSize());
            arb_shdr.setUIntArr("cntr_y", cntr_y.getReal(), cntr_y.getSize());
            arb_shdr.setUIntArr("zoom", zoom.getReal(), zoom.getSize());
            arb_shdr.setBool("smooth_color", smooth_color);
        }
        else {
            float_shdr.use();
            float_shdr.setFloat("cntr_x", cntr_x.getFloat());
            float_shdr.setFloat("cntr_y", cntr_y.getFloat());
            float_shdr.setFloat("zoom", zoom.getFloat());
            float_shdr.setBool("smooth_color", smooth_color);
        }
        glDispatchCompute((unsigned int)(((width * _AA_SCALE) + _GROUP_SIZE - 1) / _GROUP_SIZE),
            (unsigned int)(((height * _AA_SCALE) + _GROUP_SIZE - 1) / _GROUP_SIZE), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

        glViewport(0, 0, width * _AA_SCALE, height * _AA_SCALE);
        glBindFramebuffer(GL_FRAMEBUFFER, Y_FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mu_shdr.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, MAIN_TEX);
        mu_shdr.setInt("mu_tex", 0);
        mu_shdr.setFloat("time", static_cast<float>(glfwGetTime()));
        mu_shdr.setInt("render_mode", render_mode);
        mu_shdr.setBool("ok_lch", ok_lch);
        renderTexture(vao, vbo);
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, X_FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        post_shdr.use();
        post_shdr.setInt("render_mode", render_mode);
        post_shdr.setFloat("parallax_strength", 0.2);
        post_shdr.setFloat("parallax_scale", 0.1);
        post_shdr.setFloat("zoom", zoom.getFloat());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Y_TEX);
        post_shdr.setInt("color_tex", 0);
        renderTexture(vao, vbo);
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        on_shdr.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, X_TEX);
        renderTexture(vao, vbo);

        glfwSwapBuffers(window);

        debug(window, num_frames, last_fps_time, zoom);
    }

    glfwTerminate();
    return 0;
}