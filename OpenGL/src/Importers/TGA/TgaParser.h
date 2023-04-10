#pragma once
#include <cstdio>
#include <iostream>
unsigned char* LoadTGA(const char* filename, int* width, int* height, int* bpp, int desiredChannels);