#include <xsi_time.h>
#include <xsi_material.h>
#include <xsi_shader.h>
#include <xsi_shaderparameter.h>
#include <xsi_color4f.h>
#include <xsi_parameter.h>
#include <xsi_image.h>
#include <xsi_imageclip2.h>
#include <xsi_parameter.h>

#include "../../utilities/logs.h"
#include "../../utilities/arrays.h"
#include "../includes_osp.h"

//we use multiplier only for 32-bit images wiht 3 or 4 channels
ospray::cpp::Texture image_to_texture(const XSI::Image &image, const float multiplier = 1.0f)
{
	ospray::cpp::Texture texture("texture2d");

	LONG size_x = image.GetResX();
	LONG size_y = image.GetResY();
	int channels = image.GetNumChannels();
	LONG channel_size = image.GetChannelSize();
	if (channel_size == 1)
	{
		std::vector<byte> pixels(size_x * size_y * channels);
		memcpy(pixels.data(), image.GetPixelArray(), size_x * size_y * channels * channel_size);
		if (channels == 1)
		{
			texture.setParam("data", ospray::cpp::CopiedData(pixels.data(), vec2i{ size_x, size_y }));
			texture.setParam("format", OSP_TEXTURE_R8);
		}
		else if (channels == 3)
		{
			std::vector<vec3c> map_pixels(size_x * size_y);
			for (LONG i = 0; i < map_pixels.size(); i++)
			{
				map_pixels[i] = vec3c{ (int8_t)pixels[3 * i], (int8_t)pixels[3 * i + 1], (int8_t)pixels[3 * i + 2] };
			}
			texture.setParam("data", ospray::cpp::CopiedData(map_pixels.data(), vec2i{ size_x, size_y }));
			texture.setParam("format", OSP_TEXTURE_RGB8);
			map_pixels.clear();
			map_pixels.shrink_to_fit();
		}
		else if (channels == 4)
		{
			std::vector<vec4c> map_pixels(size_x * size_y);
			for (LONG i = 0; i < map_pixels.size(); i++)
			{
				map_pixels[i] = vec4c{ (int8_t)pixels[4 * i], (int8_t)pixels[4 * i + 1], (int8_t)pixels[4 * i + 2], (int8_t)pixels[4 * i + 3] };
			}
			texture.setParam("data", ospray::cpp::CopiedData(map_pixels.data(), vec2i{ size_x, size_y }));
			texture.setParam("format", OSP_TEXTURE_RGBA8);
			map_pixels.clear();
			map_pixels.shrink_to_fit();
		}
		pixels.clear();
		pixels.shrink_to_fit();
	}
	else if (channel_size == 2)
	{
		std::vector<short> pixels(size_x * size_y * channels);
		memcpy(pixels.data(), image.GetPixelArray(), size_x * size_y * channels * channel_size);
		if (channels == 1)
		{
			texture.setParam("data", ospray::cpp::CopiedData(pixels.data(), vec2i{ size_x, size_y }));
			texture.setParam("format", OSP_TEXTURE_R16);
		}
		else if (channels == 3)
		{
			std::vector<vec3s> map_pixels(size_x * size_y);
			for (LONG i = 0; i < map_pixels.size(); i++)
			{
				map_pixels[i] = vec3s{ (int16_t)pixels[3 * i], (int16_t)pixels[3 * i + 1], (int16_t)pixels[3 * i + 2] };
			}
			texture.setParam("data", ospray::cpp::CopiedData(map_pixels.data(), vec2i{ size_x, size_y }));
			texture.setParam("format", OSP_TEXTURE_RGB16);
			map_pixels.clear();
			map_pixels.shrink_to_fit();
		}
		else if (channels == 4)
		{
			std::vector<vec4s> map_pixels(size_x * size_y);
			for (LONG i = 0; i < map_pixels.size(); i++)
			{
				map_pixels[i] = vec4s{ (int16_t)pixels[4 * i], (int16_t)pixels[4 * i + 1], (int16_t)pixels[4 * i + 2], (int16_t)pixels[4 * i + 3] };
			}
			texture.setParam("data", ospray::cpp::CopiedData(map_pixels.data(), vec2i{ size_x, size_y }));
			texture.setParam("format", OSP_TEXTURE_RGBA16);
			map_pixels.clear();
			map_pixels.shrink_to_fit();
		}
		pixels.clear();
		pixels.shrink_to_fit();
	}
	else if (channel_size == 4)
	{
		std::vector<float> pixels(size_x * size_y * channels);
		memcpy(pixels.data(), image.GetPixelArray(), size_x * size_y * channels * channel_size);
		if (channels == 1)
		{
			texture.setParam("data", ospray::cpp::CopiedData(pixels.data(), vec2i{ size_x, size_y }));
			texture.setParam("format", OSP_TEXTURE_R32F);
		}
		else if (channels == 3)
		{
			std::vector<vec3f> map_pixels(size_x * size_y);
			for(LONG i = 0; i < map_pixels.size(); i++)
			{
				map_pixels[i] = vec3f{pixels[3*i] * multiplier, pixels[3*i + 1] * multiplier, pixels[3*i + 2] * multiplier };
			}
			texture.setParam("data", ospray::cpp::CopiedData(map_pixels.data(), vec2i{ size_x, size_y }));
			texture.setParam("format", OSP_TEXTURE_RGB32F);
			map_pixels.clear();
			map_pixels.shrink_to_fit();
		}
		else if (channels == 4)
		{
			std::vector<vec4f> map_pixels(size_x * size_y);
			for (LONG i = 0; i < map_pixels.size(); i++)
			{
				map_pixels[i] = vec4f{ pixels[4 * i] * multiplier, pixels[4 * i + 1] * multiplier, pixels[4 * i + 2] * multiplier, pixels[4*i + 3] * multiplier };
			}
			texture.setParam("data", ospray::cpp::CopiedData(map_pixels.data(), vec2i{ size_x, size_y }));
			texture.setParam("format", OSP_TEXTURE_RGBA32F);
			map_pixels.clear();
			map_pixels.shrink_to_fit();
		}
		pixels.clear();
		pixels.shrink_to_fit();
	}

	texture.setParam("filter", OSP_TEXTURE_FILTER_BILINEAR);
	texture.commit();

	return texture;
}

