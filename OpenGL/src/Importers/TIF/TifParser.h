#pragma once
#include <tiffio.h>
#include <tiff.h>
#include <iostream>
unsigned char* LoadTIF(const char* filename, int* width, int* height, int* bpp, int desiredChannels);