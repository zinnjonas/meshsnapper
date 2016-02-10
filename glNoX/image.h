//
// Created by jonas on 10.12.15.
//

#ifndef TEST_IMAGE_H
#define TEST_IMAGE_H

#include <GL/gl.h>
#include <string>
#include <png.h>

#include "mesh.h"

using namespace std;

enum Direction
{
  FRONT,
  TOP,
  LEFT,
  BACK,
  BOTTOM,
  RIGHT,
  END
};

void dump_image(Direction direction);

bool save_png( const string &name, GLbyte* pixels, GLint w, GLint h );

extern Mesh* mesh;
extern string image_path;

#endif //TEST_IMAGE_H
