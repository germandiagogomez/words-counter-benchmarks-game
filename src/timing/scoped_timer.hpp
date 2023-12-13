#pragma once

#include <chrono>
#include <functional>

namespace cw {

class ScopedTimer {
   public:
    ScopedTimer() = default;
    explicit ScopedTimer(std::function<void(std::chrono::duration<double> elapsed)> f)
        : _f(std::move(f)),
          _start(std::chrono::high_resolution_clock::now())
    {
    }

    ~ScopedTimer()
    {
        if (_start != decltype(_start){})
            force_finish();
    }

    void force_finish()
    {
        _f(std::chrono::high_resolution_clock::now() - _start);
        _start = {};
    }

   private:
    std::function<void(std::chrono::duration<double>)> _f = [](std::chrono::duration<double>) {
    };
    decltype(std::chrono::high_resolution_clock::now()) _start;
};

} // namespace cw
