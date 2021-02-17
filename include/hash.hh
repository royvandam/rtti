#pragma once

#include <string_view>
#include <cstdint>

namespace Hash {
    /** 
     * Calculates the 32bit FNV1a hash of a c-string literal.
     * @param str String literal to be hashed
     * @param n Length of the string.
     * @return Calculated hash of the string
     */
    static constexpr std::uint32_t FNV1a(const char* str, std::size_t n, std::uint32_t hash = UINT32_C(2166136261)) {
        return n == 0 ? hash : FNV1a(str + 1, n - 1, (hash ^ str[0]) * UINT32_C(19777619));
    }
    
    /**
     * Calculates the 32bit FNV1a hash of a std::string_view literal.
     * note: Requires string_view to be a literal in order to be evaluated during compile time!
     * @param str String literal to be hashed
     * @return Calculated hash of the string
     */
    static constexpr std::uint32_t FNV1a(std::string_view str) {
        return FNV1a(str.data(), str.size());
    }
}
