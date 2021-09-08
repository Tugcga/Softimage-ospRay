#include <xsi_x3dobject.h>
#include <xsi_time.h>
#include <xsi_primitive.h>
#include <xsi_hairprimitive.h>
#include <xsi_floatarray.h>

#include "../../utilities/logs.h"
#include "../includes_osp.h"

ospray::cpp::Instance sync_hair(const XSI::CTime &eval_time, const XSI::X3DObject &xsi_object, const OSPCurveType curve_type, const OSPCurveBasis curve_basis, const int material_index)
{
	std::vector<vec4f> points;
	std::vector<unsigned int> indices;
	std::vector<vec4f> colors;

	XSI::HairPrimitive hair_primitive(xsi_object.GetActivePrimitive(eval_time));
	LONG hairs_count = hair_primitive.GetParameterValue("TotalHairs");
	LONG strand_multiplicity = hair_primitive.GetParameterValue("StrandMult");
	if (strand_multiplicity <= 1)
	{
		strand_multiplicity = 1;
	}
	hairs_count = hairs_count * strand_multiplicity;
	XSI::CRenderHairAccessor rha = hair_primitive.GetRenderHairAccessor(hairs_count);
	LONG num_colors = rha.GetVertexColorCount();
	
	rha.Reset();
	//Set hair data
	while (rha.Next())
	{
		XSI::CLongArray vertices_count_array;  // vertex count in each hair strand
		rha.GetVerticesCount(vertices_count_array);
		LONG strands_count = vertices_count_array.GetCount();
		XSI::CFloatArray position_values;
		rha.GetVertexPositions(position_values); // get actual vertex positions for all hairs
		XSI::CFloatArray radisu_values;
		rha.GetVertexRadiusValues(radisu_values);  // get point radius
		LONG pos_k = 0;
		LONG radius_k = 0;

		XSI::CFloatArray color_values;
		LONG colors_count = 0;
		if (num_colors > 0)
		{
			XSI::CFloatArray color_values;
			rha.GetVertexColorValues(0, color_values);
			colors_count = color_values.GetCount();
		}
		//positions
		for (LONG i = 0; i < strands_count; i++)
		{
			LONG n_count = vertices_count_array[i];
			for (LONG j = 0; j < n_count; j++)
			{
				points.push_back(vec4f{position_values[pos_k], position_values[pos_k + 1], position_values[pos_k + 2], radisu_values[radius_k] });
				//set color of the point in the strand
				if (colors_count > 0)
				{//colors are exists
					if (4*i < colors_count)
					{
						colors.push_back(vec4f{ color_values[4*i], color_values[4*i + 1], color_values[4*i + 2], color_values[4*i + 3]});
					}
					else
					{
						colors.push_back(vec4f{ 0.0f, 0.0f, 0.0f, 0.0f });
					}
				}
				else
				{
					//add white color
					colors.push_back(vec4f{1.0f, 1.0f, 1.0f, 1.0f});
				}
				//add index
				if (j < n_count - 1)
				{
					indices.push_back(points.size() - 1);
				}

				pos_k = pos_k + 3;
				radius_k = radius_k + 1;
			}
		}
	}

	//set geometry
	ospray::cpp::Geometry curve_geom("curve");
	curve_geom.setParam("type", curve_type);
	if (curve_basis != OSP_LINEAR)
	{
		log("[SOP Ray Render] Non-linear curves are not supported.", XSI::siWarningMsg);
	}
	curve_geom.setParam("basis", OSP_LINEAR);
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

	ospray::cpp::Group hairs_group;
	hairs_group.setParam("geometry", ospray::cpp::CopiedData(curve_model));
	hairs_group.commit();

	ospray::cpp::Instance hair_inst(hairs_group);
	return hair_inst;
}