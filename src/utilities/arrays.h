#pragma once
#include "../render_osp/includes_osp.h"

#include <xsi_x3dobject.h>

#include <vector>

//return true if value in the array
bool is_contains(const std::vector<OSPFrameBufferChannel> &array, const OSPFrameBufferChannel value);
bool is_contains(const std::vector<ULONG> &array, const ULONG value);
bool is_contains(const XSI::CRefArray &array, const XSI::CRef &object);

//return -1, if value is not in array, otherwise return hte index
int get_index_in_array(const std::vector<ULONG> &array, const ULONG value);