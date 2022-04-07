//  Copyright (c) 2022 Shreyas Atre
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Inspiration from https://youtu.be/tF-Nz4aRWAM?t=2575

#include <hpx/local/execution.hpp>
#include <hpx/local/future.hpp>
#include <hpx/local/init.hpp>
#include <hpx/local/thread.hpp>

#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace check {
    template <typename Awaitable, typename = void>
    constexpr bool has_member_operator_co_await_v = false;

    template <typename Awaitable>
    constexpr bool has_member_operator_co_await_v<Awaitable,
        std::void_t<decltype(std::declval<Awaitable>().operator co_await())>> =
        true;

    template <typename Awaitable, typename = void>
    constexpr bool has_free_operator_co_await_v = false;

    template <typename Awaitable>
    constexpr bool has_free_operator_co_await_v<Awaitable,
        std::void_t<decltype(operator co_await(std::declval<Awaitable>()))>> =
        true;

}    // namespace check

struct Sender : hpx::future<int>
{
    auto operator co_await()
    {
        hpx::future<int> i = hpx::make_ready_future(42);

        return i;
    }
};

struct Sender2 : hpx::future<int>
{
};

hpx::future<int> async_algo(auto s)
{
    int the_answer = co_await s;
    assert(the_answer == 42);
    co_return the_answer;
}

int hpx_main()
{
    assert(check::has_member_operator_co_await_v<Sender>() == true);
    assert(check::has_member_operator_co_await_v<Sender2>() == false);
    assert(check::has_free_operator_co_await_v<Sender1>() == false);
    assert(check::has_free_operator_co_await_v<Sender2>() == false);

    auto result =
        hpx::this_thread::experimental::sync_wait(async_algo(Sender{}));

    hpx::util::format_to(
        std::cout, "num 42 == {1}\n", result);    // num 42 == 42

    return hpx::local::finalize();    // Handles HPX shutdown
}

int main(int argc, char* argv[])
{
    return hpx::local::init(hpx_main, argc, argv);
}
