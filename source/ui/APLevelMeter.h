//
// PM Daze - a phase-modulation synthesizer
//
// Copyright 2025, Greg Recco
//
// PM Daze is released under the GNU General Public Licence v3
// or later (GPL-3.0-or-later). The license is found in the "LICENSE"
// file in the root of this repository, or at
// https://www.gnu.org/licenses/gpl-3.0.en.html
//
// Source code for PM Daze is available at
// https://github.com/gregrecco67/PMDaze
//
#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "gin_plugin/gin_plugin.h"

#pragma once

class APLevelMeter final : public juce::Component, juce::Timer
{
  public:
    explicit APLevelMeter(const gin::LevelTracker &, juce::NormalisableRange<float> r = {-60, 0}, bool vertical = false);
    ~APLevelMeter() override;

    enum ColourIds
    {
        lineColourId = 0x1291e10,
        backgroundColourId = 0x1291e11,
        meterColourId = 0x1291e12,
        clipColourId = 0x1291e13
    };
    void paint(juce::Graphics &g) override;

  private:
    //==============================================================================
    void timerCallback() override;

    bool vertical = false;
    const gin::LevelTracker &tracker;
    juce::NormalisableRange<float> range{-60, 6};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APLevelMeter)
};
