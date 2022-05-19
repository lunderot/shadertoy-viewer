#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define CNFGOGL
#include "rawdraw/CNFG.h"
#include "chew.h"

void HandleKey(int keycode, int bDown) {
}
void HandleButton(int x, int y, int button, int bDown) {}
void HandleMotion(int x, int y, int mask) {}
void HandleDestroy() {}

const char *vs = "#version 120\n"
                 "attribute vec2 position;void main(){gl_Position = vec4(position, 0.0, 1.0);}";

const char *fs_end = "\nvec4 outColor;void main(){mainImage(outColor, gl_FragCoord.xy);gl_FragColor=outColor;}\n";

const char *fs_start = "#version 120\n"
                       //"precision mediump float;\n"
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

void clearGlError()
{
    while(glGetError() != GL_NO_ERROR);
}

void checkGlError(const char* file, int line)
{
    GLenum err = 0;
    while(err = glGetError())
    {
        printf("%s:%d - %x\n", file, line, err);
    }
}

#define CHECK_OPENGL_ERROR() checkGlError(__FILE__, __LINE__)

static void WriteFile(const char *filename, int gWidth, int gHeight)
{
	FILE *f;
	GLubyte *image;
	int i;

	image = malloc(gWidth * gHeight * 3 * sizeof(GLubyte));
	if(!image)
	{
		printf("Error: couldn't allocate image buffer\n");
		return;
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, gWidth, gHeight, GL_RGB, GL_UNSIGNED_BYTE, image);

	f = fopen(filename, "w");
	if(!f)
	{
		printf("Couldn't open image file: %s\n", filename);
		return;
	}
	fprintf(f,"P6\n");
	fprintf(f,"# ppm-file created by %s\n", "trdemo2");
	fprintf(f,"%i %i\n", gWidth, gHeight);
	fprintf(f,"255\n");
	fclose(f);
	f = fopen(filename, "ab");  /* now append binary data */
	if(!f)
	{
		printf("Couldn't append to image file: %s\n", filename);
		return;
	}

	for(i = 0; i < gHeight; i++)
	{
		GLubyte *rowPtr;
		/* Remember, OpenGL images are bottom to top.  Have to reverse. */
		rowPtr = image + (gHeight - 1 - i) * gWidth * 3;
		fwrite(rowPtr, 1, gWidth * 3, f);
	}

	fclose(f);
	free(image);

	printf("Wrote %d by %d image file: %s\n", gWidth, gHeight, filename);
}

int main(int argc, char *argv[])
{
    chewInit();
    CNFGSetup("shadertoy-viewer", 256, 256);
    clearGlError();

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
    CHECK_OPENGL_ERROR();
    glBindVertexArray(vao);
    CHECK_OPENGL_ERROR();

    GLuint vbo;
    glGenBuffers(1, &vbo);
    CHECK_OPENGL_ERROR();

    GLfloat quad[] = {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    CHECK_OPENGL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    CHECK_OPENGL_ERROR();

    GLuint ebo;
    glGenBuffers(1, &ebo);
    CHECK_OPENGL_ERROR();

    GLuint elements[] = {0, 1, 2, 2, 3, 0};

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    CHECK_OPENGL_ERROR();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    CHECK_OPENGL_ERROR();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    CHECK_OPENGL_ERROR();
    glShaderSource(vertexShader, 1, &vs, NULL);
    CHECK_OPENGL_ERROR();
    glCompileShader(vertexShader);
    CHECK_OPENGL_ERROR();
    checkShaderError(vertexShader, GL_COMPILE_STATUS);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    CHECK_OPENGL_ERROR();
    glShaderSource(fragmentShader, 1, &fs, NULL);
    CHECK_OPENGL_ERROR();
    glCompileShader(fragmentShader);
    CHECK_OPENGL_ERROR();
    checkShaderError(fragmentShader, GL_COMPILE_STATUS);
    CHECK_OPENGL_ERROR();

    GLuint shaderProgram = glCreateProgram();
    CHECK_OPENGL_ERROR();
    glAttachShader(shaderProgram, vertexShader);
    CHECK_OPENGL_ERROR();
    glAttachShader(shaderProgram, fragmentShader);
    CHECK_OPENGL_ERROR();
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    CHECK_OPENGL_ERROR();
    glLinkProgram(shaderProgram);
    CHECK_OPENGL_ERROR();
    glUseProgram(shaderProgram);
    CHECK_OPENGL_ERROR();

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    CHECK_OPENGL_ERROR();
    glEnableVertexAttribArray(posAttrib);
    CHECK_OPENGL_ERROR();
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
    CHECK_OPENGL_ERROR();

    GLint uniform_resolution = glGetUniformLocation(shaderProgram, "iResolution");
    CHECK_OPENGL_ERROR();
    GLint uniform_time = glGetUniformLocation(shaderProgram, "iTime");
    CHECK_OPENGL_ERROR();
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);

    unsigned long frame = 0;
    GLint uniform_frame = glGetUniformLocation(shaderProgram, "iFrame");
    CHECK_OPENGL_ERROR();
    while (1)
    {
        short w, h;
        glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
        CHECK_OPENGL_ERROR();
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECK_OPENGL_ERROR();
        CNFGHandleInput();
        CNFGGetDimensions(&w, &h);

        glUniform2f(uniform_resolution, w, h);
        CHECK_OPENGL_ERROR();

        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        glUniform1f(uniform_time, (now.tv_sec - time.tv_sec) + (now.tv_nsec - time.tv_nsec) / 1E9f);

        frame++;
        glUniform1f(uniform_frame, frame);
        CHECK_OPENGL_ERROR();

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        CHECK_OPENGL_ERROR();
        CNFGSwapBuffers();

        if (frame == 10)
        {
    	    WriteFile("pbuffer.ppm", w, h);
            exit(0);
        }

    }
}
