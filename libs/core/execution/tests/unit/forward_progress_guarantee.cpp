//  Copyright (c) 2022 Shreyas Atre
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/execution/queries/get_scheduler.hpp>
#include <hpx/execution_base/completion_scheduler.hpp>
#include <hpx/modules/testing.hpp>

#include <exception>
#include <utility>

namespace mylib {

    // CPO
    struct inline_scheduler_0
    {
        struct sender
        {
            template <template <class...> class Tuple,
                template <class...> class Variant>
            using value_types = Variant<Tuple<>>;

            template <template <class...> class Variant>
            using error_types = Variant<std::exception_ptr>;

            static constexpr bool sends_stopped = false;

            struct operation_state
            {
                void start() & noexcept {};
            };

            template <typename R>
            operation_state connect(R&&) && noexcept
            {
                return {};
            }
        };

        friend sender tag_invoke(
            hpx::execution::experimental::schedule_t, inline_scheduler_0)
        {
            return {};
        }

        constexpr friend HPX_FORCEINLINE auto tag_invoke(
            hpx::execution::experimental::get_forward_progress_guarantee_t,
            const inline_scheduler_0&) noexcept
        {
            return hpx::execution::experimental::forward_progress_guarantee::
                concurrent;
        }

    } scheduler{};

    // CPO fallback to weakly parallel
    struct inline_scheduler_1
    {
    } scheduler_no_impl{};

    // CPO fallback to weakly parallel
    struct inline_scheduler_2
    {
        // similar to std::execution
        struct __sender
        {
            using completion_signatures =
                hpx::execution::experimental::completion_signatures<
                    hpx::execution::experimental::set_value_t(),
                    hpx::execution::experimental::set_error_t(
                        std::exception_ptr)>;

            template <class R>
            friend auto tag_invoke(hpx::execution::experimental::connect_t,
                __sender, R&& rec) noexcept(std::
                    is_nothrow_constructible_v<std::remove_cvref_t<R>, R>)
            {
                return {(R &&) rec};
            }

            friend inline_scheduler_2 tag_invoke(
                hpx::execution::experimental::get_completion_scheduler_t<
                    hpx::execution::experimental::set_value_t>,
                __sender) noexcept
            {
                return {};
            }
        };

        friend __sender tag_invoke(hpx::execution::experimental::schedule_t,
            const inline_scheduler_2&) noexcept
        {
            return {};
        }
    } scheduler_tag{};

}    // namespace mylib

int main()
{
    using namespace mylib;
    static_assert(hpx::execution::experimental::get_forward_progress_guarantee(
                      scheduler) ==
            hpx::execution::experimental::forward_progress_guarantee::
                concurrent,
        "scheduler should return concurrent");

    static_assert(hpx::execution::experimental::get_forward_progress_guarantee(
                      scheduler_no_impl) ==
            hpx::execution::experimental::forward_progress_guarantee::
                weakly_parallel,
        "CPO should invoke tag_fallback which should return weakly parallel");

    static_assert(hpx::execution::experimental::get_forward_progress_guarantee(
                      scheduler_tag) ==
            hpx::execution::experimental::forward_progress_guarantee::
                weakly_parallel,
        "CPO should invoke tag_fallback which should return weakly parallel");

    {
        auto tag_invoke_return =
            hpx::execution::experimental::schedule(scheduler_tag);
        static_assert(
            hpx::execution::experimental::get_forward_progress_guarantee(
                tag_invoke_return) ==
                hpx::execution::experimental::forward_progress_guarantee::
                    weakly_parallel,
            "CPO should invoke tag_fallback which should return weakly "
            "parallel");
    }

    {
        auto tag_invoke_return =
            hpx::execution::experimental::schedule(scheduler);
        static_assert(
            hpx::execution::experimental::get_forward_progress_guarantee(
                tag_invoke_return) ==
                hpx::execution::experimental::forward_progress_guarantee::
                    weakly_parallel,
            "CPO should invoke tag_fallback which should return weakly "
            "parallel");
    }

    return hpx::util::report_errors();
}