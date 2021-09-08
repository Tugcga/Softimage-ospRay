#include <xsi_x3dobject.h>
#include <xsi_geometry.h>
#include <xsi_time.h>
#include <xsi_iceattribute.h>
#include <xsi_iceattributedataarray2D.h>
#include <xsi_primitive.h>

#include <vector>

#include "../../utilities/logs.h"
#include "../types.h"
#include "../includes_osp.h"

static XSI::CString transfer_attribute_name = "_osp_transfer";
static XSI::CString range_attribute_name = "_osp_transfer_range";
static XSI::CString isosurface_attribute_name = "_osp_isosurface";
static XSI::CString isovalue_attribute_name = "_osp_isovalue";

PointcloudType get_pointcloud_type(const XSI::CTime &eval_time, const XSI::X3DObject &xsi_object)
{
	XSI::Geometry geometry = xsi_object.GetActivePrimitive(eval_time.GetTime()).GetGeometry(eval_time.GetTime());
	
	//check is it em-fluids volume
	XSI::ICEAttribute a = geometry.GetICEAttributeFromName("__emFluid5_Enable");
	if (a.IsValid())
	{
		return PointcloudType_EmFluids;
	}

	XSI::ICEAttribute strand_count_attr = geometry.GetICEAttributeFromName("StrandCount");
	XSI::ICEAttribute strand_position_attr = geometry.GetICEAttributeFromName("StrandPosition");
	XSI::ICEAttribute point_position_attr = geometry.GetICEAttributeFromName("PointPosition");
	XSI::CICEAttributeDataArray2DVector3f strand_position_data;
	strand_position_attr.GetDataArray2D(strand_position_data);
	//may be this pointcloud contains strands
	if (strand_position_data.GetCount() > 0 && strand_count_attr.GetElementCount() > 0 && strand_position_attr.GetElementCount() > 0 && point_position_attr.GetElementCount() > 0)
	{
		return PointcloudType_Strands;
	}

	//no strands, may be it contains shape primitive
	XSI::ICEAttribute shape_attr = geometry.GetICEAttributeFromName("Shape");
	XSI::CICEAttributeDataArrayShape shape_data;
	shape_attr.GetDataArray(shape_data);
	if (shape_data.GetCount() > 0)
	{
		return PointcloudType_Primitives;
	}

	//next we can check custom volume attributes

	return PointcloudType_Undefined;
}

ospray::cpp::TransferFunction make_default_transfer_function()
{
	ospray::cpp::TransferFunction transfer_function("piecewiseLinear");
	std::vector<vec3f> colors{ vec3f{ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f } };
	std::vector<float> opacities{ 0.0f, 1.0f };
	transfer_function.setParam("color", ospray::cpp::CopiedData(colors));
	transfer_function.setParam("opacity", ospray::cpp::CopiedData(opacities));
	transfer_function.setParam("valueRange", vec2f{ 0.0f, 1.0f });
	transfer_function.commit();
	return transfer_function;
}

