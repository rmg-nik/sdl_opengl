#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <iterator>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL.h>
#include <SDL_image.h>

#include "GLProgram.hpp"

using namespace std;

#ifdef main
#undef main
#endif

const int WINDOW_W = 800;
const int WINDOW_H = 600;
const int FPS = 50;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
GLfloat FOV = 45.0f;

bool KEY_PRESSED_STATUS[1024];

struct Texture2D
{
    int w{ -1 };
    int h{ -1 };
    GLuint textureID;
};

struct TutorialData_t
{
    SDL_Window* mainwindow[1];
    SDL_Rect window_bounds;
    SDL_GLContext maincontext;    
	GLProgram shaderProgram;
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
    Texture2D texture2D_1;
    Texture2D texture2D_2;    
};

void SDLDie(const std::string& msg)
{
    cout << msg << ": " << SDL_GetError() << endl;
    SDL_Quit();
	system("pause");
    exit(1);
}

void CheckSDLError(int line = -1)
{
#ifndef NDEBUG
    const char *error = SDL_GetError();
    if (*error != '\0')
    {
        cout << "SDL Error: " << error;
        if (line != -1)
            cout << " + line: " << line;
        SDL_ClearError();
        cout << endl;
    }
#endif
}

void CheckGLError(int line = -1)
{
#ifndef NDEBUG
   auto error = glGetError();
    if (error)
    {
        auto er = gluErrorString(error);
        cout << "GL Error: " << er;
        if (line != -1)
            cout << " + line: " << line;
        cout << endl;
    }
#endif
}

Texture2D LoadTexture(const std::string& filename)
{
    auto img_data = IMG_Load(filename.c_str());

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    if (img_data->format->BytesPerPixel == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_data->w, img_data->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data->pixels);
    else if (img_data->format->BytesPerPixel == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_data->w, img_data->h, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data->pixels);
    else
        SDLDie("Not supported image pixel format");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(img_data);

    Texture2D txt2d;
    txt2d.w = img_data->w;
    txt2d.h = img_data->h;
    txt2d.textureID = id;
    return txt2d;
}

void SetupWindow(TutorialData_t* data )
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        SDLDie("Unable to initialize SDL");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    int index = 0;
    for (auto& window: data->mainwindow)
    {
        SDL_Rect window_bounds;
        SDL_GetDisplayBounds(index, &window_bounds);

        window = SDL_CreateWindow("RMG FIRST OGL", 
            window_bounds.x + window_bounds.w / 2 - WINDOW_W / 2,
            window_bounds.y + window_bounds.h / 2 - WINDOW_H / 2,
            WINDOW_W, WINDOW_H, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
        );
        if (!window)
            SDLDie("Unable to create window");
        index++;
    }

    data->maincontext = SDL_GL_CreateContext(data->mainwindow[0]);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        SDLDie((char*)glewGetErrorString(err));
    }
    //SDL_CaptureMouse(SDL_TRUE);
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_ShowCursor(0);
    SDL_WarpMouseInWindow(data->mainwindow[0], WINDOW_W / 2, WINDOW_H / 2);
    SDL_GL_SetSwapInterval(1);    
    
}

