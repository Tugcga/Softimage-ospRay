#pragma once

#include "../render_engine_osp.h"
#include "../includes_osp.h"

void set_camera_transform(ospray::cpp::Camera &osp_camera,
						  float tr_00, float tr_01, float tr_02, float tr_03,
						  float tr_10, float tr_11, float tr_12, float tr_13,
						  float tr_20, float tr_21, float tr_22, float tr_23,
						  float tr_30, float tr_31, float tr_32, float tr_33);

void sync_camera(const XSI::CTime &eval_time,
				 const XSI::Camera &xsi_camera,
				 ospray::cpp::Camera &osp_camera,
				 CameraType camera_type,
				 bool camera_architectural,
				 float camera_aperture_radius,
				 int crop_x, int crop_y, int total_width, int total_height, int frame_width, int frame_height,
				 const bool use_motion, const float shutter_time, const int steps);

ospray::cpp::Instance sync_polygonmesh(const XSI::CTime &eval_time, 
									   XSI::X3DObject &xsi_object, 
									   const int material_index, 
									   const bool ignore_vertex_colors = false);

affine3f matrix_to_affine(const XSI::MATH::CMatrix4 &xsi_matrix, const bool invert_z = false);
void sync_transform(const XSI::CTime &eval_time, 
					const XSI::X3DObject &xsi_object, 
					ospray::cpp::Instance &osp_instance, 
					const bool use_motion,
					const float shutter_time, 
					const int steps);

ospray::cpp::Light sync_light(const XSI::CTime &eval_time, 
							  XSI::Light &xsi_light, 
							  const float multiplier = 1.0f);
void update_sun(ospray::cpp::Light &light, 
				float intensity, 
				bool visible, 
				float direction_x, float direction_y, float direction_z, 
				float turbidity, 
				float albedo, 
				float horizon_extension);
ospray::cpp::Light sync_sun(float intensity,
							bool visible,
							float direction_x, float direction_y, float direction_z,
							float turbidity,
							float albedo,
							float horizon_extension);
void sync_ambient(const XSI::CTime &eval_time,
				  ospray::cpp::Light &ambient_light);
bool sync_hdri(const XSI::CTime &eval_time, 
			   ospray::cpp::Light &hdr_light);

OSPFrameBufferChannel get_render_channel(const XSI::CString channel_name);
int get_chennels_count_from_osp_buffer(const OSPFrameBufferChannel osp_buffer);
std::vector<OSPFrameBufferChannel> get_output_osp_channels(const XSI::CStringArray &output_channels);
int join_osp_channels(const std::vector<OSPFrameBufferChannel> &array, 
					  const OSPFrameBufferChannel visual);
void copy_pixels_from_osp_buffer(std::vector<float> &pixels, 
								 LONG start_shift, 
								 int width, int height, 
								 ospray::cpp::FrameBuffer &framebuffer, 
								 OSPFrameBufferChannel osp_channel, 
								 bool apply_sRGB);
void sync_renderer(ospray::cpp::Renderer &renderer, 
				   RendererType renderer_type, 
				   const XSI::Property &render_property, 
				   const XSI::CTime &eval_time);

PointcloudType get_pointcloud_type(const XSI::CTime &eval_time, 
								   const XSI::X3DObject &xsi_object);
void sync_pointcloud_primitives(const XSI::CTime &eval_time, 
								const XSI::X3DObject &xsi_object, 
								ospray::cpp::Group &spheres_group, bool &is_spheres, 
								ospray::cpp::Group &boxes_group, bool &is_boxes, 
								ospray::cpp::Group &volume_group, bool &is_volume, 
								const int material_index);

OSPCurveType get_curve_type(const int type);
OSPCurveBasis get_curve_basis(const int basis);
void sync_strands(const XSI::CTime &eval_time, 
				  const XSI::X3DObject &xsi_object, 
				  ospray::cpp::Group &strands_group, bool &is_strands, 
				  const OSPCurveType curve_type, 
				  const OSPCurveBasis curve_basis, 
				  const int material_index);

ospray::cpp::Instance sync_hair(const XSI::CTime &eval_time, 
								const XSI::X3DObject &xsi_object, 
								const OSPCurveType curve_type, 
								const OSPCurveBasis curve_basis, 
								const int material_index);

ospray::cpp::Material sync_material(const XSI::CTime &eval_time, 
									const XSI::Material &xsi_material, 
									const int render_type);