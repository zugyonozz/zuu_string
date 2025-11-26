#pragma once

/**
 * @file fstring_algorithms.hpp
 * @author zugyonozz (rafizuhayr001@gmail.com)
 * @brief String algorithms and operations for fstring
 * @version 2.0.0
 * @date 2025-11-26
 * 
 * @copyright Copyright (c) 2025
 */

#include "fstring_core.hpp"

namespace zuu {
namespace algorithms {

// ==================== Case Conversion ====================

/**
 * @brief Convert string to lowercase
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto to_lower(const basic_fstring<CharT, Cap>& str) noexcept {
    basic_fstring<CharT, Cap> result;
    for (std::size_t i = 0; i < str.length(); ++i) {
        result.append(detail::to_lower(str[i]));
    }
    return result;
}

/**
 * @brief Convert string to uppercase
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto to_upper(const basic_fstring<CharT, Cap>& str) noexcept {
    basic_fstring<CharT, Cap> result;
    for (std::size_t i = 0; i < str.length(); ++i) {
        result.append(detail::to_upper(str[i]));
    }
    return result;
}

/**
 * @brief Convert string to title case (first letter of each word capitalized)
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto to_title(const basic_fstring<CharT, Cap>& str) noexcept {
    basic_fstring<CharT, Cap> result;
    bool new_word = true;
    
    for (std::size_t i = 0; i < str.length(); ++i) {
        CharT ch = str[i];
        if (detail::is_space(ch)) {
            new_word = true;
            result.append(ch);
        } else {
            result.append(new_word ? detail::to_upper(ch) : detail::to_lower(ch));
            new_word = false;
        }
    }
    return result;
}

/**
 * @brief Convert C-string to title case
 */
template <char_t CharT>
[[nodiscard]] constexpr auto to_title(const CharT* str) noexcept {
    constexpr std::size_t max_cap = 256;
    basic_fstring<CharT, max_cap> fstr(str);
    return to_title(fstr);
}

// ==================== Trimming ====================

/**
 * @brief Trim whitespace from left
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto trim_left(const basic_fstring<CharT, Cap>& str) noexcept {
    std::size_t start = 0;
    while (start < str.length() && detail::is_space(str[start])) {
        ++start;
    }
    return str.template substr<Cap>(start);
}

/**
 * @brief Trim whitespace from right
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto trim_right(const basic_fstring<CharT, Cap>& str) noexcept {
    std::size_t end = str.length();
    while (end > 0 && detail::is_space(str[end - 1])) {
        --end;
    }
    return str.template substr<Cap>(0, end);
}

/**
 * @brief Trim whitespace from both ends
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto trim(const basic_fstring<CharT, Cap>& str) noexcept {
    return trim_right(trim_left(str));
}

// ==================== Replace Operations ====================

/**
 * @brief Replace all occurrences of a character
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto replace(const basic_fstring<CharT, Cap>& str, CharT from, CharT to) noexcept {
    basic_fstring<CharT, Cap> result;
    for (std::size_t i = 0; i < str.length(); ++i) {
        result.append(str[i] == from ? to : str[i]);
    }
    return result;
}

/**
 * @brief Replace all occurrences of a substring
 */
template <char_t CharT, std::size_t Cap, std::size_t N1, std::size_t N2>
[[nodiscard]] constexpr auto replace(const basic_fstring<CharT, Cap>& str,
                                      const basic_fstring<CharT, N1>& from,
                                      const basic_fstring<CharT, N2>& to) noexcept {
    constexpr std::size_t result_cap = Cap * 2; // Estimate larger capacity
    basic_fstring<CharT, result_cap> result;
    
    std::size_t pos = 0;
    while (pos < str.length()) {
        std::size_t found = str.find(from.data(), pos);
        if (found == basic_fstring<CharT, Cap>::npos) {
            // Copy remaining string
            while (pos < str.length() && !result.full()) {
                result.append(str[pos++]);
            }
            break;
        }
        
        // Copy before match
        while (pos < found && !result.full()) {
            result.append(str[pos++]);
        }
        
        // Copy replacement
        for (std::size_t i = 0; i < to.length() && !result.full(); ++i) {
            result.append(to[i]);
        }
        
        pos = found + from.length();
    }
    
    return result;
}

// ==================== Split Operations ====================

/**
 * @brief Split string by delimiter (returns array of substrings)
 */
template <char_t CharT, std::size_t Cap, std::size_t MaxParts = 16>
[[nodiscard]] constexpr auto split(const basic_fstring<CharT, Cap>& str, CharT delimiter) noexcept {
    struct result_t {
        basic_fstring<CharT, Cap> parts[MaxParts];
        std::size_t count = 0;
    };
    
    result_t result;
    basic_fstring<CharT, Cap> current;
    
    for (std::size_t i = 0; i < str.length() && result.count < MaxParts; ++i) {
        if (str[i] == delimiter) {
            if (!current.empty()) {
                result.parts[result.count++] = current;
                current.clear();
            }
        } else {
            current.append(str[i]);
        }
    }
    
    if (!current.empty() && result.count < MaxParts) {
        result.parts[result.count++] = current;
    }
    
    return result;
}

// ==================== Reversal ====================

/**
 * @brief Reverse string
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto reverse(const basic_fstring<CharT, Cap>& str) noexcept {
    basic_fstring<CharT, Cap> result;
    for (std::size_t i = str.length(); i > 0; --i) {
        result.append(str[i - 1]);
    }
    return result;
}

// ==================== Padding ====================

/**
 * @brief Pad string on left
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto pad_left(const basic_fstring<CharT, Cap>& str, 
                                       std::size_t width, CharT fill = CharT(' ')) noexcept {
    basic_fstring<CharT, Cap> result;
    if (str.length() < width) {
        result.append(width - str.length(), fill);
    }
    result += str;
    return result;
}

/**
 * @brief Pad string on right
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto pad_right(const basic_fstring<CharT, Cap>& str,
                                        std::size_t width, CharT fill = CharT(' ')) noexcept {
    basic_fstring<CharT, Cap> result = str;
    if (str.length() < width) {
        result.append(width - str.length(), fill);
    }
    return result;
}

/**
 * @brief Center string with padding
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto center(const basic_fstring<CharT, Cap>& str,
                                     std::size_t width, CharT fill = CharT(' ')) noexcept {
    basic_fstring<CharT, Cap> result;
    if (str.length() >= width) {
        return str;
    }
    
    std::size_t total_pad = width - str.length();
    std::size_t left_pad = total_pad / 2;
    std::size_t right_pad = total_pad - left_pad;
    
    result.append(left_pad, fill);
    result += str;
    result.append(right_pad, fill);
    
    return result;
}

// ==================== Character Statistics ====================

/**
 * @brief Count occurrences of character
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr std::size_t count(const basic_fstring<CharT, Cap>& str, CharT ch) noexcept {
    std::size_t cnt = 0;
    for (std::size_t i = 0; i < str.length(); ++i) {
        if (str[i] == ch) ++cnt;
    }
    return cnt;
}

/**
 * @brief Check if all characters are alphabetic
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr bool is_alpha(const basic_fstring<CharT, Cap>& str) noexcept {
    if (str.empty()) return false;
    for (std::size_t i = 0; i < str.length(); ++i) {
        if (!detail::is_alpha(str[i])) return false;
    }
    return true;
}

/**
 * @brief Check if all characters are digits
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr bool is_digit(const basic_fstring<CharT, Cap>& str) noexcept {
    if (str.empty()) return false;
    for (std::size_t i = 0; i < str.length(); ++i) {
        if (!detail::is_digit(str[i])) return false;
    }
    return true;
}

/**
 * @brief Check if all characters are alphanumeric
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr bool is_alnum(const basic_fstring<CharT, Cap>& str) noexcept {
    if (str.empty()) return false;
    for (std::size_t i = 0; i < str.length(); ++i) {
        if (!detail::is_alpha(str[i]) && !detail::is_digit(str[i])) return false;
    }
    return true;
}

// ==================== Join Operations ====================

/**
 * @brief Join array of strings with delimiter
 */
template <char_t CharT, std::size_t Cap, std::size_t N>
[[nodiscard]] constexpr auto join(const basic_fstring<CharT, Cap> (&parts)[N], 
                                   CharT delimiter) noexcept {
    constexpr std::size_t result_cap = Cap * N + N;
    basic_fstring<CharT, result_cap> result;
    
    for (std::size_t i = 0; i < N; ++i) {
        if (i > 0) result.append(delimiter);
        result.append(parts[i].data(), parts[i].length());
    }
    
    return result;
}

/**
 * @brief Join array of strings with delimiter string
 */
template <char_t CharT, std::size_t Cap, std::size_t N, std::size_t DelimCap>
[[nodiscard]] constexpr auto join(const basic_fstring<CharT, Cap> (&parts)[N],
                                   const basic_fstring<CharT, DelimCap>& delimiter) noexcept {
    constexpr std::size_t result_cap = Cap * N + DelimCap * N;
    basic_fstring<CharT, result_cap> result;
    
    for (std::size_t i = 0; i < N; ++i) {
        if (i > 0) result.append(delimiter.data(), delimiter.length());
        result.append(parts[i].data(), parts[i].length());
    }
    
    return result;
}

// ==================== Repeat ====================

/**
 * @brief Repeat string n times
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto repeat(const basic_fstring<CharT, Cap>& str, std::size_t times) noexcept {
    constexpr std::size_t result_cap = Cap * 16; // Reasonable limit
    basic_fstring<CharT, result_cap> result;
    
    for (std::size_t i = 0; i < times && !result.full(); ++i) {
        result.append(str.data(), str.length());
    }
    
    return result;
}

// ==================== Remove ====================

/**
 * @brief Remove all occurrences of character
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto remove(const basic_fstring<CharT, Cap>& str, CharT ch) noexcept {
    basic_fstring<CharT, Cap> result;
    for (std::size_t i = 0; i < str.length(); ++i) {
        if (str[i] != ch) {
            result.append(str[i]);
        }
    }
    return result;
}

/**
 * @brief Remove all whitespace
 */
template <char_t CharT, std::size_t Cap>
[[nodiscard]] constexpr auto remove_whitespace(const basic_fstring<CharT, Cap>& str) noexcept {
    basic_fstring<CharT, Cap> result;
    for (std::size_t i = 0; i < str.length(); ++i) {
        if (!detail::is_space(str[i])) {
            result.append(str[i]);
        }
    }
    return result;
}

// ==================== Comparison (case-insensitive) ====================

/**
 * @brief Case-insensitive comparison
 */
template <char_t CharT, std::size_t Cap1, std::size_t Cap2>
[[nodiscard]] constexpr bool equals_ignore_case(const basic_fstring<CharT, Cap1>& str1,
                                                  const basic_fstring<CharT, Cap2>& str2) noexcept {
    if (str1.length() != str2.length()) return false;
    
    for (std::size_t i = 0; i < str1.length(); ++i) {
        if (detail::to_lower(str1[i]) != detail::to_lower(str2[i])) {
            return false;
        }
    }
    return true;
}

} // namespace algorithms
} // namespace zuu
