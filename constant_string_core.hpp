/**
 * @file constant_string_core.hpp
 * @brief Core string class implementation dengan optimasi penuh
 * @version 1.0
 */

#pragma once

#include "constant_string_iterator.hpp"
#include "constant_string_detail.hpp"
#include <algorithm>
#include <compare>
#include <stdexcept>

namespace zuu {

/**
 * @brief Fixed-size compile-time string class dengan optimasi tinggi
 * 
 * @tparam CharT Character type (char, wchar_t, char8_t, char16_t, char32_t)
 * @tparam N Maximum capacity (excluding null terminator)
 * 
 * Features:
 * - Full constexpr support (C++20)
 * - Zero-overhead abstractions
 * - STL-compatible interface
 * - Optimized algorithms
 * - No dynamic memory allocation
 * 
 * @example
 * ```cpp
 * constexpr zuu::cstring<32> str("Hello");
 * static_assert(str.length() == 5);
 * static_assert(str.capacity() == 32);
 * ```
 */
template <traits::character CharT = char, std::size_t N = 0>
class string {
public:
    // ========================================================================
    // Type Definitions
    // ========================================================================
    
    using value_type             = CharT;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using reference              = CharT&;
    using const_reference        = const CharT&;
    using pointer                = CharT*;
    using const_pointer          = const CharT*;
    using iterator               = string_iterator<CharT, false>;
    using const_iterator         = string_iterator<CharT, true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /// Sentinel value untuk "not found"
    static constexpr size_type npos = static_cast<size_type>(-1);

private:
    // ========================================================================
    // Internal Storage
    // ========================================================================
    
    alignas(CharT) CharT data_[N + 1]{}; // +1 untuk null terminator
    size_type len_ = 0;

    // ========================================================================
    // Internal Helpers
    // ========================================================================
    
    /**
     * @brief Internal assignment dengan bounds checking
     */
    constexpr void assign_impl(const CharT* str, size_type count) noexcept {
        len_ = (count < N) ? count : N;
        detail::memcpy_string(data_, str, len_);
        data_[len_] = CharT{};
    }

public:
    // ========================================================================
    // Constructors & Destructor
    // ========================================================================
    
    /**
     * @brief Default constructor - empty string
     */
    constexpr string() noexcept = default;
    
    /**
     * @brief Copy constructor
     */
    constexpr string(const string&) noexcept = default;
    
    /**
     * @brief Move constructor
     */
    constexpr string(string&&) noexcept = default;
    
    /**
     * @brief Destructor
     */
    ~string() = default;

    /**
     * @brief Construct from C-string
     * @param str Null-terminated string
     */
    constexpr string(const CharT* str) noexcept {
        if (str) {
            assign_impl(str, detail::strlen(str));
        }
    }

    /**
     * @brief Construct from character array
     * @param str Character array with known size
     */
    template <size_type M>
    constexpr string(const CharT (&str)[M]) noexcept {
        assign_impl(str, M > 0 ? M - 1 : 0);
    }

    /**
     * @brief Construct with repeated character
     * @param count Number of repetitions
     * @param ch Character to repeat
     */
    constexpr string(size_type count, CharT ch) noexcept {
        len_ = (count < N) ? count : N;
        detail::memset_string(data_, ch, len_);
        data_[len_] = CharT{};
    }

    /**
     * @brief Construct from string with different capacity
     * @param other Source string
     */
    template <size_type M>
    constexpr string(const string<CharT, M>& other) noexcept {
        assign_impl(other.data(), other.length());
    }

    /**
     * @brief Construct from substring
     * @param str Source string
     * @param pos Starting position
     * @param count Number of characters (npos = all)
     */
    constexpr string(const CharT* str, size_type pos, size_type count = npos) noexcept {
        if (str) {
            size_type str_len = detail::strlen(str);
            if (pos < str_len) {
                size_type actual = (count == npos) ? (str_len - pos) : count;
                assign_impl(str + pos, actual);
            }
        }
    }

    // ========================================================================
    // Assignment Operators
    // ========================================================================
    
    /**
     * @brief Copy assignment
     */
    constexpr string& operator=(const string&) noexcept = default;
    
