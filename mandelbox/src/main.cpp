#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/matrix.hpp>

#define GLSL(src) "#version 330 core\n" #src

#define HEIGHT 600.0f
#define WIDTH  800.0f

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

    GLFWwindow * window = glfwCreateWindow(WIDTH, HEIGHT, "Tutorial 01", NULL, NULL);

    if(window==NULL)
    {
        fprintf(stderr, "Failed to open GLFW window\n");
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

    // Our surface quad will be built from two positively oriented triangles
    static const GLfloat vertices[] =
    {
       -1.0f, 1.0f,
       -1.0f,-1.0f,
        1.0f,-1.0f,

        1.0f,-1.0f,
        1.0f, 1.0f,
       -1.0f, 1.0f
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
        uniform vec2 iResolution;
        uniform float iTime;

        out vec4 outColor;

        vec4 background_color = vec4(0.1f, 0.1f, 0.1f, 1.0f);

        vec3 light1 = vec3(-10.f, 0.0f,10.f);

        vec4 boxFold(vec4 z)
        {
            float BOX_FOLD_LEN = 2.0f;
            vec3 temp = z.xyz;
            temp = BOX_FOLD_LEN * clamp(temp, -BOX_FOLD_LEN / 2.0f, BOX_FOLD_LEN / 2.0f) - temp;
            return vec4(temp, z.w);
        }


        vec4 sphereFold(vec4 z)
        {
            float mR2 = 0.25f * cos(iTime) + 0.50f;
            float fR2 = sin(iTime) + 2.00f;
            vec3 temp = z.xyz;
            vec4 ret = z * fR2 / clamp( dot(temp, temp), mR2, fR2);
            return ret;
        }

        float mandelbox(in vec3 z, out vec4 color)
        {
            const int ITERATIONS = 10;
            float MBSCALE = cos(0.05f * iTime) + -3.0f;

        	float w2;
        	vec4 w = vec4(z, 1.0);
            vec4 w0 = w;

        	color = w;

            for(int i=0; i < ITERATIONS; i++)
            {
                w = boxFold(w);
                w = sphereFold(w);
                w = MBSCALE * w + w0;
            	color = min(w, color);
            	w2 = dot(w.xyz, w.xyz);
            	if(w2 > 1000.0) break;
            }
            return 0.25f*sqrt(w2)/abs(w.w);
        }

        float sdTorus( vec3 p, vec2 t )
        {
            vec2 q = vec2(length(p.xz)-t.x,p.y);
            return length(q)-t.y;
        }

        float map(in vec3 p, out vec4 color)
        {
            //vec3 c = vec3(6.0f, 6.0f, 6.0f);
            //p = mod(p, c) - 0.5f * c;
            return mandelbox(p, color);
        }

        vec4 color_ramp(int val, int max_val)
        {
            int red;
            int green;
            int blue;

            int step = int(((float(val) * 1536.0f) / float(max_val)));

            if(step < 256)
            {
                red = 255;
                green = step % 256;
                blue = 0;
            }
            else if (step < 512)
            {
                red = 255 - (step % 256);
                green = 255;
                blue = 0;
            }
            else if (step < 768)
            {
                red = 0;
                green = 255;
                blue = step % 256;
            }
            else if (step < 1024)
            {
                red = 0;
                green = 255 - (step % 256);
                blue = 255;
            }
            else if (step < 1280)
            {
                red = step % 256;
                green = 0;
                blue = 255;
            }
            else
            {
                red = 255;
                green = 0;
                blue = 255 - (step % 256);
            }

            return vec4(float(red)/255.0f, float(green)/255.0f, float(blue)/255.0f, 1.0f);

        }

        vec3 gradient(in vec3 pos)
        {
            const float deltaf = 0.0001f;
            vec3 ret;
            vec2 delta = vec2(deltaf, 0.0f);
            vec4 temp;
            ret.x = map(pos + delta.xyy, temp) - map(pos - delta.xyy, temp);
            ret.y = map(pos + delta.yxy, temp) - map(pos - delta.yxy, temp);
            ret.z = map(pos + delta.yyx, temp) - map(pos - delta.yyx, temp);

            return normalize(ret);
        }

        float raymarch(in vec3 orig, in vec3 dir, out vec3 pos, out vec4 color, in float start,in float end, in int max_iter, out int n_iter)
        {
            float t=start;
            float dist;
            int step;

            pos = orig;

            for(step=0; step<max_iter; step++)
            {
                pos = orig + t*dir;
                dist = map(pos, color);

                if(dist < 0.01f)
                {
                    n_iter = step;
                    return t;
                }

                t += dist;

                if(t > end)
                {
                    break;
                }
            }
            n_iter = max_iter;
            return -1.0f;
        }

        vec4 render(mat4 cam, vec4 fragCoord)
        {
            const int max_iter = 100;
            float fle = 1.0f;
            int n_iter = 0;
            vec2 screen_pos = (-iResolution + 2.0f * fragCoord.xy) / iResolution.y;
            vec3 dir    = normalize((cam * vec4(screen_pos, fle, 0.0f)).xyz);
            vec3 orig   = vec3(cam[0].w, cam[1].w, cam[2].w);
            vec3 normal;
            vec3 pos;
            vec3 reflect;
            vec4 color;
            vec4 trap;
            float dist;

            dist = raymarch(orig, dir, pos, trap, 0.0f, 100.0f, max_iter, n_iter);

            if( dist < 0.0f )
            {
                return background_color;
            }
            else
            {
                return color_ramp(n_iter, max_iter);
                //color = vec4(0.0f, 0.75f, 1.0f, 1.0f);
                //normal = gradient(pos);
                //color = vec4(dot(normalize(light1-pos), normal) * color.xyz, 1.0f);
                //return color;
            }
        }

        mat4 lookat(vec3 eye, vec3 target, vec3 up)
        {
            vec3 z_axis = normalize(target - eye);
            vec3 x_axis = normalize(cross(z_axis, up));
            vec3 y_axis = normalize(cross(x_axis, z_axis));

            mat4 cam = mat4(x_axis, eye.x,
                            y_axis, eye.y,
                            z_axis, eye.z,
                            0.0f, 0.0f, 0.0f, 1.0f);

            return cam;
        }

        void main()
        {
            const int AA=1;
            const float dist = 5.0f;
            mat4 view = lookat( vec3( dist * sin(iTime), dist * sin(0.1f * iTime), dist * cos(iTime)),
                                vec3( 0.0f, 0.0f, 0.0f ),
                                vec3( 0.0f, 1.0f, 0.0f ) );

            if(AA<2)
            {
                outColor = render(view, gl_FragCoord);
            }
            else
            {
                outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
                for(int i=0; i < AA; i++)
                    for(int j=0; j < AA; j++)
                        outColor += render(view, gl_FragCoord + vec4(float(i)/AA, float(j)/AA, 0.0f, 0.0f));

                outColor /= AA*AA;
            }
        }
    );

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader);

    int vertex_status, fragment_status, log_len;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertex_status);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_status);


    if(!vertex_status)
    {
        fprintf(stderr, "Error compiling vertex shader\n");

        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &log_len);
        char * buffer = (char *)malloc(log_len);
        glGetShaderInfoLog(vertexShader, log_len, NULL, buffer);
        fprintf(stderr, "%s\n", buffer);
        free(buffer);
        return -1;
    }

    if(!fragment_status)
    {
        fprintf(stderr, "Error compiling fragment shader\n");
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_len);
        char * buffer = (char *)malloc(log_len);
        glGetShaderInfoLog(fragment_shader, log_len, NULL, buffer);
        fprintf(stderr, "%s\n", buffer);
        free(buffer);
        return -1;
    }


    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragment_shader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    int uniform_iResolution = glGetUniformLocation(shaderProgram, "iResolution");
    glUniform2f(uniform_iResolution, WIDTH, HEIGHT);

    int uniform_iTime = glGetUniformLocation(shaderProgram, "iTime");

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


    do
    {
        glUniform1f(uniform_iTime, (float)glfwGetTime());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 6);

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
