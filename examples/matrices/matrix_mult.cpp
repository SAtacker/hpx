//  Copyright (c) 2022 Shreyas Atre
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/local/algorithm.hpp>
#include <hpx/local/chrono.hpp>
#include <hpx/local/init.hpp>
#include <hpx/modules/assertion.hpp>
#include <boost/range/irange.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <vector>

template <typename T>
using integral_type = typename std::enable_if_t<std::is_integral_v<T>, T>;

template <typename T, typename T2 = integral_type<T>>
using matrix_t = std::vector<std::vector<T>>;

// template <typename T, typename T2 = integral_type<T>>
// using matrix_c_iter = std::vector<T>::iterator;

// template <typename T, typename T2 = integral_type<T>>
// using matrix_r_iter = std::vector<std::vector<T>>::iterator;

template <typename T, typename T2 = integral_type<T>>
class Matrix
{
private:
    using matrix = matrix_t<T>;
    matrix m;

public:
    explicit Matrix(std::uint64_t r = 0, std::uint64_t c = 0)
      : m(r, std::vector<T>(c)){};
    Matrix() = default;
    Matrix(matrix&& m_)
      : m(std::move(m_)){};
    Matrix(const matrix& _matrix = {{0}})
      : m(_matrix){};
    ~Matrix() {}
    const auto n_rows()
    {
        return m.size();
    }
    const auto n_cols()
    {
        return m[0].size();
    }
    auto& operator()(auto i, auto j)
    {
        return m[i][j];
    }
    friend Matrix<T> operator*(Matrix& lhs, Matrix& rhs)
    {
        std::size_t M = lhs.n_rows();    // mxp
        std::size_t N = rhs.n_cols();    // pxn
        Matrix<T> res(M, N);
        auto range = boost::irange<std::size_t>(0, M);
        hpx::for_each(hpx::execution::par, boost::begin(range),
            boost::end(range), [&](std::size_t i) {
                for (std::size_t j = 0; j < N; ++j)
                {
                    T r = T(0);
                    for (std::size_t k = 0; k < lhs.n_cols(); ++k)
                    {
                        r += lhs(i, k) * rhs(k, j);
                    }
                    res(i, j) = r;
                }
            });
        return Matrix<T>(std::move(res));
    }
    friend Matrix<T> operator*(Matrix&& lhs, Matrix&& rhs)
    {
        return lhs * rhs;
    }
    bool friend operator==(auto lhs, auto rhs)
    {
        std::size_t M = lhs.n_rows();
        std::size_t N = lhs.n_cols();
        if (M != rhs.n_rows())
        {
            return false;
        }
        if (N != rhs.n_cols())
        {
            return false;
        }
        bool res = true;
        auto range = boost::irange<std::size_t>(0, M);
        hpx::for_each(hpx::execution::par, boost::begin(range),
            boost::end(range), [&](std::size_t i) {
                for (std::size_t j = 0; j < N; ++j)
                {
                    res = lhs(i, j) == rhs(i, j);
                    if (!res)
                    {
                        return false;
                    }
                }
                return res;
            });
        return res;
    }
    friend std::ostream& operator<<(std::ostream& os, Matrix& mat)
    {
        for (std::size_t row = 0; row < mat.n_rows(); row++)
        {
            bool comma{false};
            for (std::size_t col = 0; col < mat.n_cols(); col++)
            {
                if (comma)
                    os << ", ";
                os << mat(row, col);
                comma = true;
            }
            os << "\n";
        }
        return os;
    }
    friend std::ostream& operator<<(std::ostream& os, Matrix&& mat)
    {
        return operator<<(os, mat);
    }
};

int hpx_main(hpx::program_options::variables_map& vm)
{
    {
        std::cout << Matrix<int>({
            {1, 2, 3},
            {0, 1, 0},
            {0, 0, 1},
        });

        if (Matrix<int>({
                {1, 2, 3},
                {0, 1, 0},
                {0, 0, 1},
            }) ==
            Matrix<int>({
                {1, 2, 3},
                {0, 1, 0},
                {0, 0, 1},
            }))
        {
            std::cout << "Correct comparison\n";
        }
        else
        {
            std::cout << "Incorrect comparison\n";
        }
    }

    {
        Matrix<int> a({
            {1, 2},
            {3, 4},
        });
        Matrix<int> b({
            {1, 1},
            {1, 1},
        });
        Matrix<int> ab = a * b;
        std::cout << ab;
        if (ab !=
            Matrix<int>({
                {3, 3},
                {7, 7},
            }))
        {
            std::cout << "Matrix incorrect\n";
        }
        else
        {
            std::cout << "Correct result\n";
        }
    }

    {
        Matrix<int> c({
            {1, 2},
            {3, 4},
        });
        Matrix<int> d({
            {1, 2},
            {3, 4},
        });
        Matrix<int> cd = c * d;
        std::cout << cd;
        if (cd !=
            Matrix<int>({
                {7, 10},
                {15, 20},
            }))
        {
            std::cout << "Matrix incorrect\n";
        }
        else
        {
            std::cout << "Correct result\n";
        }
    }

    {
        Matrix<int> e({
            {1, 2},
            {3, 4},
        });
        Matrix<int> f({
            {1, 2, 3},
            {4, 5, 6},
        });
        Matrix<int> ef = e * f;
        std::cout << ef;
        if (ef !=
            Matrix<int>({
                {9, 12, 15},
                {19, 26, 33},
            }))
        {
            std::cout << "Matrix incorrect\n";
        }
        else
        {
            std::cout << "Correct result\n";
        }
    }
    return hpx::local::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::local::init(hpx_main, argc, argv);
}
