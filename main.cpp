#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <cstring>
#include <Magick++.h>
#include "image.h"

using namespace std;
using namespace Magick;

string image_path;
bool info = false;
bool grab = false;
bool novis = false;
Direction direction = FRONT;
Mesh* mesh;

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

void set_camera( Direction direction )
{
  glLightfv( GL_LIGHT2, GL_POSITION, new float[4]{0.f, 0.f, 0.f, 1.f} );
  glLightfv( GL_LIGHT2, GL_SPOT_DIRECTION, new float[4]{0.f, 0.f, 1.f, 0.f} );
  switch ( direction )
  {
    default:
    case FRONT:
      gluLookAt( 0.f, 0.f, 3.f,
                 0.f, 0.f, 0.f,
                 0.f, 1.f, 0.f );
      break;

    case BACK:
      gluLookAt( 0.f, 0.f, -3.f,
                 0.f, 0.f, 0.f,
                 0.f, 1.f, 0.f );
      break;

    case TOP:
      gluLookAt( 0.f, -3.f, 0.f,
                 0.f, 0.f, 0.f,
                 0.f, 0.f, 1.f );
      break;

    case BOTTOM:
      gluLookAt( 0.f, 3.f, 0.f,
                 0.f, 0.f, 0.f,
                 0.f, 0.f, -1.f );
      break;

    case LEFT:
      gluLookAt( -3.f, 0.f, 0.f,
                 0.f, 0.f, 0.f,
                 0.f, 1.f, 0.f );
      break;

    case RIGHT:
      gluLookAt( 3.f, 0.f, 0.f,
                 0.f, 0.f, 0.f,
                 0.f, 1.f, 0.f );
      break;
  }
  glLightfv( GL_LIGHT0, GL_POSITION, new float[4]{-3.f, 1.f, 0.f, 1.0f} );
  glLightfv( GL_LIGHT1, GL_POSITION, new float[4]{0.f, 2.f, 4.f, 1.0f} );
}

void display()
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
  gluPerspective( 60.f, view[2] / view[3], 0.1f, 100.f );

  // Do the stuff to the model Matrix
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  set_camera( direction );

  // Draw the mesh
  glPushMatrix();
  glScalef( 1.f, 1.f, 1.f );
  glTranslatef( 0.f, 0.f, 0.f );
  mesh->draw();
  glPopMatrix();

  glutSwapBuffers();

  if ( grab )
  {
    dump_image( direction );
    if ( direction == RIGHT )
      exit( 0 );
    else if ( direction == LEFT )
      direction = BACK;
    else
      direction = Direction( direction + 2 );
  }
}

void repaint()
{
  glutPostRedisplay();
}

int main( int argc, char** argv )
{
  string prog;
  if ( strchr( argv[0], '/' ) == nullptr )
    prog = argv[0];
  else
    prog = strrchr( argv[0], '/' ) + 1;

  // Test for at least one parameter argument
  if ( argc < 2 )
  {
    usage( prog );
    exit( 1 );
  }

  for ( int i = 1; i < argc - 1; i++ )
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
      novis = true;
    else
      usage( prog );
  }

  mesh = new Mesh();

  if ( !mesh->load( argv[argc - 1] ))
    return 1;

  if ( info )
  {
    cout << "vertices: " << mesh->get_vertices() << endl;
    cout << "faces: " << mesh->get_faces() << endl;
    cout << "edges: " << mesh->get_edges() << endl;
  }

  if ( novis )
    exit( 0 );

  if ( grab )
    InitializeMagick( *argv );

  // GLUT environment init
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );

  // set window sizes
  glutInitWindowSize( 800, 600 );
  glutCreateWindow( argv[0] );

  glutDisplayFunc( display );
  glutIdleFunc( repaint );

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
  int i = 0;
  for ( i = 0; i < 3; i++ )
  {
    glLightfv( static_cast<GLenum>(GL_LIGHT0 + i), GL_AMBIENT, lAmbient );
    glLightfv( static_cast<GLenum>(GL_LIGHT0 + i), GL_DIFFUSE, lDiffuse );
    glLightfv( static_cast<GLenum>(GL_LIGHT0 + i), GL_SPECULAR, lSpecular );
  }

  glLightf( GL_LIGHT2, GL_SPOT_CUTOFF, 15 );
  glLightf( GL_LIGHT2, GL_SPOT_EXPONENT, 100 );

  glutMainLoop();

  return 0;
}