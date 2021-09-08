#include <xsi_framebuffer.h>
#include <xsi_renderchannel.h>
#include <xsi_scene.h>
#include <xsi_application.h>
#include <xsi_material.h>
#include <xsi_materiallibrary.h>
#include <xsi_project.h>
#include <xsi_model.h>

#include <algorithm>

#include "../utilities/logs.h"
#include "../utilities/export_common.h"
#include "../utilities/arrays.h"
#include "exporters_osp/exporter_includes.h"
#include "includes_osp.h"
#include "types.h"

//at first the base is created, then this method is called
RenderEngineOSP::RenderEngineOSP()
{
	is_init = false;
	bool active_render = false;
}

//when we delete the engine, then at first this method is called, and then the method from base class
RenderEngineOSP::~RenderEngineOSP()
{
	ospShutdown();
	is_init = false;
	bool active_render = false;
}

void RenderEngineOSP::try_to_init()
{
	if (!is_init)
	{
		OSPError init_error = ospInit();
		if (init_error != OSP_NO_ERROR)
		{
			log("[OSPRay Render] Fails to initialize the engine. error: " + XSI::CString(init_error), XSI::siWarningMsg);
		}
		else
		{
			is_init = true;
		}
	}
}

XSI::CStatus RenderEngineOSP::pre_render_engine()
{
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineOSP::pre_scene_process()
{
	try_to_init();
	m_render_context.ProgressUpdate("Prepare...", "Prepare...", 0);
	if (render_type == RenderType_Shaderball)
	{
		//start shaderball rendering
		//int this case we should setup most of the parameters manually
		visual_channel = OSP_FB_COLOR;
		apply_cc = true;
		cc_mode = 0;
		use_tonemapping = false;
		use_denoising = false;

		render_frames = 128;
		current_frame = 0;

		render_camera = ospray::cpp::Camera("perspective");
		XSI::Primitive shaderball_camera_prim(m_render_context.GetAttribute("Camera"));
		XSI::X3DObject shaderball_camera_obj = shaderball_camera_prim.GetOwners()[0];
		XSI::Camera	shaderball_xsi_camera(shaderball_camera_obj);
		sync_camera(eval_time, shaderball_xsi_camera, render_camera, Camera_Panoramic, false, 0.0f, image_corner_x, image_corner_y, image_full_size_widht, image_full_size_height, image_size_width, image_size_height, false, 0.0, 3);
		
		//camera from xsi contains wrong transfrom, so, here we shold setup it manually
		set_camera_transform(render_camera,
			0.0f, 0.0f, -1.0f, 0.0f,
			-0.34f, 0.94f, 0.0f, 0.0f,
			-0.94f, -0.34, 0.0f, 0.0f,
			15.68f, 5.89f, 0.0f, 1.0f);
		render_camera.setParam("fovy", 28.94f);
		render_camera.commit();

		render_render = ospray::cpp::Renderer("pathtracer");
		render_render.setParam("pixelSamples", 1);
		render_render.setParam("backgroundColor", vec4f{ 0.0f, 0.0f, 0.0f, 1.0f });
		render_render.commit();

		return XSI::CStatus::OK;
	}
	else
	{
		//start pass or region rendering
		//get display channel
		XSI::Framebuffer frame_buffer = m_render_context.GetDisplayFramebuffer();
		XSI::RenderChannel render_channel = frame_buffer.GetRenderChannel();
		//log("render channel: " + render_channel.GetName());
		visual_channel = get_render_channel(render_channel.GetName());

		apply_cc = m_render_property.GetParameterValue("color_use", eval_time);
		cc_mode = m_render_property.GetParameterValue("color_mode", eval_time);
		use_tonemapping = false;
		if (apply_cc && cc_mode == 1)
		{
			//we should apply tonemapping
			//remember parameters
			use_tonemapping = true;
			tonemapping_exposure = m_render_property.GetParameterValue("color_exposure", eval_time);
			tonemapping_contrast = m_render_property.GetParameterValue("color_contrast", eval_time);
			tonemapping_shoulder = m_render_property.GetParameterValue("color_shoulder", eval_time);
			tonemapping_mid_in = m_render_property.GetParameterValue("color_mid_in", eval_time);
			tonemapping_mid_out = m_render_property.GetParameterValue("color_mid_out", eval_time);
			tonemapping_hdr_max = m_render_property.GetParameterValue("color_hdr_max", eval_time);
			tonemapping_aces_color = m_render_property.GetParameterValue("color_aces_color", eval_time);
		}
		use_denoising = m_render_property.GetParameterValue("denoising_use", eval_time);
		use_motion = m_render_property.GetParameterValue("camera_motion", eval_time);
		motion_shatter_time = m_render_property.GetParameterValue("camera_motion_shutter_time", eval_time);
		motion_steps = m_render_property.GetParameterValue("camera_motion_steps", eval_time);

		int renderer = m_render_property.GetParameterValue("renderer", eval_time);
		RendererType renderer_type = renderer == 0 ? Renderer_SciVis : (renderer == 1 ? Renderer_AO : Renderer_PathTracer);

		if (renderer_type != Renderer_PathTracer && use_motion)
		{
			log("[OSPRay Render] Motion blur supported only for PathTracer renderer. Disable it.", XSI::siWarningMsg);
			use_motion = false;
		}

		//convert output channels from xsi-names to osp-names
		output_osp_channels = get_output_osp_channels(output_channels);

		render_frames = m_render_property.GetParameterValue("render_frames", eval_time);
		current_frame = 0;

		//setup camera
		XSI::Primitive camera_prim(m_render_context.GetAttribute("Camera"));
		XSI::X3DObject camera_obj = camera_prim.GetOwners()[0];
		XSI::Camera	xsi_camera(camera_obj);
		int xsi_ortho_mode = xsi_camera.GetParameterValue("proj", eval_time);
		//get additional values from render property
		bool camera_panoramic = m_render_property.GetParameterValue("camera_panoramic", eval_time);
		bool camera_architectural = m_render_property.GetParameterValue("camera_architectural", eval_time);
		float camera_aperture_radius = m_render_property.GetParameterValue("camera_aperture_radius", eval_time);

		render_camera = ospray::cpp::Camera(camera_panoramic ? "panoramic" : (xsi_ortho_mode == 1 ? "perspective" : "orthographic"));
		sync_camera(eval_time, xsi_camera, render_camera, camera_panoramic ? Camera_Panoramic : (xsi_ortho_mode == 1 ? Camera_Perspective : Camera_Orthographic), camera_architectural, camera_aperture_radius, image_corner_x, image_corner_y, image_full_size_widht, image_full_size_height, image_size_width, image_size_height, use_motion, motion_shatter_time, motion_steps);

		//setup renderer
		render_render = ospray::cpp::Renderer(renderer_type == Renderer_SciVis ? "scivis" : (renderer_type == Renderer_AO ? "ao" : "pathtracer"));
		sync_renderer(render_render, renderer_type, m_render_property, eval_time);

		XSI::CStatus status = XSI::CStatus::OK;
		if (renderer != update_last_renderer_type)
		{
			status = XSI::CStatus::Abort;
		}

		update_last_renderer_type = renderer;
		return status;
	}
}

void RenderEngineOSP::clear_buffers()
{
	update_xsi_meshes_id.clear();
	update_xsi_meshes_id.shrink_to_fit();
	update_osp_meshes_index.clear();
	update_osp_meshes_index.shrink_to_fit();
	update_xsi_lights_id.clear();
	update_xsi_lights_id.shrink_to_fit();
	update_osp_lights_index.clear();
	update_osp_lights_index.shrink_to_fit();
	update_xsi_boxes_id.clear();
	update_xsi_boxes_id.shrink_to_fit();
	update_osp_boxes_index.clear();
	update_osp_boxes_index.shrink_to_fit();
	update_xsi_spheres_id.clear();
	update_xsi_spheres_id.shrink_to_fit();
	update_osp_spheres_index.clear();
	update_osp_spheres_index.shrink_to_fit();
	update_xsi_particle_volume_id.clear();
	update_xsi_particle_volume_id.shrink_to_fit();
	update_osp_particle_volume_index.clear();
	update_osp_particle_volume_index.shrink_to_fit();
	update_xsi_strands_id.clear();
	update_xsi_strands_id.shrink_to_fit();
	update_osp_strands_index.clear();
	update_osp_strands_index.shrink_to_fit();
	update_xsi_hairs_id.clear();
	update_xsi_hairs_id.shrink_to_fit();
	update_osp_hairs_index.clear();
	update_osp_hairs_index.shrink_to_fit();
	update_xsi_materials_id.clear();
	update_xsi_materials_id.shrink_to_fit();
	update_osp_materials_index.clear();
	update_osp_materials_index.shrink_to_fit();

	create_xsi_geometries_id.clear();
	create_xsi_geometries_id.shrink_to_fit();

	world_instances.clear();
	world_instances.shrink_to_fit();
	render_materials.clear();
	render_materials.shrink_to_fit();
	world_lights.clear();
	world_lights.shrink_to_fit();
}

void RenderEngineOSP::add_particles(const XSI::X3DObject &xsi_object)
{
	//primitives pointcloud may produce three different instances in the world: spheres, boxes and particle volumes
	ospray::cpp::Group spheres_group;
	ospray::cpp::Group boxes_group;
	ospray::cpp::Group volume_group;
	bool is_spheres = false;
	bool is_boxes = false;
	bool is_volume = false;
	sync_pointcloud_primitives(eval_time, xsi_object, spheres_group, is_spheres, boxes_group, is_boxes, volume_group, is_volume, get_material_index(xsi_object));
	if (is_spheres)
	{
		ospray::cpp::Instance instance(spheres_group);
		sync_transform(eval_time, xsi_object, instance, use_motion, motion_shatter_time, motion_steps);

		//add to the world
		world_instances.push_back(instance);
		update_xsi_spheres_id.push_back(xsi_object.GetObjectID());
		update_osp_spheres_index.push_back(world_instances.size() - 1);
	}
	if (is_boxes)
	{
		ospray::cpp::Instance instance(boxes_group);
		sync_transform(eval_time, xsi_object, instance, use_motion, motion_shatter_time, motion_steps);

		world_instances.push_back(instance);
		update_xsi_boxes_id.push_back(xsi_object.GetObjectID());
		update_osp_boxes_index.push_back(world_instances.size() - 1);
	}
	if (is_volume)
	{
		ospray::cpp::Instance instance(volume_group);
		sync_transform(eval_time, xsi_object, instance, use_motion, motion_shatter_time, motion_steps);

		world_instances.push_back(instance);
		update_xsi_particle_volume_id.push_back(xsi_object.GetObjectID());
		update_osp_particle_volume_index.push_back(world_instances.size() - 1);
	}
}

void RenderEngineOSP::add_strands(const XSI::X3DObject &xsi_object)
{
	ospray::cpp::Group strands_group;
	bool is_strands = false;
	sync_strands(eval_time, xsi_object, strands_group, is_strands, get_curve_type(m_render_property.GetParameterValue("curve_type", eval_time)), get_curve_basis(m_render_property.GetParameterValue("curve_basis", eval_time)), get_material_index(xsi_object));
	if (is_strands)
	{
		ospray::cpp::Instance instance(strands_group);
		sync_transform(eval_time, xsi_object, instance, use_motion, motion_shatter_time, motion_steps);

		//next add to the world
		world_instances.push_back(instance);
		update_xsi_strands_id.push_back(xsi_object.GetObjectID());
		update_osp_strands_index.push_back(world_instances.size() - 1);
	}
}

void RenderEngineOSP::add_pointcloud(const XSI::X3DObject &xsi_object)
{
	PointcloudType pc_type = get_pointcloud_type(eval_time, xsi_object);
	if (pc_type == PointcloudType_Primitives)
	{
		add_particles(xsi_object);
	}
	else if (pc_type == PointcloudType_Strands)
	{
		add_strands(xsi_object);
	}
}

void RenderEngineOSP::add_polygonmesh(XSI::X3DObject &xsi_object, const bool ignore_vertex_colors, const bool override_material, const int override_index)
{
	ospray::cpp::Instance mesh_instance = sync_polygonmesh(eval_time, xsi_object, override_material ? override_index : get_material_index(xsi_object), ignore_vertex_colors);
	//save links for the update system
	update_xsi_meshes_id.push_back(xsi_object.GetObjectID());

	//sync transform of the object
	sync_transform(eval_time, xsi_object, mesh_instance, use_motion, motion_shatter_time, motion_steps);
	world_instances.push_back(mesh_instance);
	update_osp_meshes_index.push_back(world_instances.size() - 1);
}

void RenderEngineOSP::add_hair(const XSI::X3DObject &xsi_object)
{
	ospray::cpp::Instance hair_instance = sync_hair(eval_time, xsi_object, get_curve_type(m_render_property.GetParameterValue("curve_type", eval_time)), get_curve_basis(m_render_property.GetParameterValue("curve_basis", eval_time)), get_material_index(xsi_object));
	sync_transform(eval_time, xsi_object, hair_instance, use_motion, motion_shatter_time, motion_steps);

	update_xsi_hairs_id.push_back(xsi_object.GetObjectID());
	world_instances.push_back(hair_instance);
	update_osp_hairs_index.push_back(world_instances.size() - 1);
}

void RenderEngineOSP::add_light(XSI::Light &xsi_light, const float multiplier)
{
	if (is_xsi_object_visible(eval_time, xsi_light))
	{
		ospray::cpp::Light light = sync_light(eval_time, xsi_light, multiplier);
		update_xsi_lights_id.push_back(xsi_light.GetObjectID());
		world_lights.push_back(light);
		update_osp_lights_index.push_back(world_lights.size() - 1);
	}
}

void RenderEngineOSP::remove_instance(const ULONG index)
{
	world_instances.erase(world_instances.begin() + index);
	//and then we should decrease all indexes with bigger indexes to 1
	//in meshes, boxes and spheres
	for (LONG i = 0; i < update_osp_meshes_index.size(); i++)
	{
		if (update_osp_meshes_index[i] > index) { update_osp_meshes_index[i]--; }
	}
	for (LONG i = 0; i < update_osp_hairs_index.size(); i++)
	{
		if (update_osp_hairs_index[i] > index) { update_osp_hairs_index[i]--; }
	}
	for (LONG i = 0; i < update_osp_boxes_index.size(); i++)
	{
		if (update_osp_boxes_index[i] > index) { update_osp_boxes_index[i]--; }
	}
	for (LONG i = 0; i < update_osp_spheres_index.size(); i++)
	{
		if (update_osp_spheres_index[i] > index) { update_osp_spheres_index[i]--; }
	}
	for (LONG i = 0; i < update_osp_particle_volume_index.size(); i++)
	{
		if (update_osp_particle_volume_index[i] > index) { update_osp_particle_volume_index[i]--; }
	}
	for (LONG i = 0; i < update_osp_strands_index.size(); i++)
	{
		if (update_osp_strands_index[i] > index) { update_osp_strands_index[i]--; }
	}
}

void RenderEngineOSP::remove_pointcloud(const XSI::X3DObject &xsi_object)
{
	ULONG xsi_id = xsi_object.GetObjectID();
	int boxes_index = get_index_in_array(update_xsi_boxes_id, xsi_id);
	if (boxes_index > -1)
	{
		remove_instance(update_osp_boxes_index[boxes_index]);
		//remove this pair of indexes form xsi-to-osp arrays
		update_xsi_boxes_id.erase(update_xsi_boxes_id.begin() + boxes_index);
		update_osp_boxes_index.erase(update_osp_boxes_index.begin() + boxes_index);
	}
	//the same for spheres
	int spheres_index = get_index_in_array(update_xsi_spheres_id, xsi_id);
	if (spheres_index > -1)
	{
		remove_instance(update_osp_spheres_index[spheres_index]);
		update_xsi_spheres_id.erase(update_xsi_spheres_id.begin() + spheres_index);
		update_osp_spheres_index.erase(update_osp_spheres_index.begin() + spheres_index);
	}
	int particle_volume_index = get_index_in_array(update_xsi_particle_volume_id, xsi_id);
	if (particle_volume_index > -1)
	{
		remove_instance(update_osp_particle_volume_index[particle_volume_index]);
		update_xsi_particle_volume_id.erase(update_xsi_particle_volume_id.begin() + particle_volume_index);
		update_osp_particle_volume_index.erase(update_osp_particle_volume_index.begin() + particle_volume_index);
	}
	int strands_index = get_index_in_array(update_xsi_strands_id, xsi_id);
	if (strands_index > -1)
	{
		remove_instance(update_osp_strands_index[strands_index]);
		update_xsi_strands_id.erase(update_xsi_strands_id.begin() + strands_index);
		update_osp_strands_index.erase(update_osp_strands_index.begin() + strands_index);
	}
}

void RenderEngineOSP::remove_light(const LONG index)
{
	world_lights.erase(world_lights.begin() + index);
	//and also we should decrease indexes of all other lights
	for (LONG i = 0; i < update_osp_lights_index.size(); i++)
	{
		if (update_osp_lights_index[i] > index)
		{
			update_osp_lights_index[i] -= 1;
		}
	}
	if (update_osp_ambient_index > index) { update_osp_ambient_index--; }
	if (update_osp_sky_index > index) { update_osp_sky_index--; }
	if (update_osp_hdri_index > index) { update_osp_hdri_index--; }
}

int RenderEngineOSP::get_material_index(const XSI::X3DObject &xsi_object)
{
	XSI::Material xsi_material = xsi_object.GetMaterial();
	ULONG xsi_material_id = xsi_material.GetObjectID();
	int index = get_index_in_array(update_xsi_materials_id, xsi_material_id);
	return index;
}

void RenderEngineOSP::create_scene_object(const XSI::CRefArray& xsi_list, XSI::siClassID class_id)
{
	for (ULONG i = 0; i < xsi_list.GetCount(); i++)
	{
		XSI::X3DObject xsi_object;
		if (class_id == XSI::siX3DObjectID)
		{
			xsi_object = XSI::X3DObject(xsi_list[i]);
		}
		else if (XSI::siGeometryID)
		{
			XSI::SIObject si_ob(xsi_list[i]);
			si_ob = XSI::SIObject(si_ob.GetParent());
			si_ob = XSI::SIObject(si_ob.GetParent());
			xsi_object = XSI::X3DObject(si_ob);
		}
		
		if (is_xsi_object_visible(eval_time, xsi_object) && !is_contains(create_xsi_geometries_id, xsi_object.GetObjectID()))
		{
			if (xsi_object.GetType() == "polymsh")
			{
				add_polygonmesh(xsi_object);
				create_xsi_geometries_id.push_back(xsi_object.GetObjectID());
			}
			else if(xsi_object.GetType() == "pointcloud")
			{
				add_pointcloud(xsi_object);
				create_xsi_geometries_id.push_back(xsi_object.GetObjectID());
			}
			else if (xsi_object.GetType() == "hair")
			{
				add_hair(xsi_object);
				create_xsi_geometries_id.push_back(xsi_object.GetObjectID());
			}
			else
			{
				//unsupported onkect type
			}
		}
	}
}

void gather_all_subobjects(const XSI::X3DObject &xsi_object, XSI::CRefArray &output)
{
	output.Add(xsi_object.GetRef());
	XSI::CRefArray children = xsi_object.GetChildren();
	for (ULONG i = 0; i < children.GetCount(); i++)
	{
		gather_all_subobjects(children[i], output);
	}
}

XSI::CRefArray gather_all_subobjects(const XSI::Model &root)
{
	XSI::CRefArray output;
	XSI::CRefArray children = root.GetChildren();
	for (ULONG i = 0; i < children.GetCount(); i++)
	{
		gather_all_subobjects(children[i], output);
	}
	return output;
}

XSI::CStatus RenderEngineOSP::create_scene()
{
	clear_buffers();
	//by defaul these indexes are undefined
	update_osp_ambient_index = -1;
	update_osp_sky_index = -1;
	update_osp_hdri_index = -1;

	if (render_type == RenderType_Shaderball)
	{
		//here we should create the scene for shaderball rendering
		//read material item (possible shader node)
		XSI::CRef shaderball_item = m_render_context.GetAttribute("Material");
		XSI::siClassID shaderball_item_class = shaderball_item.GetClassID();
		if (shaderball_item_class == XSI::siMaterialID)
		{//we preview the material
			XSI::Material shaderball_material(shaderball_item);
			//sync it
			ospray::cpp::Material osp_material = sync_material(eval_time, shaderball_material, 2);  // 2 - pathtrace render
			//add to the list
			render_materials.push_back(osp_material);
			update_xsi_materials_id.push_back(shaderball_material.GetObjectID());
			update_osp_materials_index.push_back(render_materials.size() - 1);

			//get root models of the rendering scene
			XSI::CRefArray models = m_render_context.GetAttribute("Scene");
			if (models.GetCount() > 0)
			{
				//add ambient light
				ambient_light = ospray::cpp::Light("ambient");
				ambient_light.setParam("intensity", 0.025f);
				ambient_light.setParam("color", vec3f{ 1.0f, 1.0f, 1.0f });
				ambient_light.commit();
				world_lights.push_back(ambient_light);
				update_osp_ambient_index = 0;

				//the first model is the hero model
				XSI::Model hero(models[0]);
				XSI::CRefArray shaderball_objects = gather_all_subobjects(hero);
				for (LONG j = 0; j < shaderball_objects.GetCount(); j++)
				{
					XSI::X3DObject xsi_object(shaderball_objects[j]);
					XSI::CString xsi_type = xsi_object.GetType();
					if (xsi_type == "polymsh")
					{
						add_polygonmesh(xsi_object, true, true, 0);  // ignore vertex colors and override material with index 0
					}
					else if (xsi_type == "light")
					{
						XSI::Light xsi_light(xsi_object);
						add_light(xsi_light, 450.0f);
					}
				}
				//all other models are background
				//ignore it, because the default background override the camera view
				for (LONG i = 1; i < models.GetCount(); i++)
				{
					XSI::Model m(models[i]);
					XSI::CRefArray background_objects = gather_all_subobjects(m);
					for (LONG j = 0; j < background_objects.GetCount(); j++)
					{
						XSI::X3DObject xsi_object(background_objects[j]);
						if (xsi_object.GetType() == "polymsh")
						{
							//add object as background
							//add_polygonmesh(xsi_object, true, 0);
						}
					}
				}
			}
		}
		//does not create the scene, if we preview shader node
	}
	else
	{
		//create scene for pass or region rendering
		//create materials
		//get it from the scene
		XSI::Scene xsi_scene = XSI::Application().GetActiveProject().GetActiveScene();
		XSI::CRefArray material_libraries = xsi_scene.GetMaterialLibraries();
		for (LONG lib_index = 0; lib_index < material_libraries.GetCount(); lib_index++)
		{
			XSI::MaterialLibrary libray = material_libraries.GetItem(lib_index);
			XSI::CRefArray materials = libray.GetItems();
			for (LONG mat_index = 0; mat_index < materials.GetCount(); mat_index++)
			{
				XSI::Material xsi_material = materials.GetItem(mat_index);
				XSI::CRefArray used_objects = xsi_material.GetUsedBy();
				if (used_objects.GetCount() > 0)
				{
					ospray::cpp::Material osp_material = sync_material(eval_time, xsi_material, m_render_property.GetParameterValue("renderer", eval_time));
					render_materials.push_back(osp_material);
					update_xsi_materials_id.push_back(xsi_material.GetObjectID());
					update_osp_materials_index.push_back(render_materials.size() - 1);
				}
			}
		}

		//add scene polygonmesh objects
		//objects in the isolation mode
		const XSI::CRefArray& xsi_isolation_list = m_render_context.GetArrayAttribute("ObjectList");
		if (xsi_isolation_list.GetCount() > 0)
		{//we are in isolation mode
			create_scene_object(xsi_isolation_list, XSI::siX3DObjectID);
		}
		else
		{//we should chek all objects in the scene
		 //for simplicity get all X3Dobjects and then filter by their types
			const XSI::CRefArray& xsi_objects_list = XSI::Application().FindObjects(XSI::siGeometryID);
			create_scene_object(xsi_objects_list, XSI::siGeometryID);
		}

		//lights
		//ambient
		ambient_light = ospray::cpp::Light("ambient");
		sync_ambient(eval_time, ambient_light);
		world_lights.push_back(ambient_light);
		update_osp_ambient_index = 0;

		//hdri
		ospray::cpp::Light hdr_light("hdri");
		bool is_add_hdri = sync_hdri(eval_time, hdr_light);  // here we maybe load hdr light
		if (is_add_hdri)
		{//we find and setup light parameters
		 //add it to the array
			world_lights.push_back(hdr_light);
			update_osp_hdri_index = world_lights.size() - 1;
		}

		//sun/sky we can add here from render settings
		bool sun_use = m_render_property.GetParameterValue("sun_use", eval_time);
		if (sun_use)
		{//add additional sun light
			ospray::cpp::Light sun_light = sync_sun(m_render_property.GetParameterValue("sun_intensity", eval_time),
				m_render_property.GetParameterValue("sun_visible", eval_time),
				m_render_property.GetParameterValue("sun_direction_x", eval_time),
				m_render_property.GetParameterValue("sun_direction_y", eval_time),
				m_render_property.GetParameterValue("sun_direction_z", eval_time),
				m_render_property.GetParameterValue("sun_turbidity", eval_time),
				m_render_property.GetParameterValue("sun_albedo", eval_time),
				m_render_property.GetParameterValue("sun_horizon_extension", eval_time));

			world_lights.push_back(sun_light);
			update_osp_sky_index = world_lights.size() - 1;
		}

		//scene lights
		const XSI::CRefArray& xsi_scene_lights = m_render_context.GetArrayAttribute("Lights");
		for (ULONG i = 0; i < xsi_scene_lights.GetCount(); i++)
		{
			XSI::Light xsi_light(xsi_scene_lights.GetItem(i));
			add_light(xsi_light);
		}

		create_xsi_geometries_id.clear();
		create_xsi_geometries_id.shrink_to_fit();
	}

	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineOSP::update_light(const ULONG xsi_id, XSI::X3DObject &xsi_object)
{
	int light_index = get_index_in_array(update_xsi_lights_id, xsi_id);
	if (light_index > -1)
	{
		if (xsi_object.GetType() == "light")
		{
			XSI::Light xsi_light(xsi_object);
			ospray::cpp::Light light = sync_light(eval_time, xsi_light);
			world_lights[update_osp_lights_index[light_index]] = light;
		}
		else
		{
			//it should be the light, but object is not a light
			return XSI::CStatus::Abort;
		}
		return XSI::CStatus::OK;
	}
	return XSI::CStatus::Undefined;
}

XSI::CStatus RenderEngineOSP::update_scene(XSI::X3DObject &xsi_object, const UpdateType update_type)
{
	if (update_type == UpdateType_Undefined || update_type == UpdateType_Camera || update_type == UpdateType_Render)
	{
		//do nothing, because camera we update every time
		//render settings updated in special method
	}
	else if (update_type == UpdateType_Visibility)
	{
		//change visibility is not supported by this render, so recreate the scene
		return XSI::CStatus::Abort;
	}
	else if (update_type == UpdateType_Transform)
	{
		ULONG xsi_id = xsi_object.GetObjectID();
		//try to find objects in meshes
		int mesh_index = get_index_in_array(update_xsi_meshes_id, xsi_id);
		if (mesh_index > -1)
		{
			sync_transform(eval_time, xsi_object, world_instances[update_osp_meshes_index[mesh_index]], use_motion, motion_shatter_time, motion_steps);
		}
		else
		{
			//object not in meshes array, try find in others
			//may be this is a light
			XSI::CStatus upd = update_light(xsi_id, xsi_object);
			if (upd == XSI::CStatus::Abort) { return XSI::CStatus::Abort; }
			else if(upd == XSI::CStatus::Undefined)
			{
				//this is not the light, check other
				int box_id = get_index_in_array(update_xsi_boxes_id, xsi_id);
				if (box_id > -1)
				{
					sync_transform(eval_time, xsi_object, world_instances[update_osp_boxes_index[box_id]], use_motion, motion_shatter_time, motion_steps);
				}
				else
				{
					//this is not boxes, try spheres
					int sphere_id = get_index_in_array(update_xsi_spheres_id, xsi_id);
					if (sphere_id > -1)
					{
						sync_transform(eval_time, xsi_object, world_instances[update_osp_spheres_index[sphere_id]], use_motion, motion_shatter_time, motion_steps);
					}
					else
					{
						//not spheres, try other object
						int volume_id = get_index_in_array(update_xsi_particle_volume_id, xsi_id);
						if (volume_id > -1)
						{
							sync_transform(eval_time, xsi_object, world_instances[update_osp_particle_volume_index[volume_id]], use_motion, motion_shatter_time, motion_steps);
						}
						else
						{
							int strand_id = get_index_in_array(update_xsi_strands_id, xsi_id);
							if (strand_id > -1)
							{
								sync_transform(eval_time, xsi_object, world_instances[update_osp_strands_index[strand_id]], use_motion, motion_shatter_time, motion_steps);
							}
							else
							{
								int hair_id = get_index_in_array(update_xsi_hairs_id, xsi_id);
								if (hair_id > -1)
								{
									sync_transform(eval_time, xsi_object, world_instances[update_osp_hairs_index[hair_id]], use_motion, motion_shatter_time, motion_steps);
								}
								else
								{
									//we should update transform of the unknown object, skip it
								}
							}
						}
					}
				}
			}
		}
	}
	else if (update_type == UpdateType_XsiLight)
	{
		ULONG xsi_id = xsi_object.GetObjectID();
		XSI::CStatus upd = update_light(xsi_id, xsi_object);
		if (upd == XSI::CStatus::Abort)
		{
			return XSI::CStatus::Abort;
		}
		else if (upd == XSI::CStatus::Undefined)
		{
			//there are no such light, this is error, because it should be
			return XSI::CStatus::Abort;
		}
	}
	else if (update_type == UpdateType_GlobalAmbient)
	{
		//ambient always in the array and has index = 0, so, simply update it
		sync_ambient(eval_time, ambient_light);
	}
	else if (update_type == UpdateType_Mesh)
	{
		//update the actual mesh
		ULONG xsi_id = xsi_object.GetObjectID();
		//try to find objects in meshes
		int mesh_index = get_index_in_array(update_xsi_meshes_id, xsi_id);
		if (mesh_index > -1)
		{
			//rebuild the instance
			ospray::cpp::Instance mesh_instance = sync_polygonmesh(eval_time, xsi_object, get_material_index(xsi_object));
			//update it transform, because this is a new object
			sync_transform(eval_time, xsi_object, mesh_instance, use_motion, motion_shatter_time, motion_steps);

			//replace it in update array
			world_instances[update_osp_meshes_index[mesh_index]] = mesh_instance;
		}
		else
		{//there are no mesh object, but it should be
			return XSI::CStatus::Abort;
		}
	} 
	else if (update_type == UpdateType_Hair)
	{
		ULONG xsi_id = xsi_object.GetObjectID();
		int hair_index = get_index_in_array(update_xsi_hairs_id, xsi_id);
		if (hair_index > -1)
		{
			ospray::cpp::Instance hair_instance = sync_hair(eval_time, xsi_object, get_curve_type(m_render_property.GetParameterValue("curve_type", eval_time)), get_curve_basis(m_render_property.GetParameterValue("curve_basis", eval_time)), get_material_index(xsi_object));
			sync_transform(eval_time, xsi_object, hair_instance, use_motion, motion_shatter_time, motion_steps);
			world_instances[update_osp_hairs_index[hair_index]] = hair_instance;
		}
		else
		{
			return XSI::CStatus::Abort;
		}
	}
	else if (update_type == UpdateType_Pointcloud)
	{
		//we should update pointloud
		//we should remove instances, corresponds to the given id from boxes, spheres and particles volume and then create the new one
		PointcloudType pc_type = get_pointcloud_type(eval_time, xsi_object);
		if (pc_type == PointcloudType_Strands && is_xsi_object_visible(eval_time, xsi_object))
		{
			int strands_index = get_index_in_array(update_xsi_strands_id, xsi_object.GetObjectID());
			if (strands_index > -1)
			{
				//simply recreate the instance
				ospray::cpp::Group strands_group;
				bool is_strands = false;
				sync_strands(eval_time, xsi_object, strands_group, is_strands, get_curve_type(m_render_property.GetParameterValue("curve_type", eval_time)), get_curve_basis(m_render_property.GetParameterValue("curve_basis", eval_time)), get_material_index(xsi_object));
				if (is_strands)
				{
					ospray::cpp::Instance strands_instance(strands_group);
					sync_transform(eval_time, xsi_object, strands_instance, use_motion, motion_shatter_time, motion_steps);
					world_instances[update_osp_strands_index[strands_index]] = strands_instance;
				}
				else
				{
					//something wrong, recreate the scene
					return XSI::CStatus::Abort;
				}
			}
			else
			{
				return XSI::CStatus::Abort;
			}
		}
		else
		{
			//for volume pc or primitive pc - delete it and recreate
			remove_pointcloud(xsi_object);
			if (is_xsi_object_visible(eval_time, xsi_object))
			{
				add_pointcloud(xsi_object);
			}
		}
	}
	else
	{
		//unknown update type
	}
	
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineOSP::update_scene(const XSI::SIObject &si_object, const UpdateType update_type)
{
	if (update_type == UpdateType_Pass)
	{
		//try to update hdri-light from the pass environment shader
		ospray::cpp::Light hdr_light("hdri");
		bool is_add_hdri = sync_hdri(eval_time, hdr_light);
		if (is_add_hdri && update_osp_hdri_index > -1)
		{
			//change the light in the array
			world_lights[update_osp_hdri_index] = hdr_light;
		}
		else if (is_add_hdri && update_osp_hdri_index == -1)
		{
			//there is no light, but we create it
			world_lights.push_back(hdr_light);
			update_osp_hdri_index = world_lights.size() - 1;
		}
		else if (!is_add_hdri && update_osp_hdri_index > -1)
		{
			//the light was in the array, but now it's empty
			//so, we should remove the light
			remove_light(update_osp_hdri_index);
			update_osp_hdri_index = -1;
		}
	}

	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineOSP::update_scene(const XSI::Material &xsi_material)
{
	int mat_index = get_index_in_array(update_xsi_materials_id, xsi_material.GetObjectID());
	if (mat_index > -1)
	{
		ospray::cpp::Material osp_material = sync_material(eval_time, xsi_material, render_type == RenderType_Shaderball ? 2 : m_render_property.GetParameterValue("renderer", eval_time));
		render_materials[update_osp_materials_index[mat_index]] = osp_material;
	}
	else
	{
		return XSI::CStatus::Abort;
	}
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineOSP::update_scene_render()
{
	//may be we should add the sun light
	bool sun_use = m_render_property.GetParameterValue("sun_use", eval_time);
	if (sun_use && update_osp_sky_index == -1)
	{
		//no sun in lights, but we need it
		//add sun light at the end of lights
		ospray::cpp::Light sun_light = sync_sun(m_render_property.GetParameterValue("sun_intensity", eval_time),
												m_render_property.GetParameterValue("sun_visible", eval_time),
												m_render_property.GetParameterValue("sun_direction_x", eval_time),
												m_render_property.GetParameterValue("sun_direction_y", eval_time),
												m_render_property.GetParameterValue("sun_direction_z", eval_time),
												m_render_property.GetParameterValue("sun_turbidity", eval_time),
												m_render_property.GetParameterValue("sun_albedo", eval_time),
												m_render_property.GetParameterValue("sun_horizon_extension", eval_time));
		world_lights.push_back(sun_light);
		update_osp_sky_index = world_lights.size() - 1;
	}
	else if (!sun_use && update_osp_sky_index > -1)
	{
		//there is a sun, but we does not need it
		remove_light(update_osp_sky_index);
		update_osp_sky_index = -1;
	}
	else if(sun_use && update_osp_sky_index > -1)
	{
		//update sun parameters
		update_sun(world_lights[update_osp_sky_index],
					m_render_property.GetParameterValue("sun_intensity", eval_time),
					m_render_property.GetParameterValue("sun_visible", eval_time),
					m_render_property.GetParameterValue("sun_direction_x", eval_time),
					m_render_property.GetParameterValue("sun_direction_y", eval_time),
					m_render_property.GetParameterValue("sun_direction_z", eval_time),
					m_render_property.GetParameterValue("sun_turbidity", eval_time),
					m_render_property.GetParameterValue("sun_albedo", eval_time),
					m_render_property.GetParameterValue("sun_horizon_extension", eval_time));
	}

	//check, is the curve style is changes
	//if so, then we should recreate all strands
	int new_curve_type = m_render_property.GetParameterValue("curve_type", eval_time);
	if (new_curve_type != update_last_render_curve_type)
	{
		//we should recreate all strands objects
		for (LONG i = 0; i < update_xsi_strands_id.size(); i++)
		{
			ULONG xsi_id = update_xsi_strands_id[i];
			XSI::X3DObject xsi_object(XSI::Application().GetObjectFromID(xsi_id));
			ospray::cpp::Group strands_group;
			bool is_strands = false;
			sync_strands(eval_time, xsi_object, strands_group, is_strands, get_curve_type(m_render_property.GetParameterValue("curve_type", eval_time)), get_curve_basis(m_render_property.GetParameterValue("curve_basis", eval_time)), get_material_index(xsi_object));
			if (is_strands)
			{
				ospray::cpp::Instance strands_instance(strands_group);
				sync_transform(eval_time, xsi_object, strands_instance, use_motion, motion_shatter_time, motion_steps);
				world_instances[update_osp_strands_index[i]] = strands_instance;
			}
			else
			{
				//something wrong, recreate the scene
				return XSI::CStatus::Abort;
			}
		}

		//and all hair objects
		for (LONG i = 0; i < update_xsi_hairs_id.size(); i++)
		{
			ULONG xsi_id = update_xsi_hairs_id[i];
			XSI::X3DObject xsi_object(XSI::Application().GetObjectFromID(xsi_id));

			ospray::cpp::Instance hair_instance = sync_hair(eval_time, xsi_object, get_curve_type(m_render_property.GetParameterValue("curve_type", eval_time)), get_curve_basis(m_render_property.GetParameterValue("curve_basis", eval_time)), get_material_index(xsi_object));
			sync_transform(eval_time, xsi_object, hair_instance, use_motion, motion_shatter_time, motion_steps);
			world_instances[update_osp_hairs_index[i]] = hair_instance;
		}
	}

	if (use_motion && (!update_last_use_motion || update_last_motion_steps != motion_steps || abs(update_last_motion_shatter_time - motion_shatter_time) > 0.01f))
	{
		//we change motion blur parameter and the motion is active
		//then we should resync all instance transforms
		for (LONG i = 0; i < update_xsi_meshes_id.size(); i++)
		{
			XSI::X3DObject xsi_object(XSI::Application().GetObjectFromID(update_xsi_meshes_id[i]));
			sync_transform(eval_time, xsi_object, world_instances[update_osp_meshes_index[i]], use_motion, motion_shatter_time, motion_steps);
		}
		for (LONG i = 0; i < update_xsi_boxes_id.size(); i++)
		{
			XSI::X3DObject xsi_object(XSI::Application().GetObjectFromID(update_xsi_boxes_id[i]));
			sync_transform(eval_time, xsi_object, world_instances[update_osp_boxes_index[i]], use_motion, motion_shatter_time, motion_steps);
		}
		for (LONG i = 0; i < update_xsi_spheres_id.size(); i++)
		{
			XSI::X3DObject xsi_object(XSI::Application().GetObjectFromID(update_xsi_spheres_id[i]));
			sync_transform(eval_time, xsi_object, world_instances[update_osp_spheres_index[i]], use_motion, motion_shatter_time, motion_steps);
		}
		for (LONG i = 0; i < update_xsi_particle_volume_id.size(); i++)
		{
			XSI::X3DObject xsi_object(XSI::Application().GetObjectFromID(update_xsi_particle_volume_id[i]));
			sync_transform(eval_time, xsi_object, world_instances[update_osp_particle_volume_index[i]], use_motion, motion_shatter_time, motion_steps);
		}
		for (LONG i = 0; i < update_xsi_strands_id.size(); i++)
		{
			XSI::X3DObject xsi_object(XSI::Application().GetObjectFromID(update_xsi_strands_id[i]));
			sync_transform(eval_time, xsi_object, world_instances[update_osp_strands_index[i]], use_motion, motion_shatter_time, motion_steps);
		}
		for (LONG i = 0; i < update_xsi_hairs_id.size(); i++)
		{
			XSI::X3DObject xsi_object(XSI::Application().GetObjectFromID(update_xsi_hairs_id[i]));
			sync_transform(eval_time, xsi_object, world_instances[update_osp_hairs_index[i]], use_motion, motion_shatter_time, motion_steps);
		}
	}

	return XSI::CStatus::OK;
}

void RenderEngineOSP::render()
{
	//save last used render params
	update_last_render_curve_type = m_render_property.GetParameterValue("curve_type", eval_time);
	update_last_use_motion = use_motion;
	update_last_motion_shatter_time = motion_shatter_time;
	update_last_motion_steps = motion_steps;

	m_render_context.ProgressUpdate("Rendering...", "Rendering...", 0);

	ospray::cpp::World render_world;
	
	if (world_instances.size() > 0)
	{
		render_world.setParam("instance", ospray::cpp::CopiedData(world_instances));
	}

	if (world_lights.size() > 0)
	{
		render_world.setParam("light", ospray::cpp::CopiedData(world_lights));
	}
	render_world.commit();

	if (render_materials.size() > 0)
	{
		render_render.setParam("material", ospray::cpp::CopiedData(render_materials));
		render_render.commit();
	}

	//recreate render buffer
	//for the buffer we should set all output channels, visual channel and accumulation channel
	ospray::cpp::FrameBuffer framebuffer(image_size_width, image_size_height, OSP_FB_RGBA32F, join_osp_channels(output_osp_channels, visual_channel));
	if (use_denoising)
	{//crash here, we block the setting from ui
		ospray::cpp::ImageOperation denoise_operator("denoiser");
		framebuffer.setParam("imageOperation", ospray::cpp::CopiedData(denoise_operator));
		framebuffer.commit();
	}
	if (use_tonemapping)
	{
		ospray::cpp::ImageOperation tonemapper("tonemapper");
		tonemapper.setParam("exposure", tonemapping_exposure);
		tonemapper.setParam("contrast", tonemapping_contrast);
		tonemapper.setParam("shoulder", tonemapping_shoulder);
		tonemapper.setParam("midIn", tonemapping_mid_in);
		tonemapper.setParam("midOut", tonemapping_mid_out);
		tonemapper.setParam("hdrMax", tonemapping_hdr_max);
		tonemapper.setParam("acesColor", tonemapping_aces_color);
		framebuffer.setParam("imageOperation", ospray::cpp::CopiedData(tonemapper));
		framebuffer.commit();
	}
	framebuffer.clear();

	//render
	while (current_frame < render_frames)
	{
		render_task = framebuffer.renderFrame(render_render, render_camera, render_world);
		render_task.wait();
		//output buffer to the RenderTile
		copy_pixels_from_osp_buffer(visual_buffer.pixels, 0, image_size_width, image_size_height, framebuffer, visual_channel, apply_cc && cc_mode == 0);
		//next transfer visual to the tile
		m_render_context.NewFragment(RenderTile(visual_buffer.corner_x, visual_buffer.corner_y, visual_buffer.width, visual_buffer.height, visual_buffer.pixels, false, 4));

		//next save output pixels (if we need it)
		for (LONG i = 0; i < output_osp_channels.size(); i++)
		{
			//copy i-th buffer
			bool ignore_srgb = false;
			//we may ignore srgb if render output to hdr format
			if (i < output_bits.size())
			{
				int bit_size = output_bits[i];
				if (bit_size == 21)
				{
					ignore_srgb = true;
				}
			}
			copy_pixels_from_osp_buffer(output_pixels, i * image_size_width * image_size_height * 4, image_size_width, image_size_height, framebuffer, output_osp_channels[i], !ignore_srgb && apply_cc && cc_mode == 0);
		}

		current_frame++;
		m_render_context.ProgressUpdate("Rendering...", "Rendering...", int((float)current_frame * 100.0f / (float)render_frames));
	}
}

XSI::CStatus RenderEngineOSP::post_render_engine()
{
	output_osp_channels.clear();
	output_osp_channels.shrink_to_fit();

	//log render time
	double time = (finish_render_time - start_prepare_render_time) / CLOCKS_PER_SEC;
	if (render_type != RenderType_Shaderball)
	{
		log("[OSPRay Render] Render time: " + XSI::CString(time) + " sec.");
	}

	return XSI::CStatus::OK;
}

void RenderEngineOSP::abort()
{
	current_frame = render_frames;
}

void RenderEngineOSP::clear_engine()
{
	output_osp_channels.clear();
	output_osp_channels.shrink_to_fit();

	clear_buffers();
}