std::vector<XSI::ShaderParameter> get_root_shader_parameter(const XSI::CRefArray &first_level_shaders, const XSI::CString &root_parameter_name, bool check_substring = false)
{
	std::vector<XSI::ShaderParameter> to_return;
	for (ULONG i = 0; i < first_level_shaders.GetCount(); i++)
	{
		XSI::Shader shader(first_level_shaders[i]);
		XSI::CRefArray shader_params = XSI::CRefArray(shader.GetParameters());
		for (ULONG j = 0; j < shader_params.GetCount(); j++)
		{
			XSI::Parameter parameter(shader_params.GetItem(j));
			XSI::CString parameter_name = parameter.GetName();
			bool is_input;
			XSI::siShaderParameterType param_type = shader.GetShaderParameterType(parameter_name, is_input);
			//here there is a bug in Softimage: output parameter does not visible or interpreter as input, when it is a port, crreated from inside of the compound
			if (!is_input)
			{
				//this is output shader parameter
				XSI::CRefArray targets = shader.GetShaderParameterTargets(parameter_name);
				for (LONG k = 0; k < targets.GetCount(); k++)
				{
					XSI::ShaderParameter p = targets.GetItem(k);
					if ((!check_substring && p.GetName() == root_parameter_name) || (check_substring && p.GetName().FindString(root_parameter_name) != UINT_MAX))
					{
						to_return.push_back(p);
					}
				}
			}
		}
	}
	return to_return;
}

bool is_shader_compound(const XSI::Shader &shader)
{
	XSI::CRefArray sub_shaders = shader.GetAllShaders();
	return sub_shaders.GetCount() > 0;
}

//return primitive shader node, connected to the parameter
XSI::Shader get_input_node(const XSI::ShaderParameter &parameter)
{
	XSI::CRef source = parameter.GetSource();
	if (source.IsValid())
	{
		XSI::ShaderParameter source_param(source);
		XSI::Shader source_shader(source_param.GetParent());
		if (is_shader_compound(source_shader))
		{
			return get_input_node(source_param);
		}
		else
		{
			return source_shader;
		}
	}
	else
	{
		return XSI::Shader();
	}
}

XSI::CString name_to_material(const XSI::CString &name)
{
	if (name == "ospObj") { return "obj"; }
	else if (name == "ospPrincipled") { return "principled"; }
	else if (name == "ospCarPaint") { return "carPaint"; }
	else if (name == "ospMetal") { return "metal"; }
	else if (name == "ospAlloy") { return "alloy"; }
	else if (name == "ospGlass") { return "glass"; }
	else if (name == "ospThinGlass") { return "thinGlass"; }
	else if (name == "ospMetallicPaint") { return "metallicPaint"; }
	else if (name == "ospLuminous") { return "luminous"; }
	else{ return ""; }
}

