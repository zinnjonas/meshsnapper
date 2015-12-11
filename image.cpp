//
// Created by jonas on 10.12.15.
//

#include "image.h"

#if IMAGE_MAGICK
#include <Magick++/Image.h>
#include <Magick++/Montage.h>
#include <Magick++/STL.h>

using namespace Magick;
#elif PNG_LIB
#include <png.h>
#endif

void dump_image( Direction direction )
{

  GLint view[4];
  glGetIntegerv( GL_VIEWPORT, view );
  GLint w = view[2];
  GLint h = view[3];

  GLbyte* pixels = new GLbyte[3 * w * h];
  glPixelStorei( GL_PACK_ALIGNMENT, 1 );
  glReadPixels( 0, 0, w, h, GL_RGB, GL_BYTE, pixels );

#if IMAGE_MAGICK
  static list<Image> images;
  for ( int j = 0; j * 2 < h; ++j )
  {
    int x = j * w * 3;
    int y = (h - 1 - j) * w * 3;
    for ( int i = w * 3; i > 0; --i )
    {
      char tmp = pixels[x];
      pixels[x] = pixels[y];
      pixels[y] = tmp;
      x++;
      y++;
    }
  }

  Image image;
  image.read( static_cast<const size_t>(w), static_cast<const size_t>(h), "RGB", CharPixel, pixels );
  image.transparent( "black" );
  image.trim();

  images.push_back(image);

  if( direction == RIGHT)
  {
    Montage m;
    m.backgroundColor("black");
    m.geometry("300x300+2+2");
    m.tile("3x2");
    m.fileName(mesh->get_name()+".png");
    list<Image> mImages;

    montageImages(&mImages, images.begin(), images.end(), m);
    writeImages(mImages.begin(), mImages.end(), m.fileName());
  }
#else
  string name = mesh->get_name();
  name += to_string( direction );
  name += ".png";
  save_png( name, pixels, w, h );
#endif
  delete[] pixels;

}

#if PNG_LIB

bool save_png( const string& name, GLbyte* pixels, GLint w, GLint h )
{
  png_structp png = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
  if ( !png )
    return false;

  png_infop info = png_create_info_struct( png );
  if ( !info )
  {
    png_destroy_write_struct( &png, &info );
    return false;
  }

  FILE* fp = fopen( name.c_str(), "wb" );
  if ( !fp )
  {
    png_destroy_write_struct( &png, &info );
    return false;
  }

  png_init_io( png, fp );
  png_set_IHDR( png, info, static_cast<png_uint_32>(w), static_cast<png_uint_32>(h), 8 /* depth */, PNG_COLOR_TYPE_RGB,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
  png_colorp palette = ( png_colorp ) png_malloc( png, PNG_MAX_PALETTE_LENGTH * sizeof( png_color ));
  if ( !palette )
  {
    fclose( fp );
    png_destroy_write_struct( &png, &info );
    return false;
  }
  png_set_PLTE( png, info, palette, PNG_MAX_PALETTE_LENGTH );
  png_write_info( png, info );
  png_set_packing( png );

  png_bytepp rows = ( png_bytepp ) png_malloc( png, h * sizeof( png_bytep ));
  for ( int i = 0; i < h; ++i )
    rows[i] = ( png_bytep ) (pixels + (h - i - 1) * w * 3);

  png_write_image( png, rows );
  png_write_end( png, info );
  png_free( png, palette );
  png_destroy_write_struct( &png, &info );

  fclose( fp );
  delete[] rows;
  return true;
}
#endif
