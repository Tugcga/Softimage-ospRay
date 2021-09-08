#include <xsi_x3dobject.h>
#include <xsi_time.h>
#include <xsi_primitive.h>
#include <xsi_polygonmesh.h>
#include <xsi_geometryaccessor.h>

#include "../../utilities/logs.h"
#include "../includes_osp.h"

ospray::cpp::Instance sync_polygonmesh(const XSI::CTime &eval_time, XSI::X3DObject &xsi_object, const int material_index, const bool ignore_vertex_colors)
{
	int subdivs = 0;
	float ga_angle = 60.0;
	bool ga_use_angle = true;
	//we should get these parameters from geometry approximation property
	XSI::Property ga_property;
	xsi_object.GetPropertyFromName("geomapprox", ga_property);
	if (ga_property.IsValid())
	{
		subdivs = ga_property.GetParameterValue("gapproxmordrsl", eval_time);
		ga_angle = ga_property.GetParameterValue("gapproxmoan", eval_time);
		ga_use_angle = ga_property.GetParameterValue("gapproxmoad", eval_time);
	}

	XSI::Primitive xsi_primitive = xsi_object.GetActivePrimitive(eval_time);
	XSI::PolygonMesh xsi_polygonmesh = xsi_primitive.GetGeometry(eval_time, XSI::siConstructionModeSecondaryShape);
	XSI::CGeometryAccessor xsi_acc = xsi_polygonmesh.GetGeometryAccessor(XSI::siConstructionModeSecondaryShape, XSI::siCatmullClark, subdivs, false, ga_use_angle, ga_angle);

	LONG triangles_count = xsi_acc.GetTriangleCount();
	XSI::CLongArray triangle_vertexes;  // i1, i2, i3 for the first triangls, j1, j2, j3 for the second, ...
	xsi_acc.GetTriangleVertexIndices(triangle_vertexes);
	XSI::CDoubleArray vertex_positions;  // x1, y1, z1, x2, y2, z2, ...
	xsi_acc.GetVertexPositions(vertex_positions);
	XSI::CFloatArray node_normals;  // normal coordinates for each node
	xsi_acc.GetNodeNormals(node_normals);
	XSI::CLongArray triangle_nodes;  // i1, i2, i3, for the first triangle, j1, j2, j3, for the second and so on
	xsi_acc.GetTriangleNodeIndices(triangle_nodes);

	//now we are ready to fill mesh data
	//we will create separate vertex for each triangle, because we need different normals in one vertex
	std::vector<vec3f> v_position;
	std::vector<vec3f> v_normal;
	std::vector<vec3ui> indices;
	for (LONG i = 0; i < triangles_count; i++)
	{//fot the i-th triangle
		long v0 = triangle_vertexes[3 * i];
		long v1 = triangle_vertexes[3 * i + 1];
		long v2 = triangle_vertexes[3 * i + 2];
		v_position.emplace_back(vertex_positions[v0 * 3], vertex_positions[v0 * 3 + 1], vertex_positions[v0 * 3 + 2]);
		v_position.emplace_back(vertex_positions[v1 * 3], vertex_positions[v1 * 3 + 1], vertex_positions[v1 * 3 + 2]);
		v_position.emplace_back(vertex_positions[v2 * 3], vertex_positions[v2 * 3 + 1], vertex_positions[v2 * 3 + 2]);

		long n0 = triangle_nodes[3 * i];
		long n1 = triangle_nodes[3 * i + 1];
		long n2 = triangle_nodes[3 * i + 2];
		v_normal.emplace_back(node_normals[3 * n0], node_normals[3 * n0 + 1], node_normals[3 * n0 + 2]);
		v_normal.emplace_back(node_normals[3 * n1], node_normals[3 * n1 + 1], node_normals[3 * n1 + 2]);
		v_normal.emplace_back(node_normals[3 * n2], node_normals[3 * n2 + 1], node_normals[3 * n2 + 2]);

		indices.emplace_back(3*i, 3*i + 1, 3*i + 2);
	}

	ospray::cpp::Geometry osp_mesh("mesh");
	osp_mesh.setParam("vertex.position", ospray::cpp::CopiedData(v_position));
	osp_mesh.setParam("vertex.normal", ospray::cpp::CopiedData(v_normal));
	osp_mesh.setParam("index", ospray::cpp::CopiedData(indices));

	//next uvs
	XSI::CRefArray uv_refs = xsi_acc.GetUVs();
	if (uv_refs.GetCount() > 0)
	{//there are uvs on the mesh, chose the first one
		XSI::ClusterProperty uv_prop = uv_refs[0];
		XSI::CFloatArray uv_values;
		uv_prop.GetValues(uv_values);

		std::vector<vec2f> v_uv;
		for (LONG i = 0; i < triangles_count; i++)
		{
			long n0 = triangle_nodes[3 * i];
			long n1 = triangle_nodes[3 * i + 1];
			long n2 = triangle_nodes[3 * i + 2];

			float u0 = uv_values[3 * n0 + 0];
			float v0 = uv_values[3 * n0 + 1];

			float u1 = uv_values[3 * n1 + 0];
			float v1 = uv_values[3 * n1 + 1];

			float u2 = uv_values[3 * n2 + 0];
			float v2 = uv_values[3 * n2 + 1];

			v_uv.emplace_back(u0, v0);
			v_uv.emplace_back(u1, v1);
			v_uv.emplace_back(u2, v2);
		}
		osp_mesh.setParam("vertex.texcoord", ospray::cpp::CopiedData(v_uv));

		v_uv.clear();
		v_uv.shrink_to_fit();
	}

	//next vertex color
	XSI::CRefArray vertex_colors_refs = xsi_acc.GetVertexColors();
	if (vertex_colors_refs.GetCount() > 0 && !ignore_vertex_colors)
	{//use the first one
		std::vector<vec4f> v_colors;

		XSI::ClusterProperty vertex_color_prop(vertex_colors_refs[0]);
		XSI::CFloatArray vc_values;
		vertex_color_prop.GetValues(vc_values);
		for (LONG i = 0; i < triangles_count; i++)
		{
			long n0 = triangle_nodes[3 * i];
			long n1 = triangle_nodes[3 * i + 1];
			long n2 = triangle_nodes[3 * i + 2];

			v_colors.emplace_back(vc_values[4 * n0], vc_values[4 * n0 + 1], vc_values[4 * n0 + 2], vc_values[4 * n0 + 3]);
			v_colors.emplace_back(vc_values[4 * n1], vc_values[4 * n1 + 1], vc_values[4 * n1 + 2], vc_values[4 * n1 + 3]);
			v_colors.emplace_back(vc_values[4 * n2], vc_values[4 * n2 + 1], vc_values[4 * n2 + 2], vc_values[4 * n2 + 3]);
		}
		osp_mesh.setParam("vertex.color", ospray::cpp::CopiedData(v_colors));

		v_colors.clear();
		v_colors.shrink_to_fit();
	}

	osp_mesh.commit();

	v_position.clear();
	v_normal.clear();
	indices.clear();
	v_position.shrink_to_fit();
	v_normal.shrink_to_fit();
	indices.shrink_to_fit();

	ospray::cpp::GeometricModel geometry(osp_mesh);
	if (material_index > -1)
	{
		geometry.setParam("material", (uint32_t)material_index);
	}
	geometry.commit();

	ospray::cpp::Group mesh_group;
	mesh_group.setParam("geometry", ospray::cpp::CopiedData(geometry));
	mesh_group.commit();

	ospray::cpp::Instance mesh_instance(mesh_group);
	mesh_instance.commit();

	return mesh_instance;
}