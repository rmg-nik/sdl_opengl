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
#include "Camera.hpp"

using namespace std;

#ifdef main
#undef main
#endif

const int WINDOW_W = 800;
const int WINDOW_H = 600;
const int FPS = 50;

bool KEY_PRESSED_STATUS[1024];

const glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

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
    GLProgram lightShaderProgram;
    GLuint VAO;
    GLuint lightVAO;
	GLuint VBO;
    
    Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
    data->shaderProgram.InitWithFiles("vertex_shade_lighting.vs", "fragment_shader_lighting.frag");
    if (!data->shaderProgram.IsInitialized())
        return SDLDie(data->shaderProgram.GetError());

    data->lightShaderProgram.InitWithFiles("vertex_shade_lighting.vs", "fragment_shader_lighting_lamp.frag");
    if (!data->shaderProgram.IsInitialized())
        return SDLDie(data->shaderProgram.GetError());
    
    GLfloat vertices[] = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

	glGenVertexArrays(1, &data->VAO);
	glBindVertexArray(data->VAO);
	{        
		glGenBuffers(1, &data->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, data->VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		
        const GLint vertexPosition = 0;
		glVertexAttribPointer(vertexPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(vertexPosition);      
	}
	glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &data->lightVAO);
    glBindVertexArray(data->lightVAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, data->VBO);

        const GLint vertexPosition = 0;
        glVertexAttribPointer(vertexPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(vertexPosition);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void DrawScene(TutorialData_t* data)
{
    for (auto window: data->mainwindow)
    {
        SDL_GL_MakeCurrent(window, data->maincontext);
        //glViewport(0, 0, 1920, 1080);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use cooresponding shader when setting uniforms/drawing objects
        data->shaderProgram.Use();
        GLint objectColorLoc = glGetUniformLocation(data->shaderProgram.GetProgram(), "objectColor");
        GLint lightColorLoc = glGetUniformLocation(data->shaderProgram.GetProgram(), "lightColor");
        glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
        glUniform3f(lightColorLoc, 1.0f, 0.5f, 1.0f);

        // Create camera transformations
        glm::mat4 view;
        view = data->camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(data->camera.GetZoom(), (GLfloat)WINDOW_W / (GLfloat)WINDOW_H, 0.1f, 100.0f);
        // Get the uniform locations
        GLint modelLoc = glGetUniformLocation(data->shaderProgram.GetProgram(), "model");
        GLint viewLoc = glGetUniformLocation(data->shaderProgram.GetProgram(), "view");
        GLint projLoc = glGetUniformLocation(data->shaderProgram.GetProgram(), "projection");
        // Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Draw the container (using container's vertex attributes)
        glBindVertexArray(data->VAO);
        glm::mat4 model;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Also draw the lamp object, again binding the appropriate shader
        data->lightShaderProgram.Use();
        // Get location objects for the matrices on the lamp shader (these could be different on a different shader)
        modelLoc = glGetUniformLocation(data->lightShaderProgram.GetProgram(), "model");
        viewLoc = glGetUniformLocation(data->lightShaderProgram.GetProgram(), "view");
        projLoc = glGetUniformLocation(data->lightShaderProgram.GetProgram(), "projection");
        // Set matrices
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        model = glm::mat4();
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        // Draw the light object (using light's vertex attributes)
        glBindVertexArray(data->lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glFlush();
        SDL_GL_SwapWindow(window);
        CheckSDLError();
    }
}

void DestroyWindow(TutorialData_t* data)
{
	glDeleteVertexArrays(1, &data->VAO);
	glDeleteBuffers(1, &data->VBO);
    SDL_GL_DeleteContext(data->maincontext);
    for (auto w: data->mainwindow)
        SDL_DestroyWindow(w);
    SDL_Quit();
}

void DoMovement(TutorialData_t* data)
{
    static auto last_tick = SDL_GetTicks();
    auto current_tick = SDL_GetTicks();
    GLfloat deltaTime = GLfloat(current_tick - last_tick) / 1000.0f;
    
    if (KEY_PRESSED_STATUS[SDLK_w])
    {
        data->camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    }
    if (KEY_PRESSED_STATUS[SDLK_s])
    {
        data->camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    }
    if (KEY_PRESSED_STATUS[SDLK_a])
    {
        data->camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    }
    if (KEY_PRESSED_STATUS[SDLK_d])
    {
        data->camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    }
    last_tick = current_tick;
}

void HandleKeyboard(const SDL_Event& event, TutorialData_t* )
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

void HandleMouse(const SDL_Event& event, TutorialData_t* data)
{
    if (event.type == SDL_MOUSEBUTTONDOWN)
    {

    }
    else if (event.type == SDL_MOUSEBUTTONUP)
    {

    }
    else if (event.type == SDL_MOUSEMOTION)
    {        
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
        data->camera.ProcessMouseMovement(xoffset, yoffset);
    }
    else if (event.type == SDL_MOUSEWHEEL)
    {
        data->camera.ProcessMouseScroll((GLfloat)event.wheel.y);
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
        DoMovement(data);
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
                HandleKeyboard(ev, data);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEWHEEL:
                HandleMouse(ev, data);
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