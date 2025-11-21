/**
 * @file constant_string_literals.hpp
 * @brief User-defined literals untuk compile-time strings
 * @version 1.0
 */

#pragma once

#include "constant_string_conversions.hpp"

// Suppress warning for string literal operator templates (GNU/Clang extension)
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

namespace zuu::literals {

/**
 * @brief User-defined literal untuk char string
 * @example auto str = "Hello"_cs;
 */
template <zuu::traits::character CharT, CharT... Cs>
[[nodiscard]] constexpr auto operator""_cs() noexcept {
    constexpr CharT arr[] = {Cs..., CharT{}};
    return zuu::string<CharT, sizeof...(Cs)>(arr);
}

/**
 * @brief User-defined literal untuk wide string
 * @example auto str = L"Hello"_ws;
 */
template <zuu::traits::character CharT, CharT... Cs>
[[nodiscard]] constexpr auto operator""_ws() noexcept {
    constexpr CharT arr[] = {Cs..., CharT{}};
    return zuu::string<CharT, sizeof...(Cs)>(arr);
}

/**
 * @brief Literal dengan kapasitas eksplisit
 * Format: "text"_cs<N>
 * Digunakan via template: auto s = string_literal<10>("text");
 */
template <std::size_t N>
struct string_literal_t {
    template <std::size_t M>
    [[nodiscard]] constexpr auto operator()(const char (&str)[M]) const noexcept {
        return zuu::string<char, N>(str);
    }
    
    [[nodiscard]] constexpr auto operator()(const char* str) const noexcept {
        return zuu::string<char, N>(str);
    }
};

/**
 * @brief Helper untuk membuat string dengan kapasitas spesifik
 * @example auto s = string_literal<100>("Hello");
 */
template <std::size_t N>
inline constexpr string_literal_t<N> string_literal;

} // namespace zuu::literals

// Restore warnings
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// ============================================================================
// Deduction Guides
// ============================================================================

namespace zuu {

/**
 * @brief Deduction guide dari character array
 */
template <traits::character CharT, std::size_t N>
string(const CharT (&)[N]) -> string<CharT, N - 1>;

/**
 * @brief Deduction guide dari repeated character
 */
template <traits::character CharT>
string(std::size_t, CharT) -> string<CharT, 64>; // Default capacity

} // namespace zuu

// ============================================================================
// std::hash Specialization
// ============================================================================

template <zuu::traits::character CharT, std::size_t N>
struct std::hash<zuu::string<CharT, N>> {
    [[nodiscard]] constexpr std::size_t operator()(
        const zuu::string<CharT, N>& str) const noexcept {
        return str.hash();
    }
};

// ============================================================================
// std::formatter Specialization (C++20)
// ============================================================================

#if __cpp_lib_format >= 201907L
#include <format>

template <zuu::traits::character CharT, std::size_t N>
struct std::formatter<zuu::string<CharT, N>> : std::formatter<std::basic_string_view<CharT>> {
    template <typename FormatContext>
    auto format(const zuu::string<CharT, N>& str, FormatContext& ctx) const {
        return std::formatter<std::basic_string_view<CharT>>::format(
            std::basic_string_view<CharT>(str.data(), str.length()), ctx);
    }
};
#endif

// ============================================================================
// Stream Operators
// ============================================================================

namespace zuu {

/**
 * @brief Output stream operator
 */
template <traits::character CharT, std::size_t N>
inline std::basic_ostream<CharT>& operator<<(
    std::basic_ostream<CharT>& os, const string<CharT, N>& str) {
    return os << str.c_str();
}

/**
 * @brief Input stream operator
 */
template <traits::character CharT, std::size_t N>
inline std::basic_istream<CharT>& operator>>(
    std::basic_istream<CharT>& is, string<CharT, N>& str) {
    str.clear();
    CharT ch;
    
    // Skip whitespace
    while (is.get(ch) && std::isspace(ch));
    
    if (is) {
        is.putback(ch);
        // Read until whitespace or capacity reached
        while (is.get(ch) && !std::isspace(ch)) {
            if (!str.push_back(ch)) {
                is.putback(ch);
                break;
            }
        }
    }
    
    return is;
}

/**
 * @brief Get line from stream
 */
template <traits::character CharT, std::size_t N>
inline std::basic_istream<CharT>& getline(
    std::basic_istream<CharT>& is, string<CharT, N>& str, CharT delim = CharT('\n')) {
    str.clear();
    CharT ch;
    
    while (is.get(ch) && ch != delim) {
        if (!str.push_back(ch)) break;
    }
    
    return is;
}

} // namespace zuu
