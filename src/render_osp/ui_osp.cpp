#include <xsi_parameter.h>
#include <xsi_customproperty.h>
#include <xsi_ppglayout.h>
#include <xsi_ppgitem.h>
#include <xsi_context.h>
#include <xsi_ppgeventcontext.h>

#include "render_engine_osp.h"

void build_layout(XSI::PPGLayout& layout)
{
	layout.Clear();

	layout.AddTab("Rendering");
	layout.AddGroup("Samples");
	layout.AddItem("render_frames", "Farmes");
	layout.AddItem("pixel_samples", "Pixel Samples");
	layout.AddItem("max_path_length", "Max Path Length");
	layout.EndGroup();

	layout.AddGroup("Accumulation");
	layout.AddItem("min_contribution", "Min Contribution");
	layout.AddItem("variance_threshold", "Variance Threshold");
	layout.EndGroup();

	layout.AddGroup("Renderer");
	XSI::CValueArray renderer_combo(6);
	renderer_combo[0] = "SciVis"; renderer_combo[1] = 0;
	renderer_combo[2] = "AO"; renderer_combo[3] = 1;
	renderer_combo[4] = "Path Tracer"; renderer_combo[5] = 2;
	layout.AddEnumControl("renderer", renderer_combo, "Renderer", XSI::siControlCombo);

	//scivis params
	layout.AddItem("scivis_shadows", "Shadows");
	layout.AddItem("scivis_ao_samples", "AO Samples");
	layout.AddItem("scivis_ao_distance", "AO Distance");
	layout.AddItem("scivis_volume_sampling_rate", "Volume Sampling Rate");
	layout.AddItem("scivis_visible_lights", "Visible Lights");

	//ao params
	layout.AddItem("ao_ao_samples", "AO Samples");
	layout.AddItem("ao_ao_distance", "AO Distance");
	layout.AddItem("ao_ao_intencity", "AO Intencity");
	layout.AddItem("ao_volume_sampling_rate", "Volume Sampling Rate");

	//path tracer
	layout.AddItem("pt_light_samples", "Light Samples");
	layout.AddItem("pt_roulette_path_length", "Roulette Path Length");
	layout.AddItem("pt_max_contribution", "Max Contribution");
	layout.AddItem("pt_background_refraction", "Background Refraction");
	layout.EndGroup();

	layout.AddTab("Background");
	layout.AddGroup("Background");
	layout.AddColor("background_R", "Bacground Color");
	layout.AddItem("background_A", "Bacground Alpha");
	layout.EndGroup();
	layout.AddGroup("Sun and Sky");
	layout.AddItem("sun_use", "Use Sun/Sky");
	layout.AddItem("sun_visible", "Sky Visibility");
	layout.AddItem("sun_intensity", "Intensity");
	layout.AddGroup("Direction");
	layout.AddRow();
	layout.AddItem("sun_direction_x", "X");
	layout.AddItem("sun_direction_y", "Y");
	layout.AddItem("sun_direction_z", "Z");
	layout.EndRow();
	layout.EndGroup();
	layout.AddItem("sun_turbidity", "Turbidity");
	layout.AddItem("sun_albedo", "Albedo");
	layout.AddItem("sun_horizon_extension", "Horizon Extension");
	layout.EndGroup();

	layout.AddTab("Camera");
	layout.AddGroup("Camera");
	layout.AddItem("camera_aperture_radius", "Aperture Radius");
	layout.AddItem("camera_architectural", "Architectural Camera");
	layout.AddItem("camera_panoramic", "Panoramic Camera");
	layout.EndGroup();
	layout.AddGroup("Motion Blur");
	layout.AddItem("camera_motion", "Use Motion Blur");
	layout.AddItem("camera_motion_shutter_time", "Shutter Time");
	layout.AddItem("camera_motion_steps", "Steps");
	layout.EndGroup();

	layout.AddTab("Settings");
	layout.AddGroup("Antialiasing");
	XSI::CValueArray filters_combo(10);
	filters_combo[0] = "Point"; filters_combo[1] = 0;
	filters_combo[2] = "Box"; filters_combo[3] = 1;
	filters_combo[4] = "Gauss"; filters_combo[5] = 2;
	filters_combo[6] = "Mitchell"; filters_combo[7] = 3;
	filters_combo[8] = "Blackman-Harris"; filters_combo[9] = 4;
	layout.AddEnumControl("pixel_filter", filters_combo, "Pixel Filter", XSI::siControlCombo);
	layout.EndGroup();

	//crash with denoising
	//layout.AddGroup("Denoising");
	//layout.AddItem("denoising_use", "Use Denoising");
	//layout.EndGroup();

	layout.AddGroup("Curves");
	XSI::CValueArray curve_type_combo(6);
	curve_type_combo[0] = "Flat"; curve_type_combo[1] = 0;
	curve_type_combo[2] = "Round"; curve_type_combo[3] = 1;
	//curve_type_combo[4] = "Ribbon"; curve_type_combo[5] = 2;  // does not used ribbon mode, because no canonic way to define normal to the strand strip
	curve_type_combo[4] = "Disjoint"; curve_type_combo[5] = 3;
	layout.AddEnumControl("curve_type", curve_type_combo, "Curves Type", XSI::siControlCombo);

	XSI::CValueArray curve_basus_combo(10);
	curve_basus_combo[0] = "Linear"; curve_basus_combo[1] = 0;
	curve_basus_combo[2] = "Bezier"; curve_basus_combo[3] = 1;
	curve_basus_combo[4] = "BSpline"; curve_basus_combo[5] = 2;
	curve_basus_combo[6] = "Hermite"; curve_basus_combo[7] = 3;
	curve_basus_combo[8] = "Catmull-Rom"; curve_basus_combo[9] = 4;

	//layout.AddEnumControl("curve_basis", curve_basus_combo, "Curves Basis", XSI::siControlCombo);
	layout.EndGroup();

	layout.AddGroup("Colors");
	layout.AddItem("color_use", "Apply Color Correction");
	XSI::CValueArray color_mode(4);
	color_mode[0] = "sRGB"; color_mode[1] = 0;
	color_mode[2] = "Tonemapping"; color_mode[3] = 1;
	//layout.AddEnumControl("color_mode", color_mode, "Color Correction Mode");
	//use only the simple sRGB color, because something strange with colors after tonemapping, all of them are white
	layout.AddItem("color_exposure", "Exposure");
	layout.AddItem("color_contrast", "Contrast");
	layout.AddItem("color_shoulder", "Highlight");
	layout.AddItem("color_mid_in", "Mid-Level Input");
	layout.AddItem("color_mid_out", "Mid-Level Output");
	layout.AddItem("color_hdr_max", "HDR Max");
	layout.AddItem("color_aces_color", "Apply ACES");
	layout.EndGroup();
}

