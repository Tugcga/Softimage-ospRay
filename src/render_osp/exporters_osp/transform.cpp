#include <xsi_time.h>
#include <xsi_x3dobject.h>
#include <xsi_kinematics.h>
#include <xsi_transformation.h>

#include "../../utilities/logs.h"
#include "../includes_osp.h"

affine3f matrix_to_affine(const XSI::MATH::CMatrix4 &xsi_matrix, const bool invert_z = false)
{
	return affine3f(vec3f{ (float)xsi_matrix.GetValue(0, 0), (float)xsi_matrix.GetValue(0, 1), (float)xsi_matrix.GetValue(0, 2) },
					vec3f{ (float)xsi_matrix.GetValue(1, 0), (float)xsi_matrix.GetValue(1, 1), (float)xsi_matrix.GetValue(1, 2) },
					vec3f{ (invert_z ? -1.0f : 1.0f)*(float)xsi_matrix.GetValue(2, 0), (invert_z ? -1.0f : 1.0f)*(float)xsi_matrix.GetValue(2, 1), (invert_z ? -1.0f : 1.0f)*(float)xsi_matrix.GetValue(2, 2) },
					vec3f{ (float)xsi_matrix.GetValue(3, 0), (float)xsi_matrix.GetValue(3, 1), (float)xsi_matrix.GetValue(3, 2) });
}

void sync_transform(const XSI::CTime &eval_time, const XSI::X3DObject &xsi_object, ospray::cpp::Instance &osp_instance, const bool use_motion, const float shutter_time, const int steps)
{
	//get transform of the xsi object
	XSI::MATH::CTransformation xsi_tfm = xsi_object.GetKinematics().GetGlobal().GetTransform(eval_time.GetTime());
	XSI::MATH::CMatrix4 xsi_matrix = xsi_tfm.GetMatrix4();
	osp_instance.setParam("transform", matrix_to_affine(xsi_matrix));

	if (use_motion)
	{
		const float step_time = shutter_time / (steps - 1);
		const float shift_time = shutter_time / 2;
		XSI::KinematicState kinematic = xsi_object.GetKinematics().GetGlobal();
		std::vector<affine3f> motion_transforms;
		for (int s = 0; s < steps; s++)
		{
			//calculate time
			const float time = eval_time - shift_time + s * step_time;
			XSI::MATH::CTransformation time_tfm = kinematic.GetTransform(time);
			XSI::MATH::CMatrix4 time_matrix = time_tfm.GetMatrix4();
			motion_transforms.push_back(matrix_to_affine(time_matrix));
		}

		osp_instance.setParam("motion.transform", ospray::cpp::CopiedData(motion_transforms));
	}
	else
	{
		osp_instance.removeParam("motion.transform");
	}

	osp_instance.commit();
}