#pragma once
#include <xsi_string.h>

#include <vector>

//clamp value between given min and max values
float clamp_float(float value, float min, float max);

//write to the disc *.ppm image file
void write_ppm(const XSI::CString &path, int width, int height, int components, const std::vector<float> &pixels);

//write to the disc image file
//supported extensions are ppm (manual), exr(by using tinyexr), png, bmp, jpg, tga, hdr (all of the by usin stb_image_write)
bool write_float(const XSI::CString &path, const XSI::CString &ext, const XSI::CString &data_type, int width, int height, int components, const std::vector<float> &pixels);