#pragma once
#include <libheif/heif.h>
#include <iostream>
#include <vector>
unsigned char* LoadHEIF(const char* filename, int* width, int* height, int* bpp, int desiredChannels);