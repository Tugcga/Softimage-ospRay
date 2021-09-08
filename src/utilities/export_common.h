#pragma once

#include <xsi_x3dobject.h>
#include <xsi_time.h>

//return true if object have active render visibility
bool is_xsi_object_visible(const XSI::CTime &eval_time, XSI::X3DObject &xsi_object);