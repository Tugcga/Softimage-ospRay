#pragma once

#undef min
#undef max

#ifdef _WIN32
#define NOMINMAX
#include <malloc.h>
#else
#include <alloca.h>
#endif
#include <windows.h>
#include <limits>

#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"

using namespace rkcommon::math;

enum CameraType
{
	Camera_Perspective,
	Camera_Orthographic,
	Camera_Panoramic
};

enum RendererType
{
	Renderer_SciVis,
	Renderer_AO,
	Renderer_PathTracer
};