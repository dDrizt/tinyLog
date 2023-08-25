#pragma once

#include <format>
#include <source_location>
#include <iostream>
#include <cstdint>
#include <type_traits>
#include <fstream>

namespace tinylog {

#define TINYLOG_FOREACH_LOG_LEVEL(f) \
    f(trace) \
    f(debug) \
    f(info) \
    f(critical) \
    f(warn) \
    f(error) \
    f(fatal)

enum class log_level : std::uint8_t {
#define _FUNCTION(name) name,
    TINYLOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
};

namespace details {

inline std::string log_level_name(log_level lev) {
    switch (lev) {
#define _FUNCTION(name) case log_level::name: return #name;
        TINYLOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
    }
    return "unknown";
}

inline log_level log_level_from_name(std::string lev) {
#define _FUNCTION(name) if (lev == #name) return log_level::name;
    TINYLOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
    return log_level::info;
}
    
template<class T>
struct with_source_location {
private:
    T inner;
    std::source_location loc;

public:
    template<class U> requires std::constructible_from<T, U>
    consteval with_source_location(U &&inner, std::source_location loc = std::source_location::current())
    : inner(std::forward<U>(inner)), loc(std::move(loc)) {}
    constexpr T const &format() const { return inner; }
    constexpr std::source_location const &location() const { return loc; }
};

inline log_level g_max_level = [] () -> log_level {
    if (auto lev = std::getenv("TINYLOG_LEVEL")) {
        return details::log_level_from_name(lev);
    }
    return log_level::info;
} ();

inline std::ofstream g_log_file("tiny.log");

} // namespace details

inline void set_log_level(log_level lev) {
    details::g_max_level = lev;
}


template<typename... Args>
void generic_log(log_level lev, details::with_source_location<std::format_string<Args...>> fmt, Args &&...args) {
    if (lev >= details::g_max_level) {
        auto const &loc = fmt.location();
        std::cout << loc.file_name() << ":" << loc.line() << " [" << details::log_level_name(lev) << "] " << std::vformat(fmt.format().get(), std::make_format_args(args...)) << '\n';
    }
}

#define _FUNCTION(name) \
template <typename... Args> \
void log_##name(details::with_source_location<std::format_string<Args...>> fmt, Args &&...args) { \
    return generic_log(log_level::name, std::move(fmt), std::forward<Args>(args)...); \
}
TINYLOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION

#define TINYLOG_P(x) ::tinylog::log_debug(#x "={}", x)

} // namespace tinylog