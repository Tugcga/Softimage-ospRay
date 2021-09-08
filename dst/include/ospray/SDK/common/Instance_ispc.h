//
// C:/Users/visuser/gitlab-runner/builds/oGM4Jr8r/0/renderkit/ospray/build_release/ospray/common/Instance_ispc.h
// (Header automatically generated by the ispc compiler.)
// DO NOT EDIT THIS FILE.
//

#pragma once
#include <stdint.h>



#ifdef __cplusplus
namespace ispc { /* namespace */
#endif // __cplusplus

///////////////////////////////////////////////////////////////////////////
// Enumerator types with external visibility from ispc code
///////////////////////////////////////////////////////////////////////////

#ifndef __ISPC_ENUM_RTCIntersectContextFlags__
#define __ISPC_ENUM_RTCIntersectContextFlags__
enum RTCIntersectContextFlags {
    RTC_INTERSECT_CONTEXT_FLAG_NONE = 0,
    RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT = 0,
    RTC_INTERSECT_CONTEXT_FLAG_COHERENT = 1 
};
#endif

#ifndef __ISPC_ENUM_RTCError__
#define __ISPC_ENUM_RTCError__
enum RTCError {
    RTC_ERROR_NONE = 0,
    RTC_ERROR_UNKNOWN = 1,
    RTC_ERROR_INVALID_ARGUMENT = 2,
    RTC_ERROR_INVALID_OPERATION = 3,
    RTC_ERROR_OUT_OF_MEMORY = 4,
    RTC_ERROR_UNSUPPORTED_CPU = 5,
    RTC_ERROR_CANCELLED = 6 
};
#endif

#ifndef __ISPC_ENUM_RTCDeviceProperty__
#define __ISPC_ENUM_RTCDeviceProperty__
enum RTCDeviceProperty {
    RTC_DEVICE_PROPERTY_VERSION = 0,
    RTC_DEVICE_PROPERTY_VERSION_MAJOR = 1,
    RTC_DEVICE_PROPERTY_VERSION_MINOR = 2,
    RTC_DEVICE_PROPERTY_VERSION_PATCH = 3,
    RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED = 32,
    RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED = 33,
    RTC_DEVICE_PROPERTY_NATIVE_RAY16_SUPPORTED = 34,
    RTC_DEVICE_PROPERTY_RAY_STREAM_SUPPORTED = 35,
    RTC_DEVICE_PROPERTY_BACKFACE_CULLING_CURVES_ENABLED = 63,
    RTC_DEVICE_PROPERTY_RAY_MASK_SUPPORTED = 64,
    RTC_DEVICE_PROPERTY_BACKFACE_CULLING_ENABLED = 65,
    RTC_DEVICE_PROPERTY_FILTER_FUNCTION_SUPPORTED = 66,
    RTC_DEVICE_PROPERTY_IGNORE_INVALID_RAYS_ENABLED = 67,
    RTC_DEVICE_PROPERTY_COMPACT_POLYS_ENABLED = 68,
    RTC_DEVICE_PROPERTY_TRIANGLE_GEOMETRY_SUPPORTED = 96,
    RTC_DEVICE_PROPERTY_QUAD_GEOMETRY_SUPPORTED = 97,
    RTC_DEVICE_PROPERTY_SUBDIVISION_GEOMETRY_SUPPORTED = 98,
    RTC_DEVICE_PROPERTY_CURVE_GEOMETRY_SUPPORTED = 99,
    RTC_DEVICE_PROPERTY_USER_GEOMETRY_SUPPORTED = 100,
    RTC_DEVICE_PROPERTY_TASKING_SYSTEM = 128,
    RTC_DEVICE_PROPERTY_JOIN_COMMIT_SUPPORTED = 129,
    RTC_DEVICE_PROPERTY_PARALLEL_COMMIT_SUPPORTED = 130 
};
#endif

#ifndef __ISPC_ENUM_RTCBufferType__
#define __ISPC_ENUM_RTCBufferType__
enum RTCBufferType {
    RTC_BUFFER_TYPE_INDEX = 0,
    RTC_BUFFER_TYPE_VERTEX = 1,
    RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE = 2,
    RTC_BUFFER_TYPE_NORMAL = 3,
    RTC_BUFFER_TYPE_TANGENT = 4,
    RTC_BUFFER_TYPE_NORMAL_DERIVATIVE = 5,
    RTC_BUFFER_TYPE_GRID = 8,
    RTC_BUFFER_TYPE_FACE = 16,
    RTC_BUFFER_TYPE_LEVEL = 17,
    RTC_BUFFER_TYPE_EDGE_CREASE_INDEX = 18,
    RTC_BUFFER_TYPE_EDGE_CREASE_WEIGHT = 19,
    RTC_BUFFER_TYPE_VERTEX_CREASE_INDEX = 20,
    RTC_BUFFER_TYPE_VERTEX_CREASE_WEIGHT = 21,
    RTC_BUFFER_TYPE_HOLE = 22,
    RTC_BUFFER_TYPE_FLAGS = 32 
};
#endif

#ifndef __ISPC_ENUM_RTCFormat__
#define __ISPC_ENUM_RTCFormat__
enum RTCFormat {
    RTC_FORMAT_UNDEFINED = 0,
    RTC_FORMAT_UCHAR = 4097,
    RTC_FORMAT_UCHAR2 = 4098,
    RTC_FORMAT_UCHAR3 = 4099,
    RTC_FORMAT_UCHAR4 = 4100,
    RTC_FORMAT_CHAR = 8193,
    RTC_FORMAT_CHAR2 = 8194,
    RTC_FORMAT_CHAR3 = 8195,
    RTC_FORMAT_CHAR4 = 8196,
    RTC_FORMAT_USHORT = 12289,
    RTC_FORMAT_USHORT2 = 12290,
    RTC_FORMAT_USHORT3 = 12291,
    RTC_FORMAT_USHORT4 = 12292,
    RTC_FORMAT_SHORT = 16385,
    RTC_FORMAT_SHORT2 = 16386,
    RTC_FORMAT_SHORT3 = 16387,
    RTC_FORMAT_SHORT4 = 16388,
    RTC_FORMAT_UINT = 20481,
    RTC_FORMAT_UINT2 = 20482,
    RTC_FORMAT_UINT3 = 20483,
    RTC_FORMAT_UINT4 = 20484,
    RTC_FORMAT_INT = 24577,
    RTC_FORMAT_INT2 = 24578,
    RTC_FORMAT_INT3 = 24579,
    RTC_FORMAT_INT4 = 24580,
    RTC_FORMAT_ULLONG = 28673,
    RTC_FORMAT_ULLONG2 = 28674,
    RTC_FORMAT_ULLONG3 = 28675,
    RTC_FORMAT_ULLONG4 = 28676,
    RTC_FORMAT_LLONG = 32769,
    RTC_FORMAT_LLONG2 = 32770,
    RTC_FORMAT_LLONG3 = 32771,
    RTC_FORMAT_LLONG4 = 32772,
    RTC_FORMAT_FLOAT = 36865,
    RTC_FORMAT_FLOAT2 = 36866,
    RTC_FORMAT_FLOAT3 = 36867,
    RTC_FORMAT_FLOAT4 = 36868,
    RTC_FORMAT_FLOAT5 = 36869,
    RTC_FORMAT_FLOAT6 = 36870,
    RTC_FORMAT_FLOAT7 = 36871,
    RTC_FORMAT_FLOAT8 = 36872,
    RTC_FORMAT_FLOAT9 = 36873,
    RTC_FORMAT_FLOAT10 = 36874,
    RTC_FORMAT_FLOAT11 = 36875,
    RTC_FORMAT_FLOAT12 = 36876,
    RTC_FORMAT_FLOAT13 = 36877,
    RTC_FORMAT_FLOAT14 = 36878,
    RTC_FORMAT_FLOAT15 = 36879,
    RTC_FORMAT_FLOAT16 = 36880,
    RTC_FORMAT_FLOAT2X2_ROW_MAJOR = 37154,
    RTC_FORMAT_FLOAT2X3_ROW_MAJOR = 37155,
    RTC_FORMAT_FLOAT2X4_ROW_MAJOR = 37156,
    RTC_FORMAT_FLOAT3X2_ROW_MAJOR = 37170,
    RTC_FORMAT_FLOAT3X3_ROW_MAJOR = 37171,
    RTC_FORMAT_FLOAT3X4_ROW_MAJOR = 37172,
    RTC_FORMAT_FLOAT4X2_ROW_MAJOR = 37186,
    RTC_FORMAT_FLOAT4X3_ROW_MAJOR = 37187,
    RTC_FORMAT_FLOAT4X4_ROW_MAJOR = 37188,
    RTC_FORMAT_FLOAT2X2_COLUMN_MAJOR = 37410,
    RTC_FORMAT_FLOAT2X3_COLUMN_MAJOR = 37411,
    RTC_FORMAT_FLOAT2X4_COLUMN_MAJOR = 37412,
    RTC_FORMAT_FLOAT3X2_COLUMN_MAJOR = 37426,
    RTC_FORMAT_FLOAT3X3_COLUMN_MAJOR = 37427,
    RTC_FORMAT_FLOAT3X4_COLUMN_MAJOR = 37428,
    RTC_FORMAT_FLOAT4X2_COLUMN_MAJOR = 37442,
    RTC_FORMAT_FLOAT4X3_COLUMN_MAJOR = 37443,
    RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR = 37444,
    RTC_FORMAT_GRID = 40961 
};
#endif

#ifndef __ISPC_ENUM_RTCSceneFlags__
#define __ISPC_ENUM_RTCSceneFlags__
enum RTCSceneFlags {
    RTC_SCENE_FLAG_NONE = 0,
    RTC_SCENE_FLAG_DYNAMIC = 1,
    RTC_SCENE_FLAG_COMPACT = 2,
    RTC_SCENE_FLAG_ROBUST = 4,
    RTC_SCENE_FLAG_CONTEXT_FILTER_FUNCTION = 8 
};
#endif

#ifndef __ISPC_ENUM_RTCGeometryType__
#define __ISPC_ENUM_RTCGeometryType__
enum RTCGeometryType {
    RTC_GEOMETRY_TYPE_TRIANGLE = 0,
    RTC_GEOMETRY_TYPE_QUAD = 1,
    RTC_GEOMETRY_TYPE_GRID = 2,
    RTC_GEOMETRY_TYPE_SUBDIVISION = 8,
    RTC_GEOMETRY_TYPE_CONE_LINEAR_CURVE = 15,
    RTC_GEOMETRY_TYPE_FLAT_LINEAR_CURVE = 17,
    RTC_GEOMETRY_TYPE_ROUND_LINEAR_CURVE = 16,
    RTC_GEOMETRY_TYPE_ROUND_BEZIER_CURVE = 24,
    RTC_GEOMETRY_TYPE_FLAT_BEZIER_CURVE = 25,
    RTC_GEOMETRY_TYPE_NORMAL_ORIENTED_BEZIER_CURVE = 26,
    RTC_GEOMETRY_TYPE_ROUND_BSPLINE_CURVE = 32,
    RTC_GEOMETRY_TYPE_FLAT_BSPLINE_CURVE = 33,
    RTC_GEOMETRY_TYPE_NORMAL_ORIENTED_BSPLINE_CURVE = 34,
    RTC_GEOMETRY_TYPE_ROUND_HERMITE_CURVE = 40,
    RTC_GEOMETRY_TYPE_FLAT_HERMITE_CURVE = 41,
    RTC_GEOMETRY_TYPE_NORMAL_ORIENTED_HERMITE_CURVE = 42,
    RTC_GEOMETRY_TYPE_SPHERE_POINT = 50,
    RTC_GEOMETRY_TYPE_DISC_POINT = 51,
    RTC_GEOMETRY_TYPE_ORIENTED_DISC_POINT = 52,
    RTC_GEOMETRY_TYPE_ROUND_CATMULL_ROM_CURVE = 58,
    RTC_GEOMETRY_TYPE_FLAT_CATMULL_ROM_CURVE = 59,
    RTC_GEOMETRY_TYPE_NORMAL_ORIENTED_CATMULL_ROM_CURVE = 60,
    RTC_GEOMETRY_TYPE_USER = 120,
    RTC_GEOMETRY_TYPE_INSTANCE = 121 
};
#endif

#ifndef __ISPC_ENUM_RTCBuildQuality__
#define __ISPC_ENUM_RTCBuildQuality__
enum RTCBuildQuality {
    RTC_BUILD_QUALITY_LOW = 0,
    RTC_BUILD_QUALITY_MEDIUM = 1,
    RTC_BUILD_QUALITY_HIGH = 2,
    RTC_BUILD_QUALITY_REFIT = 3 
};
#endif

#ifndef __ISPC_ENUM_RTCSubdivisionMode__
#define __ISPC_ENUM_RTCSubdivisionMode__
enum RTCSubdivisionMode {
    RTC_SUBDIVISION_MODE_NO_BOUNDARY = 0,
    RTC_SUBDIVISION_MODE_SMOOTH_BOUNDARY = 1,
    RTC_SUBDIVISION_MODE_PIN_CORNERS = 2,
    RTC_SUBDIVISION_MODE_PIN_BOUNDARY = 3,
    RTC_SUBDIVISION_MODE_PIN_ALL = 4 
};
#endif


#ifndef __ISPC_ALIGN__
#if defined(__clang__) || !defined(_MSC_VER)
// Clang, GCC, ICC
#define __ISPC_ALIGN__(s) __attribute__((aligned(s)))
#define __ISPC_ALIGNED_STRUCT__(s) struct __ISPC_ALIGN__(s)
#else
// Visual Studio
#define __ISPC_ALIGN__(s) __declspec(align(s))
#define __ISPC_ALIGNED_STRUCT__(s) __ISPC_ALIGN__(s) struct
#endif
#endif

#ifndef __ISPC_STRUCT_vec3f__
#define __ISPC_STRUCT_vec3f__
struct vec3f {
    float x;
    float y;
    float z;
};
#endif

#ifndef __ISPC_STRUCT_LinearSpace3f__
#define __ISPC_STRUCT_LinearSpace3f__
struct LinearSpace3f {
    struct vec3f vx;
    struct vec3f vy;
    struct vec3f vz;
};
#endif

#ifndef __ISPC_STRUCT_AffineSpace3f__
#define __ISPC_STRUCT_AffineSpace3f__
struct AffineSpace3f {
    struct LinearSpace3f l;
    struct vec3f p;
};
#endif

#ifndef __ISPC_STRUCT_RTCCollision__
#define __ISPC_STRUCT_RTCCollision__
struct RTCCollision {
    uint32_t geomID0;
    uint32_t primID0;
    uint32_t geomID1;
    uint32_t primID1;
};
#endif

#ifndef __ISPC_STRUCT_RTCIntersectFunctionNArguments__
#define __ISPC_STRUCT_RTCIntersectFunctionNArguments__
struct RTCIntersectFunctionNArguments {
    int32_t * valid;
    void * geometryUserPtr;
    uint32_t primID;
    struct RTCIntersectContext * context;
    struct RTCRayHitN * rayhit;
    uint32_t N;
    uint32_t geomID;
};
#endif

#ifndef __ISPC_STRUCT_RTCIntersectContext__
#define __ISPC_STRUCT_RTCIntersectContext__
struct RTCIntersectContext {
    enum RTCIntersectContextFlags flags;
    void * filter;
    uint32_t instID[1];
};
#endif

#ifndef __ISPC_STRUCT_RTCFilterFunctionNArguments__
#define __ISPC_STRUCT_RTCFilterFunctionNArguments__
struct RTCFilterFunctionNArguments {
    int32_t * valid;
    void * geometryUserPtr;
    struct RTCIntersectContext * context;
    struct RTCRayN * ray;
    struct RTCHitN * hit;
    uint32_t N;
};
#endif

#ifndef __ISPC_STRUCT_RTCOccludedFunctionNArguments__
#define __ISPC_STRUCT_RTCOccludedFunctionNArguments__
struct RTCOccludedFunctionNArguments {
    int32_t * valid;
    void * geometryUserPtr;
    uint32_t primID;
    struct RTCIntersectContext * context;
    struct RTCRayN * ray;
    uint32_t N;
    uint32_t geomID;
};
#endif

#ifndef __ISPC_STRUCT_RTCBounds__
#define __ISPC_STRUCT_RTCBounds__
struct RTCBounds {
    float lower_x;
    float lower_y;
    float lower_z;
    float align0;
    float upper_x;
    float upper_y;
    float upper_z;
    float align1;
};
#endif

#ifndef __ISPC_STRUCT_RTCLinearBounds__
#define __ISPC_STRUCT_RTCLinearBounds__
struct RTCLinearBounds {
    struct RTCBounds bounds0;
    struct RTCBounds bounds1;
};
#endif

#ifndef __ISPC_STRUCT_RTCInterpolateArguments__
#define __ISPC_STRUCT_RTCInterpolateArguments__
struct RTCInterpolateArguments {
    struct RTCGeometryTy * geometry;
    uint32_t primID;
    float u;
    float v;
    enum RTCBufferType bufferType;
    uint32_t bufferSlot;
    float * P;
    float * dPdu;
    float * dPdv;
    float * ddPdudu;
    float * ddPdvdv;
    float * ddPdudv;
    uint32_t valueCount;
};
#endif

#ifndef __ISPC_STRUCT_RTCInterpolateNArguments__
#define __ISPC_STRUCT_RTCInterpolateNArguments__
struct RTCInterpolateNArguments {
    struct RTCGeometryTy * geometry;
    const void * valid;
    const uint32_t * primIDs;
    const float * u;
    const float * v;
    uint32_t N;
    enum RTCBufferType bufferType;
    uint32_t bufferSlot;
    float * P;
    float * dPdu;
    float * dPdv;
    float * ddPdudu;
    float * ddPdvdv;
    float * ddPdudv;
    uint32_t valueCount;
};
#endif

#ifndef __ISPC_STRUCT_RTCRay__
#define __ISPC_STRUCT_RTCRay__
struct RTCRay {
    float org_x;
    float org_y;
    float org_z;
    float tnear;
    float dir_x;
    float dir_y;
    float dir_z;
    float time;
    float tfar;
    uint32_t mask;
    uint32_t id;
    uint32_t flags;
};
#endif

#ifndef __ISPC_STRUCT_RTCHit__
#define __ISPC_STRUCT_RTCHit__
struct RTCHit {
    float Ng_x;
    float Ng_y;
    float Ng_z;
    float u;
    float v;
    uint32_t primID;
    uint32_t geomID;
    uint32_t instID[1];
};
#endif

#ifndef __ISPC_STRUCT_RTCRayHit__
#define __ISPC_STRUCT_RTCRayHit__
struct RTCRayHit {
    struct RTCRay ray;
    struct RTCHit hit;
};
#endif

#ifndef __ISPC_STRUCT_RTCRayNp__
#define __ISPC_STRUCT_RTCRayNp__
struct RTCRayNp {
    float * org_x;
    float * org_y;
    float * org_z;
    float * tnear;
    float * dir_x;
    float * dir_y;
    float * dir_z;
    float * time;
    float * tfar;
    uint32_t * mask;
    uint32_t * id;
    uint32_t * flags;
};
#endif

#ifndef __ISPC_STRUCT_RTCHitNp__
#define __ISPC_STRUCT_RTCHitNp__
struct RTCHitNp {
    float * Ng_x;
    float * Ng_y;
    float * Ng_z;
    float * u;
    float * v;
    uint32_t * primID;
    uint32_t * geomID;
    uint32_t * instID[1];
};
#endif

#ifndef __ISPC_STRUCT_RTCRayHitNp__
#define __ISPC_STRUCT_RTCRayHitNp__
struct RTCRayHitNp {
    struct RTCRayNp ray;
    struct RTCHitNp hit;
};
#endif

#ifndef __ISPC_STRUCT_RTCPointQuery__
#define __ISPC_STRUCT_RTCPointQuery__
struct RTCPointQuery {
    float x;
    float y;
    float z;
    float time;
    float radius;
};
#endif

#ifndef __ISPC_STRUCT_RTCPointQueryContext__
#define __ISPC_STRUCT_RTCPointQueryContext__
struct RTCPointQueryContext {
    float world2inst[1][16];
    float inst2world[1][16];
    uint32_t instID[1];
    uint32_t instStackSize;
};
#endif

#ifndef __ISPC_STRUCT_RTCPointQueryFunctionArguments__
#define __ISPC_STRUCT_RTCPointQueryFunctionArguments__
struct RTCPointQueryFunctionArguments {
    struct RTCPointQuery * query;
    void * userPtr;
    uint32_t primID;
    uint32_t geomID;
    struct RTCPointQueryContext * context;
    float similarityScale;
};
#endif

#ifndef __ISPC_STRUCT_RTCBoundsFunctionArguments__
#define __ISPC_STRUCT_RTCBoundsFunctionArguments__
struct RTCBoundsFunctionArguments {
    void * geometryUserPtr;
    uint32_t primID;
    uint32_t timeStep;
    struct RTCBounds * bounds_o;
};
#endif

#ifndef __ISPC_STRUCT_RTCDisplacementFunctionNArguments__
#define __ISPC_STRUCT_RTCDisplacementFunctionNArguments__
struct RTCDisplacementFunctionNArguments {
    void * geometryUserPtr;
    struct RTCGeometryTy * geometry;
    uint32_t primID;
    uint32_t timeStep;
    const float * u;
    const float * v;
    const float * Ng_x;
    const float * Ng_y;
    const float * Ng_z;
    float * P_x;
    float * P_y;
    float * P_z;
    uint32_t N;
};
#endif

#ifndef __ISPC_STRUCT_RTCQuaternionDecomposition__
#define __ISPC_STRUCT_RTCQuaternionDecomposition__
struct RTCQuaternionDecomposition {
    float scale_x;
    float scale_y;
    float scale_z;
    float skew_xy;
    float skew_xz;
    float skew_yz;
    float shift_x;
    float shift_y;
    float shift_z;
    float quaternion_r;
    float quaternion_i;
    float quaternion_j;
    float quaternion_k;
    float translation_x;
    float translation_y;
    float translation_z;
};
#endif

#ifndef __ISPC_STRUCT_ManagedObject__
#define __ISPC_STRUCT_ManagedObject__
struct ManagedObject {
};
#endif

#ifndef __ISPC_STRUCT_v4_varying___vkl_vec3f__
#define __ISPC_STRUCT_v4_varying___vkl_vec3f__
__ISPC_ALIGNED_STRUCT__(16) v4_varying___vkl_vec3f {
    __ISPC_ALIGN__(16) float x[4];
    __ISPC_ALIGN__(16) float y[4];
    __ISPC_ALIGN__(16) float z[4];
};
#endif

#ifndef __ISPC_STRUCT___vkl_vec3f__
#define __ISPC_STRUCT___vkl_vec3f__
struct __vkl_vec3f {
    float x;
    float y;
    float z;
};
#endif

#ifndef __ISPC_STRUCT___vkl_box3f__
#define __ISPC_STRUCT___vkl_box3f__
struct __vkl_box3f {
    struct __vkl_vec3f lower;
    struct __vkl_vec3f upper;
};
#endif

#ifndef __ISPC_STRUCT___vkl_box1f__
#define __ISPC_STRUCT___vkl_box1f__
struct __vkl_box1f {
    float lower;
    float upper;
};
#endif

#ifndef __ISPC_STRUCT_v4_varying___vkl_box1f__
#define __ISPC_STRUCT_v4_varying___vkl_box1f__
__ISPC_ALIGNED_STRUCT__(16) v4_varying___vkl_box1f {
    __ISPC_ALIGN__(16) float lower[4];
    __ISPC_ALIGN__(16) float upper[4];
};
#endif

#ifndef __ISPC_STRUCT_v4_varying_VKLHit__
#define __ISPC_STRUCT_v4_varying_VKLHit__
__ISPC_ALIGNED_STRUCT__(16) v4_varying_VKLHit {
    __ISPC_ALIGN__(16) float t[4];
    __ISPC_ALIGN__(16) float sample[4];
    __ISPC_ALIGN__(16) float epsilon[4];
};
#endif

#ifndef __ISPC_STRUCT_v4_varying_VKLInterval__
#define __ISPC_STRUCT_v4_varying_VKLInterval__
__ISPC_ALIGNED_STRUCT__(16) v4_varying_VKLInterval {
    struct v4_varying___vkl_box1f tRange;
    struct v4_varying___vkl_box1f valueRange;
    __ISPC_ALIGN__(16) float nominalDeltaT[4];
};
#endif


///////////////////////////////////////////////////////////////////////////
// Functions exported from ispc code
///////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
extern "C" {
#endif // __cplusplus
    extern void *  Instance_create(void * cppE);
    extern void Instance_set(void * _inst, void * _group, const struct AffineSpace3f &xfm, const bool motionBlur);
    extern void Instance_set_embreeGeom(void * _inst, struct RTCGeometryTy * geom);
    extern void delete_uniform(void * uptr);
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
} /* end extern C */
#endif // __cplusplus


#ifndef __ISPC_ALIGN__
#if defined(__clang__) || !defined(_MSC_VER)
// Clang, GCC, ICC
#define __ISPC_ALIGN__(s) __attribute__((aligned(s)))
#define __ISPC_ALIGNED_STRUCT__(s) struct __ISPC_ALIGN__(s)
#else
// Visual Studio
#define __ISPC_ALIGN__(s) __declspec(align(s))
#define __ISPC_ALIGNED_STRUCT__(s) __ISPC_ALIGN__(s) struct
#endif
#endif

#ifndef __ISPC_STRUCT_v8_varying___vkl_vec3f__
#define __ISPC_STRUCT_v8_varying___vkl_vec3f__
__ISPC_ALIGNED_STRUCT__(32) v8_varying___vkl_vec3f {
    __ISPC_ALIGN__(32) float x[8];
    __ISPC_ALIGN__(32) float y[8];
    __ISPC_ALIGN__(32) float z[8];
};
#endif

#ifndef __ISPC_STRUCT_v8_varying___vkl_box1f__
#define __ISPC_STRUCT_v8_varying___vkl_box1f__
__ISPC_ALIGNED_STRUCT__(32) v8_varying___vkl_box1f {
    __ISPC_ALIGN__(32) float lower[8];
    __ISPC_ALIGN__(32) float upper[8];
};
#endif

#ifndef __ISPC_STRUCT_v8_varying_VKLHit__
#define __ISPC_STRUCT_v8_varying_VKLHit__
__ISPC_ALIGNED_STRUCT__(32) v8_varying_VKLHit {
    __ISPC_ALIGN__(32) float t[8];
    __ISPC_ALIGN__(32) float sample[8];
    __ISPC_ALIGN__(32) float epsilon[8];
};
#endif

#ifndef __ISPC_STRUCT_v8_varying_VKLInterval__
#define __ISPC_STRUCT_v8_varying_VKLInterval__
__ISPC_ALIGNED_STRUCT__(32) v8_varying_VKLInterval {
    struct v8_varying___vkl_box1f tRange;
    struct v8_varying___vkl_box1f valueRange;
    __ISPC_ALIGN__(32) float nominalDeltaT[8];
};
#endif


#ifndef __ISPC_ALIGN__
#if defined(__clang__) || !defined(_MSC_VER)
// Clang, GCC, ICC
#define __ISPC_ALIGN__(s) __attribute__((aligned(s)))
#define __ISPC_ALIGNED_STRUCT__(s) struct __ISPC_ALIGN__(s)
#else
// Visual Studio
#define __ISPC_ALIGN__(s) __declspec(align(s))
#define __ISPC_ALIGNED_STRUCT__(s) __ISPC_ALIGN__(s) struct
#endif
#endif

#ifndef __ISPC_STRUCT_v16_varying___vkl_vec3f__
#define __ISPC_STRUCT_v16_varying___vkl_vec3f__
__ISPC_ALIGNED_STRUCT__(64) v16_varying___vkl_vec3f {
    __ISPC_ALIGN__(64) float x[16];
    __ISPC_ALIGN__(64) float y[16];
    __ISPC_ALIGN__(64) float z[16];
};
#endif

#ifndef __ISPC_STRUCT_v16_varying___vkl_box1f__
#define __ISPC_STRUCT_v16_varying___vkl_box1f__
__ISPC_ALIGNED_STRUCT__(64) v16_varying___vkl_box1f {
    __ISPC_ALIGN__(64) float lower[16];
    __ISPC_ALIGN__(64) float upper[16];
};
#endif

#ifndef __ISPC_STRUCT_v16_varying_VKLHit__
#define __ISPC_STRUCT_v16_varying_VKLHit__
__ISPC_ALIGNED_STRUCT__(64) v16_varying_VKLHit {
    __ISPC_ALIGN__(64) float t[16];
    __ISPC_ALIGN__(64) float sample[16];
    __ISPC_ALIGN__(64) float epsilon[16];
};
#endif

#ifndef __ISPC_STRUCT_v16_varying_VKLInterval__
#define __ISPC_STRUCT_v16_varying_VKLInterval__
__ISPC_ALIGNED_STRUCT__(64) v16_varying_VKLInterval {
    struct v16_varying___vkl_box1f tRange;
    struct v16_varying___vkl_box1f valueRange;
    __ISPC_ALIGN__(64) float nominalDeltaT[16];
};
#endif



#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus
