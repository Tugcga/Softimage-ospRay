#include <xsi_time.h>
#include <xsi_light.h>
#include <xsi_shader.h>
#include <xsi_material.h>
#include <xsi_shaderparameter.h>
#include <xsi_color4f.h>
#include <xsi_kinematics.h>
#include <xsi_project.h>
#include <xsi_scene.h>
#include <xsi_model.h>
#include <xsi_pass.h>
#include <xsi_imageclip2.h>
#include <xsi_image.h>
#include <xsi_color.h>
#include <xsi_vector3f.h>

#include "../../utilities/logs.h"
#include "../includes_osp.h"
#include "material.h"

ospray::cpp::Light sync_light(const XSI::CTime &eval_time, XSI::Light &xsi_light, const float multiplier)
{
	int light_type = xsi_light.GetParameterValue("Type", eval_time);
	bool is_area = xsi_light.GetParameterValue("LightArea", eval_time);
	float intensity = 1.0f;
	float color_r = 1.0f;
	float color_g = 1.0f;
	float color_b = 1.0f;
	float spread = 5.0f; // in degrees
	float umbra = 0.75f;
	//try to get Soft Light shader node
	XSI::CRefArray light_shaders = xsi_light.GetShaders();
	std::vector<XSI::ShaderParameter> root_parameter_array = get_root_shader_parameter(light_shaders, "LightShader");
	if (root_parameter_array.size() > 0)
	{
		//next we should find SoftLight node, connected to root_parameter
		XSI::Shader light_node = get_input_node(root_parameter_array[0]);
		if (light_node.IsValid() && light_node.GetProgID() == "Softimage.soft_light.1.0")
		{
			//this is out Soft light node and we can get parameters from this node
			//we need color, intensity, spread and umbra
			XSI::CParameterRefArray all_params = light_node.GetParameters();
			XSI::MATH::CColor4f color = ((XSI::Parameter)all_params.GetItem("color")).GetValue();
			color_r = color.GetR();
			color_g = color.GetG();
			color_b = color.GetB();
			spread = ((XSI::Parameter)all_params.GetItem("spread")).GetValue();
			umbra = ((XSI::Parameter)all_params.GetItem("factor")).GetValue();
			intensity = ((XSI::Parameter)all_params.GetItem("intensity")).GetValue();
		}
	}
	root_parameter_array.clear();
	root_parameter_array.shrink_to_fit();

	if (is_area)
	{//any light with active are we identify as area light
		ospray::cpp::Light osp_light("quad");
		osp_light.setParam("color", vec3f{ color_r, color_g, color_b });
		osp_light.setParam("intensity", intensity * multiplier);

		//next are settings
		float size_x = xsi_light.GetParameterValue("LightAreaXformSX", eval_time);
		float size_y = xsi_light.GetParameterValue("LightAreaXformSY", eval_time);


		XSI::MATH::CTransformation xsi_tfm = xsi_light.GetKinematics().GetGlobal().GetTransform();
		XSI::MATH::CVector3 xsi_position = xsi_tfm.GetTranslation();
		XSI::MATH::CMatrix4 xsi_matrix = xsi_tfm.GetMatrix4();
		XSI::MATH::CVector3 xsi_x(-xsi_matrix.GetValue(0, 0), -xsi_matrix.GetValue(0, 1), -xsi_matrix.GetValue(0, 2));
		XSI::MATH::CVector3 xsi_y(xsi_matrix.GetValue(1, 0), xsi_matrix.GetValue(1, 1), xsi_matrix.GetValue(1, 2));
		xsi_x.NormalizeInPlace();
		xsi_y.NormalizeInPlace();
		xsi_x.ScaleInPlace(size_x);
		xsi_y.ScaleInPlace(size_y);
		
		osp_light.setParam("position", vec3f{ (float)xsi_position.GetX() - 0.5f * (float)xsi_x.GetX() - 0.5f * (float)xsi_y.GetX(),
											  (float)xsi_position.GetY() - 0.5f * (float)xsi_x.GetY() - 0.5f * (float)xsi_y.GetY(),
											  (float)xsi_position.GetZ() - 0.5f * (float)xsi_x.GetZ() - 0.5f * (float)xsi_y.GetZ() });
		osp_light.setParam("edge1", vec3f{ (float)xsi_x.GetX(), (float)xsi_x.GetY(), (float)xsi_x.GetZ()});
		osp_light.setParam("edge2", vec3f{ (float)xsi_y.GetX(), (float)xsi_y.GetY(), (float)xsi_y.GetZ()});
		//area light illuminate into negative local z-direction

		osp_light.commit();
		return osp_light;
	}
	else
	{
		if (light_type == 0)
		{//point light
			ospray::cpp::Light osp_light("sphere");
			//set default parameters
			osp_light.setParam("color", vec3f{ color_r, color_g, color_b });
			osp_light.setParam("intensity", intensity * multiplier);
			//visible param is not supported

			//next point parameters
			//position
			XSI::MATH::CTransformation xsi_tfm = xsi_light.GetKinematics().GetGlobal().GetTransform();
			XSI::MATH::CVector3 xsi_position = xsi_tfm.GetTranslation();
			osp_light.setParam("position", vec3f{ (float)xsi_position.GetX(), (float)xsi_position.GetY(), (float)xsi_position.GetZ() });

			//radius = scale of the lightsource
			XSI::MATH::CVector3 xsi_scale = xsi_tfm.GetScaling();
			osp_light.setParam("radius", (float)(xsi_scale.GetX() + xsi_scale.GetY() + xsi_scale.GetZ()) / 6.0f);

			osp_light.commit();
			return osp_light;
		}
		else if (light_type == 1)
		{//infinite
			ospray::cpp::Light osp_light("distant");
			//set default parameters
			osp_light.setParam("color", vec3f{ color_r, color_g, color_b });
			osp_light.setParam("intensity", intensity * multiplier);

			//next distance parameters
			XSI::MATH::CTransformation xsi_tfm = xsi_light.GetKinematics().GetGlobal().GetTransform();
			XSI::MATH::CMatrix4 xsi_matrix = xsi_tfm.GetMatrix4();
			osp_light.setParam("direction", vec3f{ -(float)xsi_matrix.GetValue(2, 0), -(float)xsi_matrix.GetValue(2, 1), -(float)xsi_matrix.GetValue(2, 2) });
			osp_light.setParam("angularDiameter", spread);  // set angle spread as angular diameter

			osp_light.commit();
			return osp_light;
		}
		else
		{//spot
			ospray::cpp::Light osp_light("spot");
			//set default parameters
			osp_light.setParam("color", vec3f{ color_r, color_g, color_b });
			osp_light.setParam("intensity", intensity * multiplier);

			//position
			XSI::MATH::CTransformation xsi_tfm = xsi_light.GetKinematics().GetGlobal().GetTransform();
			XSI::MATH::CVector3 xsi_position = xsi_tfm.GetTranslation();
			XSI::MATH::CMatrix4 xsi_matrix = xsi_tfm.GetMatrix4();
			osp_light.setParam("position", vec3f{ (float)xsi_position.GetX(), (float)xsi_position.GetY(), (float)xsi_position.GetZ() });
			osp_light.setParam("direction", vec3f{ -(float)xsi_matrix.GetValue(2, 0), -(float)xsi_matrix.GetValue(2, 1), -(float)xsi_matrix.GetValue(2, 2) });
			float cone = (float)xsi_light.GetParameterValue("LightCone", eval_time);
			osp_light.setParam("openingAngle", cone);
			osp_light.setParam("penumbraAngle", cone * umbra);
			//radius, innderRadius, intensityDistribution and c0 are not supported

			osp_light.commit();
			return osp_light;
		}
	}
}

