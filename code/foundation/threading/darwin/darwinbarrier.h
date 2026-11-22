#pragma once
#ifndef DARWIN_DARWINBARRIER_H
#define DARWIN_DARWINBARRIER_H
#include "core/types.h"
#include <libkern/OSAtomic.h>
#define ReadWriteBarrier OSMemoryBarrier
#define MemoryBarrier OSMemoryBarrier
#endif