#include "version.h"

#define OSPRAY_MAJOR_VERSION_NUM    1
#define OSPRAY_MINOR_VERSION_NUM    0

unsigned int get_major_version()
{
	return OSPRAY_MAJOR_VERSION_NUM;
}

unsigned int get_minor_version()
{
	return OSPRAY_MINOR_VERSION_NUM;
}