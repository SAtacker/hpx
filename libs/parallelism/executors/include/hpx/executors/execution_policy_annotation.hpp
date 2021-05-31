//  Copyright (c) 2021 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file hpx/execution/execution_policy_annotation.hpp

#pragma once

#include <hpx/config.hpp>
#include <hpx/execution/executors/execution_parameters.hpp>
#include <hpx/execution/executors/rebind_executor.hpp>
#include <hpx/execution/traits/is_execution_policy.hpp>
#include <hpx/properties/property.hpp>

#include <string>
#include <type_traits>
#include <utility>

namespace hpx { namespace execution { namespace experimental {

    // with_annotation property implementation for execution policies
    // that simply forwards to the embedded executor
    template <typename ExPolicy,
        typename Enable =
            std::enable_if_t<hpx::is_execution_policy<ExPolicy>::value &&
                hpx::functional::is_tag_invocable<
                    hpx::execution::experimental::with_annotation_t,
                    typename std::decay_t<ExPolicy>::executor_type,
                    char const*>::value>>
    constexpr decltype(auto) tag_invoke(
        hpx::execution::experimental::with_annotation_t, ExPolicy&& policy,
        char const* annotation)
    {
        auto exec = hpx::execution::experimental::with_annotation(
            policy.executor(), annotation);

        return hpx::parallel::execution::create_rebound_policy(
            policy, std::move(exec), policy.parameters());
    }

    template <typename ExPolicy,
        typename Enable =
            std::enable_if_t<hpx::is_execution_policy<ExPolicy>::value &&
                hpx::functional::is_tag_invocable<
                    hpx::execution::experimental::with_annotation_t,
                    typename std::decay_t<ExPolicy>::executor_type,
                    std::string>::value>>
    decltype(auto) tag_invoke(
        hpx::execution::experimental::with_annotation_t, ExPolicy&& policy,
        std::string annotation)
    {
        auto exec = hpx::execution::experimental::with_annotation(
            policy.executor(), std::move(annotation));

        return hpx::parallel::execution::create_rebound_policy(
            policy, std::move(exec), policy.parameters());
    }

    // get_annotation property implementation for execution policies
    // that simply forwards to the embedded executor
    template <typename ExPolicy,
        typename Enable =
            std::enable_if_t<hpx::is_execution_policy<ExPolicy>::value>>
    constexpr decltype(auto) tag_invoke(
        hpx::execution::experimental::get_annotation_t, ExPolicy&& policy)
    {
        return hpx::execution::experimental::get_annotation(policy.executor());
    }
}}}    // namespace hpx::execution::experimental
