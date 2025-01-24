/*$(CopyrightHeader)*/#pragma once

enum class LogLevel : int
{
    Info,
    Warn,
    Error
};
using TLogFn = void (*)(LogLevel level, const char* msg, ...);
