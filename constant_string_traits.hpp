/**
 * @file constant_string_traits.hpp
 * @brief Type traits dan concepts untuk string library
 * @version 1.0
 */

#pragma once

#include "constant_string_fwd.hpp"
#include <type_traits>
#include <concepts>

namespace zuu::traits {

// ============================================================================
// Character Type Traits
// ============================================================================

/**
 * @brief Type trait untuk mengecek apakah T adalah tipe karakter
 */
template <typename T>
struct is_char : std::false_type {};

template <> struct is_char<char>     : std::true_type {};
template <> struct is_char<wchar_t>  : std::true_type {};
template <> struct is_char<char8_t>  : std::true_type {};
template <> struct is_char<char16_t> : std::true_type {};
template <> struct is_char<char32_t> : std::true_type {};

/**
 * @brief Concept untuk tipe karakter valid
 */
template <typename T>
concept character = is_char_v<T>;

// ============================================================================
// String Type Traits
// ============================================================================

/**
 * @brief Type trait untuk mengecek apakah T adalah zuu::string
 */
template <typename T>
struct is_zuu_string : std::false_type {};

template <character CharT, std::size_t N>
struct is_zuu_string<string<CharT, N>> : std::true_type {};

template <typename T>
inline constexpr bool is_zuu_string_v = is_zuu_string<std::remove_cv_t<T>>::value;

/**
 * @brief Concept untuk zuu::string
 */
template <typename T>
concept zuu_string = is_zuu_string_v<T>;

// ============================================================================
// Integer Type Traits untuk to_string
// ============================================================================

/**
 * @brief Concept untuk signed integer
 */
template <typename T>
concept signed_integer = std::is_integral_v<T> && std::is_signed_v<T>;

/**
 * @brief Concept untuk unsigned integer
 */
template <typename T>
concept unsigned_integer = std::is_integral_v<T> && std::is_unsigned_v<T>;

/**
 * @brief Concept untuk floating point
 */
template <typename T>
concept floating_point = std::is_floating_point_v<T>;

/**
 * @brief Concept untuk arithmetic types
 */
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

// ============================================================================
// String View Traits
// ============================================================================

/**
 * @brief Type trait untuk mengecek apakah T adalah string-like
 */
template <typename T>
struct is_string_like : std::false_type {};

template <character CharT, std::size_t N>
struct is_string_like<string<CharT, N>> : std::true_type {};

template <character CharT>
struct is_string_like<CharT*> : std::true_type {};

template <character CharT>
struct is_string_like<const CharT*> : std::true_type {};

template <character CharT, std::size_t N>
struct is_string_like<CharT[N]> : std::true_type {};

template <typename T>
inline constexpr bool is_string_like_v = is_string_like<std::remove_cv_t<T>>::value;

// ============================================================================
// Capacity Calculation
// ============================================================================

/**
 * @brief Menghitung kapasitas yang dibutuhkan untuk menyimpan integer
 */
template <typename T>
struct integer_string_capacity {
    // Max digits untuk tipe integer + sign + null terminator
    static constexpr std::size_t value = 
        std::is_signed_v<T> ? 
        (sizeof(T) * 3 + 2) :  // signed: lebih banyak untuk sign
        (sizeof(T) * 3 + 1);   // unsigned
};

template <typename T>
inline constexpr std::size_t integer_string_capacity_v = 
    integer_string_capacity<T>::value;

/**
 * @brief Menghitung kapasitas untuk floating point
 * Format: -X.XXXXXXe+XXX\0
 */
template <typename T>
struct float_string_capacity {
    static constexpr std::size_t value = sizeof(T) <= 4 ? 16 : 32;
};

template <typename T>
inline constexpr std::size_t float_string_capacity_v = 
    float_string_capacity<T>::value;

} // namespace zuu::traits
