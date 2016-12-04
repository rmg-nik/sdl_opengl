#include <stdlib.h>
#include <string>
#include <iostream>
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include "GLProgram.hpp"

using namespace std;

#ifdef main
#undef main
#endif

const int WINDOW_W = 512;
const int WINDOW_H = 512;
const int FPS = 50;

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
    Texture2D texture2D;
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

//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

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
            WINDOW_W, WINDOW_H, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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
            // Positions          // Colors           // Texture Coords
            0.8f,  0.8f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // Top Right
            0.8f, -0.8f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // Bottom Right
            -0.8f, -0.8f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // Bottom Left
            -0.8f,  0.8f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // Top Left 
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

		GLint vertexPosition = glGetAttribLocation(data->shaderProgram.GetProgram(), "position");
		glVertexAttribPointer(vertexPosition, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(vertexPosition);

		GLint vertexColor = glGetAttribLocation(data->shaderProgram.GetProgram(), "color");
		glVertexAttribPointer(vertexColor, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(vertexColor);

        GLint texCoord = glGetAttribLocation(data->shaderProgram.GetProgram(), "texCoord");
        glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(texCoord);

	}
	glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void DrawScene(TutorialData_t* data)
{
    for (auto window: data->mainwindow)
    {
        SDL_GL_MakeCurrent(window, data->maincontext);
        //glViewport(0, 0, 1920, 1080);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        data->shaderProgram.Use();

        glBindTexture(GL_TEXTURE_2D, data->texture2D.textureID);
        glBindVertexArray(data->VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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

bool Idle(TutorialData_t* data)
{
    SDL_Event event;
    bool has_changes = true;

    while (true)
    {
        if (has_changes)
            DrawScene(data);

        has_changes = false;

        if (!SDL_WaitEventTimeout(&event, 20))
            continue;

        switch (event.type)
        {
        case SDL_QUIT:
            return false;
        default:
            has_changes = true;
        }
    }

    return true;
}


int main(int argc, char *argv[])
{
    cout << "START" << endl;

    TutorialData_t data;

    SetupWindow(&data);
	SetupGL(&data);
    data.texture2D = LoadTexture("container.jpg");

    while (Idle(&data));

    DestroyWindow(&data);

    cout << endl << "END" << endl;
    system("pause");

    return 0;
}