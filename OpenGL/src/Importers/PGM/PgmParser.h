#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
unsigned char* LoadPGM(const char* filename, int* width, int* height, int* bpp, int desiredChannels);