// Trampoline TU that compiles the canonical FLIP v2 implementation from the
// framework submodule. UE/Windows headers pollute the global namespace with
// macros (PI, MIN, MAX, min, max) that collide with identifiers used inside
// the upstream FLIP_2.h. Undefining them here keeps the framework sources
// sterile and avoids a fork.

#ifdef PI
    #undef PI
#endif
#ifdef MAX
    #undef MAX
#endif
#ifdef MIN
    #undef MIN
#endif
#ifdef max
    #undef max
#endif
#ifdef min
    #undef min
#endif

#include "flip2.cpp"
