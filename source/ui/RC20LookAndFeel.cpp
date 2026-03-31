#include "RC20LookAndFeel.h"

RC20LookAndFeel::RC20LookAndFeel(juce::Colour accent) : accent_(accent) {
    // Slider text box
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFAAAAAA));
    setColour(juce::Slider::textBoxBackgroundColourId, background);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    // Label
    setColour(juce::Label::textColourId, textPrimary);

    // ComboBox
    setColour(juce::ComboBox::textColourId, textPrimary);
    setColour(juce::ComboBox::backgroundColourId, panel);

    // PopupMenu
    setColour(juce::PopupMenu::backgroundColourId, panel);
    setColour(juce::PopupMenu::textColourId, textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, accent_.withAlpha(0.25f));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

    // TextButton (preset Save/Load)
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF252538));
    setColour(juce::TextButton::textColourOffId, textPrimary);
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);
}

// ── Rotary knob ────────────────────────────────────────────────────────────────

void RC20LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                       int x,
                                       int y,
                                       int w,
                                       int h,
                                       float pos,
                                       float startAngle,
                                       float endAngle,
                                       juce::Slider&) {
    const float cx = (float)x + (float)w * 0.5f;
    const float cy = (float)y + (float)h * 0.5f;
    const float r = (float)juce::jmin(w, h) * 0.4f;
    const float trackW = 2.5f;
    const float angle = startAngle + pos * (endAngle - startAngle);

    // Background arc (full range)
    {
        juce::Path p;
        p.addCentredArc(cx, cy, r, r, 0.0f, startAngle, endAngle, true);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.strokePath(p,
                     juce::PathStrokeType(
                         trackW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Value arc
    if (pos > 0.001f) {
        juce::Path p;
        p.addCentredArc(cx, cy, r, r, 0.0f, startAngle, angle, true);
        g.setColour(accent_);
        g.strokePath(p,
                     juce::PathStrokeType(
                         trackW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Knob body
    const float kr = r * 0.62f;
    g.setColour(juce::Colour(0xFF222235));
    g.fillEllipse(cx - kr, cy - kr, kr * 2.0f, kr * 2.0f);

    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawEllipse(cx - kr, cy - kr, kr * 2.0f, kr * 2.0f, 1.0f);

    // Indicator dot
    const float dotR = 2.5f;
    const float dotDist = kr * 0.58f;
    g.setColour(accent_);
    g.fillEllipse(cx + std::sin(angle) * dotDist - dotR,
                  cy - std::cos(angle) * dotDist - dotR,
                  dotR * 2.0f,
                  dotR * 2.0f);
}

// ── Bypass LED ─────────────────────────────────────────────────────────────────

void RC20LookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool, bool) {
    // bypass param: true = bypassed. LED is lit when the module is ACTIVE (not bypassed).
    const bool active = !button.getToggleState();
    const float cx = (float)button.getWidth() * 0.5f;
    const float cy = (float)button.getHeight() * 0.5f;
    const float ledR = (float)juce::jmin(button.getWidth(), button.getHeight()) * 0.25f;

    // Outer glow when active
    if (active) {
        g.setColour(accent_.withAlpha(0.25f));
        g.fillEllipse(
            cx - ledR - 3.0f, cy - ledR - 3.0f, (ledR + 3.0f) * 2.0f, (ledR + 3.0f) * 2.0f);
    }

    g.setColour(active ? accent_ : juce::Colours::white.withAlpha(0.2f));
    g.fillEllipse(cx - ledR, cy - ledR, ledR * 2.0f, ledR * 2.0f);
}

// ── ComboBox ───────────────────────────────────────────────────────────────────

void RC20LookAndFeel::drawComboBox(
    juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&) {
    g.setColour(juce::Colour(0xFF1E1E30));
    g.fillRoundedRectangle(1.0f, 1.0f, (float)w - 2.0f, (float)h - 2.0f, 4.0f);

    g.setColour(accent_.withAlpha(0.4f));
    g.drawRoundedRectangle(1.0f, 1.0f, (float)w - 2.0f, (float)h - 2.0f, 4.0f, 1.0f);

    // Dropdown arrow
    juce::Path arrow;
    const float aX = (float)w - 14.0f;
    const float aY = ((float)h - 5.0f) * 0.5f;
    arrow.addTriangle(aX, aY, aX + 8.0f, aY, aX + 4.0f, aY + 5.0f);
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.fillPath(arrow);
}

// ── TextButton (preset bar) ────────────────────────────────────────────────────

void RC20LookAndFeel::drawButtonBackground(
    juce::Graphics& g, juce::Button& button, const juce::Colour&, bool highlighted, bool down) {
    const auto b = button.getLocalBounds().toFloat().reduced(1.0f);

    auto fill = juce::Colour(0xFF252538);
    if (down)
        fill = fill.darker(0.2f);
    else if (highlighted)
        fill = fill.brighter(0.1f);

    g.setColour(fill);
    g.fillRoundedRectangle(b, 4.0f);

    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawRoundedRectangle(b, 4.0f, 1.0f);
}

// ── Popup menu background ──────────────────────────────────────────────────────

juce::Font RC20LookAndFeel::getComboBoxFont(juce::ComboBox&) {
    return juce::Font(juce::FontOptions(12.0f));
}

void RC20LookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int w, int h) {
    g.fillAll(juce::Colour(0xFF1E1E30));
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawRect(0, 0, w, h, 1);
}