void sync_ambient(const XSI::CTime &eval_time, ospray::cpp::Light &ambient_light)
{
	//ambient light
	XSI::Project project = XSI::Application().GetActiveProject();
	XSI::Scene scene = project.GetActiveScene();
	XSI::Model root = scene.GetRoot();
	XSI::Property amb_prop;
	root.GetPropertyFromName("AmbientLighting", amb_prop);
	if (amb_prop.IsValid())
	{
		XSI::Parameter amb_param = amb_prop.GetParameter("ambience");
		XSI::CParameterRefArray color_params = amb_param.GetParameters();
		float r = color_params.GetValue("red", eval_time);
		float g = color_params.GetValue("green", eval_time);
		float b = color_params.GetValue("blue", eval_time);
		
		ambient_light.setParam("intensity", 1.0f);
		ambient_light.setParam("color", vec3f{ r, g, b });
		ambient_light.commit();
	}
	else
	{
		ambient_light.setParam("intensity", 0.0f);
		ambient_light.commit();
	}
}

//return tru if we add the light
bool sync_hdri(const XSI::CTime &eval_time, ospray::cpp::Light &hdr_light)
{
	bool is_add = false;
	XSI::Project project = XSI::Application().GetActiveProject();
	XSI::Scene scene = project.GetActiveScene();
	XSI::Pass pass = scene.GetActivePass();
	XSI::CRefArray shaders = pass.GetAllShaders();  // in fact this not all shaders, but only level 1
	std::vector<XSI::ShaderParameter> root_params_array = get_root_shader_parameter(shaders, "Item", true);
	//next we should find Environment node
	for (LONG i = 0; i < root_params_array.size(); i++)
	{
		XSI::ShaderParameter root_parameter = root_params_array[i];
		//find the node from this connection
		XSI::Shader shader = get_input_node(root_parameter);
		if (shader.IsValid())
		{
			if (shader.GetProgID() == "Softimage.sib_environment.1.0")
			{//find environment node
				XSI::Parameter tex_param = shader.GetParameter("tex");
				XSI::Parameter tfm_param = shader.GetParameter("transform");
				XSI::CParameterRefArray tfm_parameters = tfm_param.GetParameters();
				XSI::MATH::CVector3f direction(((XSI::ShaderParameter)(tfm_parameters[8])).GetValue(eval_time),
											   ((XSI::ShaderParameter)(tfm_parameters[9])).GetValue(eval_time),
											   ((XSI::ShaderParameter)(tfm_parameters[10])).GetValue(eval_time));

				XSI::Parameter intensity_param = shader.GetParameter("fg_intensity");
				XSI::CRef tex_source = tex_param.GetSource();
				if (tex_source.IsValid())
				{
					XSI::ImageClip2 clip(tex_source);
					XSI::Image clip_image = clip.GetImage(eval_time);

					hdr_light.setParam("up", vec3f{0.0f, 1.0f, 0.0f});
					hdr_light.setParam("direction", vec3f{direction.GetX(), direction.GetY(), direction.GetZ()});
					hdr_light.setParam("map", image_to_texture(clip_image, intensity_param.GetValue(eval_time)));
					hdr_light.commit();
						
					//break for loop
					i = root_params_array.size();
					is_add = true;
				}
			}
		}
	}
	root_params_array.clear();
	root_params_array.shrink_to_fit();
	return is_add;
}

void update_sun(ospray::cpp::Light &light, float intensity, bool visible, float direction_x, float direction_y, float direction_z, float turbidity, float albedo, float horizon_extension)
{
	light.setParam("intensity", intensity);
	light.setParam("visible", visible);
	light.setParam("direction", vec3f{ direction_x, direction_y, direction_z });
	light.setParam("turbidity", turbidity);
	light.setParam("albedo", albedo);
	light.setParam("horizonExtension", horizon_extension);

	light.commit();
}

ospray::cpp::Light sync_sun(float intensity, bool visible, float direction_x, float direction_y, float direction_z, float turbidity, float albedo, float horizon_extension)
{
	ospray::cpp::Light light("sunSky");
	update_sun(light, intensity, visible, direction_x, direction_y, direction_z, turbidity, albedo, horizon_extension);
	return light;
}