void sync_pointcloud_primitives(const XSI::CTime &eval_time, 
	const XSI::X3DObject &xsi_object, 
	ospray::cpp::Group &spheres_group,  bool &is_spheres, 
	ospray::cpp::Group &boxes_group, bool &is_boxes, 
	ospray::cpp::Group &volume_group, bool &is_volume, 
	const int material_index)
{
	XSI::Geometry geometry = xsi_object.GetActivePrimitive(eval_time.GetTime()).GetGeometry(eval_time.GetTime());
	//get all needed attributes
	XSI::ICEAttribute shape_attr = geometry.GetICEAttributeFromName("Shape");
	XSI::CICEAttributeDataArrayShape shape_data;
	shape_attr.GetDataArray(shape_data);  // sometimes xsi return only the first instance of the shape, but not the same number as particles count

	XSI::ICEAttribute position_attr = geometry.GetICEAttributeFromName("PointPosition");
	XSI::CICEAttributeDataArrayVector3f position_data;
	position_attr.GetDataArray(position_data);

	XSI::ICEAttribute orientation_attr = geometry.GetICEAttributeFromName("Orientation");
	XSI::CICEAttributeDataArrayRotationf rotation_data;
	orientation_attr.GetDataArray(rotation_data);

	XSI::ICEAttribute size_attr = geometry.GetICEAttributeFromName("Size");
	XSI::CICEAttributeDataArrayFloat size_data;
	size_attr.GetDataArray(size_data);
	
	XSI::ICEAttribute color_attr = geometry.GetICEAttributeFromName("Color");
	XSI::CICEAttributeDataArrayColor4f color_data;
	color_attr.GetDataArray(color_data);

	//prepare buffers for different type of data
	//for the volume
	std::vector<vec3f> volume_positions;
	std::vector<float> volume_radius;

	//for spheres
	std::vector<vec3f> spheres_positions;
	std::vector<float> spheres_radius;
	std::vector<vec4f> spheres_color;

	//boxes
	std::vector<box3f> boxes_bb;
	std::vector<vec4f> boxes_color;

	//iterate by particles
	bool is_override_shape = false;
	XSI::siICEShapeType override_shape = XSI::siICEShapeType::siICEShapePoint;
	if (shape_data.GetCount() < shape_attr.GetElementCount())
	{
		//this can be happens, when all particles has the same type
		//in this case override all particles types
		override_shape = shape_data.GetCount() > 0 ? shape_data[0].GetType() : XSI::siICEShapeType::siICEShapePoint;
		is_override_shape = true;
	}

	for (ULONG i = 0; i < shape_attr.GetElementCount(); i++)
	{
		XSI::MATH::CVector3f position = position_data[i];
		XSI::MATH::CRotationf rotation = rotation_data[i];
		XSI::MATH::CColor4f point_color = color_data[i];
		XSI::siICEShapeType shape_type = is_override_shape ? override_shape : shape_data[i].GetType();
		if (shape_type == XSI::siICEShapePoint)
		{
			//simply add position and radius to the output array
			volume_positions.push_back(vec3f{ (float)position.GetX(), (float)position.GetY() , (float)position.GetZ() });
			volume_radius.push_back(size_data[i]);
		}
		else if(shape_type == XSI::siICEShapeBox)
		{
			boxes_bb.push_back(box3f{ vec3f{position.GetX() - size_data[i], position.GetY() - size_data[i] , position.GetZ() - size_data[i] }, 
									  vec3f{ position.GetX() + size_data[i], position.GetY() + size_data[i] , position.GetZ() + size_data[i] } });
			boxes_color.push_back(vec4f{ point_color.GetR(), point_color.GetG(), point_color.GetB(), point_color.GetA() });
		}
		else if (shape_type == XSI::siICEShapeSphere)
		{
			spheres_positions.push_back(vec3f{ (float)position.GetX(), (float)position.GetY() , (float)position.GetZ() });
			spheres_radius.push_back(size_data[i]);
			spheres_color.push_back(vec4f{ point_color.GetR(), point_color.GetG(), point_color.GetB(), point_color.GetA() });
		}
		//rectangles are not suppoprted, because planes on the osp are infinite
		//all other shapes are not supported
	}

	//next create models
	if (spheres_positions.size() > 0)
	{//spheres are in the pointcliud
		ospray::cpp::Geometry spheres_geom("sphere");
		spheres_geom.setParam("sphere.position", ospray::cpp::CopiedData(spheres_positions));
		spheres_geom.setParam("sphere.radius", ospray::cpp::CopiedData(spheres_radius));
		spheres_geom.commit();

		//add to the model
		ospray::cpp::GeometricModel spheres_model(spheres_geom);
		if (material_index > -1)
		{
			spheres_model.setParam("material", (uint32_t)material_index);
		}
		spheres_model.setParam("color", ospray::cpp::CopiedData(spheres_color));
		spheres_model.commit();

		spheres_group.setParam("geometry", ospray::cpp::CopiedData(spheres_model));
		spheres_group.commit();
		is_spheres = true;
	}
	if (boxes_bb.size() > 0)
	{//boxes in the pointcloud
		ospray::cpp::Geometry boxes_geom("box");
		boxes_geom.setParam("box", ospray::cpp::CopiedData(boxes_bb));
		boxes_geom.commit();

		ospray::cpp::GeometricModel boxes_model(boxes_geom);
		if (material_index > -1)
		{
			boxes_model.setParam("material", (uint32_t)material_index);
		}
		boxes_model.setParam("color", ospray::cpp::CopiedData(boxes_color));
		boxes_model.commit();

		boxes_group.setParam("geometry", ospray::cpp::CopiedData(boxes_model));
		boxes_group.commit();
		is_boxes = true;
	}
	if (volume_positions.size() > 0)
	{//volumes are in the pointcloud
		ospray::cpp::Volume volume_vol("particle");
		volume_vol.setParam("particle.position", ospray::cpp::CopiedData(volume_positions));
		volume_vol.setParam("particle.radius", ospray::cpp::CopiedData(volume_radius));
		volume_vol.commit();

		//check is this volume should isosurfaced
		bool is_isosurface = false;
		float isovalue = 1.0f;
		XSI::ICEAttribute isosurface_attr = geometry.GetICEAttributeFromName(isosurface_attribute_name);
		XSI::ICEAttribute isovalue_attr = geometry.GetICEAttributeFromName(isovalue_attribute_name);
		if (isosurface_attr.GetElementCount() > 0 && isovalue_attr.GetElementCount() > 0 &&
			isosurface_attr.GetStructureType() == XSI::siICENodeStructureSingle && isovalue_attr.GetStructureType() == XSI::siICENodeStructureSingle &&
			isosurface_attr.GetDataType() == XSI::siICENodeDataBool && isovalue_attr.GetDataType() == XSI::siICENodeDataFloat &&
			isosurface_attr.GetContextType() == XSI::siICENodeContextSingleton && isovalue_attr.GetContextType() == XSI::siICENodeContextSingleton)
		{
			//use volume as surface
			XSI::CICEAttributeDataArrayBool isosurface_data;
			isosurface_attr.GetDataArray(isosurface_data);

			XSI::CICEAttributeDataArrayFloat isovalue_data;
			isovalue_attr.GetDataArray(isovalue_data);
			
			if (isosurface_data.GetCount() > 0)
			{
				is_isosurface = isosurface_data[0];
			}
			if (isovalue_data.GetCount() > 0)
			{
				isovalue = isovalue_data[0];
			}
		}

		if (is_isosurface)
		{
			ospray::cpp::Geometry iso_geometry("isosurface");
			iso_geometry.setParam("isovalue", isovalue);
			iso_geometry.setParam("volume", volume_vol);
			iso_geometry.commit();

			ospray::cpp::GeometricModel iso_model(iso_geometry);
			if (material_index > -1)
			{
				iso_model.setParam("material", (uint32_t)material_index);
			}
			iso_model.commit();
			volume_group.setParam("geometry", ospray::cpp::CopiedData(iso_model));
			volume_group.commit();
		}
		else
		{
			ospray::cpp::VolumetricModel volume_model(volume_vol);
			//here we should set transfer function
			//try to find ICE-attribute
			XSI::ICEAttribute transfer_attr = geometry.GetICEAttributeFromName(transfer_attribute_name);
			if (transfer_attr.GetElementCount() > 0 && transfer_attr.GetStructureType() == XSI::siICENodeStructureArray && transfer_attr.GetDataType() == XSI::siICENodeDataColor4 && transfer_attr.GetContextType() == XSI::siICENodeContextSingleton)
			{
				//get colors from attribute
				XSI::CICEAttributeDataArray2DColor4f transfer_data;
				transfer_attr.GetDataArray2D(transfer_data);
				XSI::CICEAttributeDataArrayColor4f transfer_colors_data;
				transfer_data.GetSubArray(0, transfer_colors_data);
				if (transfer_colors_data.GetCount() > 0)
				{
					//write here colors to transfer function
					ospray::cpp::TransferFunction transfer_function("piecewiseLinear");
					std::vector<vec3f> colors;
					std::vector<float> opacities;
					for (ULONG i = 0; i < transfer_colors_data.GetCount(); i++)
					{
						XSI::MATH::CColor4f c = transfer_colors_data[i];
						colors.push_back(vec3f{ c.GetR(), c.GetG(), c.GetB() });
						opacities.push_back(c.GetA());
					}
					transfer_function.setParam("color", ospray::cpp::CopiedData(colors));
					transfer_function.setParam("opacity", ospray::cpp::CopiedData(opacities));

					//try to find range attribute
					XSI::ICEAttribute transfer_range_attr = geometry.GetICEAttributeFromName(range_attribute_name);
					if (transfer_range_attr.GetElementCount() > 0 && transfer_range_attr.GetStructureType() == XSI::siICENodeStructureSingle && transfer_range_attr.GetDataType() == XSI::siICENodeDataVector2 && transfer_range_attr.GetContextType() == XSI::siICENodeContextSingleton)
					{
						XSI::CICEAttributeDataArrayVector2f range_data;
						transfer_range_attr.GetDataArray(range_data);
						XSI::MATH::CVector2f range = range_data[0];
						transfer_function.setParam("valueRange", vec2f{ range.GetX(), range.GetY() });
					}
					else
					{
						transfer_function.setParam("valueRange", vec2f{ 0.0f, 1.0f });
					}
					transfer_function.commit();
					volume_model.setParam("transferFunction", transfer_function);
				}
				else
				{
					//use default function
					volume_model.setParam("transferFunction", make_default_transfer_function());
				}
			}
			else
			{
				//use default transfer function
				volume_model.setParam("transferFunction", make_default_transfer_function());
			}
			volume_model.commit();

			volume_group.setParam("volume", ospray::cpp::CopiedData(volume_model));
			volume_group.commit();
		}

		//next we need transfer function
		is_volume = true;
	}

	//clear buffers
	volume_positions.clear();
	volume_positions.shrink_to_fit();
	volume_radius.clear();
	volume_radius.shrink_to_fit();
	spheres_positions.clear();
	spheres_positions.shrink_to_fit();
	spheres_radius.clear();
	spheres_radius.shrink_to_fit();
	spheres_color.clear();
	spheres_color.shrink_to_fit();
	boxes_bb.clear();
	boxes_bb.shrink_to_fit();
	boxes_color.clear();
	boxes_color.shrink_to_fit();
}

