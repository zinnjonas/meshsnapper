//
// Created by jonas on 10.12.15.
//

#include <string.h>
#include <fstream>
#include <iostream>
#include <cmath>
#include "mesh.h"

void get_line( ifstream& file, string& line )
{
  while ( !file.eof())
  {
    getline( file, line );
    if ( line.empty() || line.at( 0 ) == '#' || (line.find('\r') != string::npos && line.length() < 3))
    {
      cout << "empty" << endl;
      line.clear();
      continue;
    }
    return;
  }
  throw ios_base::eofbit;
}

Mesh::Mesh()
{

}

Mesh::~Mesh()
{
}

const string& Mesh::get_name() const
{
  return m_name;
}


const int& Mesh::get_vertices() const
{
  return m_vertices;
}

const int& Mesh::get_faces() const
{
  return m_faces;
}

const int& Mesh::get_edges() const
{
  return m_edges;
}

bool Mesh::load( const char* name )
{

  // Set the color of the object
  m_ambient[0] = m_specular[0] = m_diffuse[0] = 0.4f;
  m_ambient[1] = m_specular[1] = m_diffuse[1] = 0.4f;
  m_ambient[2] = m_specular[2] = m_diffuse[2] = 0.4f;
  m_shining = 50.0f;

  if ( strchr( name, '/' ) == nullptr )
    m_name = name;
  else
    m_name = strrchr( name, '/' ) + 1;
  m_name.pop_back(); // pop .off
  m_name.pop_back();
  m_name.pop_back();
  m_name.pop_back();

  size_t idx;
  float max = 0.f;
  Vertex<GLfloat> center;
  center.x = 0;
  center.y = 0;
  center.z = 0;

  ifstream file;
  file.exceptions( ifstream::failbit | ifstream::badbit );
  try
  {
    file.open( name );
    string line;

    // First line must start with an OFF
    get_line( file, line );
    if ( line.find( "OFF" ) == string::npos)
    {
      cerr << "File in wrong format" << endl;
      file.close();
      return false;
    }

    // nextline must contain vertices faces and edges
    get_line( file, line );

    m_vertices = stoi( line, &idx );
    line = line.substr(idx);
    m_faces = stoi( line, &idx );
    line = line.substr(idx);
    m_edges = stoi( line, &idx );

    // read all the verticies
    for( int v = 0; v < m_vertices; v++)
    {
      get_line(file, line);
      Vertex<GLfloat> vert;
      vert.x = stof(line, &idx);
      line = line.substr(idx);
      vert.y = stof(line, &idx);
      line = line.substr(idx);
      vert.z = stof(line);
      m_verts.push_back(vert);

      // take new max value
      max = fmaxf(fabsf(vert.x), max);
      max = fmaxf(fabsf(vert.y), max);
      max = fmaxf(fabsf(vert.z), max);

      // center of gravety
      center.x += vert.x;
      center.y += vert.y;
      center.z += vert.z;
    }

    // read all the faces
    for( int f = 0; f < m_faces; f++)
    {
      get_line(file, line);
      stoi(line, &idx);
      line = line.substr(idx);
      Vertex<int> vert;
      vert.x = stoi(line, &idx);
      line = line.substr(idx);
      vert.y = stoi(line, &idx);
      line = line.substr(idx);
      vert.z = stoi(line);
      m_indicies.push_back(vert);
    }

    file.close();
  } catch ( ifstream::failure e )
  {
    cerr << "Exception: " << e.what() << endl;
    return false;
  }
  catch ( invalid_argument& ia )
  {
    cerr << "Invalid argument: " << ia.what() << endl;
    return false;
  }

  center.x = 1.f/(float)m_vertices * center.x;
  center.y = 1.f/(float)m_vertices * center.y;
  center.z = 1.f/(float)m_vertices * center.z;

  for( auto vt = m_verts.begin(); vt != m_verts.end(); vt++)
  {
    Vertex<GLfloat> diff;
    diff.x = (*vt).x - center.x;
    diff.y = (*vt).y - center.y;
    diff.z = (*vt).z - center.z;

    vt->x = 1.f/max*diff.x;
    vt->y = 1.f/max*diff.y;
    vt->z = 1.f/max*diff.z;
  }

  for( int i = 0; i < m_vertices; i++)
  {
    Vertex<GLfloat> v;
    v.x = 0;
    v.y = 0;
    v.z = 0;
    m_normals.push_back(v);
  }

  for( Vertex<GLint> index : m_indicies)
  {

    int a = index.x;
    int b = index.y;
    int c = index.z;


    Vertex<GLfloat> e0;
    e0.x = m_verts[b].x-m_verts[a].x;
    e0.y = m_verts[b].y-m_verts[a].y;
    e0.z = m_verts[b].z-m_verts[a].z;
    Vertex<GLfloat> e1;
    e1.x = m_verts[c].x-m_verts[a].x;
    e1.y = m_verts[c].y-m_verts[a].y;
    e1.z = m_verts[c].z-m_verts[a].z;
    Vertex<GLfloat> n;
    n.x = e0.y*e1.z - e0.z*e1.y;
    n.y = e0.z*e1.x - e0.x*e1.z;
    n.z = e0.x*e1.y - e0.y*e1.x;
    float norm = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
    if( norm > 0.0001f)
    {
      n.x /= norm;
      n.y /= norm;
      n.z /= norm;
    }
    m_normals[a].x += n.x;
    m_normals[a].y += n.y;
    m_normals[a].z += n.z;
    m_normals[b].x += n.x;
    m_normals[b].y += n.y;
    m_normals[b].z += n.z;
    m_normals[c].x += n.x;
    m_normals[c].y += n.y;
    m_normals[c].z += n.z;
  }

  for(auto nt = m_normals.begin(); nt != m_normals.end(); nt++)
  {
    float norm = sqrt((*nt).x*(*nt).x + (*nt).y*(*nt).y + (*nt).z*(*nt).z);
    if( norm > 0.0001f)
    {
      nt->x /= norm;
      nt->y /= norm;
      nt->z /= norm;
    }
  }

  return true;
}

void Mesh::draw()
{
  glMaterialfv(GL_FRONT, GL_AMBIENT, m_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, m_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, m_specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_shining);
  glBegin(GL_TRIANGLES);
  glColor3f(   1.0,  1.0, 1.0 );
  for( Vertex<GLint> index : m_indicies)
  {
    glNormal3f( m_normals[index.x].x,m_normals[index.x].y,m_normals[index.x].z);
    glVertex3f( m_verts[index.x].x, m_verts[index.x].y, m_verts[index.x].z );
    glNormal3f( m_normals[index.y].x,m_normals[index.y].y,m_normals[index.y].z);
    glVertex3f( m_verts[index.y].x, m_verts[index.y].y, m_verts[index.y].z );
    glNormal3f( m_normals[index.z].x,m_normals[index.z].y,m_normals[index.z].z);
    glVertex3f( m_verts[index.z].x, m_verts[index.z].y, m_verts[index.z].z );
  }
  glEnd();
}
