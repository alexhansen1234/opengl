#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLSL(src) "#version 330 core\n" #src

int main(int argc, char ** argv)
{
    if(glfwInit() != GL_TRUE)
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    //glfwWindowHint(GLFW_SAMPLES, 4);                // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  //OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS

    GLFWwindow * window = glfwCreateWindow(500, 500, "Tutorial 01", NULL, NULL);

    if(window==NULL)
    {
        fprintf(stderr, "Failed to opne GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); // Initialize GLEW

    glewExperimental = GL_TRUE;        // Need in core profile

    if(glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to init GLEW\n");
        return -1;
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    static const GLfloat vertices[] =
    {
        -0.5f,  0.0f, 0.0f,
         0.5f,  0.0f, 0.0f,
         0.0f,  1.0f, 0.0f
    };

    GLuint vbo;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const char * vertexSource = GLSL(
        in vec3 position;

        void main() {
            gl_Position = vec4(position, 1.0);
        }
    );

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    const char * fragment_source = GLSL(
        
        void main()
        {
            gl_FragColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);
        }
    );

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader);

    int vertex_status, fragment_status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertex_status);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_status);


    if(!vertex_status)
    {
        fprintf(stderr, "Error compiling vertex shader\n");
        return -1;
    }

    if(!fragment_status)
    {
        fprintf(stderr, "Error compiling fragment shader\n");
        return -1;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragment_shader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


    do
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertexShader);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &vao);

    glfwTerminate();

    return 0;
}
