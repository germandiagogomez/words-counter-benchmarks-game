#pragma once

#include <chrono>
#include <utility>

namespace cw {

template<class F, class... Args>
auto
time_it(F&& f, Args&&... args)
{
    namespace c = std::chrono;
    namespace s = std;
    auto start = c::high_resolution_clock::now();
    auto result = std::forward<F>(f)(std::forward<Args>(args)...);
    auto elapsed = c::high_resolution_clock::now() - start;
    return s::make_pair(s::move(result), c::duration_cast<c::milliseconds>(elapsed));
}


}