    /**
     * @brief Move assignment
     */
    constexpr string& operator=(string&&) noexcept = default;

    /**
     * @brief Assign from C-string
     */
    constexpr string& operator=(const CharT* str) noexcept {
        if (str) {
            assign_impl(str, detail::strlen(str));
        } else {
            clear();
        }
        return *this;
    }

    /**
     * @brief Assign from string with different capacity
     */
    template <size_type M>
    constexpr string& operator=(const string<CharT, M>& other) noexcept {
        assign_impl(other.data(), other.length());
        return *this;
    }

    /**
     * @brief Assign single character
     */
    constexpr string& operator=(CharT ch) noexcept {
        len_ = 1;
        data_[0] = ch;
        data_[1] = CharT{};
        return *this;
    }

    // ========================================================================
    // Element Access
    // ========================================================================

    /**
     * @brief Access element with bounds checking
     * @throws std::out_of_range if pos >= length()
     */
    [[nodiscard]] constexpr reference at(size_type pos) {
        if (pos >= len_) {
            throw std::out_of_range("zuu::string::at: index out of range");
        }
        return data_[pos];
    }

    [[nodiscard]] constexpr const_reference at(size_type pos) const {
        if (pos >= len_) {
            throw std::out_of_range("zuu::string::at: index out of range");
        }
        return data_[pos];
    }

    /**
     * @brief Access element without bounds checking
     */
    [[nodiscard]] constexpr reference operator[](size_type pos) noexcept { 
        return data_[pos]; 
    }
    
