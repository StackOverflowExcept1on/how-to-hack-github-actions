#ifndef ADLER32_H
#define ADLER32_H

#include <bit>
#include <array>

namespace adler32::detail {
    __forceinline int lstrlenA_impl(LPCSTR buf) {
        int count = 0;

        while (*buf++) {
            count++;
        }

        return count;
    }

    __forceinline int lstrlenW_impl(LPCWSTR buf) {
        int count = 0;

        while (*buf++) {
            count++;
        }

        return count;
    }

    __forceinline constexpr void hash_begin(uint32_t *s1, uint32_t *s2) {
        *s1 = 1;
        *s2 = 0;
    }

    __forceinline constexpr void hash_impl(uint8_t byte, uint32_t &s1, uint32_t &s2) {
        constexpr uint32_t MOD_ADLER = 65521;

        s1 = (s1 + byte) % MOD_ADLER;
        s2 = (s2 + s1) % MOD_ADLER;
    }

    __forceinline constexpr uint32_t hash_finish(uint32_t s1, uint32_t s2) {
        return (s2 << 16) + s1;
    }

    template<typename T>
    __forceinline constexpr uint32_t adler32_compile_time(const T *buf, size_t buf_len) {
        uint32_t s1, s2;
        hash_begin(&s1, &s2);

        while (buf_len--) {
            auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(*(buf++));
            for (const uint8_t byte: bytes) {
                hash_impl(byte, s1, s2);
            }
        }

        return hash_finish(s1, s2);
    }

    __forceinline uint32_t adler32_runtime(const uint8_t *buf, size_t buf_len) {
        uint32_t s1, s2;
        hash_begin(&s1, &s2);

        while (buf_len--) {
            hash_impl(*(buf++), s1, s2);
        }

        return hash_finish(s1, s2);
    }
}

namespace adler32 {
    template<typename T, size_t N>
    __forceinline consteval uint32_t hash_fn_compile_time(const T(&buf)[N]) {
        return detail::adler32_compile_time(std::to_array(buf).data(), N - 1);
    }

    __forceinline uint32_t hash_fn(const char *buf) {
        return detail::adler32_runtime(
            reinterpret_cast<const uint8_t *>(buf),
            detail::lstrlenA_impl(buf)
        );
    }

    __forceinline uint32_t hash_fn(const wchar_t *buf) {
        return detail::adler32_runtime(
            reinterpret_cast<const uint8_t *>(buf),
            detail::lstrlenW_impl(buf) * sizeof(wchar_t)
        );
    }
}

#endif //ADLER32_H