OSPCurveType get_curve_type(const int type)
{
	if (type == 0) { return OSPCurveType::OSP_FLAT;  }
	else if (type == 1) { return OSPCurveType::OSP_ROUND; }
	//else if (type == 2) { return OSPCurveType::OSP_RIBBON; }
	else if (type == 2) { return OSPCurveType::OSP_DISJOINT; }
	else {return OSPCurveType::OSP_ROUND; }
}

OSPCurveBasis get_curve_basis(const int basis)
{
	if (basis == 0) { return OSPCurveBasis::OSP_LINEAR; }
	else if (basis == 1) { return OSPCurveBasis::OSP_BEZIER; }
	else if (basis == 2) { return OSPCurveBasis::OSP_BSPLINE; }
	else if (basis == 3) { return OSPCurveBasis::OSP_HERMITE; }
	else if (basis == 4) { return OSPCurveBasis::OSP_CATMULL_ROM; }
	else { return OSPCurveBasis::OSP_LINEAR; }
}

XSI::CString to_string(const vec4f &value)
{
	return "(" + XSI::CString(value.x) + ", " + XSI::CString(value.y) + ", " + XSI::CString(value.z) + ", " + XSI::CString(value.w) + ")";
}

XSI::CString to_string(const std::vector<vec4f> &array)
{
	if (array.size() == 0)
	{
		return "";
	}
	XSI::CString to_return = to_string(array[0]);
	for (LONG i = 1; i < array.size(); i++)
	{
		to_return += ", " + to_string(array[i]);
	}
	return to_return;
}

