#include <xsi_kinematics.h>
#include <xsi_transformation.h>
#include <xsi_time.h>
#include <xsi_camera.h>

#include "../../utilities/logs.h"
#include "../includes_osp.h"
#include "exporter_includes.h"

#define DEG2RADF(_deg) ((_deg) * (float)(M_PI / 180.0))
#define RAD2DEGF(_rad) ((_rad) * (float)(180.0 / M_PI))

void set_camera_transform(ospray::cpp::Camera &osp_camera, 
						  float tr_00, float tr_01, float tr_02, float tr_03, 
						  float tr_10, float tr_11, float tr_12, float tr_13, 
						  float tr_20, float tr_21, float tr_22, float tr_23, 
						  float tr_30, float tr_31, float tr_32, float tr_33)
{
	osp_camera.setParam("transform", 
		affine3f(vec3f{ tr_00, tr_01, tr_02 },
				 vec3f{ tr_10, tr_11, tr_12 },
				 vec3f{ tr_20, tr_21, tr_22 },
				 vec3f{ tr_30, tr_31, tr_32 }));
	osp_camera.commit();
}

void sync_camera(const XSI::CTime &eval_time, 
				 const XSI::Camera &xsi_camera, 
				 ospray::cpp::Camera &osp_camera, 
				 CameraType camera_type, 
				 bool camera_architectural, float camera_aperture_radius, 
				 int crop_x, int crop_y, int total_width, int total_height, int frame_width, int frame_height,
				 const bool use_motion, const float shutter_time, const int steps)
{
	//global position
	XSI::MATH::CTransformation xsi_tfm = xsi_camera.GetKinematics().GetGlobal().GetTransform();
	XSI::MATH::CVector3 xsi_position = xsi_tfm.GetTranslation();
	osp_camera.setParam("position", vec3f{ 0.0f, 0.0f, 0.0f });

	XSI::MATH::CMatrix4 xsi_matrix = xsi_tfm.GetMatrix4();
	osp_camera.setParam("transform", matrix_to_affine(xsi_matrix, true));

	//frame crop
	osp_camera.setParam("imageStart", vec2f{ (float)crop_x / (float)total_width, (float)crop_y / (float)total_height });
	osp_camera.setParam("imageEnd", vec2f{ (float)(crop_x + frame_width) / (float)total_width, (float)(crop_y + frame_height) / (float)total_height });

	if (camera_type == Camera_Perspective)
	{
		float aspect = xsi_camera.GetParameterValue("aspect", eval_time);
		float fov = xsi_camera.GetParameterValue("fov", eval_time);
		int fov_type = xsi_camera.GetParameterValue("fovtype", eval_time);
		if (fov_type == 0)
		{
			//vertical fov, use it
			osp_camera.setParam("fovy", fov);
		}
		else
		{
			//recalculate fov from horizontal to vertical
			fov = RAD2DEGF(2 * atan(tan(DEG2RADF(fov) / 2.0) / aspect));
			osp_camera.setParam("fovy", fov);
		}
		osp_camera.setParam("aspect", aspect);

		//calculate focus distance
		XSI::X3DObject interest = xsi_camera.GetInterest();
		XSI::MATH::CVector3 interest_position = interest.GetKinematics().GetGlobal().GetTransform().GetTranslation();
		float distance = sqrt((interest_position.GetX() - xsi_position.GetX())*(interest_position.GetX() - xsi_position.GetX()) +
			(interest_position.GetY() - xsi_position.GetY())*(interest_position.GetY() - xsi_position.GetY()) +
			(interest_position.GetZ() - xsi_position.GetZ())*(interest_position.GetZ() - xsi_position.GetZ()));
		osp_camera.setParam("focusDistance", distance);

		//set constant aperture
		osp_camera.setParam("apertureRadius", camera_aperture_radius);

		//disable architectural
		osp_camera.setParam("architectural", camera_architectural);

		//set stereo
		osp_camera.setParam("stereoMode", OSP_STEREO_NONE);
		osp_camera.setParam("interpupillaryDistance", 0.0635f);
	}
	else if (camera_type == Camera_Orthographic)
	{
		//set aspect
		osp_camera.setParam("aspect", (float)xsi_camera.GetParameterValue("aspect", eval_time));

		//and height
		osp_camera.setParam("height", (float)xsi_camera.GetParameterValue("orthoheight", eval_time));
	}

	if (use_motion)
	{
		osp_camera.setParam("shutter", range1f(0.0f, 1.0f));
		//setup camera motion
		const float step_time = shutter_time / (steps - 1);
		const float shift_time = shutter_time / 2;
		XSI::KinematicState kinematic = xsi_camera.GetKinematics().GetGlobal();
		std::vector<affine3f> motion_transforms;
		for (int s = 0; s < steps; s++)
		{
			//calculate time
			const float time = eval_time - shift_time + s * step_time;
			XSI::MATH::CTransformation time_tfm = kinematic.GetTransform(time);
			XSI::MATH::CMatrix4 time_matrix = time_tfm.GetMatrix4();
			motion_transforms.push_back(matrix_to_affine(time_matrix, true));
		}

		osp_camera.setParam("motion.transform", ospray::cpp::CopiedData(motion_transforms));
	}
	else
	{
		osp_camera.removeParam("motion.transform");
		osp_camera.removeParam("shutter");
	}

	osp_camera.commit();
}