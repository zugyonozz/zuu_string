#pragma once

/**
 * @file fstring_traits.hpp
 * @author zugyonozz (rafizuhayr001@gmail.com)
 * @brief Type traits and concepts for fstring library
 * @version 2.0.0
 * @date 2025-11-26
 * 
 * @copyright Copyright (c) 2025
 */

#include <type_traits>
#include <string>
#include <string_view>

namespace zuu {

namespace traits {

    /// Character type detection
    template <typename T> 
    struct is_char : std::false_type {};
    
    template <> struct is_char<char> : std::true_type {};
    template <> struct is_char<wchar_t> : std::true_type {};
    template <> struct is_char<char8_t> : std::true_type {};
    template <> struct is_char<char16_t> : std::true_type {};
    template <> struct is_char<char32_t> : std::true_type {};
    
    template <typename T> 
    constexpr bool is_char_v = is_char<T>::value;

    /// Raw string pointer detection
    template <typename T> 
    struct is_raw_string : std::false_type {};
    
    template <> struct is_raw_string<char*> : std::true_type {};
    template <> struct is_raw_string<const char*> : std::true_type {};
    template <> struct is_raw_string<wchar_t*> : std::true_type {};
    template <> struct is_raw_string<const wchar_t*> : std::true_type {};
    template <> struct is_raw_string<char8_t*> : std::true_type {};
    template <> struct is_raw_string<const char8_t*> : std::true_type {};
    template <> struct is_raw_string<char16_t*> : std::true_type {};
    template <> struct is_raw_string<const char16_t*> : std::true_type {};
    template <> struct is_raw_string<char32_t*> : std::true_type {};
    template <> struct is_raw_string<const char32_t*> : std::true_type {};
    
    template <typename T> 
    constexpr bool is_raw_string_v = is_raw_string<std::remove_cvref_t<T>>::value;

    /// Standard string detection
    template <typename T> 
    struct is_standard_string : std::false_type {};
    
    template <> struct is_standard_string<std::string> : std::true_type {};
    template <> struct is_standard_string<std::wstring> : std::true_type {};
    template <> struct is_standard_string<std::u8string> : std::true_type {};
    template <> struct is_standard_string<std::u16string> : std::true_type {};
    template <> struct is_standard_string<std::u32string> : std::true_type {};
    
    template <> struct is_standard_string<std::string_view> : std::true_type {};
    template <> struct is_standard_string<std::wstring_view> : std::true_type {};
    template <> struct is_standard_string<std::u8string_view> : std::true_type {};
    template <> struct is_standard_string<std::u16string_view> : std::true_type {};
    template <> struct is_standard_string<std::u32string_view> : std::true_type {};
    
    template <typename T> 
    constexpr bool is_standard_string_v = is_standard_string<std::decay_t<T>>::value;

}

/// Concepts for type constraints
template <typename T> 
concept char_t = traits::is_char_v<T>;

// Forward declarations
template <char_t CharT = char, std::size_t Cap = 0> class basic_fstring;

namespace traits {

    /// fstring detection
    template <typename T> 
    struct is_fstring : std::false_type {};
    
    template <typename CharT, std::size_t Cap> 
    struct is_fstring<basic_fstring<CharT, Cap>> : std::true_type {};
    
    template <typename T> 
    constexpr bool is_fstring_v = is_fstring<std::decay_t<T>>::value;

    /// Generic string detection
    template <typename T> 
    constexpr bool is_string_v = is_raw_string_v<T> || is_standard_string_v<T> || is_fstring_v<T>;

    /// Extract character type from string types
    template <typename T> 
    struct char_type {
        using type = void;
    };
    
    template <typename CharT, std::size_t Cap>
    struct char_type<basic_fstring<CharT, Cap>> {
        using type = CharT;
    };
    
    template <> struct char_type<std::string> { using type = char; };
    template <> struct char_type<std::wstring> { using type = wchar_t; };
    template <> struct char_type<std::u8string> { using type = char8_t; };
    template <> struct char_type<std::u16string> { using type = char16_t; };
    template <> struct char_type<std::u32string> { using type = char32_t; };
    
    template <> struct char_type<std::string_view> { using type = char; };
    template <> struct char_type<std::wstring_view> { using type = wchar_t; };
    template <> struct char_type<std::u8string_view> { using type = char8_t; };
    template <> struct char_type<std::u16string_view> { using type = char16_t; };
    template <> struct char_type<std::u32string_view> { using type = char32_t; };
    
    template <typename T>
    using char_type_t = typename char_type<std::decay_t<T>>::type;

    /// Check if two string types have compatible character types
    template <typename T1, typename T2>
    constexpr bool is_compatible_string_v = 
        is_string_v<T1> && is_string_v<T2> && 
        std::is_same_v<char_type_t<T1>, char_type_t<T2>>;

} // namespace traits


/// Concepts for type constraints
template <typename T> 
concept string_t = traits::is_string_v<T>;

template <typename T> 
concept fstring_t = traits::is_fstring_v<T>;

template <typename T1, typename T2>
concept compatible_strings = traits::is_compatible_string_v<T1, T2>;

} // namespace zuu
