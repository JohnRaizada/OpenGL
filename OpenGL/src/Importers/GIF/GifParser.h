#pragma once
#include <gif_lib.h>
#include <iostream>
#include <vector>
unsigned char* LoadGIF(const char* filename, int* width, int* height, int* bpp, int desiredChannels);