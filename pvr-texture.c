#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#define PVR_HDR_SIZE 0x20

static GLuint PVR_TextureColor(unsigned char *HDR);
static GLuint PVR_TextureFormat(unsigned char *HDR);
static GLuint PVR_TextureHeight(unsigned char *HDR);
static GLuint PVR_TextureWidth(unsigned char *HDR);

// Load a PVR texture file into memory, and then bind the texture to OpenGL.
// `fname` is the name of the PVR texture file to be opened and read.
// `isMipMapped` should be passed as 1 if the texture contains MipMap levels, 0
// otherwise. `glMipMap` should be passed as 1 if OpenGL should calculate the
// MipMap levels, 0 otherwise.
GLuint glTextureLoadPVR(
  char *fname,
  unsigned char isMipMapped,
  unsigned char glMipMap
) {
  uint8 HDR[PVR_HDR_SIZE];
  FILE *tex = NULL;
  uint16 *TEX0 = NULL;
  GLuint texColor;
  GLuint texFormat;
  GLuint texH;
  GLuint texId;
  GLuint texSize;
  GLuint texW;

  // Open the PVR texture file, and get its file size
  tex = fopen(fname, "rb");

  if (tex == NULL) {
    printf("FILE READ ERROR: %s\n", fname);

    while (1);
  }

  fseek(tex, 0, SEEK_END);

  texSize = ftell(tex) - PVR_HDR_SIZE;

  fseek(tex, 0, SEEK_SET);

  // Read the PVR texture file header
  fread(HDR, 1, PVR_HDR_SIZE, tex);

  // Extract some information from the PVR texture file header
  texColor = PVR_TextureColor(HDR);
  texFormat = PVR_TextureFormat(HDR);
  texH = PVR_TextureHeight(HDR);
  texW = PVR_TextureWidth(HDR);

  // Allocate some memory for the texture. If we are using OpenGL to build the
  // MipMap, we need to allocate enough space to hold the MipMap texture levels.
  if (!isMipMapped && glMipMap) {
    TEX0 = malloc(glKosMipMapTexSize(texW, texH));
  } else {
    TEX0 = malloc(texSize);
  }

  // Read in the PVR texture data
  fread(TEX0, 1, texSize, tex);

  // If requested, tell OpenGL to build the MipMap levels for the texture. For
  // now, the input texture to `gluBuild2DMipmaps` must have a width and height
  // divisible by two. Also, the color format is only set by the second to last
  // parameter, here as `texColor`. The color format may be either
  // `PVR_TXRFMT_RGB565`, `PVR_TXRFMT_ARGB1555`, or `PVR_TXRFMT_ARGB4444`.
  if (!isMipMapped && glMipMap) {
    gluBuild2DMipmaps(
      GL_TEXTURE_2D, GL_RGB,
      texW,
      texH,
      GL_RGB,
      texColor,
      TEX0
    );
  }

  // Generate and bind a texture as normal for OpenGL
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);

  if (texFormat & PVR_TXRFMT_VQ_ENABLE) {
    glCompressedTexImage2D(
      GL_TEXTURE_2D,
      (isMipMapped || glMipMap) ? 1 : 0,
      texFormat | texColor,
      texW,
      texH,
      0,
      texSize,
      TEX0
    );
  } else {
    glTexImage2D(
      GL_TEXTURE_2D,
      (isMipMapped || glMipMap) ? 1 : 0,
      GL_RGB,
      texW,
      texH,
      0,
      GL_RGB,
      texFormat | texColor,
      TEX0
    );
  }

  free(TEX0);

  return texId;
}

static GLuint PVR_TextureColor(unsigned char *HDR) {
  switch ((GLuint)HDR[PVR_HDR_SIZE - 8]) {
    case 0x00:
      return PVR_TXRFMT_ARGB1555; // Bi-level translucent alpha
    case 0x01:
      return PVR_TXRFMT_RGB565; // Non-translucent RGB565
    case 0x02:
      return PVR_TXRFMT_ARGB4444; // Translucent alpha
    case 0x03:
      return PVR_TXRFMT_YUV422; // Non-translucent UYVY
    case 0x04:
      return PVR_TXRFMT_BUMP; // Special bump-mapping format
    case 0x05:
      return PVR_TXRFMT_PAL4BPP; // 4-bit palleted texture
    case 0x06:
      return PVR_TXRFMT_PAL8BPP; // 8-bit palleted texture
    default:
      return PVR_TXRFMT_RGB565;
  }
}

static GLuint PVR_TextureFormat(unsigned char *HDR) {
  switch ((GLuint)HDR[PVR_HDR_SIZE - 7]) {
    case 0x01:
      return PVR_TXRFMT_TWIDDLED; // Square twiddled
    case 0x03:
      return PVR_TXRFMT_VQ_ENABLE; // VQ twiddled
    case 0x09:
      return PVR_TXRFMT_NONTWIDDLED; // Rectangle
    case 0x0b:
      return PVR_TXRFMT_STRIDE | PVR_TXRFMT_NONTWIDDLED; // Rectangular stride
    case 0x0d:
      return PVR_TXRFMT_TWIDDLED; // Rectangular twiddled
    case 0x10:
      return PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_NONTWIDDLED; // Small VQ
    default:
      return PVR_TXRFMT_NONE;
  }
}

static GLuint PVR_TextureHeight(unsigned char *HDR) {
  return (GLuint)HDR[PVR_HDR_SIZE - 2] | HDR[PVR_HDR_SIZE - 1] << 8;
}

static GLuint PVR_TextureWidth(unsigned char *HDR) {
  return (GLuint)HDR[PVR_HDR_SIZE - 4] | HDR[PVR_HDR_SIZE - 3] << 8;
}
