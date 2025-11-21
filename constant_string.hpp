/**
 * @file constant_string.hpp
 * @brief Compile-time fixed-size string library untuk C++20
 * @version 1.0
 * 
 * Library ini menyediakan implementasi string dengan ukuran tetap
 * yang dapat dievaluasi pada waktu kompilasi (constexpr).
 */

#pragma once

#include <cstddef>
#include <algorithm>
#include <compare>
#include <stdexcept>
#include <type_traits>

namespace zuu {

// ============================================================================
// SECTION: Character Traits
// ============================================================================

namespace traits {

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

template <typename T>
inline constexpr bool is_char_v = is_char<std::remove_cv_t<T>>::value;

/**
 * @brief Concept untuk tipe karakter
 */
template <typename T>
concept character = is_char_v<T>;

} // namespace traits

// ============================================================================
// SECTION: Utility Functions
// ============================================================================

namespace detail {

/**
 * @brief Menghitung panjang string pada compile-time
 */
template <traits::character CharT>
[[nodiscard]] constexpr std::size_t strlen(const CharT* str) noexcept {
    std::size_t len = 0;
    while (str[len] != CharT{}) ++len;
    return len;
}

/**
 * @brief Membandingkan dua string pada compile-time
 */
template <traits::character CharT>
[[nodiscard]] constexpr int strcmp(const CharT* a, const CharT* b, 
                                    std::size_t len) noexcept {
    for (std::size_t i = 0; i < len; ++i) {
        if (a[i] < b[i]) return -1;
        if (a[i] > b[i]) return 1;
    }
    return 0;
}

/**
 * @brief Mencari karakter dalam string
 */
template <traits::character CharT>
[[nodiscard]] constexpr std::size_t find_char(
    const CharT* str, std::size_t len, CharT ch, std::size_t pos = 0) noexcept {
    for (std::size_t i = pos; i < len; ++i) {
        if (str[i] == ch) return i;
    }
    return static_cast<std::size_t>(-1); // npos
}

} // namespace detail

// ============================================================================
// SECTION: Iterator
// ============================================================================

/**
 * @brief Iterator untuk constant::string
 */
template <traits::character CharT, bool IsConst>
class string_iterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = CharT;
    using difference_type   = std::ptrdiff_t;
    using pointer           = std::conditional_t<IsConst, const CharT*, CharT*>;
    using reference         = std::conditional_t<IsConst, const CharT&, CharT&>;

private:
    pointer ptr_ = nullptr;

public:
    constexpr string_iterator() noexcept = default;
    constexpr explicit string_iterator(pointer p) noexcept : ptr_(p) {}
    
    // Konversi dari non-const ke const iterator
    constexpr string_iterator(const string_iterator<CharT, false>& other) noexcept
        requires IsConst : ptr_(other.base()) {}

    [[nodiscard]] constexpr pointer base() const noexcept { return ptr_; }
    [[nodiscard]] constexpr reference operator*() const noexcept { return *ptr_; }
    [[nodiscard]] constexpr pointer operator->() const noexcept { return ptr_; }
    [[nodiscard]] constexpr reference operator[](difference_type n) const noexcept { 
        return ptr_[n]; 
    }

    constexpr string_iterator& operator++() noexcept { ++ptr_; return *this; }
    constexpr string_iterator& operator--() noexcept { --ptr_; return *this; }
    constexpr string_iterator operator++(int) noexcept { 
        auto tmp = *this; ++ptr_; return tmp; 
    }
    constexpr string_iterator operator--(int) noexcept { 
        auto tmp = *this; --ptr_; return tmp; 
    }

    constexpr string_iterator& operator+=(difference_type n) noexcept { 
        ptr_ += n; return *this; 
    }
    constexpr string_iterator& operator-=(difference_type n) noexcept { 
        ptr_ -= n; return *this; 
    }

