#pragma once

/**
 * @file fstring_convertions.hpp
 * @author zugyonozz (rafizuhayr001@gmail.com)
 * @brief Conversion utilities for fstring
 * @version 2.0.0
 * @date 2025-11-26
 * 
 * @copyright Copyright (c) 2025
 */

#include "fstring_core.hpp"
#include <cmath>

namespace zuu {

// ==================== String to fstring ====================

/**
 * @brief Convert C-string to fstring with automatic size
 */
template <char_t CharT>
[[nodiscard]] constexpr auto to_fstring(const CharT* value) noexcept {
    constexpr std::size_t max_size = 256; // reasonable default
    return basic_fstring<CharT, max_size>(value);
}

/**
 * @brief Convert standard string to fstring
 */
template <typename StringT>
requires (traits::is_standard_string_v<StringT>)
[[nodiscard]] constexpr auto to_fstring(const StringT& value) noexcept {
    using CharT = typename StringT::value_type;
    constexpr std::size_t max_size = 256;
    return basic_fstring<CharT, max_size>(value.data(), value.length());
}

/**
 * @brief Create fstring with specific capacity
 */
template <std::size_t Cap, char_t CharT>
[[nodiscard]] constexpr auto make_fstring(const CharT* value) noexcept {
    return basic_fstring<CharT, Cap>(value);
}

// ==================== Integer to fstring ====================

/**
 * @brief Convert integer to fstring
 * 
 * @tparam IntT Integer type
 * @tparam CharT Character type
 * @param value Integer value to convert
 * @return fstring containing string representation
 */
template <std::integral IntT, char_t CharT = char>
[[nodiscard]] constexpr auto to_fstring(IntT value) noexcept {
    constexpr std::size_t max_digits = std::numeric_limits<IntT>::digits10 + 3;
    basic_fstring<CharT, max_digits> result;
    
    if (value == 0) {
        result.append(CharT('0'));
        return result;
    }
    
    bool negative = false;
    using UIntT = std::make_unsigned_t<IntT>;
    UIntT uvalue;
    
    if constexpr (std::is_signed_v<IntT>) {
        if (value < 0) {
            negative = true;
            uvalue = static_cast<UIntT>(-(value + 1)) + 1;
        } else {
            uvalue = static_cast<UIntT>(value);
        }
    } else {
        uvalue = value;
    }
    
    // Convert digits in reverse
    CharT buffer[max_digits];
    std::size_t pos = 0;
    
    while (uvalue > 0) {
        buffer[pos++] = CharT('0') + (uvalue % 10);
        uvalue /= 10;
    }
    
    // Add sign if negative
    if (negative) {
        result.append(CharT('-'));
    }
    
    // Reverse digits into result
    while (pos > 0) {
        result.append(buffer[--pos]);
    }
    
    return result;
}

/**
 * @brief Convert integer to fstring with specific base
 */
template <std::integral IntT, char_t CharT = char>
[[nodiscard]] constexpr auto to_fstring(IntT value, int base) noexcept {
    if (base < 2 || base > 36) return basic_fstring<CharT, 1>{};
    
    constexpr std::size_t max_size = sizeof(IntT) * 8 + 2; // worst case binary + sign
    basic_fstring<CharT, max_size> result;
    
    if (value == 0) {
        result.append(CharT('0'));
        return result;
    }
    
    bool negative = false;
    using UIntT = std::make_unsigned_t<IntT>;
    UIntT uvalue;
    
    if constexpr (std::is_signed_v<IntT>) {
        if (value < 0) {
            negative = true;
            uvalue = static_cast<UIntT>(-value);
        } else {
            uvalue = static_cast<UIntT>(value);
        }
    } else {
        uvalue = value;
    }
    
    CharT buffer[max_size];
    std::size_t pos = 0;
    
    while (uvalue > 0) {
        int digit = uvalue % base;
        buffer[pos++] = (digit < 10) ? CharT('0') + digit : CharT('a') + (digit - 10);
        uvalue /= base;
    }
    
    if (negative) {
        result.append(CharT('-'));
    }
    
    while (pos > 0) {
        result.append(buffer[--pos]);
    }
    
    return result;
}

// ==================== Floating Point to fstring ====================

/**
 * @brief Convert floating point to fstring
 */
template <std::floating_point FloatT, char_t CharT = char>
[[nodiscard]] constexpr auto to_fstring(FloatT value, int precision = 6) noexcept {
    constexpr std::size_t max_size = 64;
    basic_fstring<CharT, max_size> result;
    
    // Handle special cases
    if (std::isnan(value)) {
        result = "nan";
        return result;
    }
    if (std::isinf(value)) {
        result = value < 0 ? "-inf" : "inf";
        return result;
    }
    
    // Handle negative
    if (value < 0) {
        result.append(CharT('-'));
        value = -value;
    }
    
    // Integer part
    auto int_part = static_cast<long long>(value);
    auto int_str = to_fstring<long long, CharT>(int_part);
    result.append(int_str.data(), int_str.length());
    
    // Decimal part
    if (precision > 0) {
        result.append(CharT('.'));
        FloatT frac = value - static_cast<FloatT>(int_part);
        
        for (int i = 0; i < precision && !result.full(); ++i) {
            frac *= 10;
            int digit = static_cast<int>(frac);
            result.append(CharT('0') + digit);
            frac -= digit;
        }
    }
    
    return result;
}

/**
 * @brief Convert floating point to fstring with scientific notation
 */
template <std::floating_point FloatT, char_t CharT = char>
[[nodiscard]] constexpr auto to_fstring_scientific(FloatT value, int precision = 6) noexcept {
    constexpr std::size_t max_size = 64;
    basic_fstring<CharT, max_size> result;
    
    if (std::isnan(value)) {
        result = "nan";
        return result;
    }
    if (std::isinf(value)) {
        result = value < 0 ? "-inf" : "inf";
        return result;
    }
    
    if (value < 0) {
        result.append(CharT('-'));
        value = -value;
    }
    
    // Calculate exponent
    int exponent = 0;
    if (value != 0) {
        exponent = static_cast<int>(std::floor(std::log10(value)));
        value /= std::pow(10, exponent);
    }
    
    // Mantissa
    auto mantissa = to_fstring<FloatT, CharT>(value, precision);
    result.append(mantissa.data(), mantissa.length());
    
    // Exponent
    result.append(CharT('e'));
    if (exponent >= 0) result.append(CharT('+'));
    auto exp_str = to_fstring<int, CharT>(exponent);
    result.append(exp_str.data(), exp_str.length());
    
    return result;
}

// ==================== fstring to Number ====================

/**
 * @brief Parse integer from fstring
 */
template <std::integral IntT, char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr IntT parse_int(const basic_fstring<CharT, Cap>& str, int base = 10) noexcept {
    if (str.empty()) return IntT{};
    
    IntT result = 0;
    std::size_t i = 0;
    bool negative = false;
    
    // Handle sign
    if constexpr (std::is_signed_v<IntT>) {
        if (str[0] == CharT('-')) {
            negative = true;
            ++i;
        } else if (str[0] == CharT('+')) {
            ++i;
        }
    }
    
    // Parse digits
    for (; i < str.length(); ++i) {
        CharT ch = str[i];
        int digit = -1;
        
        if (ch >= CharT('0') && ch <= CharT('9')) {
            digit = ch - CharT('0');
        } else if (ch >= CharT('a') && ch <= CharT('z')) {
            digit = 10 + (ch - CharT('a'));
        } else if (ch >= CharT('A') && ch <= CharT('Z')) {
            digit = 10 + (ch - CharT('A'));
        }
        
        if (digit < 0 || digit >= base) break;
        
        result = result * base + digit;
    }
    
    return negative ? -result : result;
}

/**
 * @brief Parse floating point from fstring
 */
template <std::floating_point FloatT, char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr FloatT parse_float(const basic_fstring<CharT, Cap>& str) noexcept {
    if (str.empty()) return FloatT{};
    
    FloatT result = 0;
    std::size_t i = 0;
    bool negative = false;
    
    // Handle sign
    if (str[0] == CharT('-')) {
        negative = true;
        ++i;
    } else if (str[0] == CharT('+')) {
        ++i;
    }
    
    // Integer part
    while (i < str.length() && detail::is_digit(str[i])) {
        result = result * 10 + (str[i] - CharT('0'));
        ++i;
    }
    
    // Decimal part
    if (i < str.length() && str[i] == CharT('.')) {
        ++i;
        FloatT fraction = 0.1;
        while (i < str.length() && detail::is_digit(str[i])) {
            result += (str[i] - CharT('0')) * fraction;
            fraction *= 0.1;
            ++i;
        }
    }
    
    // Exponent part
    if (i < str.length() && (str[i] == CharT('e') || str[i] == CharT('E'))) {
        ++i;
        bool exp_negative = false;
        if (i < str.length() && str[i] == CharT('-')) {
            exp_negative = true;
            ++i;
        } else if (i < str.length() && str[i] == CharT('+')) {
            ++i;
        }
        
        int exponent = 0;
        while (i < str.length() && detail::is_digit(str[i])) {
            exponent = exponent * 10 + (str[i] - CharT('0'));
            ++i;
        }
        
        if (exp_negative) exponent = -exponent;
        result *= std::pow(FloatT(10), exponent);
    }
    
    return negative ? -result : result;
}

// ==================== Boolean Conversion ====================

/**
 * @brief Boolean to fstring
 */
template <char_t CharT = char>
[[nodiscard]] constexpr auto to_fstring(bool value) noexcept {
    basic_fstring<CharT, 5> result;
    if (value) {
        result = "true";
    } else {
        result = "false";
    }
    return result;
}

/**
 * @brief Parse boolean from fstring
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr bool parse_bool(const basic_fstring<CharT, Cap>& str) noexcept {
    if (str == "true" || str == "1" || str == "yes" || str == "on") return true;
    return false;
}

// ==================== String Builder ====================

/**
 * @brief Build fstring from multiple arguments
 */
struct builder {
    /**
     * @brief Concatenate multiple fstrings
     */
    template <typename Fstring, typename... Fstrings>
    requires (traits::is_fstring_v<Fstring> && 
              ((traits::is_fstring_v<Fstrings> && 
                std::is_same_v<typename Fstring::value_type, typename Fstrings::value_type>) && ...))
    [[nodiscard]] constexpr auto operator()(const Fstring& first, const Fstrings&... rest) const noexcept {
        constexpr std::size_t total_cap = Fstring::capacity + (Fstrings::capacity + ...);
        basic_fstring<typename Fstring::value_type, total_cap> result(first.data(), first.length());
        (result.append(rest.data(), rest.length()), ...);
        return result;
    }
};

// ==================== Format Utilities ====================

/**
 * @brief Format integer with padding
 */
template <std::integral IntT, char_t CharT = char>
[[nodiscard]] constexpr auto format_int(IntT value, std::size_t width, CharT fill = CharT(' ')) noexcept {
    auto str = to_fstring<IntT, CharT>(value);
    constexpr std::size_t max_size = std::numeric_limits<IntT>::digits10 + 32;
    basic_fstring<CharT, max_size> result;
    
    if (str.length() < width) {
        result.append(width - str.length(), fill);
    }
    result.append(str.data(), str.length());
    return result;
}

/**
 * @brief Format with hex representation
 */
template <std::integral IntT, char_t CharT = char>
[[nodiscard]] constexpr auto to_hex(IntT value, bool uppercase = false) noexcept {
    constexpr std::size_t max_size = sizeof(IntT) * 2 + 3; // 0x prefix + digits
    basic_fstring<CharT, max_size> result("0x");
    
    using UIntT = std::make_unsigned_t<IntT>;
    UIntT uvalue = static_cast<UIntT>(value);
    
    CharT buffer[max_size];
    std::size_t pos = 0;
    
    do {
        int digit = uvalue % 16;
        if (digit < 10) {
            buffer[pos++] = CharT('0') + digit;
        } else {
            buffer[pos++] = (uppercase ? CharT('A') : CharT('a')) + (digit - 10);
        }
        uvalue /= 16;
    } while (uvalue > 0);
    
    while (pos > 0) {
        result.append(buffer[--pos]);
    }
    
    return result;
}

/**
 * @brief Format with binary representation
 */
template <std::integral IntT, char_t CharT = char>
[[nodiscard]] constexpr auto to_binary(IntT value) noexcept {
    constexpr std::size_t max_size = sizeof(IntT) * 8 + 3; // 0b prefix + bits
    basic_fstring<CharT, max_size> result("0b");
    
    using UIntT = std::make_unsigned_t<IntT>;
    UIntT uvalue = static_cast<UIntT>(value);
    
    CharT buffer[max_size];
    std::size_t pos = 0;
    
    do {
        buffer[pos++] = CharT('0') + (uvalue % 2);
        uvalue /= 2;
    } while (uvalue > 0);
    
    while (pos > 0) {
        result.append(buffer[--pos]);
    }
    
    return result;
}

} // namespace zuu
