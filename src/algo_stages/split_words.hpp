#pragma once

#include <types/dynamic_buffer.hpp>

#include <span>
#include <cstdlib>
#include <string_view>
#include <vector>
#include <ranges>


namespace cw {

template<class StringType = std::string_view>
std::vector<StringType>
split_in_words(std::span<char const> buf) noexcept
{
    auto cur = std::ranges::begin(buf);
    auto end = std::ranges::end(buf);
    if (cur == end) [[unlikely]]
        return {};
    while (cur != end && std::isspace(static_cast<unsigned char>(*++cur))) {
    }
    auto prev = cur;
    if (cur == end) [[unlikely]]
        return {};
    std::vector<StringType> words;
    words.reserve(buf.size() / 3);
    for (;;) {
        cur = std::find_if(/*std::execution::unseq, */ cur, end, [](unsigned char c) {
            return std::isspace(c);
        });
        words.emplace_back(prev, cur);
        if (cur == end) [[unlikely]]
            return words;
        while (std::isspace(*++cur) && cur != end) {
        }
        if (cur == end) [[unlikely]]
            return words;
        prev = cur;
    }
}

template<class StringType = std::string_view>
std::vector<StringType>
split_in_words(const FastDynamicBuffer& buf) noexcept
{
    auto cur = reinterpret_cast<const char*>(std::addressof(*buf.begin()));
    auto end = reinterpret_cast<const char*>(std::addressof(*buf.end()));
    return split_in_words<StringType>(std::span<char const>(cur, end));
}

// Split in words pointing to the bytes in the buffer.
// The DynamicBuffer must be kept alive so that the result does not dangle
template<class StringType = std::string_view>
std::vector<StringType>
split_in_words(const DynamicBuffer& buf) noexcept
{
    auto cur = reinterpret_cast<const char*>(std::addressof(*buf.begin()));
    auto end = reinterpret_cast<const char*>(std::addressof(*buf.end()));
    return split_in_words<StringType>(std::span<char const>(cur, end));
}


}