void set_motion(XSI::CustomProperty& prop, const bool use_motion)
{
	XSI::CParameterRefArray prop_array = prop.GetParameters();
	XSI::Parameter camera_motion_shutter_time = prop_array.GetItem("camera_motion_shutter_time");
	camera_motion_shutter_time.PutCapabilityFlag(XSI::siNotInspectable, !use_motion);

	XSI::Parameter camera_motion_steps = prop_array.GetItem("camera_motion_steps");
	camera_motion_steps.PutCapabilityFlag(XSI::siNotInspectable, !use_motion);
}

void set_sun_sky_mode(XSI::CustomProperty& prop, const bool use_sky)
{
	XSI::CParameterRefArray prop_array = prop.GetParameters();
	XSI::Parameter sun_intensity = prop_array.GetItem("sun_intensity");
	sun_intensity.PutCapabilityFlag(XSI::siNotInspectable, !use_sky);

	XSI::Parameter sun_visible = prop_array.GetItem("sun_visible");
	sun_visible.PutCapabilityFlag(XSI::siNotInspectable, !use_sky);

	XSI::Parameter sun_direction_x = prop_array.GetItem("sun_direction_x");
	sun_direction_x.PutCapabilityFlag(XSI::siNotInspectable, !use_sky);

	XSI::Parameter sun_direction_y = prop_array.GetItem("sun_direction_y");
	sun_direction_y.PutCapabilityFlag(XSI::siNotInspectable, !use_sky);

	XSI::Parameter sun_direction_z = prop_array.GetItem("sun_direction_z");
	sun_direction_z.PutCapabilityFlag(XSI::siNotInspectable, !use_sky);

	XSI::Parameter sun_turbidity = prop_array.GetItem("sun_turbidity");
	sun_turbidity.PutCapabilityFlag(XSI::siNotInspectable, !use_sky);

	XSI::Parameter sun_albedo = prop_array.GetItem("sun_albedo");
	sun_albedo.PutCapabilityFlag(XSI::siNotInspectable, !use_sky);

	XSI::Parameter sun_horizon_extension = prop_array.GetItem("sun_horizon_extension");
	sun_horizon_extension.PutCapabilityFlag(XSI::siNotInspectable, !use_sky);
}

