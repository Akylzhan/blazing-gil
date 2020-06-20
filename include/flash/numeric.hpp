#pragma once

#include <blaze/Blaze.h>
#include <blaze/math/typetraits/IsVector.h>
#include <blaze/math/typetraits/UnderlyingElement.h>
#include <blaze/math/views/Submatrix.h>
#include <flash/convolution.hpp>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <type_traits>

namespace flash
{
namespace detail
{
template <typename Expr>
auto compute_diffusivity(Expr& nabla, double kappa)
{
    return blaze::exp(-(nabla / kappa) % (nabla / kappa));
}
} // namespace detail

template <typename T>
blaze::DynamicMatrix<bool> nonmax_map(const blaze::DynamicMatrix<T>& input, std::size_t window_size,
                                      bool padding_value = false)
{
    const auto middle = window_size / 2 + 1;
    blaze::DynamicMatrix<bool> result(input.rows(), input.columns(), padding_value);
    // will substract window_size, thus until .rows()
    for (std::size_t i = window_size; i < input.rows(); ++i) {
        // will substract window_size, thus until .columns()
        for (std::size_t j = window_size; j < input.columns(); ++j) {
            auto submatrix =
                blaze::submatrix(input, i - window_size, j - window_size, window_size, window_size);
            auto max = blaze::max(submatrix);
            auto other_max_flag = false;
            for (std::size_t ii = 0; ii < window_size; ++ii) {
                for (std::size_t jj = 0; jj < window_size; ++jj) {
                    if (ii == middle && jj == middle) {
                        continue;
                    }
                    if (submatrix(ii, jj) == max) {
                        other_max_flag = true;
                        break;
                    }
                }
            }

            result(i, j) = other_max_flag && (max == submatrix(middle, middle));
        }
    }

    return result;
}

blaze::DynamicMatrix<std::int64_t> harris(const blaze::DynamicMatrix<std::int64_t>& image, double k)
{
    auto dx = flash::convolve(image, flash::sobel_x);
    auto dy = flash::convolve(image, flash::sobel_y);

    auto dx_2 = dx % dx;
    auto dy_2 = dy % dy;

    auto dxdy = flash::convolve(dx, flash::sobel_y);

    auto ktrace_2 = (dx_2 + dy_2) % (dx_2 + dy_2) * k;
    auto det = dx_2 % dy_2 - dxdy % dxdy;
    return det - ktrace_2;
}

struct hessian_result {
    blaze::DynamicMatrix<std::int32_t> determinants;
    blaze::DynamicMatrix<std::int32_t> traces;
};

hessian_result hessian(const blaze::DynamicMatrix<std::uint8_t>& input)
{
    blaze::DynamicMatrix<std::int32_t> extended = input;
    auto dx = flash::convolve(extended, flash::sobel_x);
    auto dy = flash::convolve(extended, flash::sobel_y);

    auto ddxx = flash::convolve(dx, flash::sobel_x);
    auto dxdy = flash::convolve(dx, flash::sobel_y);
    auto ddyy = flash::convolve(dy, flash::sobel_y);

    auto det = ddxx % ddyy - dxdy % dxdy;
    auto trace = ddxx + ddyy;
    return {det, trace};
}

template <typename T>
struct identify_type;

// template <typename MT, bool StorageOrder>
// auto anisotropic_diffusion(const blaze::DenseMatrix<MT, StorageOrder>& input, double kappa,
//                            std::uint64_t iteration_count)
// {
//     using element_type = blaze::UnderlyingElement_t<MT>;
//     using output_element_type =
//         std::conditional_t<blaze::IsVector_v<element_type>,
//                            blaze::StaticVector<double, element_type::size()>,
//                            double>;
//     using compute_element_type = blaze::StaticVector<output_element_type, 8>;
//     using matrix_type = blaze::DynamicMatrix<output_element_type, StorageOrder>;
//     using compute_matrix_type = blaze::DynamicMatrix<compute_element_type, StorageOrder>;
//     matrix_type output(input);
//     compute_matrix_type nabla(output.rows(), output.columns());
//     // compute_matrix_type c(output.rows(), output.columns());
//     for (std::uint64_t i = 0; i < iteration_count; ++i) {
//         const auto rows = output.rows();
//         const auto columns = output.columns();
//         const auto zero = output_element_type(0);
//         const auto zero_vector = compute_element_type(zero);
//         // middle case
//         nabla =
//             blaze::generate(nabla.rows(),
//                             nabla.columns(),
//                             [&output, rows, columns, zero_vector](std::size_t i, std::size_t j) {
//                                 if (i == 0 || i == rows - 1 || j == 0 || j == columns - 1) {
//                                     return zero_vector;
//                                 }
//                                 const auto current = output(i, j);
//                                 return compute_element_type{output(i - 1, j) - current,     //
//                                 north
//                                                             output(i + 1, j) - current,     //
//                                                             south output(i, j + 1) - current, //
//                                                             east output(i, j - 1) - current, //
//                                                             west output(i - 1, j + 1) - current,
//                                                             // NE output(i - 1, j - 1) - current,
//                                                             // NW output(i + 1, j + 1) - current,
//                                                             // SE output(i + 1, j - 1) -
//                                                             current}; // SW
//                             });

//         // std::cout << blaze::isZero(nabla) << '\n';

//         // upper row
//         // const auto first_row = blaze::evaluate(
//         //     blaze::generate(columns, [&output, columns, zero, zero_vector](std::size_t j) {
//         //         if (j == 0 || j == columns - 1) {
//         //             return zero_vector;
//         //         }
//         //         // std::cout << j << '\n';
//         //         const auto current = output(0, j);
//         //         return compute_element_type{zero,
//         //                                     output(1, j) - current,
//         //                                     output(0, j + 1) - current,
//         //                                     output(0, j - 1) - current,
//         //                                     zero,
//         //                                     zero,
//         //                                     output(1, j + 1) - current,
//         //                                     output(1, j - 1) - current};
//         //     }));
//         // const auto last_row = blaze::evaluate(
//         //     blaze::generate(columns, [&output, rows, columns, zero, zero_vector](std::size_t
//         j) {
//         //         if (j == 0 || j == columns - 1) {
//         //             return zero_vector;
//         //         }
//         //         const auto current = output(rows - 1, j);
//         //         return compute_element_type{output(rows - 2, j) - current,
//         //                                     zero,
//         //                                     output(rows - 1, j + 1) - current,
//         //                                     output(rows - 1, j - 1) - current,
//         //                                     zero,
//         //                                     zero,
//         //                                     output(rows - 1, j + 1) - current,
//         //                                     output(rows - 1, j - 1) - current};
//         //     }));
//         // const auto first_column = blaze::evaluate(
//         //     blaze::generate(columns, [&output, rows, columns, zero, zero_vector](std::size_t
//         j) {
//         //         if (j == 0 || j == rows - 1) {
//         //             return zero_vector;
//         //         }
//         //         const auto current = output(j, 0);
//         //         return compute_element_type{output(j - 1, 0) - current,
//         //                                     output(j + 1, 0) - current,
//         //                                     output(j, 1) - current,
//         //                                     zero,
//         //                                     output(j - 1, 1) - current,
//         //                                     zero,
//         //                                     output(j + 1, 1) - current,
//         //                                     zero};
//         //     }));
//         // const auto last_column = blaze::evaluate(
//         //     blaze::generate(columns, [&output, rows, columns, zero, zero_vector](std::size_t
//         j) {
//         //         if (j == 0 || j == rows - 1) {
//         //             return zero_vector;
//         //         }
//         //         const auto current = output(j, columns - 1);
//         //         return compute_element_type{output(j - 1, columns - 1) - current,
//         //                                     output(j + 1, columns - 1) - current,
//         //                                     zero,
//         //                                     output(j, columns - 2) - current,
//         //                                     zero,
//         //                                     output(j - 1, columns - 2) - current,
//         //                                     zero,
//         //                                     output(j + 1, columns - 2)};
//         //     }));
//         // for (std::size_t j = 0; j < columns; ++j) {
//         //     nabla(0, j) = first_row[j];
//         //     nabla(rows - 1, j) = last_row[j];
//         // }

//         // for (std::size_t j = 0; j < columns; ++j) {
//         //     nabla(j, 0) = first_column[j];
//         //     nabla(j, columns - 1) = last_column[j];
//         // }

//         // auto current = output(0, 0);
//         // nabla(0, 0) = {zero,
//         //                output(1, 0) - current,
//         //                output(0, 1) - current,
//         //                zero,
//         //                zero,
//         //                output(1, 1) - current,
//         //                zero,
//         //                zero};
//         // current = output(0, columns - 1);
//         // nabla(0, columns - 1) = {zero,
//         //                          output(1, columns - 1) - current,
//         //                          zero,
//         //                          output(0, columns - 2) - current,
//         //                          zero,
//         //                          zero,
//         //                          zero,
//         //                          output(1, columns - 2) - current};
//         // current = output(rows - 1, 0);
//         // nabla(rows - 1, 0) = {output(rows - 2, 0) - current,
//         //                       zero,
//         //                       output(rows - 1, 1) - current,
//         //                       zero,
//         //                       output(rows - 2, 1) - current,
//         //                       zero,
//         //                       zero,
//         //                       zero};
//         // current = output(rows - 1, columns - 1);
//         // nabla(rows - 1, columns - 1) = {output(rows - 2, columns - 1) - current,
//         //                                 zero,
//         //                                 zero,
//         //                                 output(rows - 1, columns - 2) - current,
//         //                                 zero,
//         //                                 output(rows - 2, columns - 2) - current,
//         //                                 zero,
//         //                                 zero};

//         auto c = blaze::map(nabla, [kappa](const auto& element) {
//             if (blaze::isZero(element)) {
//                 // std::cout << "encountered zero\n";
//                 return element;
//             }
//             auto c_element = blaze::evaluate(element / kappa);
//             auto half = 0.5;
//             return compute_element_type{
//                 blaze::exp(-c_element[0] * c_element[0]),
//                 blaze::exp(-c_element[1] * c_element[1]),
//                 blaze::exp(-c_element[2] * c_element[2]),
//                 blaze::exp(-c_element[3] * c_element[3]),
//                 blaze::exp(-c_element[4] * c_element[4]) * half,
//                 blaze::exp(-c_element[5] * c_element[5]) * half,
//                 blaze::exp(-c_element[6] * c_element[6]) * half,
//                 blaze::exp(-c_element[7] * c_element[7]) * half,
//             };
//         });
//         nabla %= c;
//         auto sum = blaze::evaluate(
//             blaze::map(nabla, [](auto element) { return blaze::sum(element) * 1.0 / 7; }));
//         // std::cout << sum << '\n';
//         output += sum;
//     }

//     return output;
// }

template <typename MT, bool StorageOrder, bool OutputStorageOrder = StorageOrder>
auto anisotropic_diffusion(const blaze::DenseMatrix<MT, StorageOrder>& input, double delta_t,
                           double kappa, std::uint64_t iteration_count)
{
    using element_type = blaze::UnderlyingElement_t<MT>;
    using output_element_type =
        std::conditional_t<blaze::IsVector_v<element_type>,
                           blaze::StaticVector<double, element_type::size()>,
                           double>;
    using compute_element_type = blaze::StaticVector<output_element_type, 4>;
    using output_matrix_type = blaze::DynamicMatrix<output_element_type, OutputStorageOrder>;
    using compute_matrix_type = blaze::DynamicMatrix<compute_element_type, StorageOrder>;

    const auto rows = (~input).rows();
    const auto columns = (~input).columns();
    output_matrix_type output(rows + 2, columns + 2, output_element_type(0));
    auto output_area = blaze::submatrix(output, 1, 1, rows, columns);
    output_area = input;

    compute_matrix_type nabla(rows, columns);
    compute_matrix_type diffusivity(rows, columns);

    for (std::uint64_t counter = 0; counter < iteration_count; ++counter) {
        nabla = blaze::generate(
            rows,
            columns,
            [&output, rows, columns](std::size_t relative_i, std::size_t relative_j) {
                auto i = relative_i + 1;
                auto j = relative_j + 1;
                const auto& current = output(i, j);
                compute_element_type result{output(i - 1, j) - current,
                                            output(i + 1, j) - current,
                                            output(i, j - 1) - current,
                                            output(i, j + 1) - current};
                return result;
            });

        diffusivity = blaze::map(nabla, [kappa](auto value) {
            value /= kappa;
            value *= value;
            return blaze::exp(-value);
        });

        auto product = nabla % diffusivity;
        auto sum = blaze::map(product, [](const auto value) {
            // if constexpr (std::is_same_v<double, decltype(value)>) {
            //     return value;
            // } else {
            //     return blaze::sum(value);
            // }
            return blaze::sum(value);
        });

        output_area = output_area + sum * delta_t;
    }

    return output_matrix_type(output_area);
}
} // namespace flash
