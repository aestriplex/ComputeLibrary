/*
 * Copyright (c) 2023-2025 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef ACL_TESTS_VALIDATION_FIXTURES_REORDERFIXTURE_H
#define ACL_TESTS_VALIDATION_FIXTURES_REORDERFIXTURE_H

#include "arm_compute/core/TensorShape.h"
#include "arm_compute/core/Types.h"
#include "tests/AssetsLibrary.h"
#include "tests/Globals.h"
#include "tests/IAccessor.h"
#include "tests/framework/Asserts.h"
#include "tests/framework/Fixture.h"
#include "tests/validation/reference/Reorder.h"
#include "src/core/NEON/kernels/arm_gemm/utils.hpp"

namespace arm_compute
{
namespace test
{
namespace validation
{
/** [ReorderLayer fixture] **/
template <typename TensorType, typename AccessorType, typename FunctionType, typename TOut, typename TIn>
class ReorderValidationFixture : public framework::Fixture
{
public:
    void setup(TensorShape input_shape, TensorShape output_shape, WeightFormat input_wf, WeightFormat output_wf, DataType input_data_type, DataType output_data_type, bool transpose)
    {
        FunctionType reorder;
        TensorInfo input_tensor_info(input_shape, 1, input_data_type);
        TensorInfo output_tensor_info(output_shape, 1, output_data_type);
        if (reorder.validate(&input_tensor_info, &output_tensor_info, input_wf, output_wf, transpose /* transpose */)) {
            _target    = compute_target(input_shape, output_shape, input_wf, output_wf, input_data_type, output_data_type, transpose);
            _reference = compute_reference(input_shape, output_shape, output_wf, input_data_type, output_data_type, transpose);
        }
    }

    protected:
    template <typename U>
    void fill(U &&tensor)
    {
        library->fill_tensor_uniform(tensor, 0);
    }

    TensorType compute_target(const TensorShape &input_shape, const TensorShape &output_shape, WeightFormat input_wf, WeightFormat output_wf, DataType input_data_type, DataType output_data_type, bool transpose)
    {
        // Create tensors
        TensorType src = create_tensor<TensorType>(input_shape, input_data_type);
        TensorType dst = create_tensor<TensorType>(output_shape, output_data_type);

        // Create and configure function
        FunctionType reorder;

        reorder.configure(&src, &dst, input_wf, output_wf, transpose);

        ARM_COMPUTE_ASSERT(src.info()->is_resizable());
        ARM_COMPUTE_ASSERT(dst.info()->is_resizable());

        // Allocate tensors
        src.allocator()->allocate();
        dst.allocator()->allocate();

        ARM_COMPUTE_ASSERT(!src.info()->is_resizable());
        ARM_COMPUTE_ASSERT(!dst.info()->is_resizable());

        // Fill tensors
        fill(AccessorType(src));

        // Compute function
        reorder.run();

        return dst;
    }

    SimpleTensor<TOut> compute_reference(const TensorShape &input_shape, const TensorShape &output_shape, WeightFormat output_wf, DataType input_data_type, DataType output_data_type, bool transpose)
    {
        // Create reference
        SimpleTensor<TIn> src{ input_shape, input_data_type };

        // Fill reference
        fill(src);

        return reference::reorder_layer<TOut, TIn>(src, output_shape, output_wf, output_data_type, transpose);
    }

    TensorType      _target{};
    SimpleTensor<TOut> _reference{};
};
/** [ReorderLayer fixture] **/
} // namespace validation
} // namespace test
} // namespace arm_compute
#endif // ACL_TESTS_VALIDATION_FIXTURES_REORDERFIXTURE_H
