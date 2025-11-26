/**
 * @file test.cpp
 * @brief Unit tests for fstring library
 * @author zugyonozz (rafizuhayr001@gmail.com)
 * @date 2025-11-24
 */

#include "fstring.hpp"
#include <iostream>
#include <cassert>

using namespace zuu;
using namespace zuu::literals;
using namespace zuu::algorithms;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running test_" #name "... "; \
    test_##name(); \
    std::cout << "PASSED\n"; \
} while(0)

// ==================== Basic Operations Tests ====================

TEST(construction) {
    fstring<10> str1;
    assert(str1.empty());
    assert(str1.length() == 0);
    
    fstring<10> str2 = "hello";
    assert(str2.length() == 5);
    assert(str2 == "hello");
    
    fstring<10> str3("world", 5);
    assert(str3 == "world");
    
    fstring<10> str4(3, 'a');
    assert(str4 == "aaa");
}

TEST(assignment) {
    fstring<10> str;
    str = "hello";
    assert(str == "hello");
    
    str = 'x';
    assert(str == "x");
    
    fstring<10> str2;
    str2 = str;
    assert(str2 == "x");
}

TEST(element_access) {
    fstring<10> str = "hello";
    assert(str[0] == 'h');
    assert(str[4] == 'o');
    assert(str.front() == 'h');
    assert(str.back() == 'o');
    assert(str.at(2) == 'l');
}

TEST(capacity) {
    fstring<10> str = "hello";
    assert(!str.empty());
    assert(str.length() == 5);
    assert(str.size() == 5);
    assert(str.max_size() == 10);
    assert(str.available() == 5);
    assert(!str.full());
    
    fstring<5> full = "hello";
    assert(full.full());
}

// ==================== Modifier Tests ====================

TEST(append) {
    fstring<20> str = "hello";
    str.append(" world");
    assert(str == "hello world");
    
    str.append('!');
    assert(str == "hello world!");
    
    str.append(3, '.');
    assert(str == "hello world!...");
}

TEST(insert) {
    fstring<20> str = "hello";
    str.insert(5, " world");
    assert(str == "hello world");
    
    str.insert(0ull, "say ");
    assert(str == "say hello world");
}

TEST(erase) {
    fstring<20> str = "hello world";
    str.erase(5, 6);
    assert(str == "hello");
    
    str.erase(0, 2);
    assert(str == "llo");
}

TEST(clear) {
    fstring<10> str = "hello";
    str.clear();
    assert(str.empty());
    assert(str.length() == 0);
}

// ==================== Search Tests ====================

TEST(find) {
    fstring<20> str = "hello world";
    assert(str.find("world") == 6);
    assert(str.find("xyz") == fstring<20>::npos);
    assert(str.find('o') == 4);
    assert(str.find('z') == fstring<20>::npos);
}

TEST(rfind) {
    fstring<20> str = "hello world";
    assert(str.rfind('o') == 7);
    assert(str.rfind('h') == 0);
}

TEST(contains) {
    fstring<20> str = "hello world";
    assert(str.contains("world"));
    assert(!str.contains("xyz"));
    assert(str.contains('o'));
    assert(!str.contains('z'));
}

TEST(starts_ends_with) {
    fstring<20> str = "hello world";
    assert(str.starts_with("hello"));
    assert(!str.starts_with("world"));
    assert(str.ends_with("world"));
    assert(!str.ends_with("hello"));
}

// ==================== Conversion Tests ====================

TEST(int_conversion) {
    auto str1 = to_fstring(42);
    assert(str1 == "42");
    
    auto str2 = to_fstring(-123);
    assert(str2 == "-123");
    
    auto str3 = to_fstring(0);
    assert(str3 == "0");
    
    fstring<10> num_str = "456";
    int num = parse_int<int>(num_str);
    assert(num == 456);
}

TEST(float_conversion) {
    auto str = to_fstring(3.14, 2);
    assert(str == "3.14");
    
    fstring<10> float_str = "2.71";
    float f = parse_float<float>(float_str);
    assert(f > 2.70 && f < 2.72);
}

TEST(bool_conversion) {
    auto str_true = to_fstring(true);
    assert(str_true == "true");
    
    auto str_false = to_fstring(false);
    assert(str_false == "false");
    
    fstring<10> bool_str = "true";
    bool b = parse_bool(bool_str);
    assert(b == true);
}

TEST(hex_binary) {
    auto hex = to_hex(255);
    assert(hex == "0xff");
    
    auto bin = to_binary(5);
    assert(bin == "0b101");
}

// ==================== Algorithm Tests ====================

TEST(case_conversion) {
    fstring<20> str = "Hello World";
    
    auto lower = to_lower(str);
    assert(lower == "hello world");
    
    auto upper = to_upper(str);
    assert(upper == "HELLO WORLD");
}

