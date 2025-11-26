#pragma once

/**
 * @file fstring_core.hpp
 * @author zugyonozz (rafizuhayr001@gmail.com)
 * @brief Core implementation of basic_fstring class
 * @version 2.0.0
 * @date 2025-11-26
 * 
 * @copyright Copyright (c) 2025
 * 
 * basic_fstring is a fixed-capacity string class that:
 * - Has compile-time fixed capacity (no heap allocation)
 * - Supports all standard character types (char, wchar_t, char8_t, etc.)
 * - Provides constexpr operations
 * - Is trivially copyable for better performance
 * - Compatible with standard string and string_view
 */

#include "fstring_utils.hpp"
#include "fstring_config.hpp"
#include <stdexcept>
#include <iostream>

namespace zuu {

/**
 * @brief Fixed-capacity string class
 * 
 * @tparam CharT Character type (char, wchar_t, char8_t, char16_t, char32_t)
 * @tparam Cap Maximum capacity (excluding null terminator)
 */
template <char_t CharT, std::size_t Cap>
class basic_fstring {
public:
    // Standard container type aliases
    using value_type = CharT;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = CharT*;
    using const_pointer = const CharT*;
    using reference = CharT&;
    using const_reference = const CharT&;
    using iterator = CharT*;
    using const_iterator = const CharT*;
    
    /// Maximum capacity (excluding null terminator)
    static constexpr size_type capacity = Cap;
    
    /// Special value for "not found"
    static constexpr size_type npos = static_cast<size_type>(-1);

private:
    template <std::size_t N>
    using Other = basic_fstring<value_type, N>;
    
    alignas(CharT) value_type data_[Cap + 1]{}; ///< Internal buffer (null-terminated)
    size_type length_{};                          ///< Current string length

    /// Store string data with length checking
    constexpr void store(const_pointer str, size_type len) noexcept {
        if (str == nullptr) return;
        length_ = detail::min{}(capacity, len);
        detail::copy{}(data_, str, length_);
        data_[length_] = value_type{};
    }

    /// Shift characters right (for insertion)
    constexpr void shift_right(size_type index, size_type count) noexcept {
        if (count == 0 || index > length_) return;
        size_type move_count = detail::min{}(length_ - index, capacity - index - count);
        detail::move{}(data_ + index + count, data_ + index, move_count);
    }

    /// Shift characters left (for erasure)
    constexpr void shift_left(size_type index, size_type count) noexcept {
        if (count == 0 || index >= length_) return;
        size_type move_count = length_ - index - count;
        detail::move{}(data_ + index, data_ + index + count, move_count);
    }

public:
    // ==================== Constructors & Assignment ====================
    
    /// Default constructor
    constexpr basic_fstring() noexcept = default;
    
    /// Copy constructor
    constexpr basic_fstring(const basic_fstring&) noexcept = default;
    
    /// Move constructor
    constexpr basic_fstring(basic_fstring&&) noexcept = default;
    
    /// Copy assignment
    constexpr basic_fstring& operator=(const basic_fstring&) noexcept = default;
    
    /// Move assignment
    constexpr basic_fstring& operator=(basic_fstring&&) noexcept = default;
    
    /// Destructor
    ~basic_fstring() = default;

    /**
     * @brief Construct from string literal
     * @param str String literal
     */
    template <size_type N>
    constexpr basic_fstring(const value_type (&str)[N]) noexcept {
        store(str, detail::min{}(Cap, N - 1));
    }

    /**
     * @brief Construct from C-string
     * @param str Null-terminated C-string
     */
    constexpr explicit basic_fstring(const_pointer str) noexcept {
        if (str) store(str, detail::length(str));
    }

    /**
     * @brief Construct from C-string with length
     * @param str C-string
     * @param len Length of string
     */
    constexpr basic_fstring(const_pointer str, size_type len) noexcept {
        if (str) store(str, len);
    }

    /**
     * @brief Construct by filling with character
     * @param count Number of characters
     * @param ch Character to fill
     */
    constexpr basic_fstring(size_type count, value_type ch) noexcept {
        length_ = detail::min{}(capacity, count);
        detail::fill{}(data_, ch, length_);
        data_[length_] = value_type{};
    }

    /**
     * @brief Construct from string_view
     * @param sv String view
     */
    constexpr explicit basic_fstring(std::basic_string_view<CharT> sv) noexcept {
        store(sv.data(), sv.length());
    }

    /**
     * @brief Construct from std::basic_string
     * @param str Standard string
     */
    constexpr explicit basic_fstring(const std::basic_string<CharT>& str) noexcept {
        store(str.data(), str.length());
    }

    // ==================== Assignment Operators ====================

