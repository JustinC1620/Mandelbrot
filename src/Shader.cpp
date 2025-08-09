/********************************************
* File: Shader.cpp
* Author: Justin Collier
* Description: Implementation of Shader class
*     A basic class for using OpenGl shaders
********************************************/
#include <Shader.h>

// Default constructor
// Sets _id to -1, an invalid shader progam id
Shader::Shader()
{
    _id = -1;
}


// Vertex & Fragment Shader Constructor
// Creates a vertex & fragment shader program from their
//     respective source codes.
// Params:
//     vert_path - Path to vertex shader source code as string
//     frag_path - Path to fragment shader source code as string
Shader::Shader(const std::string vert_path, const std::string frag_path)
{
    unsigned int vert_shad = NULL, frag_shad = NULL;

    _id = glCreateProgram();                                  // Create shader program
    compileShader(vert_path, GL_VERTEX_SHADER, vert_shad);    // Compile vertex shader
    compileShader(frag_path, GL_FRAGMENT_SHADER, frag_shad);  // Compile fragment shader
    glAttachShader(_id, vert_shad);                           // Attach vertex shader
    glAttachShader(_id, frag_shad);                           // Attach fragment shader
    linkShaderProgram();                                      // Link shader program
    glDeleteShader(vert_shad); glDeleteShader(frag_shad);     // Delete shaders
}

// Compute Shader Constructor
// Creates a compute shader program from the its source code.
// Params:
//     comp_path - Path to compute shader source code as string
Shader::Shader(const std::string comp_path)
{
    unsigned int comp_shad = NULL;

    _id = glCreateProgram();                                  // Create shader program
    compileShader(comp_path, GL_COMPUTE_SHADER, comp_shad);    // Compile compute shader
    glAttachShader(_id, comp_shad);                           // Attach compute shader
    linkShaderProgram();                                      // Link shader program
    glDeleteShader(comp_shad);                                // Delete shader
}

// getId()
// Returns id of shader program
unsigned int Shader::getId() const { return _id; }


// setBool
// Sets uniform bool for shader
void Shader::setBool(const std::string& name, bool val) const
{
    glUniform1i(glGetUniformLocation(_id, name.c_str()), (int)val);
}


// setInt
// Sets uniform int for shader
void Shader::setInt(const std::string& name, int val) const
{
    glUniform1i(glGetUniformLocation(_id, name.c_str()), val);
}


void Shader::setUIntArr(const std::string& name, GLuint* val, int len) const
{
    glUniform1uiv(glGetUniformLocation(_id, name.c_str()), len, val);
}


// setFloat
// Sets uniform float for shader
void Shader::setFloat(const std::string& name, float val) const
{
    glUniform1f(glGetUniformLocation(_id, name.c_str()), val);
}


void Shader::setFloatArr(const std::string& name, float* val, int len) const
{
    glUniform1fv(glGetUniformLocation(_id, name.c_str()), len, val);
}


// use()
// Sets the shader as the curretly used shader
void Shader::use() { glUseProgram(_id); }


// compileShader()
// Compiles GLSL shader from file name
void Shader::compileShader(std::string f_path, int type, unsigned int& id)
{
    int success;
    const char* src;
    char err_msg[_BUFF_SIZE];
    std::string str;

    readShader(f_path, str);                                // Read in shader file
    src = str.c_str();
    id = glCreateShader(type);                              // Create shader
    glShaderSource(id, 1, &src, NULL);                      // Bind shader source
    glCompileShader(id);                                    // Compile shader

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);         // Checks if compilation was successful
    if (!success)                                           // If compilation failed
    {
        glGetShaderInfoLog(id, _BUFF_SIZE, NULL, err_msg);  // Get error message
        throw ShaderERROR(f_path + "::COMPILATION_FAILED\n" + err_msg);
    }
}


// readShader()
// Reads GLSL files into passed source string
void Shader::readShader(std::string f_path, std::string& src)
{
    std::ifstream ifs;
    std::stringstream ss;
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        ifs.open(f_path);
        ss << ifs.rdbuf();
        ifs.close();
        src = ss.str();
    }
    catch (std::ifstream::failure e)
    {
        throw ShaderERROR(f_path + "::FILE_NOT_SUCCESFULLY_READ");
    }
}


// linkShaderProgram()
// Links shaders to program
void Shader::linkShaderProgram() const
{
    int success;
    char err_msg[_BUFF_SIZE];

    glLinkProgram(_id);                                      // Link shader program
    glGetProgramiv(_id, GL_LINK_STATUS, &success);           // Checks if linking was successful
    if (!success)                                            // If linking failed
    {
        glGetShaderInfoLog(_id, _BUFF_SIZE, NULL, err_msg);  // Gets error message
        throw ShaderERROR("PROGRAM_ID-" + std::to_string(_id) + "::LINKING_FAILED\n" + err_msg);
    }
}