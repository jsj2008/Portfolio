// MyMeshTest.cpp
// -- test of triangular mesh derived from Mesh class.
// cs200 9/17
//
// To compile from the Visual Studio command prompt:
//   cl /EHsc MyMeshTest.cpp MyMesh.cpp Affine.lib opengl32.lib
//      glew32.lib SDL2.lib SDL2main.lib /link /subsystem:console
// To compile from a Linux command line:
//   g++ MyMeshTest.cpp MyMesh.cpp Affine.cpp -lGL -lGLEW -lSDL2

#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "MyMesh.h"
using namespace std;


const float PI = 4.0f*atan(1.0f);
const Point O(0,0);


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void resize(int W, int H);
  private:
    GLint ucolor,
          utransform,
          aposition;
    GLuint vertex_buffer,
           edge_buffer,
           face_buffer;
    MyMesh mesh;
    vector<Affine> bg_obj2world;
    Affine fg_obj2world;
    float fg_rotation_rate;
};


Client::Client(void) {
  // compile shaders
  const char *fragment_shader_text =
    "#version 130\n\
     uniform vec3 color;\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = vec4(color,1);\
     }";
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader,1,&fragment_shader_text,0);
  glCompileShader(fshader);

  const char *vertex_shader_text =
    "#version 130\n\
     attribute vec3 position;\
     uniform mat3 transform;\
     void main() {\
       vec3 v = transform * position;\
       gl_Position = vec4(v.x,v.y,0,1);\
     }";
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader,1,&vertex_shader_text,0);
  glCompileShader(vshader);

  GLuint program = glCreateProgram();
  glAttachShader(program,fshader);
  glAttachShader(program,vshader);
  glLinkProgram(program);
  glUseProgram(program);

  // get shader parameter locations
  ucolor = glGetUniformLocation(program,"color");
  utransform = glGetUniformLocation(program,"transform");
  aposition = glGetAttribLocation(program,"position");
  glEnableVertexAttribArray(aposition);

  // upload mesh vertices to GPU
  glGenBuffers(1,&vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER,sizeof(Point)*mesh.vertexCount(),
               &(mesh.vertexArray()[0].x),GL_STATIC_DRAW);

  // upload edge list to GPU
  glGenBuffers(1,&edge_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,edge_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(Mesh::Edge)*mesh.edgeCount(),
               &(mesh.edgeArray()[0].index1),GL_STATIC_DRAW);

  // upload face list to GPU
  glGenBuffers(1,&face_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(Mesh::Face)*mesh.faceCount(),
               &(mesh.faceArray()[0].index1),GL_STATIC_DRAW);

  // background object modeling transformations
  const int COUNT = 8;
  float aspect = mesh.dimensions().x/mesh.dimensions().y,
        dx = (aspect < 1) ? 2.0f/COUNT : 2.0f*aspect/COUNT,
        dy = (aspect < 1) ? 2.0f/COUNT/aspect : 2.0f/COUNT,
        oscale = (aspect < 1) ? 0.95f*dx/mesh.dimensions().x
                              : 0.95f*dy/mesh.dimensions().y;
  for (int i=0; i < COUNT; ++i) {
    for (int j=0; j < COUNT; ++j) {
      float sign_x = (i%2 == 0) ? 1 : -1,
            sign_y = (j%2 == 0) ? 1 : -1;
      Affine obj2world = translate(Vector((i+0.5f)*dx-1,(j+0.5f)*dy-1))
                         * scale(sign_x*oscale,sign_y*oscale)
                         * translate(O-mesh.center());
      bg_obj2world.push_back(obj2world);
    }
  }

  // foreground object modeling transformation (initial)
  fg_rotation_rate = 2*PI/5.0f;
  fg_obj2world = ((aspect < 1) ? scale(1.0f/mesh.dimensions().y)
                               : scale(1.0f/mesh.dimensions().x))
                 * translate(O-mesh.center());
}


Client::~Client(void) {
  glDeleteBuffers(1,&face_buffer);
  glDeleteBuffers(1,&edge_buffer);
  glDeleteBuffers(1,&vertex_buffer);
  GLint program;
  glGetIntegerv(GL_CURRENT_PROGRAM,&program);
  glUseProgram(0);
  GLuint shaders[2];
  GLsizei count;
  glGetAttachedShaders(program,2,&count,shaders);
  for (int i=0; i < count; ++i)
    glDeleteShader(shaders[i]);
  glDeleteProgram(program);
}


void Client::draw(double dt) {
  // clear frame buffer
  glClearColor(1,1,1,1);
  glClear(GL_COLOR_BUFFER_BIT);

  // set vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  glVertexAttribPointer(aposition,3,GL_FLOAT,false,sizeof(Point),0);

  // draw background objects
  for (unsigned i=0; i < bg_obj2world.size(); ++i) {
    // set vertex transformation in shader
    glUniformMatrix3fv(utransform,1,true,&(bg_obj2world[i][0][0]));
    // draw faces
    glUniform3f(ucolor,0.75f,0,1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
    glDrawElements(GL_TRIANGLES,3*mesh.faceCount(),GL_UNSIGNED_INT,0);
    // draw edges
    glUniform3f(ucolor,0,0,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,edge_buffer);
    glDrawElements(GL_LINES,2*mesh.edgeCount(),GL_UNSIGNED_INT,0);
  }

  // draw foreground object
  //   update modeling transform
  fg_obj2world = rotate(fg_rotation_rate*dt) * fg_obj2world;
  //   set vertex transformation in shader
  glUniformMatrix3fv(utransform,1,true,&(fg_obj2world[0][0]));
  //   draw faces
  glUniform3f(ucolor,0,1,0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
  glDrawElements(GL_TRIANGLES,3*mesh.faceCount(),GL_UNSIGNED_INT,0);
  //   draw edges
  glUniform3f(ucolor,0,0,0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,edge_buffer);
  glDrawElements(GL_LINES,2*mesh.edgeCount(),GL_UNSIGNED_INT,0);
}


void Client::resize(int W, int H) {
  int D = min(W,H);
  glViewport(0,0,D,D);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "MyMesh Demo";
  int width = 600,
      height = 600;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GLEW: get function bindings (if possible)
  glewInit();
  if (!GLEW_VERSION_2_0) {
    cout << "needs OpenGL version 2.0 or better" << endl;
    return -1;
  }

  // animation loop
  bool done = false;
  Client *client = new Client();
  Uint32 ticks_last = SDL_GetTicks();
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          done = true;
          break;
        case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_ESCAPE)
            done = true;
          break;
        case SDL_WINDOWEVENT:
          if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            client->resize(event.window.data1,event.window.data2);
          break;
      }
    }
    Uint32 ticks = SDL_GetTicks();
    double dt = 0.001*(ticks - ticks_last);
    ticks_last = ticks;
    client->draw(dt);
    SDL_GL_SwapWindow(window);
  }

  // clean up
  delete client;
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

