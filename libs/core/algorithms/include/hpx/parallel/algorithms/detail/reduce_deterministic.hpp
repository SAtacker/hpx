//  Copyright (c) 2024 Shreyas Atre
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/functional/detail/tag_fallback_invoke.hpp>
#include <hpx/functional/invoke.hpp>
#include <hpx/parallel/algorithms/detail/rfa.hpp>
#include <hpx/parallel/util/loop.hpp>
#include <hpx/type_support/pack.hpp>

#include <cstddef>
#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>
#include "rfa.hpp"

namespace hpx::parallel::detail {

    template <typename ExPolicy>
    struct sequential_reduce_deterministic_t final
      : hpx::functional::detail::tag_fallback<
            sequential_reduce_deterministic_t<ExPolicy>>
    {
    private:
        template <typename InIterB, typename InIterE, typename T,
            typename Reduce>
        friend constexpr T tag_fallback_invoke(
            sequential_reduce_deterministic_t, ExPolicy&&, InIterB first,
            InIterE last, T init, Reduce&& r)
        {
            /// TODO: Put constraint on Reduce to be a binary plus operator
            (void) r;
            hpx::parallel::detail::rfa::RFA_bins<T> bins;
            bins.initialize_bins();
            std::memcpy(rfa::__rfa_bin_host_buffer__, &bins, sizeof(bins));

            hpx::parallel::detail::rfa::ReproducibleFloatingAccumulator<T> rfa;
            rfa.set_max_abs_val(init);
            rfa.unsafe_add(init);
            rfa.renorm();
            size_t count = 0;
            T max_val = std::abs(*first);
            for (auto e = first; e != last; ++e)
            {
                T temp_max_val = std::abs(static_cast<T>(*e));
                if (max_val < temp_max_val)
                {
                    rfa.set_max_abs_val(temp_max_val);
                    max_val = temp_max_val;
                }
                rfa.unsafe_add(*e);
                count++;
                if (count == rfa.endurance())
                {
                    rfa.renorm();
                    count = 0;
                }
            }
            return rfa.conv();
        }
    };

    template <typename ExPolicy>
    struct sequential_reduce_deterministic_rfa_t final
      : hpx::functional::detail::tag_fallback<
            sequential_reduce_deterministic_rfa_t<ExPolicy>>
    {
    private:
        template <typename InIterB, typename InIterE, typename T,
            typename Reduce>
        friend constexpr hpx::parallel::detail::rfa::
            ReproducibleFloatingAccumulator<T>
            tag_fallback_invoke(sequential_reduce_deterministic_rfa_t,
                ExPolicy&&, InIterB first, InIterE last, T init, Reduce&& r)
        {
            hpx::parallel::detail::rfa::RFA_bins<T> bins;
            bins.initialize_bins();
            std::memcpy(rfa::bin_host_buffer, &bins, sizeof(bins));

            hpx::parallel::detail::rfa::ReproducibleFloatingAccumulator<T> rfa;

            for (auto e = first; e != last; ++e)
            {
                rfa += *e;
            }
            return rfa;
        }

        template <typename InIterB, typename T, typename Reduce>
        friend constexpr hpx::parallel::detail::rfa::
            ReproducibleFloatingAccumulator<T>
            tag_fallback_invoke(sequential_reduce_deterministic_rfa_t,
                ExPolicy&&, InIterB first, std::size_t size, T init, Reduce&& r)
        {
            hpx::parallel::detail::rfa::RFA_bins<T> bins;
            bins.initialize_bins();
            std::memcpy(rfa::bin_host_buffer, &bins, sizeof(bins));

            hpx::parallel::detail::rfa::ReproducibleFloatingAccumulator<T> rfa;
            auto e = first;
            for (std::size_t i = 0; i < size; ++i, ++e)
            {
                rfa += *e;
            }
            return rfa;
        }

        // template <typename InIterB, typename InIterE, typename T,
        //     typename Reduce
        //     // typename = std::enable_if_t<hpx::util::contains<T,
        //     //     hpx::parallel::detail::rfa::ReproducibleFloatingAccumulator<
        //     //         float>,
        //     //     hpx::parallel::detail::rfa::ReproducibleFloatingAccumulator<
        //     //         double>>::value>
        //     >
        // friend constexpr T tag_fallback_invoke(
        //     sequential_reduce_deterministic_rfa_t, ExPolicy&&, InIterB first,
        //     InIterE last, T init, Reduce&& r)
        // {
        //     static_assert(hpx::util::contains<T,
        //         hpx::parallel::detail::rfa::ReproducibleFloatingAccumulator<
        //             float>,
        //         hpx::parallel::detail::rfa::ReproducibleFloatingAccumulator<
        //             double>>::value);
        //     hpx::parallel::detail::rfa::RFA_bins<T> bins;
        //     bins.initialize_bins();
        //     std::memcpy(rfa::bin_host_buffer, &bins, sizeof(bins));

        //     hpx::parallel::detail::rfa::ReproducibleFloatingAccumulator<T> rfa;
        //     rfa.set_max_abs_val(init);
        //     rfa.unsafe_add(init);
        //     rfa.renorm();
        //     for (auto e = first; e != last; ++e)
        //     {
        //         rfa += *e;
        //     }
        //     return rfa.conv();
        // }
    };

#if !defined(HPX_COMPUTE_DEVICE_CODE)
    template <typename ExPolicy>
    inline constexpr sequential_reduce_deterministic_t<ExPolicy>
        sequential_reduce_deterministic =
            sequential_reduce_deterministic_t<ExPolicy>{};
#else
    template <typename ExPolicy, typename... Args>
    HPX_HOST_DEVICE HPX_FORCEINLINE auto sequential_reduce_deterministic(
        Args&&... args)
    {
        return sequential_reduce_deterministic_t<ExPolicy>{}(
            std::forward<Args>(args)...);
    }
#endif

#if !defined(HPX_COMPUTE_DEVICE_CODE)
    template <typename ExPolicy>
    inline constexpr sequential_reduce_deterministic_rfa_t<ExPolicy>
        sequential_reduce_deterministic_rfa =
            sequential_reduce_deterministic_rfa_t<ExPolicy>{};
#else
    template <typename ExPolicy, typename... Args>
    HPX_HOST_DEVICE HPX_FORCEINLINE auto sequential_reduce_deterministic_rfa(
        Args&&... args)
    {
        return sequential_reduce_deterministic_rfa_t<ExPolicy>{}(
            std::forward<Args>(args)...);
    }
#endif
}    // namespace hpx::parallel::detail
