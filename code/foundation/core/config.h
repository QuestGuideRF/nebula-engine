#pragma once
#ifdef __WIN32__
#undef __WIN32__
#endif
#if (WIN32 || _WIN32)
#define __WIN32__ (1)
#endif
#ifdef __GNUC__
#ifndef __LINUX__
#define __LINUX__ (1)
#endif
#endif
#ifdef _DEBUG
#define NEBULA_DEBUG (1)
#endif
static const int JobMaxSliceSize = 0xFFFF;
#if PUBLIC_BUILD
#define __NEBULA_NO_ASSERT__ (0)
#else
#define __NEBULA_NO_ASSERT__ (0)
#endif
#define NEBULA_NATIVE_ARCHIVE_SUPPORT (0)
#if NEBULA_DEBUG
#if __WIN32__
#define NEBULA_MEMORY_STATS (1)
#else
#define NEBULA_MEMORY_STATS (0)
#endif
#define NEBULA_MEMORY_ADVANCED_DEBUGGING (0)
#else
#define NEBULA_MEMORY_STATS (0)
#define NEBULA_MEMORY_ADVANCED_DEBUGGING (0)
#endif
#if (__LINUX__)
#define NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES (0)
#else
#define NEBULA_ENABLE_THREADLOCAL_STRINGATOM_TABLES (1)
#endif
#define NEBULA_ENABLE_GLOBAL_STRINGBUFFER_GROWTH (1)
#define NEBULA_GLOBAL_STRINGBUFFER_CHUNKSIZE (32 * 1024)
#define NEBULA_ANIMATIONSYSTEM_VERBOSELOG (0)
#define NEBULA_ANIMATIONSYSTEM_FRAMEDUMP (0)
#if NEBULA_DEBUG
#define NEBULA_BOUNDSCHECKS (1)
#else
#define NEBULA_BOUNDSCHECKS (0)
#endif
#if PUBLIC_BUILD
#define __NEBULA_HTTP__ (0)
#else
#define __NEBULA_HTTP__ (1)
#endif
#if PUBLIC_BUILD
    #define NEBULA_ENABLE_PROFILING (0)
#elif __NEBULA_HTTP__
    #define NEBULA_ENABLE_PROFILING (1)
#else
    #define NEBULA_ENABLE_PROFILING (0)
#endif
#if NEBULA_ENABLE_PROFILING
#define NEBULA_ENABLE_PERFORMANCE_WARNINGS (1)
#endif
#define NEBULA_MAXPATH (512)
#define NEBULA_LEGACY_SUPPORT (1)
#define NEBULA_ENABLE_MINIDUMPS (0)
#define NEBULA_WINDOW_CLASS "Nebula::MainWindow"
#define NEBULA_CONSOLE_HISTORY_SIZE (256)
#define NEBULA_XMLREADER_LEGACY_VECTORS (1)
#define DEFAULT_IO_SCHEME "file"
#if 0
#FIXME add defines for these in cmake instead
#if __VULKAN__
#define NEBULA_DEFAULT_FRAMESHADER_NAME "vkdefault"
#else
#error "Для платформы не определен фреймшейдер по умолчанию."
#endif
#endif
#define USE_CURL 1
#if __WIN32__ || __LINUX__
#define NEBULA_TEXTURE_EXTENSION ".dds"
#define NEBULA_SURFACE_EXTENSION ".sur"
#define NEBULA_MESH_EXTENSION ".nvx"
#endif
#ifdef _MSC_VER
#define __VC__ (1)
#endif
#if defined __GNUC__
#define __cdecl
#define __forceinline inline __attribute__((always_inline))
#pragma GCC diagnostic ignored "-Wmultichar"
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
#if !defined(__GNUC__)
#define  __attribute__(x)
#endif
#if __WIN32__
#define __maxTextureBytes__ (524288000)
#else
#define __maxTextureBytes__ (268435456)
#endif
#define NEBULA_THREAD_DEFAULTSTACKSIZE 65536