    /**
     * @brief Assign from C-string
     */
    constexpr basic_fstring& operator=(const_pointer str) noexcept {
        if (str) {
            store(str, detail::length(str));
        } else {
            clear();
        }
        return *this;
    }

    /**
     * @brief Assign from character
     */
    constexpr basic_fstring& operator=(value_type ch) noexcept {
        length_ = 1;
        data_[0] = ch;
        data_[1] = value_type{};
        return *this;
    }

    // ==================== Element Access ====================

    /**
     * @brief Access character with bounds checking
     * @throws std::out_of_range if index >= length()
     */
    [[nodiscard]] constexpr const_reference at(size_type index) const {
        if constexpr (config::enable_bounds_check) {
            if (index >= length_) {
                throw std::out_of_range("fstring: index out of range");
            }
        }
        return data_[index];
    }

    /**
     * @brief Access character with bounds checking
     * @throws std::out_of_range if index >= length()
     */
    [[nodiscard]] constexpr reference at(size_type index) {
        if constexpr (config::enable_bounds_check) {
            if (index >= length_) {
                throw std::out_of_range("fstring: index out of range");
            }
        }
        return data_[index];
    }

    /**
     * @brief Access character without bounds checking
     */
    [[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept {
        return data_[index];
    }

    /**
     * @brief Access character without bounds checking
     */
    [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
        return data_[index];
    }

    /**
     * @brief Access first character
     */
    [[nodiscard]] constexpr reference front() noexcept {
        return data_[0];
    }

    /**
     * @brief Access first character
     */
    [[nodiscard]] constexpr const_reference front() const noexcept {
        return data_[0];
    }

    /**
     * @brief Access last character
     */
    [[nodiscard]] constexpr reference back() noexcept {
        return data_[length_ - 1];
    }

    /**
     * @brief Access last character
     */
    [[nodiscard]] constexpr const_reference back() const noexcept {
        return data_[length_ - 1];
    }

    /**
     * @brief Get pointer to underlying data
     */
    [[nodiscard]] constexpr const_pointer data() const noexcept {
        return data_;
    }

    /**
     * @brief Get pointer to underlying data
     */
    [[nodiscard]] constexpr pointer data() noexcept {
        return data_;
    }

    /**
     * @brief Get C-style null-terminated string
     */
    [[nodiscard]] constexpr const_pointer c_str() const noexcept {
        return data_;
    }

    // ==================== Iterators ====================

    [[nodiscard]] constexpr iterator begin() noexcept { return data_; }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return data_; }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return data_; }
    
    [[nodiscard]] constexpr iterator end() noexcept { return data_ + length_; }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return data_ + length_; }
    [[nodiscard]] constexpr const_iterator cend() const noexcept { return data_ + length_; }

    // ==================== Capacity ====================

    /**
     * @brief Check if string is empty
     */
    [[nodiscard]] constexpr bool empty() const noexcept {
        return length_ == 0;
    }

    /**
     * @brief Get current string length
     */
    [[nodiscard]] constexpr size_type length() const noexcept {
        return length_;
    }

    /**
     * @brief Get current string size (same as length)
     */
    [[nodiscard]] constexpr size_type size() const noexcept {
        return length_;
    }

