#pragma once
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
unsigned char* LoadPNM(const char* filename, int* width, int* height, int* bpp, int desiredChannels);