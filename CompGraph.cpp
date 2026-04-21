#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define GLHW_DDL
#define GLEW_DDL

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <iostream>

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 256.0f;
float lastY = 256.0f;
bool firstMouse = true;
float mouseSensitivity = 0.1f;

float cameraSpeed = 0.005f;

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

int main()
{
    std::cout << "Hello triangle with camera!\n";
    std::cout << "Controls: WASD - move, Mouse - look around, ESC - exit\n";

    glfwInit();

    if (!glfwInit()) {
        fprintf(stderr, "ERROR GLFW3:\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* MyWindow = glfwCreateWindow(512, 512, "My window - Camera Lab4", NULL, NULL);

    if (!MyWindow) {
        glfwTerminate();
        return 1;
    }

    glewExperimental = GL_TRUE;
    glfwMakeContextCurrent(MyWindow);

    glfwSetCursorPosCallback(MyWindow, mouse_callback);
    glfwSetInputMode(MyWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLenum ret = glewInit();
    if (GLEW_OK != ret) {
        fprintf(stderr, "ERROR: %s \n", glewGetErrorString(ret));
        return 1;
    }


    float point[] = {
        0.0f,  0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    GLuint vAo, vBo;
    glGenVertexArrays(1, &vAo);
    glGenBuffers(1, &vBo);

    glBindVertexArray(vAo);
    glBindBuffer(GL_ARRAY_BUFFER, vBo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    const char* vert_shader =
        "#version 460 \n"
        "layout (location=0) in vec3 vp;"
        "uniform mat4 model;"
        "uniform mat4 view;"
        "uniform mat4 projection;"
        "void main() {"
        " gl_Position = projection * view * model * vec4(vp, 1.0);"
        "}";

    const char* frag_shader =
        "#version 460 \n"
        "uniform vec4 ourColour;"
        "out vec4 frag_colour;"
        "void main() {"
        " frag_colour = ourColour;"
        "}";

    GLuint vesrts = glCreateShader(GL_VERTEX_SHADER);
    GLuint frags = glCreateShader(GL_FRAGMENT_SHADER);
    GLint success;
    GLchar infoLog[1024];

    glShaderSource(vesrts, 1, &vert_shader, NULL);
    glShaderSource(frags, 1, &frag_shader, NULL);

    glCompileShader(vesrts);
    glGetShaderiv(vesrts, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vesrts, sizeof(infoLog), 0, infoLog);
        fprintf(stderr, "VERTEX SHADER ERROR: %s \n", infoLog);
    }

    glCompileShader(frags);
    glGetShaderiv(frags, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(frags, sizeof(infoLog), 0, infoLog);
        fprintf(stderr, "FRAGMENT SHADER ERROR: %s \n", infoLog);
    }

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vesrts);
    glAttachShader(shader_program, frags);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, sizeof(infoLog), 0, infoLog);
        fprintf(stderr, "SHADER LINK ERROR: %s \n", infoLog);
    }

    glEnable(GL_DEPTH_TEST);

    int modelLoc = glGetUniformLocation(shader_program, "model");
    int viewLoc = glGetUniformLocation(shader_program, "view");
    int projLoc = glGetUniformLocation(shader_program, "projection");
    int colorLoc = glGetUniformLocation(shader_program, "ourColour");

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(MyWindow)) {
        processInput(MyWindow);

        glfwPollEvents();

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glUseProgram(shader_program);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);

        glBindVertexArray(vAo);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(MyWindow);
    }

    glDeleteVertexArrays(1, &vAo);
    glDeleteBuffers(1, &vBo);
    glDeleteProgram(shader_program);
    glDeleteShader(vesrts);
    glDeleteShader(frags);

    glfwTerminate();
    return 0;
}