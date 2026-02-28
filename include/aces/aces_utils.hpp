#ifndef ACES_UTILS_HPP
#define ACES_UTILS_HPP

/**
 * @file aces_utils.hpp
 * @brief Utility functions for ESMF-Kokkos interoperability.
 */

#include "ESMC.h"
#include "aces/aces_state.hpp"

namespace aces {

/**
 * @brief Wraps an ESMC_Field into an UnmanagedHostView3D.
 *
 * This function extracts the raw data pointer from an ESMF Field and
 * wraps it in a Kokkos View with LayoutLeft to match ESMF's column-major
 * memory layout. The memory remains managed by ESMF.
 *
 * @param field The ESMC_Field to wrap.
 * @param dim1 Size of the first dimension (usually lon/nx).
 * @param dim2 Size of the second dimension (usually lat/ny).
 * @param dim3 Size of the third dimension (usually lev/nz).
 * @return UnmanagedHostView3D Wrapped Kokkos View pointing to ESMF data.
 */
inline UnmanagedHostView3D WrapESMCField(ESMC_Field field, int dim1, int dim2,
                                         int dim3) {
  int rc;
  double* dataPtr = static_cast<double*>(ESMC_FieldGetPtr(field, 0, &rc));
  return UnmanagedHostView3D(dataPtr, dim1, dim2, dim3);
}

}  // namespace aces

#endif  // ACES_UTILS_HPP