void SetupGL(TutorialData_t* data)
{
    data->shaderProgram.InitWithFiles("vertex_shader.vs", "fragment_shader.frag");
    if (!data->shaderProgram.IsInitialized())
        return SDLDie(data->shaderProgram.GetError());
    

	glGenVertexArrays(1, &data->VAO);

	glBindVertexArray(data->VAO);
	{
        GLfloat vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };

		GLuint indices[] = {  // Note that we start from 0!
			0, 1, 3, 
            1, 2, 3
		};

		glGenBuffers(1, &data->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, data->VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &data->EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		//GLint vertexPosition = glGetAttribLocation(data->shaderProgram.GetProgram(), "position");
        const GLint vertexPosition = 0;
		glVertexAttribPointer(vertexPosition, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(vertexPosition);

        //GLint texCoord = glGetAttribLocation(data->shaderProgram.GetProgram(), "texCoord");
        const GLint texCoord = 1;
        glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(texCoord);

        //GLint vertexColor = glGetAttribLocation(data->shaderProgram.GetProgram(), "color");
//      const GLint vertexColor = 2;
// 		glVertexAttribPointer(vertexColor, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
// 		glEnableVertexAttribArray(vertexColor);

	}
	glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void ApplyTransform(TutorialData_t* data, glm::vec3 translate_vec)
{
    glm::mat4 model;    
    model = glm::translate(model, translate_vec);
    model = glm::rotate(model, (GLfloat)(translate_vec.x), glm::vec3(0.5f, 1.0f, 0.0f));
    
    glm::mat4 view;
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(FOV), (GLfloat)WINDOW_W / WINDOW_H, 0.1f, 100.0f);

    GLuint modelLoc = glGetUniformLocation(data->shaderProgram.GetProgram(), "model");
    GLuint viewLoc = glGetUniformLocation(data->shaderProgram.GetProgram(), "view");
    GLuint projectionLoc = glGetUniformLocation(data->shaderProgram.GetProgram(), "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

}

void DrawScene(TutorialData_t* data)
{
    for (auto window: data->mainwindow)
    {
        SDL_GL_MakeCurrent(window, data->maincontext);
        //glViewport(0, 0, 1920, 1080);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        data->shaderProgram.Use();        

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, data->texture2D_1.textureID);
        glUniform1i(glGetUniformLocation(data->shaderProgram.GetProgram(), "ourTexture1"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, data->texture2D_2.textureID);
        glUniform1i(glGetUniformLocation(data->shaderProgram.GetProgram(), "ourTexture2"), 1);

        glBindVertexArray(data->VAO);

        glm::vec3 cubePositions[] = {
            glm::vec3(0.0f,  0.0f,  0.0f),
            glm::vec3(2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f,  2.0f, -2.5f),
            glm::vec3(1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
        };
        for (auto&x : cubePositions)
        {
            ApplyTransform(data, x);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFlush();
        SDL_GL_SwapWindow(window);
        CheckSDLError();
    }
}

void DestroyWindow(TutorialData_t* data)
{
	glDeleteVertexArrays(1, &data->VAO);
	glDeleteBuffers(1, &data->VBO);
	glDeleteBuffers(1, &data->EBO);
    SDL_GL_DeleteContext(data->maincontext);
    for (auto w: data->mainwindow)
        SDL_DestroyWindow(w);
    SDL_Quit();
}

void DoMovement()
{
    static auto last_tick = SDL_GetTicks();
    auto current_tick = SDL_GetTicks();
    GLfloat cameraSpeed = 0.001f * (current_tick - last_tick);
    last_tick = current_tick;
    if (KEY_PRESSED_STATUS[SDLK_w])
    {
        cameraPos += cameraSpeed * glm::normalize(cameraFront);
    }
    if (KEY_PRESSED_STATUS[SDLK_s])
    {
        cameraPos -= cameraSpeed * glm::normalize(cameraFront);
    }
    if (KEY_PRESSED_STATUS[SDLK_a])
    {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (KEY_PRESSED_STATUS[SDLK_d])
    {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

void HandleKeyboard(const SDL_Event& event)
{    
    if (event.type == SDL_KEYDOWN)
    {
        KEY_PRESSED_STATUS[event.key.keysym.sym] = true;
    }
    else if (event.type == SDL_KEYUP)
    {
        KEY_PRESSED_STATUS[event.key.keysym.sym] = false;
    }
}

void HandleMouse(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEBUTTONDOWN)
    {

    }
    else if (event.type == SDL_MOUSEBUTTONUP)
    {

    }
    else if (event.type == SDL_MOUSEMOTION)
    {        
        static GLfloat pitch = 0.0f;
        static GLfloat yaw = -90.0f;
        static GLfloat lastX, lastY;
        static bool firstMouse = true;

        GLfloat xpos = (GLfloat)event.motion.x;
        GLfloat ypos = (GLfloat)event.motion.y;
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        GLfloat xoffset = xpos - lastX;
        GLfloat yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        GLfloat sensitivity = 0.05f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

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
    else if (event.type == SDL_MOUSEWHEEL)
    {
        FOV -= 1.5f * (GLfloat)event.wheel.y;
        if (FOV <= 1.0f)
            FOV = 1.0f;
        if (FOV >= 90.0f)
            FOV = 90.0f;
    }

}

bool Idle(TutorialData_t* data)
{
    SDL_Event event;
    const int MAX_EVENTS = 100;
    SDL_Event events_array[MAX_EVENTS + 1];

    bool has_changes = true;

    while (true)
    {
        DoMovement();
        if (has_changes)
        {
            DrawScene(data);
        }

        //has_changes = false;

        if (!SDL_WaitEventTimeout(&event, 10))
            continue;

        int count = 0;
        events_array[0] = event;
        count = SDL_PeepEvents(events_array + 1, MAX_EVENTS, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

        for (int i = 0; i < count + 1; ++i)
        {
            auto& ev = events_array[i];
            switch (ev.type)
            {
            case SDL_QUIT:
                return false;
                //TODO: process keys and mouse in separate methods
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                {
                    if (ev.key.keysym.sym == SDLK_ESCAPE)
                        return false;
                }
                HandleKeyboard(ev);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEWHEEL:
                HandleMouse(ev);
                break;
            default:
                break;
            }
        }        
    }

    return true;
}

void main_function(int argc, char* argv[])
{
    TutorialData_t data;

    SetupWindow(&data);
    SetupGL(&data);
    data.texture2D_1 = LoadTexture("container.jpg");
    data.texture2D_2 = LoadTexture("awesomeface.png");

    while (Idle(&data));

    DestroyWindow(&data);

    return;
}

void test_function(int argc, char* argv[])
{

    return;
}

int main(int argc, char *argv[])
{
    cout << "START" << endl;

    main_function(argc, argv);
    //test_function(argc, argv);

    cout << endl << "END" << endl;
    system("pause");

    return 0;
}