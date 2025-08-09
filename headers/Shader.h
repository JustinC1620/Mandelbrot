/********************************************
* File: Shader.h
* Author: Justin Collier
* Description: Header of Shader class
*     A basic class for using OpenGl shaders
********************************************/
#pragma once

// OpenGL/glad
#include <glad/glad.h>

// StdLib
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Shader Class
class Shader {
public:
    // Constructors
    Shader();                                                          // Default constructor
    Shader(const std::string vert_path, const std::string frag_path);  // Vertex & fragment shader constructor
    Shader(const std::string comp_path);                               // Compute shader constructor
    // Getters
    unsigned int getId() const;                                        // Get shader id

    // Public Functions
    void setBool(const std::string& name, bool val) const;             // Sets bool uniform
    void setInt(const std::string& name, int val) const;               // Sets int uniform
    void setUIntArr(const std::string& name, GLuint* val, int len) const;  // Sets unsigned int uniform array
    void setFloat(const std::string& name, float val) const;           // Sets float uniform
    void setFloatArr(const std::string& name, float* val, int len) const;  // Sets float uniform array
    void use();                                                        // Activates shader

private:
    // Private Constants
    static const int _BUFF_SIZE = 512;                                 // Size of buffer for errors

    // Private Variables
    unsigned int _id;                                                  // Shader id

    // Private Functions
    static void compileShader(std::string f_name, int type, unsigned int& id);
    static void readShader(std::string f_name, std::string& source);
    void linkShaderProgram() const;

};

// ShaderERROR Class
class ShaderERROR : public std::runtime_error {
private:
    std::string _msg;
public:
    ShaderERROR(const std::string& msg) : std::runtime_error(msg) { _msg = "SHADER_ERROR::" + msg; }
    const char* what() const noexcept override { return _msg.c_str(); }  
};