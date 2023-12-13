#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <count_words/cw_api.hpp>

#include <cstddef>
#include <memory>
#include <utility>

namespace cw {

struct CW_API FastDynamicBuffer {
    explicit FastDynamicBuffer(boost::iostreams::mapped_file_source& s) : _s(std::move(s)) {}

    FastDynamicBuffer(const FastDynamicBuffer&) = delete;
    FastDynamicBuffer& operator=(const FastDynamicBuffer&) = delete;

    FastDynamicBuffer(FastDynamicBuffer&&) = default;
    FastDynamicBuffer& operator=(FastDynamicBuffer&&) = default;
    boost::iostreams::mapped_file_source _s;

    auto begin() const noexcept
    {
        return reinterpret_cast<std::byte const*>(std::addressof(*_s.begin()));
    }

    auto end() const noexcept
    {
        return reinterpret_cast<std::byte const*>(std::addressof(*_s.begin()) + _s.size());
    }

    auto size() const noexcept { return _s.size(); }
};

struct CW_API DynamicBuffer {
    explicit DynamicBuffer(std::size_t num_bytes)
        :
#if defined(__cpp_lib_smart_ptr_for_overwrite) && __cpp_lib_smart_ptr_for_overwrite >= 202002L
        _data(std::make_unique_for_overwrite<std::byte[]>(num_bytes))
#else
        _data(std::make_unique<std::byte[]>(num_bytes))
#endif
    ,
          _size(num_bytes)
    {
    }

    std::unique_ptr<std::byte[]> _data;
    std::size_t _size;

    std::byte& operator[](std::size_t offset) { return _data[offset]; }

    std::byte const& operator[](std::size_t offset) const { return _data[offset]; }
    auto begin() const noexcept { return std::addressof(_data[0]); }

    auto end() const noexcept { return (std::addressof(_data[_size - 1])) + 1; }

    auto size() const noexcept { return _size; }
};

using DynamicBufferPtr = std::shared_ptr<DynamicBuffer>;

} // namespace cw
