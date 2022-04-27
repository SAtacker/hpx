//  Copyright (c) 2022 Shreyas Atre
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/execution/queries/get_scheduler.hpp>
#include <hpx/modules/testing.hpp>

#include <exception>
#include <utility>

namespace mylib {

    // CPO
    struct inline_scheduler_0
    {
        constexpr friend HPX_FORCEINLINE auto tag_invoke(
            hpx::execution::experimental::get_forward_progress_guarantee_t,
            const inline_scheduler_0&) noexcept
        {
            return hpx::execution::experimental::forward_progress_guarantee::
                concurrent;
        }
    } scheduler{};

    // CPO fallback to parallel
    struct inline_scheduler_1
    {
        constexpr friend HPX_FORCEINLINE auto tag_invoke(
            hpx::execution::experimental::get_forward_progress_guarantee_t,
            const inline_scheduler_1&) noexcept
        {
            return true;
        }

    } scheduler_return_invalid{};

    // CPO fallback to parallel
    struct inline_scheduler_2
    {
        constexpr friend HPX_FORCEINLINE auto tag_invoke(
            hpx::execution::experimental::get_forward_progress_guarantee_t,
            inline_scheduler_2) noexcept
        {
            return hpx::execution::experimental::forward_progress_guarantee::
                concurrent;
        }

    } scheduler_no_const_ref{};

}    // namespace mylib

int main()
{
    using namespace mylib;
    static_assert(hpx::execution::experimental::get_forward_progress_guarantee(
                      scheduler) ==
            hpx::execution::experimental::forward_progress_guarantee::
                concurrent,
        "scheduler::fpg should return concurrent");

    static_assert(hpx::execution::experimental::get_forward_progress_guarantee(
                      scheduler_return_invalid) ==
            hpx::execution::experimental::forward_progress_guarantee::parallel,
        "CPO should invoke tag_fallback which should return parallel");

    static_assert(hpx::execution::experimental::get_forward_progress_guarantee(
                      scheduler_no_const_ref) ==
            hpx::execution::experimental::forward_progress_guarantee::parallel,
        "CPO should invoke tag_fallback which should return parallel");

    return hpx::util::report_errors();
}