#pragma once

#ifdef APIENTRY
#undef APIENTRY
#endif

#include <memory>

#define NOMINMAX
#include <spdlog/spdlog.h>
#undef NOMINMAX

class Log
{
private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;

public:
    static void Init();

    inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
    inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
};

// Core log macros
#define LOG_CORE_TRACE(...)    ::Log::getCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...)     ::Log::getCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARN(...)     ::Log::getCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...)    ::Log::getCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...) ::Log::getCoreLogger()->critical(__VA_ARGS__)
#define LOG_LOG_ASSERT(x, y) \
       if (!x)                   \
       {                         \
         ELY_CORE_ERROR(y); \
         __debugbreak();         \
       }

// Client log macros
#define LOG_TRACE(...)         ::Log::getClientLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)          ::Log::getClientLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)          ::Log::getClientLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)         ::Log::getClientLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)      ::Log::getClientLogger()->critical(__VA_ARGS__)