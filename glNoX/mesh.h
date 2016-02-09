//
// Created by jonas on 10.12.15.
//

#ifndef TEST_MESH_H
#define TEST_MESH_H

#include <string>
#include <GL/gl.h>
#include <vector>
#include <list>

using namespace std;

template <typename T>
struct Vertex
{
  T x;
  T y;
  T z;
};

using namespace std;

class Mesh
{
  public:
    Mesh();
    virtual ~Mesh();


    const string& get_name() const;
    const int& get_vertices() const;
    const int& get_faces() const;
    const int& get_edges() const;

    void draw();

    bool load( const char* name );

  protected:

    string          m_name;
    int             m_vertices;
    int             m_faces;
    int             m_edges;
    vector< Vertex<GLfloat> > m_verts;
    vector< Vertex<GLint> >   m_indicies;
    vector< Vertex<GLfloat> > m_normals;
    GLfloat      m_ambient[3];
    GLfloat      m_diffuse[3];
    GLfloat      m_specular[3];
    GLfloat         m_shining;
};


#endif //TEST_MESH_H
