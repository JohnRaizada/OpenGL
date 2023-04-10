#pragma once
#include <cstdio>
#include <png.h>
#include <stdlib.h>
unsigned char* LoadPNG(const char* filename, int* width, int* height, int* bpp, int desiredChannels);
void ToggleFlipped(bool value);
