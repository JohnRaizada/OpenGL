#pragma once
#include <cstdio>
#include <iostream>
#include <webp/decode.h>
unsigned char* LoadWebP(const char* filename, int* width, int* height, int* bpp, int desiredChannels);