XSI::ShaderParameter get_final_shader_parameter(XSI::ShaderParameter &parameter)
{
	XSI::CRef source = parameter.GetSource();
	if (source.IsValid())
	{
		return get_final_shader_parameter(XSI::ShaderParameter(source));
	}
	return parameter;
}

XSI::MATH::CColor4f get_shader_color(const XSI::CTime &eval_time, const XSI::ShaderParameter &parameter)
{
	XSI::CParameterRefArray paramsArray = parameter.GetParameters();
	LONG count = paramsArray.GetCount();
	XSI::Parameter p[4];
	p[0] = XSI::Parameter(paramsArray[0]);
	p[1] = XSI::Parameter(paramsArray[1]);
	p[2] = XSI::Parameter(paramsArray[2]);

	if (count == 3)
	{
		return XSI::MATH::CColor4f(p[0].GetValue(eval_time), p[1].GetValue(eval_time), p[2].GetValue(eval_time), 1.0f);
	}
	else if (count == 4)
	{
		p[3] = XSI::Parameter(paramsArray[3]);
		return XSI::MATH::CColor4f(p[0].GetValue(eval_time), p[1].GetValue(eval_time), p[2].GetValue(eval_time), p[3].GetValue(eval_time));
	}
	return XSI::MATH::CColor4f(0.0f, 0.0f, 0.0f, 0.0f);
}

//some material parameters does not supports texturing
bool is_parameter_allow_texture(const XSI::CString &material_name, const XSI::CString &parameter_name)
{
	if (material_name == "obj" && parameter_name == "tf")
	{
		return false;
	}
	if (material_name == "metal")
	{
		return parameter_name == "roughness";
	}
	if (material_name == "glass" || material_name == "luminous")
	{
		return false;
	}
	if (material_name == "thinGlass")
	{
		return parameter_name == "attenuationColor";
	}
	if (material_name == "metallicPaint")
	{
		return parameter_name == "baseColor";
	}
	return true;
}

