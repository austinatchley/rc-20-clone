#pragma once

/**
 * Central registry of all APVTS parameter ID strings.
 *
 * All parameter IDs are snake_case.  Keep this file as the single source of
 * truth — never hard-code an ID string elsewhere.
 */
namespace ParameterIDs
{
    // ── Global ────────────────────────────────────────────────────────────────
    static constexpr const char* drift        = "drift";
    static constexpr const char* output_level = "output_level";

    // ── Noise ─────────────────────────────────────────────────────────────────
    static constexpr const char* noise_bypass = "noise_bypass";
    static constexpr const char* noise_amount = "noise_amount";
    static constexpr const char* noise_type   = "noise_type";

    // ── Wobble ────────────────────────────────────────────────────────────────
    static constexpr const char* wobble_bypass = "wobble_bypass";
    static constexpr const char* wobble_amount = "wobble_amount";
    static constexpr const char* wobble_mode   = "wobble_mode";

    // ── Distortion ────────────────────────────────────────────────────────────
    static constexpr const char* distortion_bypass = "distortion_bypass";
    static constexpr const char* distortion_amount = "distortion_amount";
    static constexpr const char* distortion_mode   = "distortion_mode";

    // ── Space ─────────────────────────────────────────────────────────────────
    static constexpr const char* space_bypass = "space_bypass";
    static constexpr const char* space_amount = "space_amount";
    static constexpr const char* space_mode   = "space_mode";
    static constexpr const char* space_size   = "space_size";
    static constexpr const char* space_tone   = "space_tone";

    // ── Magic ─────────────────────────────────────────────────────────────────
    static constexpr const char* magic_bypass = "magic_bypass";
    static constexpr const char* magic_amount = "magic_amount";
    static constexpr const char* magic_mode   = "magic_mode";

    // ── Limit ─────────────────────────────────────────────────────────────────
    static constexpr const char* limit_bypass = "limit_bypass";
    static constexpr const char* limit_amount = "limit_amount";
    static constexpr const char* limit_mode   = "limit_mode";

} // namespace ParameterIDs
