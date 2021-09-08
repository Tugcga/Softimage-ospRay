#pragma once
#include <xsi_shaderarrayparameter.h>
#include <xsi_shader.h>
#include <xsi_string.h>
#include <xsi_image.h>

#include <vector>

ospray::cpp::Texture image_to_texture(const XSI::Image &image, const float multiplier = 1.0f);
std::vector<XSI::ShaderParameter> get_root_shader_parameter(const XSI::CRefArray &first_level_shaders, const XSI::CString &root_parameter_name, bool check_substring = false);
bool is_shader_compound(const XSI::Shader &shader);
XSI::Shader get_input_node(const XSI::ShaderParameter &parameter);