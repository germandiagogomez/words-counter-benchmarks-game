#pragma once

#include <absl/hash/hash.h>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

namespace cw {

struct ImmutableString {
    using Ptr = boost::local_shared_ptr<std::string const>;

    ImmutableString(std::string_view str)
        : _value(boost::make_local_shared<std::string const>(str.data(), str.size()))
    {
    }

   public:
    ImmutableString() = default;
    static ImmutableString create(std::string_view str)
    {
        thread_local absl::flat_hash_map<std::string_view, ImmutableString> instances;
        if (auto it = instances.find(str); it != instances.end()) {
            return it->second;
        }
        auto new_str = ImmutableString(str);
        instances[str] = new_str;
        return new_str;
    }

    template<typename H>
    friend H AbslHashValue(H h, const ImmutableString& str);

    friend bool operator==(const ImmutableString& lhs, const ImmutableString& rhs);

   private:
    Ptr _value{nullptr};
};

bool
operator==(const ImmutableString& lhs, const ImmutableString& rhs)
{
    return lhs._value == rhs._value;
}

template<typename H>
H
AbslHashValue(H h, const ImmutableString& str)
{
    return H::combine(std::move(h), str._value.get());
}

} // namespace cw
