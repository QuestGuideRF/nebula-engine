#pragma once
#ifdef __clang__
#define UNUSED(x) __attribute__((unused)) x
#else
#define UNUSED(x) x
#endif