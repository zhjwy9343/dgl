/*!
 *  Copyright (c) 2020 by Contributors
 * \file array/cpu/spmm.h
 * \brief Segment reduce kernel function header.
 */
#ifndef DGL_ARRAY_CPU_SEGMENT_REDUCE_H_
#define DGL_ARRAY_CPU_SEGMENT_REDUCE_H_

#include <dgl/array.h>

namespace dgl {
namespace aten {
namespace cpu {

/*!
 * \brief CPU kernel of segment sum.
 * \param feat The input tensor.
 * \param offsets The offset tensor storing the ranges of segments.
 * \param out The output tensor.
 */
template <typename IdType, typename DType>
void SegmentSum(NDArray feat, NDArray offsets, NDArray out) {
  int n = out->shape[0];
  int dim = 1;
  for (int i = 1; i < out->ndim; ++i)
    dim *= out->shape[i];
  const DType* feat_data = feat.Ptr<DType>();
  const IdType* offsets_data = offsets.Ptr<IdType>();
  DType *out_data = out.Ptr<DType>();
#pragma omp parallel for
  for (int i = 0; i < n; ++i) {
    for (IdType j = offsets_data[i]; j < offsets_data[i + 1]; ++j) {
      for (int k = 0; k < dim; ++k) {
        out_data[i * dim + k] += feat_data[j * dim + k];
      }
    }
  }
}

/*!
 * \brief CPU kernel of segment min/max.
 * \param feat The input tensor.
 * \param offsets The offset tensor storing the ranges of segments.
 * \param out The output tensor.
 * \param arg An auxiliary tensor storing the argmin/max information
 *        used in backward phase.
 */
template <typename IdType, typename DType, typename Cmp>
void SegmentCmp(NDArray feat, NDArray offsets,
                NDArray out, NDArray arg) {
  int n = out->shape[0];
  int dim = 1;
  for (int i = 1; i < out->ndim; ++i)
    dim *= out->shape[i];
  const DType* feat_data = feat.Ptr<DType>();
  const IdType* offsets_data = offsets.Ptr<IdType>();
  DType *out_data = out.Ptr<DType>();
  IdType *arg_data = arg.Ptr<IdType>();
  std::fill(out_data, out_data + out.NumElements(), Cmp::zero);
  std::fill(arg_data, arg_data + arg.NumElements(), -1);
#pragma omp parallel for
  for (int i = 0; i < n; ++i) {
    for (IdType j = offsets_data[i]; j < offsets_data[i + 1]; ++j) {
      for (int k = 0; k < dim; ++k) {
        const DType val = feat_data[j * dim + k];
        if (Cmp::Call(out_data[i * dim + k], val)) {
          out_data[i * dim + k] = val;
          arg_data[i * dim + k] = j;
        }
      }
    }
  }
}

/*!
 * \brief CPU kernel of Scatter Add (on first dimension) operator.
 * \note math equation: out[idx[i], *] += feat[i, *]
 * \param feat The input tensor.
 * \param idx The indices tensor.
 * \param out The output tensor.
 */
template <typename IdType, typename DType>
void ScatterAdd(NDArray feat, NDArray idx, NDArray out) {
  int n = feat->shape[0];
  int dim = 1;
  for (int i = 1; i < out->ndim; ++i)
    dim *= out->shape[i];
  const DType* feat_data = feat.Ptr<DType>();
  const IdType* idx_data = idx.Ptr<IdType>();
  DType* out_data = out.Ptr<DType>();
#pragma omp parallel for
  for (int i = 0; i < n; ++i) {
    const int write_row = idx_data[i];
    for (int k = 0; k < dim; ++k) {
#pragma omp atomic
      out_data[write_row * dim + k] += feat_data[i * dim + k];
    }
  }
}

/*!
 * \brief CPU kernel of backward phase of segment min/max.
 * \note math equation: out[arg[i, k], k] = feat[i, k]
 * \param feat The input tensor.
 * \param arg The argmin/argmax tensor.
 * \param out The output tensor.
 */
template <typename IdType, typename DType>
void BackwardSegmentCmp(NDArray feat, NDArray arg, NDArray out) {
  int n = feat->shape[0];
  int dim = 1;
  for (int i = 1; i < out->ndim; ++i)
    dim *= out->shape[i];
  const DType* feat_data = feat.Ptr<DType>();
  const IdType* arg_data = arg.Ptr<IdType>();
  DType* out_data = out.Ptr<DType>();
#pragma omp parallel for
  for (int i = 0; i < n; ++i) {
    for (int k = 0; k < dim; ++k) {
      int write_row = arg_data[i * dim + k];
      if (write_row >= 0)
        out_data[write_row * dim + k] = feat_data[i * dim + k];
    }
  }
}

}  // namespace cpu
}  // namespace aten
}  // namespace dgl

#endif  // DGL_ARRAY_CPU_SEGMENT_REDUCE_H_