void sync_strands(const XSI::CTime &eval_time, const XSI::X3DObject &xsi_object, ospray::cpp::Group &strands_group, bool &is_strands, const OSPCurveType curve_type, const OSPCurveBasis curve_basis, const int material_index)
{
	if (curve_basis != OSPCurveBasis::OSP_LINEAR)
	{
		log("[OSPRay Render] Non-linear curves type is not supported, the render may crash.", XSI::siWarningMsg);
		is_strands = false;
	}
	else
	{
		//get strands data from ICE
		XSI::Geometry xsi_geometry = xsi_object.GetActivePrimitive(eval_time.GetTime()).GetGeometry(eval_time.GetTime());
		XSI::ICEAttribute point_position_attr = xsi_geometry.GetICEAttributeFromName("PointPosition");
		XSI::ICEAttribute strand_position_attr = xsi_geometry.GetICEAttributeFromName("StrandPosition");
		XSI::ICEAttribute size_attr = xsi_geometry.GetICEAttributeFromName("Size");
		XSI::ICEAttribute color_attr = xsi_geometry.GetICEAttributeFromName("Color");
		//fill data arrays
		XSI::CICEAttributeDataArrayVector3f point_position_data;
		point_position_attr.GetDataArray(point_position_data);

		XSI::CICEAttributeDataArray2DVector3f strand_position_data;
		strand_position_attr.GetDataArray2D(strand_position_data);

		XSI::CICEAttributeDataArrayFloat size_data;
		size_attr.GetDataArray(size_data);

		XSI::CICEAttributeDataArrayColor4f color_data;
		color_attr.GetDataArray(color_data);

		XSI::CICEAttributeDataArrayVector3f one_strand_data;
		strand_position_data.GetSubArray(0, one_strand_data);

		ULONG curves_count = point_position_data.GetCount();
		ULONG keys_count = one_strand_data.GetCount();  // pase position, and then keys_count additional positions for the strand

		std::vector<vec4f> points;
		std::vector<unsigned int> indices;
		std::vector<vec4f> colors;

		if (curves_count > 0 && keys_count > 0)
		{
			for (ULONG curve_index = 0; curve_index < curves_count; curve_index++)
			{
				XSI::MATH::CVector3f position = point_position_data[curve_index];
				XSI::MATH::CColor4f point_color = color_data[curve_index];
				points.push_back(vec4f{position.GetX(), position.GetY(), position.GetZ(), size_data[curve_index]});
				indices.push_back(points.size() - 1);
				colors.push_back(vec4f{ point_color.GetR(), point_color.GetG(), point_color.GetB(), point_color.GetA()});
				strand_position_data.GetSubArray(curve_index, one_strand_data);
				for (ULONG k_index = 0; k_index < one_strand_data.GetCount(); k_index++)
				{
					XSI::MATH::CVector3f s_position = one_strand_data[k_index];
					points.push_back(vec4f{ s_position.GetX(), s_position.GetY(), s_position.GetZ(), size_data[curve_index] });
					colors.push_back(vec4f{ point_color.GetR(), point_color.GetG(), point_color.GetB(), point_color.GetA() });
					if (k_index < one_strand_data.GetCount() - 1)
					{
						indices.push_back(points.size() - 1);
					}
				}
			}

			ospray::cpp::Geometry curve_geom("curve");
			curve_geom.setParam("type", curve_type);
			curve_geom.setParam("basis", curve_basis);
			curve_geom.setParam("vertex.position_radius", ospray::cpp::CopiedData(points));
			curve_geom.setParam("vertex.color", ospray::cpp::CopiedData(colors));
			curve_geom.setParam("index", ospray::cpp::CopiedData(indices));
			curve_geom.commit();

			ospray::cpp::GeometricModel curve_model(curve_geom);
			if (material_index > -1)
			{
				curve_model.setParam("material", (uint32_t)material_index);
			}
			curve_model.commit();
			strands_group.setParam("geometry", ospray::cpp::CopiedData(curve_model));
			strands_group.commit();
			is_strands = true;
		}
		else
		{
			//empty strands
			is_strands = false;
		}
	}
	
}