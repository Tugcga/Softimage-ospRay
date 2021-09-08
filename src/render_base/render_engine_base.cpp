#include <xsi_framebuffer.h>
#include <xsi_primitive.h>
#include <xsi_projectitem.h>
#include <xsi_shader.h>
#include <xsi_clusterproperty.h>

#include <ctime>

#include "render_engine_base.h"
#include "../utilities/logs.h"
#include "../si_callbacks/si_callbacks.h"
#include "../utilities/write_image.h"
#include "../utilities/arrays.h"

RenderEngineBase::RenderEngineBase()
{
	ready_to_render = true;
	force_recreate_scene = true;
	prev_isolated_objects.Clear();
}

RenderEngineBase::~RenderEngineBase()
{
	ready_to_render = false;
	force_recreate_scene = true;
	prev_isolated_objects.Clear();
	visual_buffer.clear();
}

void RenderEngineBase::set_render_options_name(const XSI::CString &options_name)
{
	XSI::CStringArray parts = options_name.Split(" ");
	render_options_name = XSI::CString(parts[0]);
	for (LONG i = 0; i < parts.GetCount(); i++)
	{
		render_options_name += "_" + parts[i];
	}
}

//next three mathods define ui behaviour
XSI::CStatus RenderEngineBase::render_options_update(XSI::PPGEventContext &event_context)
{
	log("[Base Render] Render options update is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineBase::render_option_define_layout(XSI::Context &contextt)
{
	log("[Base Render] Layout define is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineBase::render_option_define(XSI::CustomProperty &property)
{
	log("[Base Render] Render parameters definitions is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

//next methods should be implemented in render engine
XSI::CStatus RenderEngineBase::pre_render_engine()
{
	log("[Base Render] Pre-render method is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineBase::pre_scene_process()
{
	log("[Base Render] Pre-scene method is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineBase::create_scene()
{
	log("[Base Render] Create scene method is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineBase::update_scene(XSI::X3DObject &xsi_object, const UpdateType update_type)
{
	log("[Base Render] Update scene for X3DObject is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineBase::update_scene(const XSI::SIObject &si_object, const UpdateType update_type)
{
	log("[Base Render] Update scene for SIObject is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineBase::update_scene(const XSI::Material &xsi_material)
{
	log("[Base Render] Update scene for material is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineBase::update_scene_render()
{
	log("[Base Render] Update scene for render parameters is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

XSI::CStatus RenderEngineBase::post_render_engine()
{
	log("[Base Render] Post-render method is not implemented", XSI::siWarningMsg);
	return XSI::CStatus::OK;
}

void RenderEngineBase::abort()
{
	log("[Base Render] Abort render engine is not implemented", XSI::siWarningMsg);
}

void RenderEngineBase::clear_engine()
{
	log("[Base Render] Clear render engine is not implemented", XSI::siWarningMsg);
}

//main render method, where we start and update rendering process
void RenderEngineBase::render()
{
	//this is very basic render process
	//we fill the frame by randomly selected grey color
	//create random color
	float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	std::vector<float> frame_pixels(visual_buffer.width * visual_buffer.height * 4);
	for (ULONG y = 0; y < visual_buffer.height; y++)
	{
		for (ULONG x = 0; x < visual_buffer.width; x++)
		{
			frame_pixels[4 * (visual_buffer.width * y + x)] = r;
			frame_pixels[4 * (visual_buffer.width * y + x) + 1] = r;
			frame_pixels[4 * (visual_buffer.width * y + x) + 2] = r;
			frame_pixels[4 * (visual_buffer.width * y + x) + 3] = 1.0;
		}
	}
	m_render_context.NewFragment(RenderTile(visual_buffer.corner_x, visual_buffer.corner_y, visual_buffer.width, visual_buffer.height, frame_pixels, false, 4));
	frame_pixels.clear();
	frame_pixels.shrink_to_fit();
}

//---------------------------------------------------------------------------------
//----------------------------------called from Softimage--------------------------

inline bool is_file_exists(const std::string& name)
{
	if (FILE *file = fopen(name.c_str(), "r"))
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
}

XSI::CStatus RenderEngineBase::pre_render(XSI::RendererContext &render_context)
{
	//save render context
	m_render_context = render_context;
	m_render_property = m_render_context.GetRendererProperty(eval_time);
	//current time
	eval_time = render_context.GetTime();
	start_render_time = clock();

	//get pathes to save images
	output_paths.Clear();
	bool is_skip = m_render_context.GetAttribute("SkipExistingFiles");
	bool file_output = m_render_context.GetAttribute("FileOutput");

	XSI::CRefArray frame_buffers = render_context.GetFramebuffers();
	const LONG fb_count = frame_buffers.GetCount();
	for (LONG i = 0; i < fb_count; ++i)
	{
		XSI::Framebuffer fb(frame_buffers[i]);
		XSI::CValue enbable_val = fb.GetParameterValue("Enabled", eval_time.GetTime());
		if (file_output && enbable_val)
		{
			XSI::CString output_path = fb.GetResolvedPath(eval_time);
			//check is this file exists if is_skip is active
			if (is_skip && is_file_exists(output_path.GetAsciiString()))
			{
				//we shpuld skip this pass to render
				log("[Base Render] File " + output_path + " exists, skip it.");
			}
			else
			{
				//add path to the render
				output_paths.Add(output_path);

				XSI::CString fb_format = fb.GetParameterValue("Format");
				XSI::CString fb_data_type = fb.GetParameterValue("DataType");
				int fb_bits = fb.GetParameterValue("BitDepth");
				XSI::CString fb_channel_name = remove_digits(fb.GetName());
				
				//save output data to array buffers
				output_formats.Add(fb_format);
				output_data_types.Add(fb_data_type);
				output_channels.Add(fb_channel_name);
				output_bits.push_back(fb_bits);
			}
		}
	}

	XSI::CString render_type_str = render_context.GetAttribute("RenderType");
	render_type = render_type_str == XSI::CString("Pass") ? RenderType_Pass :
		(render_type_str == XSI::CString("Region") ? RenderType_Region : (
			render_type_str == XSI::CString("Shaderball") ? RenderType_Shaderball : RenderType_Rendermap));

	if (render_type == RenderType_Pass && file_output && output_paths.GetCount() == 0)
	{//this is Pass render, but nothing to render
		return XSI::CStatus::Abort;
	}
	
	return pre_render_engine();
}

XSI::CStatus RenderEngineBase::interrupt_update_scene()
{
	ready_to_render = true;
	return end_render_event(m_render_context, output_paths, true);
}

bool RenderEngineBase::is_recreate_isolated_view(const XSI::CRefArray &visible_objects)
{
	ULONG current_isolated = visible_objects.GetCount();
	ULONG prev_isolated = prev_isolated_objects.GetCount();
	if (current_isolated != prev_isolated)
	{//switch from one mode to other
		if (current_isolated == 0)
		{//now the mode is non-isolated
			prev_isolated_objects.Clear();
		}
		else
		{//the mode is isolated, save links to objects
			prev_isolated_objects.Clear();
			for (LONG i = 0; i < visible_objects.GetCount(); i++)
			{
				prev_isolated_objects.Add(visible_objects[i]);
			}
		}

		return true;
	}

	//mode is the same as in the previous render session
	if (current_isolated == 0)
	{// no isolation
		prev_isolated_objects.Clear();
		return false;
	}
	else
	{//isolation, and previous is also isolation
		bool is_recreate = false;
		for (LONG i = 0; i < visible_objects.GetCount(); i++)
		{
			if (!is_contains(prev_isolated_objects, visible_objects[i]))
			{
				i = visible_objects.GetCount();
				is_recreate = true;
			}
		}
		//recreate links
		prev_isolated_objects.Clear();
		for (LONG i = 0; i < visible_objects.GetCount(); i++)
		{
			prev_isolated_objects.Add(visible_objects[i]);
		}
		return is_recreate;
	}
}

void RenderEngineBase::scene_process()
{
	ready_to_render = false;

	//setup visual buffer
	visual_buffer = RenderVisualBuffer(
		(ULONG)m_render_context.GetAttribute("ImageWidth"),
		(ULONG)m_render_context.GetAttribute("ImageHeight"),
		(ULONG)m_render_context.GetAttribute("CropLeft"),
		(ULONG)m_render_context.GetAttribute("CropBottom"),
		(ULONG)m_render_context.GetAttribute("CropWidth"),
		(ULONG)m_render_context.GetAttribute("CropHeight"),
		4);

	//get render image and how many frames we should render
	image_full_size_widht = m_render_context.GetAttribute("ImageWidth");
	image_full_size_height = m_render_context.GetAttribute("ImageHeight");
	image_corner_x = m_render_context.GetAttribute("CropLeft");
	image_corner_y = m_render_context.GetAttribute("CropBottom");
	image_size_width = m_render_context.GetAttribute("CropWidth");
	image_size_height = m_render_context.GetAttribute("CropHeight");

	if (output_paths.GetCount() > 0)
	{//prepare output pixel buffers
		output_pixels = std::vector<float>(image_size_width * image_size_height * 4 * output_paths.GetCount(), 0.0f);
	}

	//next all other general staff for the engine
	XSI::CStatus status = pre_scene_process();
	//if status is Abort, then drop update and recreate the scene

	//next we start update or rectreate the scene
	//we should recreate the scene when 
	//- force it
	//- with Pass render mode
	//- rendermap render mode
	//- empty dirty list
	//- recreate isolated view
	//- if we have undefined dirty list update

	XSI::CValue dirty_refs_value = m_render_context.GetAttribute("DirtyList");

	if (status != XSI::CStatus::OK ||
		force_recreate_scene || 
		render_type == RenderType_Pass || 
		render_type == RenderType_Rendermap ||
		dirty_refs_value.IsEmpty())
	{//recreate the scene
		force_recreate_scene = false;
		create_scene();
	}
	else
	{
		//check isolated view
		XSI::CRefArray visible_objects = m_render_context.GetAttribute("ObjectList");
		//get object in the isolated view
		if (visible_objects.GetCount() > 0)
		{//add to objects in isolated view all lights
			XSI::CRefArray lights_array = m_render_context.GetAttribute("Lights");
			for (LONG i = 0; i < lights_array.GetCount(); i++)
			{
				XSI::CRef ref = lights_array.GetItem(i);
				if (!is_contains(visible_objects, ref))
				{
					visible_objects.Add(ref);
				}
			}
		}
		bool is_isolated_new = is_recreate_isolated_view(visible_objects);
		if (is_isolated_new)
		{
			//new isolation view, recreate the scene
			force_recreate_scene = false;
			create_scene();
		}
		else
		{
			//so, here we can update the scene
			//we should check all dirty objects
			XSI::Primitive camera_prim(m_render_context.GetAttribute("Camera"));
			XSI::X3DObject camera_obj = camera_prim.GetOwners()[0];

			XSI::CRefArray dirty_refs = dirty_refs_value;
			for (LONG i = 0; i < dirty_refs.GetCount(); i++)
			{
				XSI::CRef in_ref(dirty_refs[i]);
				XSI::SIObject xsi_obj = XSI::SIObject(in_ref);
				XSI::siClassID class_id = in_ref.GetClassID();
				XSI::CStatus update_status(XSI::CStatus::Undefined);
				switch (class_id)
				{
					case XSI::siStaticKinematicStateID:
					case XSI::siConstraintWithUpVectorID:
					case XSI::siKinematicStateID:
					{
						bool is_global = strstr(in_ref.GetAsText().GetAsciiString(), ".global");
						if (is_global)
						{
							//update global transform
							XSI::X3DObject xsi_3d_obj(XSI::SIObject(XSI::SIObject(in_ref).GetParent()).GetParent());
							if (xsi_3d_obj.GetObjectID() == camera_obj.GetObjectID())
							{
								//update camera transform
								update_status = update_scene(xsi_3d_obj, UpdateType_Camera);
							}
							else
							{
								//update global position of the scene object
								//but here we can move not only geometric object
								update_status = update_scene(xsi_3d_obj, UpdateType_Transform);
							}
						}
						//local and other transform should be ignored
						break;
					}
					case XSI::siCustomPrimitiveID:
					case XSI::siPrimitiveID:
					case XSI::siClusterID:
					case XSI::siClusterPropertyID:
					{
						XSI::X3DObject xsi_3d_obj(xsi_obj.GetParent());
						if (xsi_obj.GetType() == XSI::siPolyMeshType)
						{
							update_status = update_scene(xsi_3d_obj, UpdateType_Mesh);
						}
						else if (xsi_obj.GetType() == "pointcloud")
						{
							update_status = update_scene(xsi_3d_obj, UpdateType_Pointcloud);
						}
						else if (xsi_obj.GetType() == "light")
						{
							update_status = update_scene(xsi_3d_obj, UpdateType_XsiLight);
						}
						else if (xsi_obj.GetType() == "camera" && xsi_3d_obj.GetObjectID() == camera_obj.GetObjectID())
						{
							update_status = update_scene(xsi_3d_obj, UpdateType_Camera);
						}
						else if (xsi_obj.GetType() == "poly" || xsi_obj.GetType() == "sample")
						{
							//change cluster in the polygonmesh
							update_status = update_scene(XSI::X3DObject(XSI::SIObject(xsi_obj.GetParent()).GetParent()), UpdateType_Mesh);
						}
						else
						{
							//unknown update of the primitive or cluster
						}
						
						break;
					}
					case XSI::siMaterialID:
					{
						XSI::Material xsi_material(in_ref);
						update_status = update_scene(xsi_material);
						break;
					}
					case XSI::siParameterID:
					{
						//here called update for the ambience parameter, but we catch it in another place
						//or some other unknown parameter
						break;
					}
					case XSI::siCustomPropertyID:
					case XSI::siPropertyID:
					{
						XSI::CString property_type(xsi_obj.GetType());
						XSI::X3DObject xsi_3d_obj(xsi_obj.GetParent());
						if (property_type == "visibility")
						{
							update_status = update_scene(xsi_3d_obj, UpdateType_Visibility);
						}
						else if (property_type == "geomapprox")
						{
							update_status = update_scene(xsi_3d_obj, UpdateType_Mesh);
						}
						else if (property_type == "RenderRegion")
						{
							update_status = update_scene_render();
						}
						else if (property_type == render_options_name)
						{
							update_status = update_scene_render();
						}
						else if (property_type == "AmbientLighting")
						{
							update_status = update_scene(xsi_3d_obj, UpdateType_GlobalAmbient);
						}
						else
						{
							//get parent object, and if this is a pointcloud, hair or mesh - update it
							if (xsi_3d_obj.GetType() == XSI::siPolyMeshType)
							{
								update_status = update_scene(xsi_3d_obj, UpdateType_Mesh);
							}
							else if (xsi_3d_obj.GetType() == "pointcloud")
							{
								update_status = update_scene(xsi_3d_obj, UpdateType_Pointcloud);
							}
							else if (xsi_3d_obj.GetType() == "light")
							{
								update_status = update_scene(xsi_3d_obj, UpdateType_XsiLight);
							}
							else
							{
								//update unknown property
							}
						}
						break;
					}
					case XSI::siHairPrimitiveID:
					{
						XSI::X3DObject xsi_3d_obj(xsi_obj.GetParent());
						update_status = update_scene(xsi_3d_obj, UpdateType_Hair);
						break; 
					}
					case XSI::siShaderID:
					{
						XSI::Shader xsi_shader(in_ref);
						XSI::CRef shader_root = xsi_shader.GetRoot();
						if (shader_root.GetClassID() == XSI::siMaterialID)
						{
							XSI::Material shader_material(shader_root);
							update_status = update_scene(shader_material);
						}
						else
						{
							//update shader inside material
							//ignore this update, because we will catch material update in other place
						}
						break;
					}
					case XSI::siSIObjectID:
					case XSI::siX3DObjectID:
					{
						//ignore this update
						break;
					}
					case XSI::siPassID:
					{
						update_status = update_scene(xsi_obj, UpdateType_Pass);
						break;
					}
					default:
					{
						//unknown update
					}
				}

				if (update_status == XSI::CStatus::Abort)
				{
					//update is fail, recreate the scene
					force_recreate_scene = false;
					create_scene();
					break;
				}
			}
		}
	}

	//after scene creation, clear dirty list
	bool empty_dirty_list = dirty_refs_value.IsEmpty();
	if (empty_dirty_list)
	{
		m_render_context.SetObjectClean(XSI::CRef());
	}
	else
	{
		XSI::CRefArray dirty_refs = dirty_refs_value;
		for (int i = 0; i < dirty_refs.GetCount(); i++)
		{
			m_render_context.SetObjectClean(dirty_refs[i]);
		}
	}
}

XSI::CStatus RenderEngineBase::start_render()
{
	XSI::CStatus status;

	if (begin_render_event(m_render_context, output_paths) != XSI::CStatus::OK)
	{
		return XSI::CStatus::Fail;
	}

	m_render_context.NewFrame(visual_buffer.full_width, visual_buffer.full_height);

	start_prepare_render_time = clock();

	render();

	finish_render_time = clock();

	if (end_render_event(m_render_context, output_paths, false) != XSI::CStatus::OK)
	{
		return XSI::CStatus::Fail;
	}
	ready_to_render = true;
	return status;
}

XSI::CStatus RenderEngineBase::post_render()
{
	//save output images
	for (LONG i = 0; i < output_paths.GetCount(); i++)
	{
		//construct subvector
		std::vector<float>::const_iterator first = output_pixels.begin() + i * image_size_width * image_size_height * 4;
		std::vector<float>::const_iterator last = output_pixels.begin() + (i + 1) * image_size_width * image_size_height * 4;
		std::vector<float> pixels(first, last);

		//sve these pixels
		write_float(output_paths[i], output_formats[i], output_data_types[i], image_size_width, image_size_height, 4, pixels);

		pixels.clear();
		pixels.shrink_to_fit();
	}

	XSI::CStatus status = post_render_engine();

	//clear output arrays if we need this
	output_paths.Clear();
	output_formats.Clear();
	output_data_types.Clear();
	output_channels.Clear();
	output_bits.clear();
	output_bits.shrink_to_fit();
	//clear output_pixels
	output_pixels.clear();
	output_pixels.shrink_to_fit();

	return status;
}

void RenderEngineBase::on_object_add(XSI::CRef& in_ctxt)
{
	force_recreate_scene = true;
}

void RenderEngineBase::on_object_remove(XSI::CRef& in_ctxt)
{
	force_recreate_scene = true;
}

void RenderEngineBase::clear()
{
	output_paths.Clear();
	prev_isolated_objects.Clear();
	visual_buffer.clear();

	output_formats.Clear();
	output_data_types.Clear();
	output_channels.Clear();
	output_bits.clear();
	output_bits.shrink_to_fit();
	output_pixels.clear();
	output_pixels.shrink_to_fit();

	clear_engine();
}