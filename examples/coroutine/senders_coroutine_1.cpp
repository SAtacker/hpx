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

struct Sender : hpx::future<int>
{
    auto operator co_await()
    {
        hpx::future<int> i = hpx::make_ready_future(42);

        return i;
    }
};

hpx::future<int> async_algo(auto s)
{
    int the_answer = co_await s;
    assert(the_answer == 42);
    co_return the_answer;
}

int hpx_main()
{
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