    [[nodiscard]] constexpr string_iterator operator+(difference_type n) const noexcept {
        return string_iterator(ptr_ + n);
    }
    [[nodiscard]] constexpr string_iterator operator-(difference_type n) const noexcept {
        return string_iterator(ptr_ - n);
    }
    [[nodiscard]] constexpr difference_type operator-(
        const string_iterator& other) const noexcept {
        return ptr_ - other.ptr_;
    }

    [[nodiscard]] constexpr auto operator<=>(
        const string_iterator&) const noexcept = default;
};

template <traits::character CharT, bool IsConst>
[[nodiscard]] constexpr string_iterator<CharT, IsConst> operator+(
    typename string_iterator<CharT, IsConst>::difference_type n,
    const string_iterator<CharT, IsConst>& it) noexcept {
    return it + n;
}

// ============================================================================
// SECTION: Main String Class
// ============================================================================

/**
 * @brief Fixed-size compile-time string class
 * 
 * @tparam CharT Tipe karakter (char, wchar_t, char8_t, char16_t, char32_t)
 * @tparam N Kapasitas maksimum string (tidak termasuk null terminator)
 * 
 * @example
 * ```cpp
 * constexpr cstr::string<char, 10> str("Hello");
 * static_assert(str.length() == 5);
 * static_assert(str.capacity() == 10);
 * ```
 */
template <traits::character CharT = char, std::size_t N = 0>
class string {
public:
    // Type aliases
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

    /// Nilai sentinel untuk "tidak ditemukan"
    static constexpr size_type npos = static_cast<size_type>(-1);

private:
    CharT data_[N + 1]{};
    size_type len_ = 0;

    /**
     * @brief Internal assignment dari C-string
     */
    constexpr void assign_impl(const CharT* str, size_type count) noexcept {
        len_ = (count < N) ? count : N;
        for (size_type i = 0; i < len_; ++i) {
            data_[i] = str[i];
        }
        data_[len_] = CharT{};
    }

public:
    // ========================================================================
    // Constructors
    // ========================================================================

    constexpr string() noexcept = default;
    constexpr string(const string&) noexcept = default;
    constexpr string(string&&) noexcept = default;
    constexpr string& operator=(const string&) noexcept = default;
    constexpr string& operator=(string&&) noexcept = default;

    /**
     * @brief Konstruktor dari C-string
     */
    constexpr string(const CharT* str) noexcept {
        assign_impl(str, detail::strlen(str));
    }

    /**
     * @brief Konstruktor dari array karakter
     */
    template <size_type M>
    constexpr string(const CharT (&str)[M]) noexcept {
        assign_impl(str, M > 0 ? M - 1 : 0);
    }

    /**
     * @brief Konstruktor dengan pengulangan karakter
     * @param count Jumlah pengulangan
     * @param ch Karakter yang diulang
     */
    constexpr string(size_type count, CharT ch) noexcept {
        len_ = (count < N) ? count : N;
        for (size_type i = 0; i < len_; ++i) {
            data_[i] = ch;
        }
        data_[len_] = CharT{};
    }

    /**
     * @brief Konstruktor dari string lain dengan ukuran berbeda
     */
    template <size_type M>
    constexpr string(const string<CharT, M>& other) noexcept {
        assign_impl(other.data(), other.length());
    }

    // ========================================================================
    // Assignment Operators
    // ========================================================================

    constexpr string& operator=(const CharT* str) noexcept {
        assign_impl(str, detail::strlen(str));
        return *this;
    }

    template <size_type M>
    constexpr string& operator=(const string<CharT, M>& other) noexcept {
        assign_impl(other.data(), other.length());
        return *this;
    }

    // ========================================================================
    // Element Access
    // ========================================================================

    /**
     * @brief Akses elemen dengan bounds checking
     * @throws std::out_of_range jika pos >= length()
     */
    [[nodiscard]] constexpr reference at(size_type pos) {
        if (pos >= len_) throw std::out_of_range("string::at: index out of range");
        return data_[pos];
    }

    [[nodiscard]] constexpr const_reference at(size_type pos) const {
        if (pos >= len_) throw std::out_of_range("string::at: index out of range");
        return data_[pos];
    }