    /**
     * @brief Get maximum capacity
     */
    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return capacity;
    }

    /**
     * @brief Get available space
     */
    [[nodiscard]] constexpr size_type available() const noexcept {
        return capacity - length_;
    }

    /**
     * @brief Check if string is at full capacity
     */
    [[nodiscard]] constexpr bool full() const noexcept {
        return length_ == capacity;
    }

    // ==================== Modifiers ====================

    /**
     * @brief Clear the string
     */
    constexpr void clear() noexcept {
        length_ = 0;
        data_[0] = value_type{};
    }

    /**
     * @brief Append C-string
     */
    constexpr basic_fstring& append(const_pointer str) noexcept {
        if (str && !full()) {
            size_type len = detail::length(str);
            size_type count = detail::min{}(available(), len);
            detail::copy{}(data_ + length_, str, count);
            length_ += count;
            data_[length_] = value_type{};
        }
        return *this;
    }

    /**
     * @brief Append C-string with length
     */
    constexpr basic_fstring& append(const_pointer str, size_type len) noexcept {
        if (str && !full()) {
            size_type count = detail::min{}(available(), len);
            detail::copy{}(data_ + length_, str, count);
            length_ += count;
            data_[length_] = value_type{};
        }
        return *this;
    }

    /**
     * @brief Append single character
     */
    constexpr basic_fstring& append(value_type ch) noexcept {
        if (!full()) {
            data_[length_++] = ch;
            data_[length_] = value_type{};
        }
        return *this;
    }

    /**
     * @brief Append multiple characters
     */
    constexpr basic_fstring& append(size_type count, value_type ch) noexcept {
        count = detail::min{}(count, available());
        detail::fill{}(data_ + length_, ch, count);
        length_ += count;
        data_[length_] = value_type{};
        return *this;
    }

    /**
     * @brief Push character to back
     */
    constexpr void push_back(value_type ch) noexcept {
        append(ch);
    }

    /**
     * @brief Pop character from back
     */
    constexpr void pop_back() noexcept {
        if (length_ > 0) {
            data_[--length_] = value_type{};
        }
    }

    /**
     * @brief Insert C-string at position
     */
    constexpr basic_fstring& insert(size_type index, const_pointer str) noexcept {
        if (str && !full() && index <= length_) {
            size_type len = detail::length(str);
            size_type count = detail::min{}(available(), len);
            if (count > 0) {
                shift_right(index, count);
                detail::copy{}(data_ + index, str, count);
                length_ += count;
                data_[length_] = value_type{};
            }
        }
        return *this;
    }

    /**
     * @brief Insert at iterator position
     */
    constexpr basic_fstring& insert(const_iterator pos, const_pointer str) noexcept {
        if (pos >= begin() && pos <= end()) {
            return insert(pos - data_, str);
        }
        return *this;
    }

    /**
     * @brief Insert character at position
     */
    constexpr basic_fstring& insert(size_type index, value_type ch) noexcept {
        if (!full() && index <= length_) {
            shift_right(index, 1);
            data_[index] = ch;
            ++length_;
            data_[length_] = value_type{};
        }
        return *this;
    }

    /**
     * @brief Erase characters
     */
    constexpr basic_fstring& erase(size_type index = 0, size_type count = npos) noexcept {
        if (index < length_) {
            if (count == npos || index + count >= length_) {
                length_ = index;
            } else {
                shift_left(index, count);
                length_ -= count;
            }
            data_[length_] = value_type{};
        }
        return *this;
    }

    /**
     * @brief Erase at iterator position
     */
    constexpr iterator erase(const_iterator pos) noexcept {
        if (pos >= begin() && pos < end()) {
            size_type index = pos - data_;
            erase(index, 1);
            return data_ + index;
        }
        return end();
    }

    /**
     * @brief Erase range
     */
    constexpr iterator erase(const_iterator first, const_iterator last) noexcept {
        if (first >= begin() && first < end() && last >= first && last <= end()) {
            size_type index = first - data_;
            size_type count = last - first;
            erase(index, count);
            return data_ + index;
        }
        return end();
    }

    /**
     * @brief Resize string
     */
    constexpr void resize(size_type new_size, value_type ch = value_type{}) noexcept {
        new_size = detail::min{}(new_size, capacity);
        if (new_size > length_) {
            detail::fill{}(data_ + length_, ch, new_size - length_);
        }
        length_ = new_size;
        data_[length_] = value_type{};
    }

    /**
     * @brief Swap with another fstring
     */
    constexpr void swap(basic_fstring& other) noexcept {
        basic_fstring temp = *this;
        *this = other;
        other = temp;
    }

    // ==================== Concatenation ====================

    /**
     * @brief Concatenate operator +=
     */
    template <std::size_t N>
    constexpr basic_fstring& operator+=(const Other<N>& other) noexcept {
        return append(other.data(), other.length());
    }

    constexpr basic_fstring& operator+=(const_pointer str) noexcept {
        return append(str);
    }

    constexpr basic_fstring& operator+=(value_type ch) noexcept {
        return append(ch);
    }

    /**
     * @brief Concatenate operator +
     */
    template <std::size_t N>
    [[nodiscard]] constexpr auto operator+(const Other<N>& other) const noexcept {
        basic_fstring<CharT, Cap + N> result(data_, length_);
        result.append(other.data(), other.length());
        return result;
    }

    template <size_type N>
    [[nodiscard]] constexpr auto operator+(const value_type (&str)[N]) const noexcept {
        basic_fstring<CharT, Cap + N - 1> result(data_, length_);
        result.append(str, N - 1);
        return result;
    }

    // ==================== Comparison ====================

    [[nodiscard]] constexpr auto operator<=>(const basic_fstring& other) const noexcept = default;
    
    template <std::size_t N>
    [[nodiscard]] constexpr auto operator==(const Other<N>& other) const noexcept {
        int cmp = detail::compare{}(data_, other.data_, detail::min{}(length_, other.length_));
        if (cmp != 0) return cmp == 0;
        return length_ == other.length_;
    }

    [[nodiscard]] constexpr bool operator==(const_pointer str) const noexcept {
        if (str == nullptr) return empty();
        size_type len = detail::length(str);
        if (len != length_) return false;
        return detail::compare{}(data_, str, length_) == 0;
    }

    // ==================== String Operations ====================

    /**
     * @brief Get substring
     */
    template <std::size_t N = Cap>
    [[nodiscard]] constexpr auto substr(size_type pos = 0, size_type count = npos) const noexcept {
        if (pos >= length_) return basic_fstring<CharT, N>{};
        if (count == npos || pos + count > length_) {
            count = length_ - pos;
        }
        return basic_fstring<CharT, N>(data_ + pos, count);
    }

    /**
     * @brief Find substring
     */
    [[nodiscard]] constexpr size_type find(const_pointer str, size_type pos = 0) const noexcept {
        if (str == nullptr || pos >= length_) return npos;
        size_type len = detail::length(str);
        auto result = detail::find_str(data_ + pos, length_ - pos, str, len);
        return result ? static_cast<size_type>(result - data_) : npos;
    }

    /**
     * @brief Find character
     */
    [[nodiscard]] constexpr size_type find(value_type ch, size_type pos = 0) const noexcept {
        if (pos >= length_) return npos;
        auto result = detail::find(data_ + pos, length_ - pos, ch);
        return result ? static_cast<size_type>(result - data_) : npos;
    }

    /**
     * @brief Reverse find character
     */
    [[nodiscard]] constexpr size_type rfind(value_type ch, size_type pos = npos) const noexcept {
        if (empty()) return npos;
        size_type search_len = (pos == npos || pos >= length_) ? length_ : pos + 1;
        auto result = detail::rfind(data_, search_len, ch);
        return result ? static_cast<size_type>(result - data_) : npos;
    }

    /**
     * @brief Check if string starts with prefix
     */
    [[nodiscard]] constexpr bool starts_with(const_pointer prefix) const noexcept {
        if (prefix == nullptr) return true;
        size_type len = detail::length(prefix);
        if (len > length_) return false;
        return detail::compare{}(data_, prefix, len) == 0;
    }

    [[nodiscard]] constexpr bool starts_with(value_type ch) const noexcept {
        return !empty() && data_[0] == ch;
    }

    /**
     * @brief Check if string ends with suffix
     */
    [[nodiscard]] constexpr bool ends_with(const_pointer suffix) const noexcept {
        if (suffix == nullptr) return true;
        size_type len = detail::length(suffix);
        if (len > length_) return false;
        return detail::compare{}(data_ + length_ - len, suffix, len) == 0;
    }

    [[nodiscard]] constexpr bool ends_with(value_type ch) const noexcept {
        return !empty() && data_[length_ - 1] == ch;
    }

    /**
     * @brief Check if string contains substring
     */
    [[nodiscard]] constexpr bool contains(const_pointer str) const noexcept {
        return find(str) != npos;
    }

    [[nodiscard]] constexpr bool contains(value_type ch) const noexcept {
        return find(ch) != npos;
    }

    /**
     * @brief Convert to string_view
     */
    [[nodiscard]] constexpr operator std::basic_string_view<CharT>() const noexcept {
        return std::basic_string_view<CharT>(data_, length_);
    }

    /**
     * @brief Convert to std::basic_string
     */
    [[nodiscard]] constexpr std::basic_string<CharT> to_string() const {
        return std::basic_string<CharT>(data_, length_);
    }
};

// ==================== Deduction Guides ====================

template <char_t CharT, std::size_t N>
basic_fstring(const CharT (&)[N]) -> basic_fstring<CharT, N - 1>;

// ==================== Type Aliases ====================

template <std::size_t Cap> using fstring = basic_fstring<char, Cap>;
template <std::size_t Cap> using wfstring = basic_fstring<wchar_t, Cap>;
template <std::size_t Cap> using u8fstring = basic_fstring<char8_t, Cap>;
template <std::size_t Cap> using u16fstring = basic_fstring<char16_t, Cap>;
template <std::size_t Cap> using u32fstring = basic_fstring<char32_t, Cap>;

// ==================== Stream Operators ====================

template <char_t CharT, std::size_t Cap>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const basic_fstring<CharT, Cap>& str) {
    return os << str.data();
}

template <char_t CharT, std::size_t Cap>
std::basic_istream<CharT>& operator>>(std::basic_istream<CharT>& is, basic_fstring<CharT, Cap>& str) {
    str.clear();
    CharT ch;
    while (is.get(ch) && !detail::is_space(ch) && !str.full()) {
        str.append(ch);
    }
    return is;
}

} // namespace zuu
