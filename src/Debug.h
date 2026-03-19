#pragma once

#include <cstdio>
#include <cstdarg>

// Debug logging toggles
inline bool g_DebugESP = false;     // Set true to enable ESP debug output
inline bool g_DebugAimbot = true;   // Set false to disable aimbot debug output

inline void DebugLog(const char* fmt, ...) {
    if (!g_DebugESP) return;
    printf("[DEBUG] ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}