    [[nodiscard]] constexpr reference operator[](size_type pos) noexcept { 
        return data_[pos]; 
    }
    [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept { 
        return data_[pos]; 
    }

    [[nodiscard]] constexpr reference front() noexcept { return data_[0]; }
    [[nodiscard]] constexpr const_reference front() const noexcept { return data_[0]; }
    [[nodiscard]] constexpr reference back() noexcept { return data_[len_ - 1]; }
    [[nodiscard]] constexpr const_reference back() const noexcept { return data_[len_ - 1]; }
    [[nodiscard]] constexpr pointer data() noexcept { return data_; }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return data_; }
    [[nodiscard]] constexpr const_pointer c_str() const noexcept { return data_; }

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
    [[nodiscard]] constexpr reverse_iterator rend() noexcept { 
        return reverse_iterator(begin()); 
    }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { 
        return const_reverse_iterator(begin()); 
    }

    // ========================================================================
    // Capacity
    // ========================================================================

    [[nodiscard]] constexpr bool empty() const noexcept { return len_ == 0; }
    [[nodiscard]] constexpr size_type length() const noexcept { return len_; }
    [[nodiscard]] constexpr size_type size() const noexcept { return len_; }
    [[nodiscard]] constexpr size_type capacity() const noexcept { return N; }
    [[nodiscard]] constexpr size_type max_size() const noexcept { return N; }

    // ========================================================================
    // Modifiers
    // ========================================================================

    /**
     * @brief Mengosongkan string
     */
    constexpr void clear() noexcept {
        len_ = 0;
        data_[0] = CharT{};
    }

    /**
     * @brief Menambahkan karakter di akhir
     * @return true jika berhasil, false jika kapasitas penuh
     */
    constexpr bool push_back(CharT ch) noexcept {
        if (len_ >= N) return false;
        data_[len_++] = ch;
        data_[len_] = CharT{};
        return true;
    }

    /**
     * @brief Menghapus karakter terakhir
     */
    constexpr void pop_back() noexcept {
        if (len_ > 0) {
            data_[--len_] = CharT{};
        }
    }

    /**
     * @brief Menambahkan string di akhir
     * @return Jumlah karakter yang berhasil ditambahkan
     */
    template <size_type M>
    constexpr size_type append(const string<CharT, M>& other) noexcept {
        return append(other.data(), other.length());
    }

    constexpr size_type append(const CharT* str) noexcept {
        return append(str, detail::strlen(str));
    }

    constexpr size_type append(const CharT* str, size_type count) noexcept {
        size_type to_copy = std::min(count, N - len_);
        for (size_type i = 0; i < to_copy; ++i) {
            data_[len_ + i] = str[i];
        }
        len_ += to_copy;
        data_[len_] = CharT{};
        return to_copy;
    }

    constexpr size_type append(size_type count, CharT ch) noexcept {
        size_type to_add = std::min(count, N - len_);
        for (size_type i = 0; i < to_add; ++i) {
            data_[len_ + i] = ch;
        }
        len_ += to_add;
        data_[len_] = CharT{};
        return to_add;
    }

    /**
     * @brief Operator += untuk append
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
     * @brief Menghapus karakter dari posisi tertentu
     */
    constexpr string& erase(size_type pos = 0, size_type count = npos) noexcept {
        if (pos >= len_) return *this;
        
        size_type to_erase = std::min(count, len_ - pos);
        size_type new_len = len_ - to_erase;
        
        for (size_type i = pos; i < new_len; ++i) {
            data_[i] = data_[i + to_erase];
        }
        len_ = new_len;
        data_[len_] = CharT{};
        return *this;
    }

    /**
     * @brief Menyisipkan string pada posisi tertentu
     */
    constexpr string& insert(size_type pos, const CharT* str, size_type count) noexcept {
        if (pos > len_) return *this;
        
        size_type space = N - len_;
        size_type to_insert = std::min(count, space);
        
        // Geser karakter ke kanan
        for (size_type i = len_; i > pos; --i) {
            if (i + to_insert <= N) {
                data_[i + to_insert - 1] = data_[i - 1];
            }
        }
        
        // Sisipkan karakter baru
        for (size_type i = 0; i < to_insert && pos + i < N; ++i) {
            data_[pos + i] = str[i];
        }
        
        len_ = std::min(len_ + to_insert, N);
        data_[len_] = CharT{};
        return *this;
    }

