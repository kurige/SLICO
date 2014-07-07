
#include <iostream>
#include <inttypes.h>
#include <IL/il.h>

#include "SLIC.h"

const int superpixel_count = 2000;
const double compactness = 5.0;

int main(int argc, char **argv) {
  // Initialize DevIL library
  ilInit();

  ILuint ImgId = 0;
  {
    ilGenImages(1, &ImgId);
    ilBindImage(ImgId);

    // Load image
    ILboolean b = ilLoadImage(argv[1]);

    if (b) {
      std::cout << "Image loaded successfully" << std::endl;
    } else {
      std::cout << "Failed to load image" << std::endl;
      return 0;
    }

    ILenum error = ilGetError();
    if (error) {
      // const char *errStr = iluErrorString(error);
      std::cout << "Error: " << error << std::endl;
      return 0;
    }
  }

  // Get image width and height
  ILuint width, height, format;
  width = ilGetInteger(IL_IMAGE_WIDTH);
  height = ilGetInteger(IL_IMAGE_HEIGHT);
  format = ilGetInteger(IL_IMAGE_FORMAT);

  std::cout << "Width: " << width << std::endl;
  std::cout << "Height: " << height << std::endl;
  std::cout << "Format: " << format << std::endl;

  // Copy out pixel data
  unsigned char *data = new unsigned char[width * height * 3];
  ILuint copied = ilCopyPixels(0, 0, 0,          // X, Y, and Z offsets
                               width,            // Width
                               height,           // Height
                               1,                // Depth
                               IL_RGB,           // Format
                               IL_UNSIGNED_BYTE, // Data type
                               data);

  ILenum error = ilGetError();
  if (error) {
    // const char *errStr = iluErrorString(error);
    std::cout << "Error: " << error << std::endl;
    return 0;
  }

  // Convert array of r, g, b unsigned bytes from DevIL to array of ARGB unsigned ints for SLICO
  uint32_t *int_data = new uint32_t[width * height];
  for (int i = 0; i < width * height; i++) {
    unsigned char r, g, b;
    r = data[i * 3];
    g = data[i * 3 + 1];
    b = data[i * 3 + 2];
    int_data[i] = 0xff | (r << 8) | (g << 16) | (b << 24);
  }

  // Perform SLICO
  {
    int sz = width*height;
    if(superpixel_count > sz) {
      std::cout << "Number of superpixels exceeds number of pixels in the image" << std::endl;
      return 0;
    }

    int* labels = new int[sz];
    int numlabels(0);
    SLIC slic;
    slic.PerformSLICO_ForGivenK(int_data, width, height, labels, numlabels, superpixel_count, compactness);
    slic.DrawContoursAroundSegments(int_data, labels, width, height, 0);
    // slic.DrawContoursAroundSegmentsTwoColors(int_data, labels, width, height);
    // slic.SaveSuperpixelLabels(labels,width,height,picvec[k],saveLocation);
    // if(labels) delete [] labels;
  }

  for (int i = 0; i < width * height; i++) {
    unsigned char r = (int_data[i] & 0x00FF0000) >> 16;
    unsigned char g = (int_data[i] & 0x0000FF00) >> 8;
    unsigned char b = (int_data[i] & 0x000000FF);

    data[i] = r;
    data[i + 1] = g;
    data[i + 2] = b;
  }
  
  // Save image back to disk
  {
    ILboolean b = ilTexImage(width, height, 1, 1, IL_RGB, IL_UNSIGNED_BYTE, data);

    if (!b) {
      std::cout << "Unable to create new image from SLICO generated image data";
      return 0;
    }

    ILenum error = ilGetError();
    if (error) {
      // const char *errStr = iluErrorString(error);
      std::cout << "Error: " << error << std::endl;
      return 0;
    }

    // ilSetPixels(0, 0, 0, width, height, 1, IL_RGB, IL_UNSIGNED_BYTE, data);
    ilSaveImage("slico.jpg");
  }

  // Unbind and remove image memory
  ilBindImage(0);
  ilDeleteImage(ImgId);

  return 0;
}
