// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../math/vec.h"

namespace rkcommon {

  using namespace math;

  template <int NDIMS>
  struct multidim_index_iterator;

  template <int NDIMS>
  struct multidim_index_sequence
  {
    static_assert(NDIMS == 2 || NDIMS == 3,
                  "rkcommon::multidim_index_sequence is currently limited to"
                  " only 2 or 3 dimensions. (NDIMS == 2 || NDIMS == 3)");

    multidim_index_sequence(const vec_t<size_t, NDIMS> &_dims);

    size_t flatten(const vec_t<size_t, NDIMS> &coords) const;

    vec_t<size_t, NDIMS> reshape(size_t i) const;

    vec_t<size_t, NDIMS> dimensions() const;

    size_t total_indices() const;

    multidim_index_iterator<NDIMS> begin() const;
    multidim_index_iterator<NDIMS> end() const;

   private:
    vec_t<size_t, NDIMS> dims{0};
  };

  using index_sequence_2D = multidim_index_sequence<2>;
  using index_sequence_3D = multidim_index_sequence<3>;

  template <int NDIMS>
  struct multidim_index_iterator
  {
    multidim_index_iterator(const vec_t<size_t, NDIMS> &_dims) : dims(_dims) {}
    multidim_index_iterator(const vec_t<size_t, NDIMS> &_dims, size_t start)
        : multidim_index_iterator(_dims)
    {
      current_index = start;
    }

    // Traditional iterator interface methods //

    vec_t<size_t, NDIMS> operator*() const;

    multidim_index_iterator operator++();
    multidim_index_iterator &operator++(int);

    multidim_index_iterator operator--();
    multidim_index_iterator &operator--(int);

    multidim_index_iterator &operator+(const multidim_index_iterator &other);
    multidim_index_iterator &operator-(const multidim_index_iterator &other);

    multidim_index_iterator &operator+(size_t other);
    multidim_index_iterator &operator-(size_t other);

    bool operator==(const multidim_index_iterator &other) const;
    bool operator!=(const multidim_index_iterator &other) const;

    // Extra helper methods //

    void jump_to(size_t index);
    size_t current() const;

   private:
    multidim_index_sequence<NDIMS> dims;
    size_t current_index{0};
  };

  // Inlined multidim_index_sequence definitions //////////////////////////////

  template <int NDIMS>
  inline multidim_index_sequence<NDIMS>::multidim_index_sequence(
      const vec_t<size_t, NDIMS> &_dims)
      : dims(_dims)
  {
  }

  template <>
  inline size_t index_sequence_2D::flatten(const vec_t<size_t, 2> &coords) const
  {
    return coords.x + dims.x * coords.y;
  }

  template <>
  inline size_t index_sequence_3D::flatten(const vec_t<size_t, 3> &coords) const
  {
    return coords.x + dims.x * (coords.y + dims.y * coords.z);
  }

  template <>
  inline vec_t<size_t, 2> index_sequence_2D::reshape(size_t i) const
  {
    size_t y = i / dims.x;
    size_t x = i % dims.x;
    return vec_t<size_t, 2>(x, y);
  }

  template <>
  inline vec_t<size_t, 3> index_sequence_3D::reshape(size_t i) const
  {
    size_t z = i / (dims.x * dims.y);
    i -= (z * dims.x * dims.y);
    size_t y = i / dims.x;
    size_t x = i % dims.x;
    return vec_t<size_t, 3>(x, y, z);
  }

  template <int NDIMS>
  inline vec_t<size_t, NDIMS> multidim_index_sequence<NDIMS>::dimensions() const
  {
    return dims;
  }

  template <int NDIMS>
  inline size_t multidim_index_sequence<NDIMS>::total_indices() const
  {
    return dims.long_product();
  }

  template <int NDIMS>
  multidim_index_iterator<NDIMS> multidim_index_sequence<NDIMS>::begin() const
  {
    return multidim_index_iterator<NDIMS>(dims, 0);
  }

  template <int NDIMS>
  multidim_index_iterator<NDIMS> multidim_index_sequence<NDIMS>::end() const
  {
    return multidim_index_iterator<NDIMS>(dims, total_indices());
  }

  // Inlined multidim_index_iterator definitions //////////////////////////////

  template <int NDIMS>
  inline vec_t<size_t, NDIMS> multidim_index_iterator<NDIMS>::operator*() const
  {
    return dims.reshape(current_index);
  }

  template <int NDIMS>
  inline multidim_index_iterator<NDIMS>
  multidim_index_iterator<NDIMS>::operator++()
  {
    return multidim_index_iterator<NDIMS>(dims.dimensions(), ++current_index);
  }

  template <int NDIMS>
  inline multidim_index_iterator<NDIMS>
      &multidim_index_iterator<NDIMS>::operator++(int)
  {
    current_index++;
    return *this;
  }

  template <int NDIMS>
  inline multidim_index_iterator<NDIMS>
  multidim_index_iterator<NDIMS>::operator--()
  {
    return multidim_index_iterator<NDIMS>(dims.dimensions(), --current_index);
  }

  template <int NDIMS>
  inline multidim_index_iterator<NDIMS>
      &multidim_index_iterator<NDIMS>::operator--(int)
  {
    current_index--;
    return *this;
  }

  template <int NDIMS>
  inline multidim_index_iterator<NDIMS>
      &multidim_index_iterator<NDIMS>::operator+(
          const multidim_index_iterator &other)
  {
    current_index += other.current_index;
    return *this;
  }

  template <int NDIMS>
  inline multidim_index_iterator<NDIMS>
      &multidim_index_iterator<NDIMS>::operator-(
          const multidim_index_iterator &other)
  {
    current_index -= other.current_index;
    return *this;
  }

  template <int NDIMS>
  inline multidim_index_iterator<NDIMS>
      &multidim_index_iterator<NDIMS>::operator+(size_t offset)
  {
    current_index += offset;
    return *this;
  }

  template <int NDIMS>
  inline multidim_index_iterator<NDIMS>
      &multidim_index_iterator<NDIMS>::operator-(size_t offset)
  {
    current_index -= offset;
    return *this;
  }

  template <int NDIMS>
  inline bool multidim_index_iterator<NDIMS>::operator==(
      const multidim_index_iterator &other) const
  {
    return dims.dimensions() == other.dims.dimensions() &&
           current_index == other.current_index;
  }

  template <int NDIMS>
  inline bool multidim_index_iterator<NDIMS>::operator!=(
      const multidim_index_iterator &other) const
  {
    return !(*this == other);
  }

  template <int NDIMS>
  inline void multidim_index_iterator<NDIMS>::jump_to(size_t index)
  {
    current_index = index;
  }

  template <int NDIMS>
  inline size_t multidim_index_iterator<NDIMS>::current() const
  {
    return current_index;
  }

}  // namespace rkcommon
