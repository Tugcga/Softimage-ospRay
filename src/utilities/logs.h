#pragma once

#include <xsi_application.h>
#include <xsi_string.h>
#include <xsi_color.h>
#include <xsi_color4f.h>

#include <vector>

//output the message to the console
void log(const XSI::CString &message, XSI::siSeverityType level = XSI::siSeverityType::siInfoMsg);

//convert data to string
XSI::CString to_string(const std::vector<ULONG> &array);
XSI::CString to_string(const std::vector<unsigned int> &array);
XSI::CString to_string(const std::vector<float> &array);
XSI::CString to_string(const XSI::CColor &color);
XSI::CString to_string(const XSI::MATH::CColor4f &color);
XSI::CString to_string(const XSI::MATH::CVector3 &vector);

//remove digits from the given string
XSI::CString remove_digits(const XSI::CString& orignal_str);