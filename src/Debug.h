#pragma once

#include <cstdio>
#include <cstdarg>

// Debug logging for ESP - logs to console every N ms when enabled
inline bool g_DebugESP = true;  // Set to false to disable debug output

inline void DebugLog(const char* fmt, ...) {
    if (!g_DebugESP) return;
    printf("[DEBUG] ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}