    /**
     * @brief Mengganti karakter pada rentang tertentu
     */
    constexpr string& replace(size_type pos, size_type count, 
                              const CharT* str, size_type str_len) noexcept {
        erase(pos, count);
        insert(pos, str, str_len);
        return *this;
    }

    /**
     * @brief Membalik string
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
     * @brief Mengubah ke huruf besar (hanya untuk char)
     */
    constexpr string& to_upper() noexcept requires std::same_as<CharT, char> {
        for (size_type i = 0; i < len_; ++i) {
            if (data_[i] >= 'a' && data_[i] <= 'z') {
                data_[i] -= 32;
            }
        }
        return *this;
    }

    /**
     * @brief Mengubah ke huruf kecil (hanya untuk char)
     */
    constexpr string& to_lower() noexcept requires std::same_as<CharT, char> {
        for (size_type i = 0; i < len_; ++i) {
            if (data_[i] >= 'A' && data_[i] <= 'Z') {
                data_[i] += 32;
            }
        }
        return *this;
    }

    /**
     * @brief Menghapus whitespace di awal dan akhir
     */
    constexpr string& trim() noexcept requires std::same_as<CharT, char> {
        // Trim kiri
        size_type start = 0;
        while (start < len_ && (data_[start] == ' ' || data_[start] == '\t' || 
               data_[start] == '\n' || data_[start] == '\r')) {
            ++start;
        }
        
        // Trim kanan
        size_type end = len_;
        while (end > start && (data_[end - 1] == ' ' || data_[end - 1] == '\t' || 
               data_[end - 1] == '\n' || data_[end - 1] == '\r')) {
            --end;
        }
        
        if (start > 0 || end < len_) {
            len_ = end - start;
            for (size_type i = 0; i < len_; ++i) {
                data_[i] = data_[start + i];
            }
            data_[len_] = CharT{};
        }
        return *this;
    }

    // ========================================================================
    // Search Operations
    // ========================================================================

    /**
     * @brief Mencari substring
     */
    [[nodiscard]] constexpr size_type find(const CharT* str, size_type pos = 0) const noexcept {
        size_type str_len = detail::strlen(str);
        if (str_len == 0) return pos <= len_ ? pos : npos;
        if (str_len > len_) return npos;
        
        for (size_type i = pos; i <= len_ - str_len; ++i) {
            bool found = true;
            for (size_type j = 0; j < str_len; ++j) {
                if (data_[i + j] != str[j]) {
                    found = false;
                    break;
                }
            }
            if (found) return i;
        }
        return npos;
    }

    template <size_type M>
    [[nodiscard]] constexpr size_type find(const string<CharT, M>& str, 
                                            size_type pos = 0) const noexcept {
        return find(str.data(), pos);
    }

    /**
     * @brief Mencari karakter
     */
    [[nodiscard]] constexpr size_type find(CharT ch, size_type pos = 0) const noexcept {
        return detail::find_char(data_, len_, ch, pos);
    }

    /**
     * @brief Mencari substring dari belakang
     */
    [[nodiscard]] constexpr size_type rfind(CharT ch, size_type pos = npos) const noexcept {
        if (len_ == 0) return npos;
        size_type start = (pos >= len_) ? len_ - 1 : pos;
        for (size_type i = start + 1; i > 0; --i) {
            if (data_[i - 1] == ch) return i - 1;
        }
        return npos;
    }

    /**
     * @brief Mengecek apakah dimulai dengan prefix
     */
    [[nodiscard]] constexpr bool starts_with(const CharT* prefix) const noexcept {
        size_type prefix_len = detail::strlen(prefix);
        if (prefix_len > len_) return false;
        for (size_type i = 0; i < prefix_len; ++i) {
            if (data_[i] != prefix[i]) return false;
        }
        return true;
    }