void set_renderer(XSI::CustomProperty& prop, const int mode)
{
	XSI::CParameterRefArray prop_array = prop.GetParameters();

	XSI::Parameter scivis_shadows = prop_array.GetItem("scivis_shadows");
	scivis_shadows.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 0));

	XSI::Parameter scivis_ao_samples = prop_array.GetItem("scivis_ao_samples");
	scivis_ao_samples.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 0));

	XSI::Parameter scivis_ao_distance = prop_array.GetItem("scivis_ao_distance");
	scivis_ao_distance.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 0));

	XSI::Parameter scivis_volume_sampling_rate = prop_array.GetItem("scivis_volume_sampling_rate");
	scivis_volume_sampling_rate.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 0));

	XSI::Parameter scivis_visible_lights = prop_array.GetItem("scivis_visible_lights");
	scivis_visible_lights.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 0));

	//-----------------------------------------------------------------
	XSI::Parameter ao_ao_samples = prop_array.GetItem("ao_ao_samples");
	ao_ao_samples.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 1));

	XSI::Parameter ao_ao_distance = prop_array.GetItem("ao_ao_distance");
	ao_ao_distance.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 1));

	XSI::Parameter ao_ao_intencity = prop_array.GetItem("ao_ao_intencity");
	ao_ao_intencity.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 1));

	XSI::Parameter ao_volume_sampling_rate = prop_array.GetItem("ao_volume_sampling_rate");
	ao_volume_sampling_rate.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 1));

	//-----------------------------------------------------------------
	XSI::Parameter pt_light_samples = prop_array.GetItem("pt_light_samples");
	pt_light_samples.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 2));

	XSI::Parameter pt_roulette_path_length = prop_array.GetItem("pt_roulette_path_length");
	pt_roulette_path_length.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 2));

	XSI::Parameter pt_max_contribution = prop_array.GetItem("pt_max_contribution");
	pt_max_contribution.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 2));

	XSI::Parameter pt_background_refraction = prop_array.GetItem("pt_background_refraction");
	pt_background_refraction.PutCapabilityFlag(XSI::siNotInspectable, !(mode == 2));
}

void set_color_mode(XSI::CustomProperty& prop, const int mode)
{
	XSI::CParameterRefArray prop_array = prop.GetParameters();

	XSI::Parameter color_exposure = prop_array.GetItem("color_exposure");
	color_exposure.PutCapabilityFlag(XSI::siNotInspectable, (mode == 0));

	XSI::Parameter color_contrast = prop_array.GetItem("color_contrast");
	color_contrast.PutCapabilityFlag(XSI::siNotInspectable, (mode == 0));

	XSI::Parameter color_shoulder = prop_array.GetItem("color_shoulder");
	color_shoulder.PutCapabilityFlag(XSI::siNotInspectable, (mode == 0));

	XSI::Parameter color_mid_in = prop_array.GetItem("color_mid_in");
	color_mid_in.PutCapabilityFlag(XSI::siNotInspectable, (mode == 0));

	XSI::Parameter color_mid_out = prop_array.GetItem("color_mid_out");
	color_mid_out.PutCapabilityFlag(XSI::siNotInspectable, (mode == 0));

	XSI::Parameter color_hdr_max = prop_array.GetItem("color_hdr_max");
	color_hdr_max.PutCapabilityFlag(XSI::siNotInspectable, (mode == 0));

	XSI::Parameter color_aces_color = prop_array.GetItem("color_aces_color");
	color_aces_color.PutCapabilityFlag(XSI::siNotInspectable, (mode == 0));
}


