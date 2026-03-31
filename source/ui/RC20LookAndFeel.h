#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Custom LookAndFeel for the RC-20 Clone plugin.
 *
 * Each ModuleComponent creates its own instance with its per-module accent
 * colour.  The global editor creates one instance with a neutral accent for
 * the shared controls (drift, output, preset bar).
 */
class RC20LookAndFeel final : public juce::LookAndFeel_V4
{
  public:
    explicit RC20LookAndFeel(juce::Colour accent = juce::Colour(0xFF888888));

    // ── Colour palette ────────────────────────────────────────────────────────
    static inline const juce::Colour background { 0xFF1A1A2E };
    static inline const juce::Colour panel       { 0xFF1F2040 };
    static inline const juce::Colour textPrimary { 0xFFDDDDCC };
    static inline const juce::Colour textMuted   { 0xFF888877 };

    // Per-module accent colours
    static inline const juce::Colour accentNoise      { 0xFFCC6644 };
    static inline const juce::Colour accentWobble     { 0xFF44AACC };
    static inline const juce::Colour accentDistortion { 0xFFCC4444 };
    static inline const juce::Colour accentSpace      { 0xFF6644CC };
    static inline const juce::Colour accentMagic      { 0xFF44CC88 };
    static inline const juce::Colour accentLimit      { 0xFFCCAA44 };

    // ── Overrides ─────────────────────────────────────────────────────────────
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics&, int w, int h, bool isButtonDown,
                      int bX, int bY, int bW, int bH,
                      juce::ComboBox&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawPopupMenuBackground(juce::Graphics&, int w, int h) override;

  private:
    juce::Colour accent_;
};