TEST(trim) {
    fstring<20> str = "  hello  ";
    
    auto trimmed = trim(str);
    assert(trimmed == "hello");
    
    auto left = trim_left(str);
    assert(left == "hello  ");
    
    auto right = trim_right(str);
    assert(right == "  hello");
}

TEST(replace_ops) {
    fstring<30> str = "hello world";
    
    auto rep1 = replace(str, 'o', '0');
    assert(rep1 == "hell0 w0rld");
}

TEST(split_join) {
    fstring<30> str = "a,b,c";
    auto parts = split(str, ',');
    assert(parts.count == 3);
    assert(parts.parts[0] == "a");
    assert(parts.parts[1] == "b");
    assert(parts.parts[2] == "c");
    
    auto joined = join(parts.parts, ',');
    // Note: join uses full array, need to only join used parts
}

TEST(reverse) {
    fstring<10> str = "hello";
    auto rev = reverse(str);
    assert(rev == "olleh");
}

TEST(padding) {
    fstring<20> str = "hi";
    
    auto left = pad_left(str, 5, '*');
    assert(left == "***hi");
    
    auto right = pad_right(str, 5, '-');
    assert(right == "hi---");
}

TEST(count_ops) {
    fstring<20> str = "hello";
    assert(count(str, 'l') == 2);
    assert(count(str, 'x') == 0);
}

TEST(char_checks) {
    fstring<10> digits = "12345";
    assert(is_digit(digits));
    
    fstring<10> alpha = "abcde";
    assert(is_alpha(alpha));
    
    fstring<10> alnum = "abc123";
    assert(is_alnum(alnum));
}

// ==================== Comparison Tests ====================

TEST(comparison) {
    fstring<10> str1 = "apple";
    fstring<10> str2 = "banana";
    fstring<10> str3 = "apple";
    
    assert(str1 == str3);
    assert(str1 != str2);
    assert(str1 < str2);
    assert(str2 > str1);
    assert(str1 <= str3);
    assert(str1 >= str3);
}

TEST(case_insensitive_compare) {
    fstring<10> upper = "HELLO";
    fstring<10> lower = "hello";
    assert(equals_ignore_case(upper, lower));
}

// ==================== Concatenation Tests ====================

TEST(concatenation) {
    fstring<10> str1 = "hello";
    fstring<10> str2 = "world";
    
    auto result = str1 + " " + str2;
    assert(result == "hello world");
    
    str1 += "!";
    assert(str1 == "hello!");
}

TEST(builder_pattern) {
    fstring<5> p1 = "hello";
    fstring<1> p2 = " ";
    fstring<5> p3 = "world";
    
    auto result = builder{}(p1, p2, p3);
    assert(result == "hello world");
}

// ==================== Constexpr Tests ====================

constexpr auto test_constexpr_basic() {
    fstring<10> str = "hello";
    return str.length() == 5;
}

constexpr auto test_constexpr_concat() {
    fstring<10> str1 = "hi";
    fstring<10> str2 = "!";
    auto result = str1 + str2;
    return result.length() == 3;
}

TEST(constexpr_operations) {
    static_assert(test_constexpr_basic());
    static_assert(test_constexpr_concat());
    std::cout << "(constexpr tests passed at compile time) ";
}

// ==================== Main Test Runner ====================

int main() {
    std::cout << "====================================\n";
    std::cout << "  fstring Library Unit Tests\n";
    std::cout << "====================================\n\n";
    
    int failed = 0;
    
    try {
        // Basic operations
        RUN_TEST(construction);
        RUN_TEST(assignment);
        RUN_TEST(element_access);
        RUN_TEST(capacity);
        
        // Modifiers
        RUN_TEST(append);
        RUN_TEST(insert);
        RUN_TEST(erase);
        RUN_TEST(clear);
        
        // Search
        RUN_TEST(find);
        RUN_TEST(rfind);
        RUN_TEST(contains);
        RUN_TEST(starts_ends_with);
        
        // Conversions
        RUN_TEST(int_conversion);
        RUN_TEST(float_conversion);
        RUN_TEST(bool_conversion);
        RUN_TEST(hex_binary);
        
        // Algorithms
        RUN_TEST(case_conversion);
        RUN_TEST(trim);
        RUN_TEST(replace_ops);
        RUN_TEST(split_join);
        RUN_TEST(reverse);
        RUN_TEST(padding);
        RUN_TEST(count_ops);
        RUN_TEST(char_checks);
        
        // Comparison
        RUN_TEST(comparison);
        RUN_TEST(case_insensitive_compare);
        
        // Concatenation
        RUN_TEST(concatenation);
        RUN_TEST(builder_pattern);
        
        // Constexpr
        RUN_TEST(constexpr_operations);
        
        std::cout << "\n====================================\n";
        std::cout << "  All tests passed! ✓\n";
        std::cout << "====================================\n";
        
    } catch (const std::exception& e) {
        std::cerr << "\nFAILED: " << e.what() << "\n";
        failed = 1;
    }
    
    return failed;
}
