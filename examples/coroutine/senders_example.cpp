//  Copyright (c) 2022 Shreyas Atre
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// This is a purely local version demonstrating the proposed extension to
// C++ implementing resumable functions (see N3564,
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3564.pdf). The
// necessary transformations are performed by hand.

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

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2300r4.html#example-hello-world
 * This example demonstrates the basics of schedulers, senders, and receivers
 */
int hpx_main()
{
    hpx::execution::experimental::thread_pool_scheduler scheduler{};

    /**
     * execution::schedule, which returns a sender that completes
     * on the scheduler.  A sender describes asynchronous work and 
     * sends a signal (value, error, or stopped) to some recipient(s) 
     * when that work completes.
     */
    auto begin = hpx::execution::experimental::schedule(scheduler);

    /**
     * then: A sender adaptor that takes an input sender and a std::invocable,
     * and calls the invocable on the signal sent by the input sender.
     * hi: The sender returned by then sends the result of that invocation
     */
    auto hi = hpx::execution::experimental::then(begin, [] {
        std::cout << "Hello world! Have an int.\n";
        return 13;
    });

    /**
     * Now, we add another operation to the chain,we get sent a value -
     * the int from the previous step. We add 42 to it, and then return 
     * the result.
     */
    auto add_42 = hpx::execution::experimental::then(
        hi, [](int arg) { return arg + 42; });

    /** 
     * Finally, we’re ready to submit the entire asynchronous pipeline and
     * wait for its completion. Everything up until this point has been
     * completely asynchronous; the work may not have even started yet.
     * To ensure the work has started and then block pending its completion,
     * we use this_thread::sync_wait,
     */
    auto result = hpx::this_thread::experimental::sync_wait(add_42);

    hpx::util::format_to(std::cout, "num 42 + 13 = {1}\n", result);

    return hpx::local::finalize();    // Handles HPX shutdown
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    return hpx::local::init(hpx_main, argc, argv);
}
