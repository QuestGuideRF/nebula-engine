#pragma once
#ifndef POSIX_POSIXBARRIER_H
#define POSIX_POSIXBARRIER_H
inline void ReadWriteBarrier() {asm volatile("": : :"memory");}
inline void MemoryBarrier() {asm volatile("mfence":::"memory");}
#endif