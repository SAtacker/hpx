//  Copyright (c) 2019 Mikael Simberg
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>
#include <hpx/logging.hpp>
#include <hpx/logging/config/defines.hpp>

#if defined(HPX_LOGGING_HAVE_DEPRECATION_WARNINGS)
#if defined(HPX_MSVC)
#pragma message("The header hpx/util/logging.hpp is deprecated,                \
    please include hpx/logging.hpp instead")
#else
#warning "The header hpx/util/logging.hpp is deprecated,                       \
    please include hpx/logging.hpp instead"
#endif
#endif