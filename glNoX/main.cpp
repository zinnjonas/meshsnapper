#include <iostream>
#include <string>
#include <cstring>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "image.h"
#include "mesh.h"

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = 0;

using namespace std;

Mesh* mesh;
string image_path;
bool grab = false;
bool info = false;
bool windowless = true;

//---- inform user about the usage
void usage( const string& name )
{
  cout << "usage " << name << " [flags] mesh" << endl;
  cout << "\tflags: " << endl;
  cout << "\t\t-grab [path]" << "\tgrabs images of all 6 directions and store them with mesh name in either the working dir or given on" << endl;
  cout << "\t\t-info" << "\tprints some infos about the mesh" << endl;
  cout << "\t\t-novis" << "\texit after loading" << endl;
  cout << "\tmesh:\ta mesh in as off" << endl;
  cout << "\texample:" << endl;
  cout << "\t\t" << name << " -grab -info mesh.off" << endl;
}

int main(int argc, const char* argv[])
{

  static int visual_attribs[] = {
      GLX_DRAWABLE_TYPE   , GLX_PBUFFER_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      None
  };

  int context_attribs[] = {
      GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
      GLX_CONTEXT_MINOR_VERSION_ARB, 0,
      None
  };

  Direction direction = FRONT;
  Display* dpy = XOpenDisplay(0);
  int fbcount = 0;
  GLXFBConfig* fbc = nullptr;
  GLXContext ctx;
  GLXPbuffer pbuf;

  // Get program name
  string prog;
  if ( strchr( argv[0], '/' ) == nullptr )
    prog = argv[0];
  else
    prog = strrchr( argv[0], '/' ) + 1;

  // assign optional parameters
  int i;
  for ( i = 1; i < argc - 1; i++ )
  {
    if ( !strcmp( argv[i], "-info" ))
      info = true;
    else if ( !strcmp( argv[i], "-grab" ))
    {
      grab = true;
      if ( i + 1 != argc - 1 )
      {
        if ( argv[i + 1][0] != '-' )
        {
          i++;
          image_path = argv[i];
        }
        else
        {
          image_path = ".";
        }
      }
    }
    else if ( !strcmp( argv[i], "-novis" ))
      windowless = true;
    else
      usage( prog );
  }

  // Test for at least one parameter argument
  if ( (argc-i) != 1 )
  {
    usage( prog );
    exit( 1 );
  }

  /* open display */
  if ( ! (dpy = XOpenDisplay(0)) ){
    cerr << "Failed to open display" << endl;
    exit(1);
  }

  /* get framebuffer configs, any is usable (might want to add proper attribs) */
  if ( !(fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount) ) ){
    cerr << "Failed to get FBConfig" << endl;
    exit(1);
  }

  /* get the required extensions */
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB");
  glXMakeContextCurrentARB = (glXMakeContextCurrentARBProc)glXGetProcAddressARB( (const GLubyte *) "glXMakeContextCurrent");
  if ( !(glXCreateContextAttribsARB && glXMakeContextCurrentARB) ){
    cerr << "missing support for GLX_ARB_create_context" << endl;
    XFree(fbc);
    exit(1);
  }

  /* create a context using glXCreateContextAttribsARB */
  if ( !( ctx = glXCreateContextAttribsARB(dpy, fbc[0], 0, True, context_attribs)) ){
    cerr << "Failed to create opengl context" << endl;
    XFree(fbc);
    exit(1);
  }

  /* create temporary pbuffer */
  int pbuffer_attribs[] = {
      GLX_PBUFFER_WIDTH, 512,
      GLX_PBUFFER_HEIGHT, 512,
      None
  };
  pbuf = glXCreatePbuffer(dpy, fbc[0], pbuffer_attribs);

  XFree(fbc);
  XSync(dpy, False);

  /* try to make it the current context */
  if ( !glXMakeContextCurrent(dpy, pbuf, pbuf, ctx) ){
    /* some drivers does not support context without default framebuffer, so fallback on
     * using the default window.
     */
    if ( !glXMakeContextCurrent(dpy, DefaultRootWindow(dpy), DefaultRootWindow(dpy), ctx) ){
      cerr << "failed to make current" << endl;
      exit(1);
    }
  }

  glEnable( GL_DEPTH_TEST );
  glShadeModel( GL_SMOOTH );

  glEnable( GL_NORMALIZE );
  glEnable( GL_LIGHTING );
  glEnable( GL_LIGHT0 );
  glEnable( GL_LIGHT1 );
  glEnable( GL_LIGHT2 );

  GLfloat lAmbient[3] = {0.2f, 0.2f, 0.2f};
  GLfloat lDiffuse[3] = {1.0f, 1.0f, 1.0f};
  GLfloat lSpecular[3] = {1.0f, 1.0f, 1.0f};

  for ( i = 0; i < 3; i++ )
  {
    glLightfv( static_cast<GLenum>(GL_LIGHT0 + i), GL_AMBIENT, lAmbient );
    glLightfv( static_cast<GLenum>(GL_LIGHT0 + i), GL_DIFFUSE, lDiffuse );
    glLightfv( static_cast<GLenum>(GL_LIGHT0 + i), GL_SPECULAR, lSpecular );
  }

  glLightf( GL_LIGHT2, GL_SPOT_CUTOFF, 15 );
  glLightf( GL_LIGHT2, GL_SPOT_EXPONENT, 100 );

  /* try it out */
  printf("vendor: %s\n", (const char*)glGetString(GL_VENDOR));

  mesh = new Mesh();
  if( mesh->load(argv[argc - 1]))
  {
      // Print infos if requested via -info flag
      if( info)
      {
        cout << "vertices: " << mesh->get_vertices() << endl;
        cout << "faces: " << mesh->get_faces() << endl;
        cout << "edges: " << mesh->get_edges() << endl;
      }

      while(true)
      {
        GLint view[4];

        // Clear screen
        glClearColor( 0.f, 0.f, 0.f, 0.f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // Do the stuff to the projection Matrix
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        // Set shape of 3d viewspace
        glGetIntegerv( GL_VIEWPORT, view );

        // Do the stuff to the model Matrix
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        // Place lights
        glLightfv( GL_LIGHT2, GL_POSITION, new float[4]{0.f, 0.f, 0.f, 1.f} );
        glLightfv( GL_LIGHT2, GL_SPOT_DIRECTION, new float[4]{0.f, 0.f, 1.f, 0.f} );
        // move camera (usefull gluLookat replacement)
        glLightfv( GL_LIGHT0, GL_POSITION, new float[4]{-3.f, 1.f, 0.f, 1.0f} );
        glLightfv( GL_LIGHT1, GL_POSITION, new float[4]{0.f, 2.f, 4.f, 1.0f} );

        // Draw the mesh
        glPushMatrix();
        glScalef( 0.9f, 0.9f, 0.9f );
        glScalef( 0.9f, 0.9f, 0.9f );

        // We roteate the object instead of the camera, is easy and we only have one object
        switch (direction)
        {
          case FRONT:
            glRotatef(0.0f,0.0f,0.0f,0.0f);
            break;
          case BACK:
            glRotatef(180.0f,0.0f,1.0f,0.0f);
            break;
          case LEFT:
            glRotatef(90.0f,-1.0f,0.0f,0.0f);
            break;
          case RIGHT:
            glRotatef(90.0f,1.0f,0.0f,0.0f);
            break;
          case TOP:
            glRotatef(90.0f,0.0f,-1.0f,0.0f);
            break;
          case BOTTOM:
            glRotatef(90.0f,0.0f,1.0f,0.0f);
            break;
        }

        glTranslatef( 0.f, 0.f, 0.f );
        mesh->draw();
        glPopMatrix();


        dump_image( direction );
        direction = Direction( direction+1);
        if ( (grab && direction == END ) || (!grab && windowless))
        {
          break;
        }
      }
  }


  // clean up
  glXMakeCurrent( dpy, 0, 0 );
  glXDestroyContext( dpy, ctx );

  XCloseDisplay( dpy );

  return 0;
}