XSI::CStatus RenderEngineOSP::render_options_update(XSI::PPGEventContext &event_context)
{
	XSI::PPGEventContext::PPGEvent event_id = event_context.GetEventID();
	if (event_id == XSI::PPGEventContext::siOnInit)
	{
		XSI::CustomProperty cp_source = event_context.GetSource();
		XSI::Parameter sun_use = cp_source.GetParameters().GetItem("sun_use");
		set_sun_sky_mode(cp_source, sun_use.GetValue());

		XSI::Parameter renderer = cp_source.GetParameters().GetItem("renderer");
		set_renderer(cp_source, renderer.GetValue());

		XSI::Parameter color_mode = cp_source.GetParameters().GetItem("color_mode");
		set_color_mode(cp_source, color_mode.GetValue());

		XSI::Parameter camera_motion = cp_source.GetParameters().GetItem("camera_motion");
		set_motion(cp_source, camera_motion.GetValue());

		XSI::PPGLayout prop_layout = cp_source.GetPPGLayout();
		build_layout(prop_layout);
	}
	else if (event_id == XSI::PPGEventContext::siParameterChange)
	{
		XSI::Parameter changed = event_context.GetSource();
		XSI::CustomProperty prop = changed.GetParent();
		XSI::CString param_name = changed.GetScriptName();

		if (param_name == "sun_use")
		{
			XSI::Parameter sun_use = prop.GetParameters().GetItem("sun_use");
			set_sun_sky_mode(prop, sun_use.GetValue());

			event_context.PutAttribute("Refresh", true);
		}
		else if (param_name == "renderer")
		{
			XSI::Parameter renderer = prop.GetParameters().GetItem("renderer");
			set_renderer(prop, renderer.GetValue());

			event_context.PutAttribute("Refresh", true);
		}
		else if (param_name == "color_mode")
		{
			XSI::Parameter color_mode = prop.GetParameters().GetItem("color_mode");
			set_color_mode(prop, color_mode.GetValue());

			event_context.PutAttribute("Refresh", true);
		}
		else if (param_name == "camera_motion")
		{
			XSI::Parameter camera_motion = prop.GetParameters().GetItem("camera_motion");
			set_motion(prop, camera_motion.GetValue());

			event_context.PutAttribute("Refresh", true);
		}
	}
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineOSP::render_option_define_layout(XSI::Context &context)
{
	XSI::PPGLayout layout = context.GetSource();
	XSI::Parameter parameter = context.GetSource();
	XSI::CustomProperty property = parameter.GetParent();

	build_layout(layout);

	set_sun_sky_mode(property, false);
	set_renderer(property, 0);

	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineOSP::render_option_define(XSI::CustomProperty &property)
{
	const int caps = XSI::siPersistable | XSI::siAnimatable;
	XSI::Parameter param;

	//rendering
	property.AddParameter("pixel_samples", XSI::CValue::siInt4, caps, "", "", 1, 1, 4096, 1, 128, param);
	property.AddParameter("max_path_length", XSI::CValue::siInt4, caps, "", "", 20, 1, 1024, 1, 48, param);
	property.AddParameter("min_contribution", XSI::CValue::siFloat, caps, "", "", 0.001f, 0.0, 1.0f, 0.0f, 0.1f, param);
	property.AddParameter("variance_threshold", XSI::CValue::siFloat, caps, "", "", 0.0f, 0.0, 1.0f, 0.0f, 0.1f, param);
	property.AddParameter("background_R", XSI::CValue::siDouble, caps, "", "", 0.2, param);
	property.AddParameter("background_G", XSI::CValue::siDouble, caps, "", "", 0.2, param);
	property.AddParameter("background_B", XSI::CValue::siDouble, caps, "", "", 0.2, param);
	property.AddParameter("background_A", XSI::CValue::siDouble, caps, "", "", 0.0, 0.0, 1.0, 0.0, 1.0, param);
	property.AddParameter("pixel_filter", XSI::CValue::siUInt1, caps, "", "", 1, param);

	property.AddParameter("render_frames", XSI::CValue::siInt4, caps, "", "", 1, 1, 4096, 1, 128, param);
	property.AddParameter("renderer", XSI::CValue::siUInt1, caps, "", "", 0, param);

	//scivis
	property.AddParameter("scivis_shadows", XSI::CValue::siBool, caps, "", "", false, param);
	property.AddParameter("scivis_ao_samples", XSI::CValue::siInt4, caps, "", "", 0, 0, 4096, 0, 128, param);
	property.AddParameter("scivis_ao_distance", XSI::CValue::siFloat, caps, "", "", 0.0f, 0.0f, 4096.0*4096.0, 0.0, 12.0, param);  // 0.0 - infinite
	property.AddParameter("scivis_volume_sampling_rate", XSI::CValue::siFloat, caps, "", "", 1.0f, 0.0f, 128.0, 0.0, 2.0, param);
	property.AddParameter("scivis_visible_lights", XSI::CValue::siBool, caps, "", "", false, param);

	//ao
	property.AddParameter("ao_ao_samples", XSI::CValue::siInt4, caps, "", "", 1, 1, 4096, 1, 128, param);
	property.AddParameter("ao_ao_distance", XSI::CValue::siFloat, caps, "", "", 0.0f, 0.0f, 4096.0*4096.0, 0.0, 12.0, param);  // 0.0 - infinite
	property.AddParameter("ao_ao_intencity", XSI::CValue::siFloat, caps, "", "", 1.0f, 0.0f, 1.0, 0.0, 1.0, param);
	property.AddParameter("ao_volume_sampling_rate", XSI::CValue::siFloat, caps, "", "", 1.0f, 0.0f, 128.0, 0.0, 2.0, param);

	//path tracer
	property.AddParameter("pt_light_samples", XSI::CValue::siInt4, caps, "", "", 0, 0, 4096, 0, 16, param);  //0 - infinite
	property.AddParameter("pt_roulette_path_length", XSI::CValue::siInt4, caps, "", "", 5, 0, 4096, 0, 16, param);
	property.AddParameter("pt_max_contribution", XSI::CValue::siFloat, caps, "", "", 0.0f, 0.0f, 4096.0, 0.0, 12.0, param);  // 0.0 - infinite
	property.AddParameter("pt_background_refraction", XSI::CValue::siBool, caps, "", "", false, param);

	//additional camera settings
	property.AddParameter("camera_aperture_radius", XSI::CValue::siFloat, caps, "", "", 0.0f, 0.0f, 1024.0, 0.0, 40.0, param);
	property.AddParameter("camera_architectural", XSI::CValue::siBool, caps, "", "", false, param);
	property.AddParameter("camera_panoramic", XSI::CValue::siBool, caps, "", "", false, param);
	property.AddParameter("camera_motion", XSI::CValue::siBool, caps, "", "", false, param);
	property.AddParameter("camera_motion_shutter_time", XSI::CValue::siFloat, caps, "", "", 1.0f, 0.0f, 1024.0f, 0.0f, 2.0f, param);
	property.AddParameter("camera_motion_steps", XSI::CValue::siInt4, caps, "", "", 3, 3, 1024, 3, 5, param);

	//sun-sky additional light
	property.AddParameter("sun_use", XSI::CValue::siBool, caps, "", "", false, param);
	property.AddParameter("sun_intensity", XSI::CValue::siFloat, caps, "", "", 1.0f, 0.0f, 1024.0, 0.0, 12.0, param);
	property.AddParameter("sun_visible", XSI::CValue::siBool, caps, "", "", false, param);
	property.AddParameter("sun_direction_x", XSI::CValue::siFloat, caps, "", "", 0.0f, -1.0f, 1.0f, -1.0, 1.0, param);
	property.AddParameter("sun_direction_y", XSI::CValue::siFloat, caps, "", "", -1.0f, -1.0f, 1.0f, -1.0, 1.0, param);
	property.AddParameter("sun_direction_z", XSI::CValue::siFloat, caps, "", "", 0.0f, -1.0f, 1.0f, -1.0, 1.0, param);
	property.AddParameter("sun_turbidity", XSI::CValue::siFloat, caps, "", "", 3.0f, 1.0f, 10.0f, 1.0, 10.0, param);
	property.AddParameter("sun_albedo", XSI::CValue::siFloat, caps, "", "", 0.3f, 0.0f, 1.0f, 0.0, 1.0, param);
	property.AddParameter("sun_horizon_extension", XSI::CValue::siFloat, caps, "", "", 0.01f, 0.0f, 1.0f, 0.0, 1.0, param);

	//settings
	property.AddParameter("curve_type", XSI::CValue::siUInt1, caps, "", "", 1, param);
	property.AddParameter("curve_basis", XSI::CValue::siUInt1, caps, "", "", 0, param);

	property.AddParameter("denoising_use", XSI::CValue::siBool, caps, "", "", false, param);

	property.AddParameter("color_use", XSI::CValue::siBool, caps, "", "", true, param);
	property.AddParameter("color_mode", XSI::CValue::siUInt1, caps, "", "", 0, param);
	property.AddParameter("color_exposure", XSI::CValue::siFloat, caps, "", "", 1.0, 0.0, 10.0, 0.5, 2.0, param);
	property.AddParameter("color_contrast", XSI::CValue::siFloat, caps, "", "", 1.6773, 0.0, 10.0, 1.0, 2.0, param);
	property.AddParameter("color_shoulder", XSI::CValue::siFloat, caps, "", "", 0.9714, 0.0, 10.0, 0.9, 1.0, param);
	property.AddParameter("color_mid_in", XSI::CValue::siFloat, caps, "", "", 0.18, 0.0, 1.0, 0.0, 1.0, param);
	property.AddParameter("color_mid_out", XSI::CValue::siFloat, caps, "", "", 0.18, 0.0, 1.0, 0.0, 1.0, param);
	property.AddParameter("color_hdr_max", XSI::CValue::siFloat, caps, "", "", 11.0785, 0.0, 1024.0, 0.0, 24.0, param);
	property.AddParameter("color_aces_color", XSI::CValue::siBool, caps, "", "", true, param);

	return XSI::CStatus::OK;
}