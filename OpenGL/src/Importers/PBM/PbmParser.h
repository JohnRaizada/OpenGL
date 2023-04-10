#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
unsigned char* LoadPBM(const char* filename, int* width, int* height, int* bpp, int desiredChannels);