#pragma once
#include <xsi_camera.h>
#include <xsi_primitive.h>
#include <xsi_light.h>

#include "ospray/ospray_cpp.h"

#include "types.h"
#include "../render_base/render_engine_base.h"

class RenderEngineOSP : public RenderEngineBase
{
public:
	RenderEngineOSP();
	~RenderEngineOSP();

	void try_to_init();

	XSI::CStatus render_option_define(XSI::CustomProperty &property);
	XSI::CStatus render_option_define_layout(XSI::Context &context);
	XSI::CStatus render_options_update(XSI::PPGEventContext &event_context);

	XSI::CStatus pre_render_engine();  // this called before the xsi scene is locked, after skiping existed images
	XSI::CStatus pre_scene_process();  // this method is called after xsi scene is locked
	XSI::CStatus create_scene();
	XSI::CStatus update_scene(XSI::X3DObject &xsi_object, const UpdateType update_type);
	XSI::CStatus update_scene(const XSI::SIObject &si_object, const UpdateType update_type);
	XSI::CStatus update_scene(const XSI::Material &xsi_material);
	XSI::CStatus update_scene_render();
	void render();
	XSI::CStatus post_render_engine();

	void abort();
	void clear_engine();

private:
	bool is_init = false;
	bool active_render = false;

	int render_frames;
	int current_frame;

	bool apply_cc;
	int cc_mode;
	bool use_denoising;
	bool use_motion;
	float motion_shatter_time;
	int motion_steps;

	bool use_tonemapping;
	float tonemapping_exposure;
	float tonemapping_contrast;
	float tonemapping_shoulder;
	float tonemapping_mid_in;
	float tonemapping_mid_out;
	float tonemapping_hdr_max;
	bool tonemapping_aces_color;
	
	ospray::cpp::Light ambient_light;
	ospray::cpp::Camera render_camera;
	ospray::cpp::Renderer render_render;
	ospray::cpp::Future render_task;
	OSPFrameBufferChannel visual_channel;
	std::vector<OSPFrameBufferChannel> output_osp_channels;

	std::vector<ospray::cpp::Instance> world_instances;
	std::vector<ospray::cpp::Light> world_lights;
	std::vector<ospray::cpp::Material> render_materials;

	std::vector<ULONG> update_xsi_lights_id;
	std::vector<ULONG> update_osp_lights_index;  // here we store indexes of light in world_lights array
	int update_osp_ambient_index;  // light indexes in world_lights array
	int update_osp_sky_index;
	int update_osp_hdri_index;

	std::vector<ULONG> update_xsi_meshes_id;  // mesh ids
	std::vector<ULONG> update_osp_meshes_index;  // corresponding indexes in world_instances array
	std::vector<ULONG> update_xsi_boxes_id;
	std::vector<ULONG> update_osp_boxes_index;
	std::vector<ULONG> update_xsi_spheres_id;
	std::vector<ULONG> update_osp_spheres_index;
	std::vector<ULONG> update_xsi_particle_volume_id;
	std::vector<ULONG> update_osp_particle_volume_index;
	std::vector<ULONG> update_xsi_strands_id;
	std::vector<ULONG> update_osp_strands_index;
	std::vector<ULONG> update_xsi_hairs_id;
	std::vector<ULONG> update_osp_hairs_index;
	std::vector<ULONG> update_xsi_materials_id;
	std::vector<ULONG> update_osp_materials_index;

	int update_last_render_curve_type;  // store here last value of the curve type
	int update_last_renderer_type;
	bool update_last_use_motion;
	float update_last_motion_shatter_time;
	int update_last_motion_steps;

	std::vector<ULONG> create_xsi_geometries_id;  // store here id-s of added objects, use this array to check is we try to add the duplicate (because FindObject(geometry) may return x2 pointclouds)

	XSI::CStatus update_light(const ULONG xsi_id, XSI::X3DObject &xsi_object);
	void create_scene_object(const XSI::CRefArray& xsi_list, XSI::siClassID class_id);
	void remove_light(const LONG index);
	void add_particles(const XSI::X3DObject &xsi_object);
	void add_strands(const XSI::X3DObject &xsi_object);
	void add_pointcloud(const XSI::X3DObject &xsi_object);
	void add_polygonmesh(XSI::X3DObject &xsi_object, const bool ignore_vertex_colors = false, const bool override_material = false, const int override_index = 0);
	void add_light(XSI::Light &xsi_light, const float multiplier = 1.0f);
	void RenderEngineOSP::add_hair(const XSI::X3DObject &xsi_object);
	void remove_pointcloud(const XSI::X3DObject &xsi_object);
	void remove_instance(const ULONG index);
	int get_material_index(const XSI::X3DObject &xsi_object);  // return index in the render_materials of the object's material
	void clear_buffers();
};