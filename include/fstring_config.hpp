#pragma once

/**
 * @file fstring_config.hpp
 * @author zugyonozz (rafizuhayr001@gmail.com)
 * @brief Configuration and compile-time settings for fstring library
 * @version 2.0.0
 * @date 2025-11-26
 * 
 * @copyright Copyright (c) 2025
 */

#include <cstddef>

namespace zuu {

/// Configuration namespace for fstring library
namespace config {
    /// Default small string optimization size
    constexpr std::size_t default_sso_size = 32;
    
    /// Enable bounds checking in release mode
    #ifndef FSTRING_DISABLE_BOUNDS_CHECK
        constexpr bool enable_bounds_check = true;
    #else
        constexpr bool enable_bounds_check = false;
    #endif
    
    /// Enable constexpr evaluation
    constexpr bool enable_constexpr = true;
    
    /// Maximum recursion depth for string operations
    constexpr std::size_t max_recursion_depth = 100;
} // namespace config

/// Version information
namespace version {
    constexpr int major = 2;
    constexpr int minor = 0;
    constexpr int patch = 0;
    constexpr const char* string = "2.0.0";
} // namespace version

} // namespace zuu