ospray::cpp::Material sync_material(const XSI::CTime &eval_time, const XSI::Material &xsi_material, const int render_type)
{
	std::string render_string = render_type == 0 ? "scivis" : (render_type == 1 ? "ao" : "pathtracer");

	std::vector<ULONG> xsi_clips_id;  //store here id-s of loaded textures
	std::vector<ospray::cpp::Texture> osp_textures;
	
	XSI::CRefArray root_shaders = xsi_material.GetShaders();
	std::vector<XSI::ShaderParameter> surface_ports = get_root_shader_parameter(root_shaders, "surface");
	if (surface_ports.size() > 0)
	{
		XSI::ShaderParameter surface_port = surface_ports[0];
		//next we should go by connections and create material
		//in this render all materials are very simple: only one main shader node and may be some connections to image nodes
		XSI::Shader main_node = get_input_node(surface_port);
		if (main_node.IsValid())
		{
			XSI::CString node_progid = main_node.GetProgID();
			XSI::CParameterRefArray node_params = main_node.GetParameters();
			XSI::Parameter param_name = node_params.GetItem("Name");
			if (param_name.IsValid())
			{
				XSI::CString node_name = param_name.GetValue(eval_time);
				XSI::CString material_name = name_to_material(node_name);
				if (material_name.Length() > 0)
				{
					if ((render_string == "scivis" || render_string == "ao") && material_name != "obj")
					{
						log("[OSPRay Render] " + XSI::CString(render_string.c_str()) + " supports only obj material. Create default material.", XSI::siWarningMsg);
						return ospray::cpp::Material(render_string, "obj");
					}
					//create material
					ospray::cpp::Material material(render_string, material_name.GetAsciiString());

					for (LONG i = 0; i < node_params.GetCount(); i++)
					{
						XSI::CRef node_parameter_ref(node_params[i]);
						if (node_parameter_ref.GetClassID() == XSI::siShaderParameterID)
						{
							XSI::ShaderParameter node_parameter(node_parameter_ref);
							XSI::CString parameter_name = node_parameter.GetName();
							if (parameter_name != "out")
							{//ignore out shader parameter
								XSI::ShaderParameter node_final_parameter = get_final_shader_parameter(node_parameter);  // shader parameter, which connect to the given parameter
								XSI::CRef node_parameter_parent = node_final_parameter.GetParent();  // shader node, which connected to the given parameter (may be the same node, if no connections, compound node, if connection ended ar compound port, other shader node)
								XSI::Shader node_parameter_parent_shader(node_parameter_parent);
								bool is_parent_shader_compound = is_shader_compound(node_parameter_parent_shader);  // true if the end of the connection on the compount input port
								bool is_connected = !(node_final_parameter == node_parameter);  // true if there is connection from the parameter
								bool is_allow_texture = is_parameter_allow_texture(material_name, parameter_name);

								XSI::siShaderParameterDataType data_type = node_parameter.GetDataType();
								//set parameter values
								if (data_type == XSI::siShaderDataTypeScalar)
								{
									float value = (is_connected && is_parent_shader_compound) ? node_final_parameter.GetValue(eval_time) : node_parameter.GetValue(eval_time);
									material.setParam(parameter_name.GetAsciiString(), value);
								}
								else if (data_type == XSI::siShaderDataTypeColor3)
								{
									XSI::MATH::CColor4f value;
									if (is_connected && is_parent_shader_compound)
									{
										value = get_shader_color(eval_time, node_final_parameter);
									}
									else
									{
										value = node_parameter.GetValue(eval_time);
									}
									if (parameter_name != "map_bump")
									{
										material.setParam(parameter_name.GetAsciiString(), vec3f{ (float)value.GetR(), (float)value.GetG(), (float)value.GetB() });
									}
								}
								//and may be we should add the texture
								//pass through color-to-scalar
								if (is_allow_texture && is_connected && !is_parent_shader_compound && node_parameter_parent_shader.GetProgID() == "Softimage.sib_color_to_scalar.1.0")
								{
									//we should get source of the converter node as parent shader
									XSI::ShaderParameter input_param = node_parameter_parent_shader.GetParameter("input");
									XSI::ShaderParameter input_final_parameter = get_final_shader_parameter(input_param);
									XSI::CRef input_parameter_parent = input_final_parameter.GetParent();
									XSI::Shader input_parameter_parent_shader(input_parameter_parent);
									if (input_param != input_final_parameter && !is_shader_compound(input_parameter_parent_shader))
									{
										node_parameter_parent_shader = input_parameter_parent_shader;
									}
								}
								if (is_allow_texture && is_connected && !is_parent_shader_compound && node_parameter_parent_shader.GetProgID() == "Softimage.txt2d-image-explicit.1.0")
								{
									XSI::Parameter tex_param = node_parameter_parent_shader.GetParameter("tex");
									XSI::CRef tex_source = tex_param.GetSource();
									if (tex_source.IsValid())
									{
										XSI::ImageClip2 clip(tex_source);
										XSI::Image clip_image = clip.GetImage(eval_time);

										ULONG clip_id = clip.GetObjectID();
										int tex_index = get_index_in_array(xsi_clips_id, clip_id);
										if (tex_index == -1)
										{
											ospray::cpp::Texture texture = image_to_texture(clip_image);
											xsi_clips_id.push_back(clip_id);
											osp_textures.push_back(texture);
											tex_index = osp_textures.size() - 1;
										}
										//transformation
										XSI::Parameter repeats_param = node_parameter_parent_shader.GetParameter("repeats");
										XSI::CParameterRefArray repeats_params = repeats_param.GetParameters();
										float repeat_x = ((XSI::Parameter)repeats_params[0]).GetValue();
										float repeat_y = ((XSI::Parameter)repeats_params[1]).GetValue();
										float repeat_z = ((XSI::Parameter)repeats_params[2]).GetValue();

										vec2f scale{ 1.0f / repeat_x, 1.0f / repeat_y};
										vec2f translation{ -(1.0f - 1.0f / repeat_x) * repeat_x / 2.0f, -(1.0f - 1.0f / repeat_y) * repeat_y / 2.0f };
										if (parameter_name == "map_bump")
										{
											material.setParam(parameter_name.GetAsciiString(), osp_textures[tex_index]);
											material.setParam((parameter_name + ".scale").GetAsciiString(), scale);
											material.setParam((parameter_name + ".translation").GetAsciiString(), translation);
										}
										else
										{
											material.setParam(("map_" + parameter_name).GetAsciiString(), osp_textures[tex_index]);
											material.setParam(("map_" + parameter_name + ".scale").GetAsciiString(), scale);
											material.setParam(("map_" + parameter_name + ".translation").GetAsciiString(), translation);
										}
									}
								}
							}
						}
					}
					material.commit();
					return material;
				}
				else
				{
					return ospray::cpp::Material(render_string, "obj");
				}
			}
			else
			{
				return ospray::cpp::Material(render_string, "obj");
			}
		}
		else
		{
			return ospray::cpp::Material(render_string, "obj");
		}
	}
	else
	{
		return ospray::cpp::Material(render_string, "obj");
	}

	return ospray::cpp::Material(render_string, "obj");
}