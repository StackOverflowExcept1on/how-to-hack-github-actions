#ifndef FX_MUXER_H
#define FX_MUXER_H

/// Reverse engineered header based on dotnet sources

/// @see https://github.com/dotnet/runtime/blob/main/src/native/corehost/fxr/host_context.h
/// used as pointer type
struct host_context_t;

/// @see https://github.com/dotnet/runtime/blob/main/src/native/corehost/fxr/fx_muxer.h
/// class with static methods
class fx_muxer_t {
public:
    /// const host_context_t* fx_muxer_t::get_active_host_context()
    /// @see https://github.com/dotnet/runtime/blob/main/src/native/corehost/fxr/fx_muxer.cpp
    static const host_context_t *get_active_host_context();
};

#endif //FX_MUXER_H