    [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept { 
        return data_[pos]; 
    }

    /**
     * @brief Access first character
     */
    [[nodiscard]] constexpr reference front() noexcept { 
        return data_[0]; 
    }
    
    [[nodiscard]] constexpr const_reference front() const noexcept { 
        return data_[0]; 
    }

    /**
     * @brief Access last character
     */
    [[nodiscard]] constexpr reference back() noexcept { 
        return data_[len_ > 0 ? len_ - 1 : 0]; 
    }
    
    [[nodiscard]] constexpr const_reference back() const noexcept { 
        return data_[len_ > 0 ? len_ - 1 : 0]; 
    }

    /**
     * @brief Get pointer to internal buffer
     */
    [[nodiscard]] constexpr pointer data() noexcept { 
        return data_; 
    }
    
    [[nodiscard]] constexpr const_pointer data() const noexcept { 
        return data_; 
    }

    /**
     * @brief Get null-terminated C-string
     */
    [[nodiscard]] constexpr const_pointer c_str() const noexcept { 
        return data_; 
    }

    // ========================================================================
    // Iterators
    // ========================================================================

    [[nodiscard]] constexpr iterator begin() noexcept { 
        return iterator(data_); 
    }
    
    [[nodiscard]] constexpr const_iterator begin() const noexcept { 
        return const_iterator(data_); 
    }
    
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { 
        return const_iterator(data_); 
    }

    [[nodiscard]] constexpr iterator end() noexcept { 
        return iterator(data_ + len_); 
    }
    
    [[nodiscard]] constexpr const_iterator end() const noexcept { 
        return const_iterator(data_ + len_); 
    }
    
    [[nodiscard]] constexpr const_iterator cend() const noexcept { 
        return const_iterator(data_ + len_); 
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { 
        return reverse_iterator(end()); 
    }
    
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { 
        return const_reverse_iterator(end()); 
    }
    
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { 
        return const_reverse_iterator(end()); 
    }

    [[nodiscard]] constexpr reverse_iterator rend() noexcept { 
        return reverse_iterator(begin()); 
    }
    
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { 
        return const_reverse_iterator(begin()); 
    }
    
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { 
        return const_reverse_iterator(begin()); 
    }

    // ========================================================================
    // Capacity
    // ========================================================================

    /**
     * @brief Check if string is empty
     */
    [[nodiscard]] constexpr bool empty() const noexcept { 
        return len_ == 0; 
    }

    /**
     * @brief Get string length
     */
    [[nodiscard]] constexpr size_type length() const noexcept { 
        return len_; 
    }

    /**
     * @brief Get string size (same as length)
     */
    [[nodiscard]] constexpr size_type size() const noexcept { 
        return len_; 
    }

    /**
     * @brief Get maximum capacity
     */
    [[nodiscard]] constexpr size_type capacity() const noexcept { 
        return N; 
    }

    /**
     * @brief Get maximum size (same as capacity)
     */
    [[nodiscard]] constexpr size_type max_size() const noexcept { 
        return N; 
    }

    /**
     * @brief Get available space
     */
    [[nodiscard]] constexpr size_type available() const noexcept {
        return N - len_;
    }

    // ========================================================================
    // Modifiers
    // ========================================================================

    /**
     * @brief Clear string content
     */
    constexpr void clear() noexcept {
        len_ = 0;
        data_[0] = CharT{};
    }

    /**
     * @brief Add character to end
     * @return true if successful, false if capacity reached
     */
    constexpr bool push_back(CharT ch) noexcept {
        if (len_ >= N) return false;
        data_[len_++] = ch;
        data_[len_] = CharT{};
        return true;
    }

    /**
     * @brief Remove last character
     */
    constexpr void pop_back() noexcept {
        if (len_ > 0) {
            data_[--len_] = CharT{};
        }
    }

    /**
     * @brief Append string
     * @return Number of characters actually added
     */
    template <size_type M>
    constexpr size_type append(const string<CharT, M>& other) noexcept {
        return append(other.data(), other.length());
    }

    constexpr size_type append(const CharT* str) noexcept {
        return str ? append(str, detail::strlen(str)) : 0;
    }

    constexpr size_type append(const CharT* str, size_type count) noexcept {
        if (!str || count == 0 || len_ >= N) return 0;
        
        size_type to_copy = std::min(count, N - len_);
        detail::memcpy_string(data_ + len_, str, to_copy);
        len_ += to_copy;
        data_[len_] = CharT{};
        return to_copy;
    }

    constexpr size_type append(size_type count, CharT ch) noexcept {
        if (count == 0 || len_ >= N) return 0;
        
        size_type to_add = std::min(count, N - len_);
        detail::memset_string(data_ + len_, ch, to_add);
        len_ += to_add;
        data_[len_] = CharT{};
        return to_add;
    }

    /**
     * @brief Append operators
     */
    template <size_type M>
    constexpr string& operator+=(const string<CharT, M>& other) noexcept {
        append(other);
        return *this;
    }

    constexpr string& operator+=(const CharT* str) noexcept {
        append(str);
        return *this;
    }

    constexpr string& operator+=(CharT ch) noexcept {
        push_back(ch);
        return *this;
    }

    /**
     * @brief Insert string at position
     */
    constexpr string& insert(size_type pos, const CharT* str, size_type count) noexcept {
        if (!str || pos > len_ || count == 0 || len_ >= N) return *this;
        
        size_type space = N - len_;
        size_type to_insert = std::min(count, space);
        
        // Shift characters right
        size_type to_move = len_ - pos;
        if (to_move > 0) {
            detail::memmove_string(data_ + pos + to_insert, data_ + pos, to_move);
        }
        
        // Insert new characters
        detail::memcpy_string(data_ + pos, str, to_insert);
        
        len_ = std::min(len_ + to_insert, N);
        data_[len_] = CharT{};
        return *this;
    }

    constexpr string& insert(size_type pos, const CharT* str) noexcept {
        return str ? insert(pos, str, detail::strlen(str)) : *this;
    }

    template <size_type M>
    constexpr string& insert(size_type pos, const string<CharT, M>& str) noexcept {
        return insert(pos, str.data(), str.length());
    }

    constexpr string& insert(size_type pos, size_type count, CharT ch) noexcept {
        if (pos > len_ || count == 0 || len_ >= N) return *this;
        
        size_type space = N - len_;
        size_type to_insert = std::min(count, space);
        
        // Shift characters right
        size_type to_move = len_ - pos;
        if (to_move > 0) {
            detail::memmove_string(data_ + pos + to_insert, data_ + pos, to_move);
        }
        
        // Insert new characters
        detail::memset_string(data_ + pos, ch, to_insert);
        
        len_ = std::min(len_ + to_insert, N);
        data_[len_] = CharT{};
        return *this;
    }

    /**
     * @brief Erase characters
     */
    constexpr string& erase(size_type pos = 0, size_type count = npos) noexcept {
        if (pos >= len_) return *this;
        
        size_type to_erase = std::min(count, len_ - pos);
        size_type new_len = len_ - to_erase;
        
        // Shift left
        size_type to_move = len_ - (pos + to_erase);
        if (to_move > 0) {
            detail::memmove_string(data_ + pos, data_ + pos + to_erase, to_move);
        }
        
        len_ = new_len;
        data_[len_] = CharT{};
        return *this;
    }

    /**
     * @brief Replace range with string
     */
    constexpr string& replace(size_type pos, size_type count, 
                              const CharT* str, size_type str_len) noexcept {
        if (!str || pos > len_) return *this;
        
        // Erase then insert is simpler and equally efficient for constexpr
        erase(pos, count);
        insert(pos, str, str_len);
        return *this;
    }

    constexpr string& replace(size_type pos, size_type count, const CharT* str) noexcept {
        return str ? replace(pos, count, str, detail::strlen(str)) : *this;
    }

    template <size_type M>
    constexpr string& replace(size_type pos, size_type count, 
                              const string<CharT, M>& str) noexcept {
        return replace(pos, count, str.data(), str.length());
    }

    /**
     * @brief Resize string
     */
    constexpr void resize(size_type count, CharT ch = CharT{}) noexcept {
        if (count > N) count = N;
        
        if (count > len_) {
            // Expand
            detail::memset_string(data_ + len_, ch, count - len_);
        }
        
        len_ = count;
        data_[len_] = CharT{};
    }

    /**
     * @brief Swap contents
     */
    constexpr void swap(string& other) noexcept {
        string temp = *this;
        *this = other;
        other = temp;
    }

    // ========================================================================
    // String Transformations
    // ========================================================================

    /**
     * @brief Reverse string in-place
     */
    constexpr string& reverse() noexcept {
        for (size_type i = 0; i < len_ / 2; ++i) {
            CharT tmp = data_[i];
            data_[i] = data_[len_ - 1 - i];
            data_[len_ - 1 - i] = tmp;
        }
        return *this;
    }

    /**
     * @brief Convert to uppercase (char only)
     */
    constexpr string& to_upper() noexcept requires std::same_as<CharT, char> {
        for (size_type i = 0; i < len_; ++i) {
            data_[i] = detail::to_upper(data_[i]);
        }
        return *this;
    }

    /**
     * @brief Convert to lowercase (char only)
     */
    constexpr string& to_lower() noexcept requires std::same_as<CharT, char> {
        for (size_type i = 0; i < len_; ++i) {
            data_[i] = detail::to_lower(data_[i]);
        }
        return *this;
    }

    /**
     * @brief Trim whitespace from both ends
     */
    constexpr string& trim() noexcept requires std::same_as<CharT, char> {
        // Trim left
        size_type start = 0;
        while (start < len_ && detail::is_space(data_[start])) {
            ++start;
        }
        
        // Trim right
        size_type end = len_;
        while (end > start && detail::is_space(data_[end - 1])) {
            --end;
        }
        
        // Move if needed
        if (start > 0 || end < len_) {
            len_ = end - start;
            if (start > 0) {
                detail::memmove_string(data_, data_ + start, len_);
            }
            data_[len_] = CharT{};
        }
        return *this;
    }

    /**
     * @brief Trim left whitespace
     */
    constexpr string& trim_left() noexcept requires std::same_as<CharT, char> {
        size_type start = 0;
        while (start < len_ && detail::is_space(data_[start])) {
            ++start;
        }
        
        if (start > 0) {
            len_ -= start;
            detail::memmove_string(data_, data_ + start, len_);
            data_[len_] = CharT{};
        }
        return *this;
    }

    /**
     * @brief Trim right whitespace
     */
    constexpr string& trim_right() noexcept requires std::same_as<CharT, char> {
        while (len_ > 0 && detail::is_space(data_[len_ - 1])) {
            --len_;
        }
        data_[len_] = CharT{};
        return *this;
    }
} ;

} // namespace zuu
