#pragma once
#include <JuceHeader.h>

namespace daisysp_gui {

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
  CustomLookAndFeel() {
    static const auto gold = juce::Colour::fromString("FFD4AF37");
    static const auto darkGrey = juce::Colour::fromString("FF1A1A1A");
    static const auto offWhite = juce::Colour::fromString("FFF0F0F0");
    static const auto brass = juce::Colour::fromString("FFB5A642");

    metropolisTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MetropolisRegular_otf,
        BinaryData::MetropolisRegular_otfSize);

    setColour(juce::Slider::thumbColourId, gold);
    setColour(juce::Slider::rotarySliderFillColourId, gold);
    setColour(juce::Slider::rotarySliderOutlineColourId, offWhite);
    setColour(juce::Label::textColourId, offWhite);
    setColour(juce::ResizableWindow::backgroundColourId, darkGrey);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black);
    setColour(juce::Slider::textBoxTextColourId, gold);
    setColour(juce::Slider::textBoxOutlineColourId, brass);

    setColour(juce::ComboBox::backgroundColourId, darkGrey);
    setColour(juce::ComboBox::outlineColourId, gold);
    setColour(juce::ComboBox::arrowColourId, gold);
    setColour(juce::ComboBox::textColourId, gold);

    setColour(juce::PopupMenu::backgroundColourId, darkGrey);
    setColour(juce::PopupMenu::textColourId, gold);
    setColour(juce::PopupMenu::highlightedBackgroundColourId,
              gold.withAlpha(0.3f));
    setColour(juce::PopupMenu::highlightedBackgroundColourId,
              gold.withAlpha(0.3f));
    setColour(juce::PopupMenu::highlightedTextColourId, offWhite);

    setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    setColour(juce::TextEditor::textColourId, gold);
    setColour(juce::TextEditor::highlightColourId, gold.withAlpha(0.3f));
    setColour(juce::TextEditor::highlightedTextColourId, offWhite);
    setColour(juce::TextEditor::outlineColourId, gold);
    setColour(juce::TextEditor::focusedOutlineColourId, gold);
  }

  juce::Font getCustomFont(float height) {
    return juce::Font(metropolisTypeface).withHeight(height);
  }

  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle,
                        juce::Slider &slider) override {
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Outer Ring (Brass)
    g.setColour(juce::Colour::fromString("FFB5A642"));
    g.drawEllipse(rx, ry, rw, rw, 2.0f);

    // Inner Circle (Dark)
    g.setColour(juce::Colour::fromString("FF0F0F0F"));
    g.fillEllipse(rx + 2, ry + 2, rw - 4, rw - 4);

    juce::Path p;
    auto pointerLength = radius * 0.8f;
    auto pointerThickness = 3.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness,
                   pointerLength);
    p.applyTransform(
        juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(juce::Colour::fromString("FFD4AF37"));
    g.fillPath(p);

    // Decorative center dot
    g.fillEllipse(centreX - 3, centreY - 3, 6, 6);

    // Modulation Indicator
    if (auto *modValueProp = slider.getProperties().getVarPointer("modValue")) {
      if (!modValueProp->isVoid()) {
        float modVal = (float)*modValueProp;
        float modAngle =
            rotaryStartAngle + modVal * (rotaryEndAngle - rotaryStartAngle);

        // Draw Indicator (Small Orb)
        auto modRadius = radius * 0.85f;
        juce::Path modPath;
        modPath.addEllipse(-3.0f, -modRadius, 6.0f, 6.0f);
        modPath.applyTransform(
            juce::AffineTransform::rotation(modAngle).translated(centreX,
                                                                 centreY));

        g.setColour(juce::Colour::fromString("FFFFFFFF").withAlpha(0.8f));
        g.fillPath(modPath);

        juce::Path arc;
        float start = std::min(angle, modAngle);
        float end = std::max(angle, modAngle);
        if (end - start > 0.01f) {
          arc.addCentredArc(centreX, centreY, radius * 0.9f, radius * 0.9f,
                            0.0f, start, end, true);
          g.setColour(juce::Colour::fromString("FFFFFFFF").withAlpha(0.3f));
          g.strokePath(arc, juce::PathStrokeType(2.0f));
        }
      }
    }
  }

  void drawLabel(juce::Graphics &g, juce::Label &label) override {
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited()) {
      auto alpha = label.isEnabled() ? 1.0f : 0.5f;

      juce::Font font = getCustomFont(14.0f);

      g.setColour(label.findColour(juce::Label::textColourId)
                      .withMultipliedAlpha(alpha));
      g.setFont(font);

      auto textArea =
          getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

      g.drawFittedText(
          label.getText(), textArea, label.getJustificationType(),
          juce::jmax(1, (int)(textArea.getHeight() / font.getHeight())),
          label.getMinimumHorizontalScale());

      g.setColour(label.findColour(juce::Label::outlineColourId)
                      .withMultipliedAlpha(alpha));
    } else if (label.isEnabled()) {
      g.setColour(label.findColour(juce::Label::outlineColourId));
    }

    g.drawRoundedRectangle(label.getLocalBounds().toFloat(), 3.0f, 1.0f);
  }

  void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown,
                    int buttonX, int buttonY, int buttonW, int buttonH,
                    juce::ComboBox &box) override {
    auto cornerSize =
        box.findParentComponentOfClass<juce::GroupComponent>() != nullptr
            ? 0.0f
            : 3.0f;
    juce::Rectangle<int> boxBounds(0, 0, width, height);

    g.setColour(juce::Colour::fromString("FF1A1A1A"));
    g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

    g.setColour(juce::Colour::fromString("FFD4AF37"));
    g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize,
                           1.0f);

    // Arrow
    juce::Path path;
    path.startNewSubPath((float)buttonX + 3.0f,
                         (float)buttonY + (float)buttonH * 0.5f - 2.0f);
    path.lineTo((float)(buttonX + buttonW) - 3.0f,
                (float)buttonY + (float)buttonH * 0.5f - 2.0f);
    path.lineTo((float)buttonX + (float)buttonW * 0.5f,
                (float)buttonY + (float)buttonH * 0.5f + 3.0f);
    path.closeSubPath();

    g.setColour(juce::Colour::fromString("FFD4AF37"));
    g.fillPath(path);
  }

  void drawPopupMenuBackground(juce::Graphics &g, int width,
                               int height) override {
    g.fillAll(juce::Colour::fromString("FF1A1A1A"));
    g.setColour(juce::Colour::fromString("FFD4AF37"));
    g.drawRect(0, 0, width, height);
  }

  void drawButtonBackground(juce::Graphics &g, juce::Button &button,
                            const juce::Colour &backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override {
    auto cornerSize = 3.0f;
    auto bounds = button.getLocalBounds().toFloat();

    auto baseColour = juce::Colour::fromString("FF1A1A1A");

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
      baseColour = baseColour.brighter(0.2f);

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, cornerSize);

    g.setColour(juce::Colour::fromString("FFD4AF37"));
    g.drawRoundedRectangle(bounds.reduced(0.5f, 0.5f), cornerSize, 1.0f);
  }

  void drawButtonText(juce::Graphics &g, juce::TextButton &button,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override {
    juce::Font font = getCustomFont(14.0f);
    g.setFont(font);
    g.setColour(juce::Colour::fromString("FFD4AF37"));

    const int yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
    const int cornerSize =
        juce::jmin(button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
    const int leftIndent = juce::jmin(
        fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = juce::jmin(
        fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
      g.drawFittedText(button.getButtonText(), leftIndent, yIndent, textWidth,
                       button.getHeight() - yIndent * 2,
                       juce::Justification::centred, 2);
  }

  void drawTickBox(juce::Graphics &g, juce::Component &component, float x,
                   float y, float w, float h, bool ticked, bool isEnabled,
                   bool shouldDrawButtonAsHighlighted,
                   bool shouldDrawButtonAsDown) override {
    juce::ignoreUnused(isEnabled, shouldDrawButtonAsHighlighted,
                       shouldDrawButtonAsDown);

    juce::Rectangle<float> tickBounds(x, y, w, h);

    g.setColour(juce::Colour::fromString("FF1A1A1A"));
    g.fillRoundedRectangle(tickBounds, 3.0f);

    g.setColour(juce::Colour::fromString("FFD4AF37"));
    g.drawRoundedRectangle(tickBounds, 3.0f, 1.0f);

    if (ticked) {
      g.setColour(juce::Colour::fromString("FFD4AF37"));
      auto tick = getTickShape(1.0f);
      g.fillPath(tick, tick.getTransformToScaleToFit(
                           tickBounds.reduced(4, 5).toFloat(), false));
    }
  }

  void drawTooltip(juce::Graphics &g, const juce::String &text, int width,
                   int height) override {
    juce::Rectangle<int> bounds(width, height);
    auto cornerSize = 3.0f;

    g.setColour(juce::Colour::fromString("FF1A1A1A"));
    g.fillRoundedRectangle(bounds.toFloat(), cornerSize);

    g.setColour(juce::Colour::fromString("FFD4AF37"));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f, 0.5f), cornerSize,
                           1.0f);

    g.setColour(juce::Colour::fromString("FFD4AF37"));
    g.setFont(getCustomFont(14.0f));
    g.drawFittedText(text, bounds.reduced(5), juce::Justification::centred, 2);
  }

  void drawAlertBox(juce::Graphics &g, juce::AlertWindow &alert,
                    const juce::Rectangle<int> &textArea,
                    juce::TextLayout &textLayout) override {
    auto bounds = alert.getLocalBounds().toFloat();
    auto cornerSize = 10.0f;

    juce::Colour gradientStart = juce::Colour::fromString("FF1A1A1A");
    juce::Colour gradientEnd = juce::Colour::fromString("FF2A2A2A");

    juce::Colour gold = juce::Colour::fromString("FFD4AF37");

    juce::ColourGradient cg(gradientStart, bounds.getTopLeft(), gradientEnd,
                            bounds.getBottomRight(), false);
    cg.addColour(0.5, gradientStart.interpolatedWith(gold, 0.1f));

    g.setGradientFill(cg);
    g.fillRoundedRectangle(bounds, cornerSize);

    g.setColour(gold);
    g.drawRoundedRectangle(bounds.reduced(1.0f), cornerSize, 2.0f);

    g.setColour(alert.findColour(juce::AlertWindow::textColourId));
    g.setFont(getCustomFont(15.0f));

    textLayout.draw(g, textArea.toFloat());
  }

  juce::Typeface::Ptr getTypefaceForFont(const juce::Font &) override {
    return metropolisTypeface;
  }

private:
  juce::Typeface::Ptr metropolisTypeface;
};

} // namespace daisysp_gui
