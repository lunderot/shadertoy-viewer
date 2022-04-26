#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define CNFGCONTEXTONLY
#include "rawdraw/CNFG.h"
#include "chew.h"

void HandleKey(int keycode, int bDown) {
    if(keycode == CNFG_KEY_ESCAPE)
        exit( 0 );
}
void HandleButton(int x, int y, int button, int bDown) {}
void HandleMotion(int x, int y, int mask) {}
void HandleDestroy() {}

const char *vs = "#version 300 es\n"
                 "in vec2 position;void main(){gl_Position = vec4(position, 0.0, 1.0);}";

const char *fs_end = "\nout vec4 outColor;void main(){mainImage(outColor, gl_FragCoord.xy);}\n";

const char *fs_start = "#version 300 es\n"
                       "precision mediump float;\n"
                       "uniform vec2 iResolution;\n"
                       "uniform float iTime;\n"
                       //"uniform float iTimeDelta;"
                       "uniform float iFrame;\n"
    //"uniform float iChannelTime[4];"
    //"uniform vec4 iMouse;"
    //"uniform vec4 iDate;"
    //"uniform float iSampleRate;"
    //"uniform vec3 iChannelResolution[4];"
    //"uniform samplerXX iChanneli;"
    ;
void checkShaderError(GLuint shader, GLuint flag)
{
    GLint success = 0;
    GLchar error[1024] = {0};

    glGetShaderiv(shader, flag, &success);

    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, sizeof(error), NULL, error);
        puts(error);
    }
}

int main(int argc, char *argv[])
{
    chewInit();
    CNFGSetup("shadertoy-viewer", 256, 256);

    GLchar shader[1024 * 10] = {0};
    strcpy(shader, fs_start);

    FILE *f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    fread(shader + strlen(fs_start), size, 1, f);
    strcpy(shader + strlen(shader), fs_end);
    fclose(f);

    const char *fs = shader;

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat quad[] = {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint elements[] = {0, 1, 2, 2, 3, 0};

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);
    checkShaderError(vertexShader, GL_COMPILE_STATUS);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);
    checkShaderError(fragmentShader, GL_COMPILE_STATUS);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

    GLint uniform_resolution = glGetUniformLocation(shaderProgram, "iResolution");
    GLint uniform_time = glGetUniformLocation(shaderProgram, "iTime");
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);

    unsigned long frame = 0;
    GLint uniform_frame = glGetUniformLocation(shaderProgram, "iFrame");
    while (1)
    {
        short w, h;
        CNFGClearFrame();
        CNFGHandleInput();
        CNFGGetDimensions(&w, &h);

        glUniform2f(uniform_resolution, w, h);

        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        glUniform1f(uniform_time, (now.tv_sec - time.tv_sec) + (now.tv_nsec - time.tv_nsec) / 1E9f);

        frame++;
        glUniform1f(uniform_frame, frame);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        CNFGSwapBuffers();
    }
}
