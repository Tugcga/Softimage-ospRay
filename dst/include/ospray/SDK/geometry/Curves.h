// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Geometry.h"

namespace ospray {

struct OSPRAY_SDK_INTERFACE Curves : public Geometry
{
  Curves();
  virtual ~Curves() override = default;
  virtual std::string toString() const override;

  virtual void commit() override;

  virtual size_t numPrimitives() const override;

 protected:
  Ref<const DataT<vec4f>> vertexData;
  Ref<const DataT<uint32_t>> indexData;
  Ref<const DataT<vec3f>> normalData;
  Ref<const DataT<vec4f>> tangentData;
  Ref<const DataT<vec4f>> colorData;
  Ref<const DataT<vec2f>> texcoordData;

  RTCGeometryType embreeCurveType;

  OSPCurveType curveType{OSP_UNKNOWN_CURVE_TYPE};
  OSPCurveBasis curveBasis{OSP_UNKNOWN_CURVE_BASIS};

 private:
  void createEmbreeGeometry();
};

} // namespace ospray
