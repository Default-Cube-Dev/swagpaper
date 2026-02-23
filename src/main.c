#include "../protocol/wlr-layer-shell-unstable-v1.h"
#include <EGL/egl.h>
#include <stdbool.h>
#define WOIDSHELL_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../include/woidshell.h"
#include "../include/stb_image.h"
#include <GLES2/gl2.h>

void draw(void* data);

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    TexCoord = texCoord;
}
)";

// Fragment shader source code
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D ourTexture;
void main() {
    FragColor = texture(ourTexture, TexCoord);
}
)";

// Function to compile a shader
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::COMPILATION_FAILED\n");
    }
    return shader;
}

// Function to create a shader program
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::PROGRAM::LINKING_FAILED\n");
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Function to load and create a texture
GLuint loadTexture(const char* path) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        return texture;
    } else {
        printf("Failed to load texture");
        return 0;
    }
}

// Function to draw a rectangle with a texture
void drawTexturedRectangle(float x, float y, float w, float h, GLuint texture) {
    // Vertex data for the rectangle
    float vertices[] = {
        x,     y,     0.0f, 0.0f,
        x + w, y,     1.0f, 0.0f,
        x,     y + h, 0.0f, 1.0f,
        x + w, y + h, 1.0f, 1.0f
    };

    // Indices for the rectangle
    unsigned int indices[] = {
        0, 1, 2,
        1, 3, 2
    };

    // VBO and VAO
    GLuint VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);


    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Draw the rectangle
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Clean up
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}


WS_Shell shell = {0};
GLuint wallpaper;
int main(int argc, char *argv[]) {

    WS_ShellSettings settings = {
        .layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND,
        .width = 1920,
        .height = 1080,
        .exclusive_zone = 0,
        .margin_b = 0,
        .margin_t = 0,
        .margin_l = 0,
        .margin_r = 0,
        .anchor = 0,
        .drawfunc = draw,
    };
    shell.settings = &settings;

    WS_ShellInit(&shell);

    glViewport(0, 0, shell.settings->width, shell.settings->height);
    GLuint prog = createShaderProgram();
    glUseProgram(prog);
    stbi_set_flip_vertically_on_load(true);
    wallpaper = loadTexture("/home/btw/.wallpaper/brown_city_planet_w.jpg");

    while (!WS_ShellShouldClose(&shell)) {};

    WS_ShellDestroy(&shell);

    return 0;
}


void draw(void* data) {

    eglMakeCurrent(shell.egl->display, shell.egl->surface, shell.egl->surface, shell.egl->context);
    glClearColor(0.8, 0.8, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawTexturedRectangle(-1.0, -1.0, 2.0, 2.0, wallpaper);


    WS_ExecFrameCallback(&shell);
}
