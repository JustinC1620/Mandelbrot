#pragma once

#define _WINDOW_NAME "Mandelbrot"
#define _ON_SCREEN_VERTEX "src/on.vert"
#define _ON_SCREEN_FRAGMENT "src/on.frag"
#define _OFF_SCREEN_VERTEX "src/off.vert"
#define _OFF_SCREEN_FRAGMENT "src/off.frag"
#define _COMPUTE_SHADER "src/cs.comp"

#include <Real32.h>
#include <Shader.h>
#include <GLFW/glfw3.h> 
#include <cmath>

const int _DEFAULT_WIDTH = 1920;
const int _DEFAULT_HEIGHT = 1080;
const int _AA_SCALE = 2;
const int _GROUP_SIZE = 32;


extern unsigned int X_FBO;
extern unsigned int Y_FBO;
extern unsigned int MAIN_TEX;
extern unsigned int X_TEX;
extern unsigned int Y_TEX;
extern int width;
extern int height;

bool init(std::string w_name, GLFWwindow*& window);
void resizeScreen(GLFWwindow* window, int width, int height);
bool linkShaderProgram(unsigned int vert_shad, unsigned int frag_shad, unsigned int& shad_prog);
void processInput(GLFWwindow* window, Real32& zoom, Real32& cntr_x, Real32& cntr_y);
void renderTexture(unsigned int& vao, unsigned int& vbo);
void queryWorkGroup(int max_cwg_count[3], int max_cwg_size[3], int& max_cwg_invo);
void debug(int& num_frames, float& last_time, const Real32& zoom);