    [[nodiscard]] constexpr bool starts_with(CharT ch) const noexcept {
        return len_ > 0 && data_[0] == ch;
    }

    /**
     * @brief Mengecek apakah diakhiri dengan suffix
     */
    [[nodiscard]] constexpr bool ends_with(const CharT* suffix) const noexcept {
        size_type suffix_len = detail::strlen(suffix);
        if (suffix_len > len_) return false;
        for (size_type i = 0; i < suffix_len; ++i) {
            if (data_[len_ - suffix_len + i] != suffix[i]) return false;
        }
        return true;
    }

    [[nodiscard]] constexpr bool ends_with(CharT ch) const noexcept {
        return len_ > 0 && data_[len_ - 1] == ch;
    }

    /**
     * @brief Mengecek apakah mengandung substring
     */
    [[nodiscard]] constexpr bool contains(const CharT* str) const noexcept {
        return find(str) != npos;
    }

    [[nodiscard]] constexpr bool contains(CharT ch) const noexcept {
        return find(ch) != npos;
    }

    /**
     * @brief Menghitung kemunculan karakter
     */
    [[nodiscard]] constexpr size_type count(CharT ch) const noexcept {
        size_type result = 0;
        for (size_type i = 0; i < len_; ++i) {
            if (data_[i] == ch) ++result;
        }
        return result;
    }

    // ========================================================================
    // Substring Operations
    // ========================================================================

    /**
     * @brief Mengambil substring
     * @return String baru dengan kapasitas yang sama
     */
    [[nodiscard]] constexpr string substr(size_type pos = 0, 
                                          size_type count = npos) const noexcept {
        string result;
        if (pos >= len_) return result;
        
        size_type actual_count = std::min(count, len_ - pos);
        for (size_type i = 0; i < actual_count && i < N; ++i) {
            result.data_[i] = data_[pos + i];
        }
        result.len_ = std::min(actual_count, N);
        result.data_[result.len_] = CharT{};
        return result;
    }

    // ========================================================================
    // Comparison
    // ========================================================================

    template <size_type M>
    [[nodiscard]] constexpr int compare(const string<CharT, M>& other) const noexcept {
        size_type min_len = std::min(len_, other.length());
        int result = detail::strcmp(data_, other.data(), min_len);
        if (result != 0) return result;
        if (len_ < other.length()) return -1;
        if (len_ > other.length()) return 1;
        return 0;
    }

    [[nodiscard]] constexpr int compare(const CharT* str) const noexcept {
        size_type str_len = detail::strlen(str);
        size_type min_len = std::min(len_, str_len);
        int result = detail::strcmp(data_, str, min_len);
        if (result != 0) return result;
        if (len_ < str_len) return -1;
        if (len_ > str_len) return 1;
        return 0;
    }

    // ========================================================================
    // Comparison Operators
    // ========================================================================

    template <size_type M>
    [[nodiscard]] constexpr bool operator==(const string<CharT, M>& other) const noexcept {
        return compare(other) == 0;
    }

    [[nodiscard]] constexpr bool operator==(const CharT* str) const noexcept {
        return compare(str) == 0;
    }

    template <size_type M>
    [[nodiscard]] constexpr auto operator<=>(const string<CharT, M>& other) const noexcept {
        int cmp = compare(other);
        if (cmp < 0) return std::strong_ordering::less;
        if (cmp > 0) return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }

    [[nodiscard]] constexpr auto operator<=>(const CharT* str) const noexcept {
        int cmp = compare(str);
        if (cmp < 0) return std::strong_ordering::less;
        if (cmp > 0) return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }

    // ========================================================================
    // Hash Support
    // ========================================================================

    /**
     * @brief Menghitung hash FNV-1a pada compile-time
     */
    [[nodiscard]] constexpr std::size_t hash() const noexcept {
        std::size_t h = 14695981039346656037ULL;
        for (size_type i = 0; i < len_; ++i) {
            h ^= static_cast<std::size_t>(data_[i]);
            h *= 1099511628211ULL;
        }
        return h;
    }
};

