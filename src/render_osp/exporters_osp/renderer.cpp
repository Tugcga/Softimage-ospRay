#include <xsi_property.h>
#include <xsi_time.h>

#include "../../utilities/logs.h"
#include "../../utilities/arrays.h"
#include "../includes_osp.h"

OSPFrameBufferChannel get_render_channel(const XSI::CString channel_name)
{
	if (channel_name == "ospColor" || channel_name == "Main"){ return OSP_FB_COLOR; }
	else if (channel_name == "ospDepth") { return OSP_FB_DEPTH; }
	else if (channel_name == "ospNormal") { return OSP_FB_NORMAL; }
	else if (channel_name == "ospAlbedo") { return OSP_FB_ALBEDO; }
	else { log("Unknown render channel " + channel_name + ", change to color.");  return OSP_FB_COLOR; }
}

int get_chennels_count_from_osp_buffer(const OSPFrameBufferChannel osp_buffer)
{
	if (osp_buffer == OSP_FB_COLOR) { return 4; }
	else if (osp_buffer == OSP_FB_DEPTH) { return 1; }
	else if (osp_buffer == OSP_FB_NORMAL) { return 3; }
	else {/*albedo*/ return 3; }
}

std::vector<OSPFrameBufferChannel> get_output_osp_channels(const XSI::CStringArray &output_channels)
{
	std::vector<OSPFrameBufferChannel> to_return(output_channels.GetCount());
	for (LONG i = 0; i < output_channels.GetCount(); i++)
	{
		to_return[i] = get_render_channel(output_channels[i]);
	}
	return to_return;
}

inline float linear_to_srgb(float v)
{
	if (v <= 0.0f)
	{
		return 0.0;
	}
	if (v <= 0.0031308f)
	{
		return  v * 12.92f;
	}
	if (v > 1.0f)
	{
		return 1.0f;
	}
	return 1.055f * pow(v, 1.0f / 2.4f) - 0.055f;
}

void copy_pixels_from_osp_buffer(std::vector<float> &pixels, LONG start_shift, int width, int height, ospray::cpp::FrameBuffer &framebuffer, OSPFrameBufferChannel osp_channel, bool apply_sRGB)
{
	bool apply_s_rgb = apply_sRGB && (osp_channel == OSP_FB_COLOR);
	//we apply srbg only for color channel, and if we need it
	float *fb = (float *)framebuffer.map(osp_channel);
	int channels = get_chennels_count_from_osp_buffer(osp_channel);
	//copy to visual buffer
	//write pixels to visual buffer, whcich always have 4 channels
	for (ULONG pixel_index = 0; pixel_index < width * height; pixel_index++)
	{
		for (int c = 0; c < channels; c++)
		{
			pixels[start_shift + 4 * pixel_index + c] = apply_s_rgb ? linear_to_srgb(fb[c]) : fb[c];
		}
		if (channels == 3)
		{//add alpha = 1.0
			pixels[start_shift + 4 * pixel_index + 3] = 1.0f;
		}
		if (channels == 1)
		{//for depth, copy it to other image channels
			pixels[start_shift + 4 * pixel_index + 1] = pixels[start_shift + 4 * pixel_index];
			pixels[start_shift + 4 * pixel_index + 2] = pixels[start_shift + 4 * pixel_index];
			pixels[start_shift + 4 * pixel_index + 3] = 1.0f;
		}
		fb += channels;
	}
	framebuffer.unmap(fb);
}

int join_osp_channels(const std::vector<OSPFrameBufferChannel> &array, const OSPFrameBufferChannel visual)
{//and also we should add accum
	if (array.size() == 0)
	{
		return visual | OSP_FB_ACCUM;
	}
	else
	{
		int to_return = visual | OSP_FB_ACCUM;
		std::vector<OSPFrameBufferChannel> done{ visual, OSP_FB_ACCUM };
		for (LONG i = 0; i < array.size(); i++)
		{
			OSPFrameBufferChannel v = array[i];
			if (!is_contains(done, v))
			{
				to_return |= v;
				done.push_back(v);
			}
		}
		done.clear();
		done.shrink_to_fit();

		return to_return;
	}
}

void sync_renderer(ospray::cpp::Renderer &renderer, RendererType renderer_type, const XSI::Property &render_property, const XSI::CTime &eval_time)
{
	renderer.setParam("pixelSamples", (int)render_property.GetParameterValue("pixel_samples", eval_time));
	renderer.setParam("maxPathLength", (int)render_property.GetParameterValue("max_path_length", eval_time));
	renderer.setParam("minContribution", (float)render_property.GetParameterValue("min_contribution", eval_time));
	renderer.setParam("varianceThreshold", (float)render_property.GetParameterValue("variance_threshold", eval_time));
	renderer.setParam("pixelFilter", (UCHAR)render_property.GetParameterValue("pixel_filter", eval_time));

	//sync background
	renderer.setParam("backgroundColor", vec4f{ render_property.GetParameterValue("background_R", eval_time),
												render_property.GetParameterValue("background_G", eval_time),
												render_property.GetParameterValue("background_B", eval_time),
												render_property.GetParameterValue("background_A", eval_time) });
	if (renderer_type == Renderer_SciVis)
	{
		renderer.setParam("shadows", (bool)render_property.GetParameterValue("scivis_shadows", eval_time));
		renderer.setParam("aoSamples", (int)render_property.GetParameterValue("scivis_ao_samples", eval_time));
		float scivis_ao_distance = (float)render_property.GetParameterValue("scivis_ao_distance", eval_time);
		if (scivis_ao_distance > 0.001f)
		{//0.0 is default infinite value
			renderer.setParam("aoDistance", scivis_ao_distance);
		}
		else
		{
			renderer.removeParam("aoDistance");
		}
		renderer.setParam("volumeSamplingRate", (float)render_property.GetParameterValue("scivis_volume_sampling_rate", eval_time));
		renderer.setParam("visibleLights", (bool)render_property.GetParameterValue("scivis_visible_lights", eval_time));
	}
	else if(renderer_type == Renderer_AO)
	{
		renderer.setParam("aoSamples", (int)render_property.GetParameterValue("ao_ao_samples", eval_time));
		float ao_ao_distance = (float)render_property.GetParameterValue("ao_ao_distance", eval_time);
		if (ao_ao_distance > 0.001f)
		{
			renderer.setParam("aoDistance", ao_ao_distance);
		}
		else
		{
			renderer.removeParam("aoDistance");
		}
		renderer.setParam("aoIntensity", (float)render_property.GetParameterValue("ao_ao_intencity", eval_time));
		renderer.setParam("volumeSamplingRate", (float)render_property.GetParameterValue("ao_volume_sampling_rate", eval_time));
	}
	else
	{//pathtracer
		int pt_light_samples = (int)render_property.GetParameterValue("pt_light_samples", eval_time);
		if (pt_light_samples > 0)
		{
			renderer.setParam("lightSamples", pt_light_samples);
		}
		else
		{
			renderer.removeParam("lightSamples");
		}
		renderer.setParam("roulettePathLength", (int)render_property.GetParameterValue("pt_roulette_path_length", eval_time));
		float pt_max_contribution = (float)render_property.GetParameterValue("pt_max_contribution", eval_time);
		if (pt_max_contribution > 0.001f)
		{
			renderer.setParam("maxContribution", pt_max_contribution);
		}
		else
		{
			renderer.removeParam("maxContribution");
		}
		renderer.setParam("backgroundRefraction", (bool)render_property.GetParameterValue("pt_background_refraction", eval_time));
	}

	renderer.commit();
}