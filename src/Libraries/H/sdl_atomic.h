#ifndef SDL_ATOMIC_H
#define SDL_ATOMIC_H

#ifdef __AROS__
#ifdef USE_SDL

#ifdef __cplusplus
#include <atomic>
extern "C" {
#endif

// 1. Replicate the SDL_atomic_t structure
typedef struct {
    #ifdef __cplusplus
    std::atomic<int> value;
    #else
    volatile int value; // Fallback structure layout for pure C compiler passes
    #endif
} SDL_atomic_t;

// 2. Implement the standard SDL2 atomic functions using inline functions
#ifdef __cplusplus

inline int SDL_AtomicSet(SDL_atomic_t *a, int v) {
    return a->value.exchange(v);
}

inline int SDL_AtomicGet(SDL_atomic_t *a) {
    return a->value.load();
}

inline int SDL_AtomicAdd(SDL_atomic_t *a, int v) {
    // std::atomic::fetch_add returns the value PRIOR to the addition (matching SDL2 behavior)
    return a->value.fetch_add(v);
}

#else // Pure C implementation using GCC compiler extensions (works for cross-compilers)

static inline int SDL_AtomicSet(SDL_atomic_t *a, int v) {
    return __atomic_exchange_n(&(a->value), v, __ATOMIC_SEQ_CST);
}

static inline int SDL_AtomicGet(SDL_atomic_t *a) {
    return __atomic_load_n(&(a->value), __ATOMIC_SEQ_CST);
}

static inline int SDL_AtomicAdd(SDL_atomic_t *a, int v) {
    return __atomic_fetch_add(&(a->value), v, __ATOMIC_SEQ_CST);
}

#endif

// 3. Implement the convenient macro helpers for incrementing/decrementing
#define SDL_AtomicIncRef(a) SDL_AtomicAdd(a, 1)
#define SDL_AtomicDecRef(a) (SDL_AtomicAdd(a, -1) == 1)

#ifdef __cplusplus
}
#endif

#endif
#endif

#endif // SDL_ATOMIC_H