// ============================================================================
// SECTION: Non-member Functions
// ============================================================================

/**
 * @brief Concatenate dua string (hasil kapasitas = N1 + N2)
 */
template <traits::character CharT, std::size_t N1, std::size_t N2>
[[nodiscard]] constexpr auto concat(const string<CharT, N1>& a, 
                                    const string<CharT, N2>& b) noexcept {
    string<CharT, N1 + N2> result;
    result.append(a);
    result.append(b);
    return result;
}

/**
 * @brief Operator + untuk concatenate
 */
template <traits::character CharT, std::size_t N1, std::size_t N2>
[[nodiscard]] constexpr auto operator+(const string<CharT, N1>& a, 
                                        const string<CharT, N2>& b) noexcept {
    return concat(a, b);
}

/**
 * @brief Membuat substring dengan kapasitas baru
 */
template <std::size_t NewN, traits::character CharT, std::size_t N>
[[nodiscard]] constexpr auto substr(const string<CharT, N>& str,
                                    std::size_t pos = 0,
                                    std::size_t count = string<CharT, N>::npos) noexcept {
    string<CharT, NewN> result;
    if (pos < str.length()) {
        std::size_t actual = std::min(count, str.length() - pos);
        actual = std::min(actual, NewN);
        for (std::size_t i = 0; i < actual; ++i) {
            result.push_back(str[pos + i]);
        }
    }
    return result;
}

/**
 * @brief Membuat salinan yang direverse
 */
template <traits::character CharT, std::size_t N>
[[nodiscard]] constexpr auto reversed(const string<CharT, N>& str) noexcept {
    string<CharT, N> result = str;
    result.reverse();
    return result;
}

/**
 * @brief Membuat salinan uppercase
 */
template <std::size_t N>
[[nodiscard]] constexpr auto to_upper(const string<char, N>& str) noexcept {
    string<char, N> result = str;
    result.to_upper();
    return result;
}

/**
 * @brief Membuat salinan lowercase
 */
template <std::size_t N>
[[nodiscard]] constexpr auto to_lower(const string<char, N>& str) noexcept {
    string<char, N> result = str;
    result.to_lower();
    return result;
}

/**
 * @brief Membuat salinan yang di-trim
 */
template <std::size_t N>
[[nodiscard]] constexpr auto trimmed(const string<char, N>& str) noexcept {
    string<char, N> result = str;
    result.trim();
    return result;
}

// ============================================================================
// SECTION: Deduction Guides
// ============================================================================

template <traits::character CharT, std::size_t N>
string(const CharT (&)[N]) -> string<CharT, N - 1>;

// ============================================================================
// SECTION: User-Defined Literals
// ============================================================================

namespace literals {

/**
 * @brief User-defined literal untuk membuat compile-time string
 * @example auto str = "Hello"_cs;
 */
template <traits::character CharT, CharT... Cs>
[[nodiscard]] constexpr auto operator""_cs() noexcept {
    constexpr CharT arr[] = {Cs..., CharT{}};
    return string<CharT, sizeof...(Cs)>(arr);
}

} // namespace literals

// ============================================================================
// SECTION: Type Aliases
// ============================================================================

template <std::size_t N> using cstring   = string<char, N>;
template <std::size_t N> using cwstring  = string<wchar_t, N>;
template <std::size_t N> using cu8string = string<char8_t, N>;
template <std::size_t N> using cu16string = string<char16_t, N>;
template <std::size_t N> using cu32string = string<char32_t, N>;

} // namespace zuu

// ============================================================================
// SECTION: std::hash Specialization
// ============================================================================

template <zuu::traits::character CharT, std::size_t N>
struct std::hash<zuu::string<CharT, N>> {
    [[nodiscard]] constexpr std::size_t operator()(
        const zuu::string<CharT, N>& str) const noexcept {
        return str.hash();
    }
};
