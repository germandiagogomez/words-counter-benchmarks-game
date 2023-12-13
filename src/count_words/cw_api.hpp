#pragma once

#ifdef CW_EXPORTS
#if CW_STATIC_LIB_CONFIG == 0 && defined(_WIN32)
#define CW_API __declspec(dllexport)
#else
#define CW_API [[gnu::visibility("default")]]
#endif
#elif CW_STATIC_LIB_CONFIG == 0
#if _WIN32
#define CW_API __declspec(dllimport)
#else
#define CW_API
#endif
#elif CW_STATIC_LIB_CONFIG == 1
#define CW_API
#endif
