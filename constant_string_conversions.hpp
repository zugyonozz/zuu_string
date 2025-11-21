/**
 * @file constant_string_conversions.hpp
 * @brief Conversion functions: to_string, from_string, std::string interop
 * @version 1.0.2
 */

#pragma once

#include "constant_string_class.hpp"
#include <string>
#include <string_view>
#include <cmath>
#include <limits>

namespace zuu {

// ============================================================================
// Integer to String Conversion
// ============================================================================

/**
 * @brief Convert signed integer to string
 * @tparam T Signed integer type
 * @tparam N Capacity (auto-calculated if not specified)
 * @param value Integer value
 * @return String representation
 * 
 * @example
 * constexpr auto str = to_string<int>(-12345);
 * static_assert(str == "-12345");
 */
template <traits::signed_integer T, 
          std::size_t N = traits::integer_string_capacity_v<T>>
[[nodiscard]] constexpr auto to_string(T value) noexcept {
    string<char, N> result;
    
    char buffer[N + 1];
    std::size_t len = detail::int_to_chars(buffer, N, value);
    result.append(buffer, len);
    
    return result;
}

/**
 * @brief Convert unsigned integer to string
 */
template <traits::unsigned_integer T, 
          std::size_t N = traits::integer_string_capacity_v<T>>
[[nodiscard]] constexpr auto to_string(T value) noexcept {
    string<char, N> result;
    
    char buffer[N + 1];
    std::size_t len = detail::uint_to_chars(buffer, N, value);
    result.append(buffer, len);
    
    return result;
}

/**
 * @brief Convert integer to string with specific base (2-36)
 * @param value Integer value  
 * @param base Number base (2-36)
 * @return String representation
 */
template <std::size_t N = 65>
[[nodiscard]] constexpr auto to_string_base(unsigned long long value, int base) noexcept {
    string<char, N> result;
    
    if (base < 2 || base > 36) {
        return result; // Invalid base
    }
    
    if (value == 0) {
        result.push_back('0');
        return result;
    }
    
    char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char buffer[N];
    std::size_t pos = 0;
    
    while (value > 0 && pos < N) {
        buffer[pos++] = digits[value % base];
        value /= base;
    }
    
    // Reverse
    for (std::size_t i = 0; i < pos; ++i) {
        result.push_back(buffer[pos - 1 - i]);
    }
    
    return result;
}

// ============================================================================
// Floating Point to String Conversion
// ============================================================================

namespace detail {

/**
 * @brief Helper untuk floating point conversion
 */
template <traits::floating_point T>
constexpr void float_to_chars_simple(
    char* buffer, std::size_t capacity, T value, int precision, std::size_t& len) noexcept {
    
    len = 0;
    if (capacity == 0) return;
    
    // Handle special cases
    if (std::isnan(value)) {
        if (capacity >= 3) {
            buffer[len++] = 'n';
            buffer[len++] = 'a';
            buffer[len++] = 'n';
        }
        return;
    }
    
    if (std::isinf(value)) {
        if (value < 0) {
            if (capacity >= 4) buffer[len++] = '-';
        }
        if (capacity - len >= 3) {
            buffer[len++] = 'i';
            buffer[len++] = 'n';
            buffer[len++] = 'f';
        }
        return;
    }
    
    // Handle negative
    if (value < 0) {
        buffer[len++] = '-';
        value = -value;
    }
    
    // Get integer part
    using ull = unsigned long long;
    ull int_part = static_cast<ull>(value);
    T frac_part = value - static_cast<T>(int_part);
    
    // Convert integer part
    char temp[32];
    std::size_t temp_len = uint_to_chars(temp, 32, int_part);
    for (std::size_t i = 0; i < temp_len && len < capacity; ++i) {
        buffer[len++] = temp[i];
    }
    
    // Add decimal point
    if (precision > 0 && len < capacity) {
        buffer[len++] = '.';
        
        // Convert fractional part
        for (int i = 0; i < precision && len < capacity; ++i) {
            frac_part *= 10;
            int digit = static_cast<int>(frac_part);
            buffer[len++] = '0' + digit;
            frac_part -= digit;
        }
    }
}

} // namespace detail

/**
 * @brief Convert floating point to string
 * @param value Floating point value
 * @param precision Number of decimal places (default 6)
 * @return String representation
 * 
 * @note This is a simple implementation. For full printf-style formatting,
 *       use std::to_chars or std::format in C++20
 */
template <traits::floating_point T, 
          std::size_t N = traits::float_string_capacity_v<T>>
[[nodiscard]] constexpr auto to_string(T value, int precision = 6) noexcept {
    string<char, N> result;
    
    // Simple constexpr-compatible implementation
    char buffer[N];
    std::size_t len = 0;
    
    detail::float_to_chars_simple(buffer, N, value, precision, len);
    
    result.append(buffer, len);
    return result;
}

// ============================================================================
// Boolean to String
// ============================================================================

/**
 * @brief Convert boolean to string
 * @param value Boolean value
 * @param alpha If true, returns "true"/"false", else "1"/"0"
 */
template <std::size_t N = 5>
[[nodiscard]] constexpr auto to_string(bool value, bool alpha = true) noexcept {
    string<char, N> result;
    
    if (alpha) {
        result = value ? "true" : "false";
    } else {
        result = value ? "1" : "0";
    }
    
    return result;
}

// ============================================================================
// Pointer to String
// ============================================================================

/**
 * @brief Convert pointer to hexadecimal string
 */
template <std::size_t N = 18> // "0x" + 16 hex digits
[[nodiscard]] constexpr auto to_string(const void* ptr) noexcept {
    string<char, N> result("0x");
    
    auto value = reinterpret_cast<std::uintptr_t>(ptr);
    
    // Convert to hex
    char digits[] = "0123456789abcdef";
    char buffer[16];
    int pos = 0;
    
    if (value == 0) {
        result.push_back('0');
        return result;
    }
    
    while (value > 0) {
        buffer[pos++] = digits[value % 16];
        value /= 16;
    }
    
    // Reverse and append
    for (int i = pos - 1; i >= 0; --i) {
        result.push_back(buffer[i]);
    }
    
    return result;
}

// ============================================================================
// String to Integer Conversion
// ============================================================================

/**
 * @brief Parse integer from string
 * @param str String to parse
 * @param value Output value
 * @param base Number base (2-36, 0 for auto-detect)
 * @return true if successful
 */
template <traits::signed_integer T, traits::character CharT, std::size_t N>
[[nodiscard]] constexpr bool from_string(const string<CharT, N>& str, T& value, int base = 10) noexcept {
    if (str.empty() || base < 0 || base == 1 || base > 36) return false;
    
    std::size_t pos = 0;
    bool negative = false;
    
    // Skip whitespace
    while (pos < str.length() && detail::is_space(str[pos])) ++pos;
    if (pos >= str.length()) return false;
    
    // Check sign
    if (str[pos] == '-') {
        negative = true;
        ++pos;
    } else if (str[pos] == '+') {
        ++pos;
    }
    
    if (pos >= str.length()) return false;
    
    // Auto-detect base
    if (base == 0) {
        if (str[pos] == '0') {
            if (pos + 1 < str.length() && (str[pos + 1] == 'x' || str[pos + 1] == 'X')) {
                base = 16;
                pos += 2;
            } else if (pos + 1 < str.length() && (str[pos + 1] == 'b' || str[pos + 1] == 'B')) {
                base = 2;
                pos += 2;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    }
    
    // Parse digits
    using UT = std::make_unsigned_t<T>;
    UT result = 0;
    bool has_digits = false;
    
    while (pos < str.length()) {
        char ch = str[pos];
        int digit_value = -1;
        
        if (ch >= '0' && ch <= '9') {
            digit_value = ch - '0';
        } else if (ch >= 'a' && ch <= 'z') {
            digit_value = ch - 'a' + 10;
        } else if (ch >= 'A' && ch <= 'Z') {
            digit_value = ch - 'A' + 10;
        }
        
        if (digit_value < 0 || digit_value >= base) break;
        
        // Check overflow
        UT old_result = result;
        result = result * base + digit_value;
        if (result < old_result) return false; // Overflow
        
        has_digits = true;
        ++pos;
    }
    
    if (!has_digits) return false;
    
    // Apply sign and check range
    if (negative) {
        if (result > static_cast<UT>(std::numeric_limits<T>::max()) + 1) return false;
        value = -static_cast<T>(result);
    } else {
        if (result > static_cast<UT>(std::numeric_limits<T>::max())) return false;
        value = static_cast<T>(result);
    }
    
    return true;
}

/**
 * @brief Parse unsigned integer from string
 */
template <traits::unsigned_integer T, traits::character CharT, std::size_t N>
[[nodiscard]] constexpr bool from_string(const string<CharT, N>& str, T& value, int base = 10) noexcept {
    if (str.empty() || base < 0 || base == 1 || base > 36) return false;
    
    std::size_t pos = 0;
    
    // Skip whitespace
    while (pos < str.length() && detail::is_space(str[pos])) ++pos;
    if (pos >= str.length()) return false;
    
    // Skip '+'
    if (str[pos] == '+') ++pos;
    
    // Auto-detect base
    if (base == 0) {
        if (str[pos] == '0') {
            if (pos + 1 < str.length() && (str[pos + 1] == 'x' || str[pos + 1] == 'X')) {
                base = 16;
                pos += 2;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    }
    
    // Parse
    T result = 0;
    bool has_digits = false;
    
    while (pos < str.length()) {
        char ch = str[pos];
        int digit_value = -1;
        
        if (ch >= '0' && ch <= '9') {
            digit_value = ch - '0';
        } else if (ch >= 'a' && ch <= 'z') {
            digit_value = ch - 'a' + 10;
        } else if (ch >= 'A' && ch <= 'Z') {
            digit_value = ch - 'A' + 10;
        }
        
        if (digit_value < 0 || digit_value >= base) break;
        
        T old_result = result;
        result = result * base + digit_value;
        if (result < old_result) return false; // Overflow
        
        has_digits = true;
        ++pos;
    }
    
    if (!has_digits) return false;
    value = result;
    return true;
}

// ============================================================================
// Conversion to/from std::string
// ============================================================================

/**
 * @brief Convert zuu::string to std::string
 */
template <traits::character CharT, std::size_t N>
[[nodiscard]] inline std::basic_string<CharT> to_std_string(const string<CharT, N>& str) {
    return std::basic_string<CharT>(str.data(), str.length());
}

/**
 * @brief Convert zuu::string to std::string_view
 */
template <traits::character CharT, std::size_t N>
[[nodiscard]] constexpr std::basic_string_view<CharT> to_string_view(const string<CharT, N>& str) noexcept {
    return std::basic_string_view<CharT>(str.data(), str.length());
}

/**
 * @brief Create zuu::string from std::string
 * @tparam N Capacity for result (must be specified)
 */
template <std::size_t N, typename CharT>
[[nodiscard]] inline string<CharT, N> from_std_string(const std::basic_string<CharT>& str) {
    string<CharT, N> result;
    result.append(str.data(), str.length());
    return result;
}

/**
 * @brief Create zuu::string from std::string_view
 */
template <std::size_t N, typename CharT>
[[nodiscard]] constexpr string<CharT, N> from_string_view(std::basic_string_view<CharT> sv) noexcept {
    string<CharT, N> result;
    result.append(sv.data(), sv.length());
    return result;
}

// ============================================================================
// Format String (simple version)
// ============================================================================

/**
 * @brief Simple format string replacement
 * Replaces "{}" with string representations of args
 * 
 * @example
 * auto s = format<100>("Hello {} {}", "world", 42);
 * // Result: "Hello world 42"
 */
template <std::size_t N, typename... Args>
[[nodiscard]] inline auto format(const char* fmt, Args&&... args) {
    string<char, N> result;
    
    // Simple implementation: find {} and replace
    std::array<string<char, 64>, sizeof...(Args)> arg_strs = {
        to_string(std::forward<Args>(args))...
    };
    
    std::size_t arg_idx = 0;
    std::size_t pos = 0;
    std::size_t fmt_len = detail::strlen(fmt);
    
    while (pos < fmt_len) {
        if (fmt[pos] == '{' && pos + 1 < fmt_len && fmt[pos + 1] == '}') {
            if (arg_idx < sizeof...(Args)) {
                result.append(arg_strs[arg_idx++]);
            }
            pos += 2;
        } else {
            result.push_back(fmt[pos]);
            ++pos;
        }
    }
    
    return result;
}

} // namespace zuu
