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

#include <gin_plugin/gin_plugin.h>
#include "BinaryData.h"
#include "dsp/PMProcessor.h"
#include "MainPanels.h"

//==============================================================================
class MainEditor : public juce::Component, public gin::Parameter::ParameterListener
{
  public:
    explicit MainEditor(PMProcessor &proc_);
    ~MainEditor() override;

    void resized() override;
    void valueUpdated(gin::Parameter *) override {}
    static void setGrid(gin::ParamBox *box, float x, float y, float heds, float w, float h);
    void paint(juce::Graphics &g) override;

  private:
    PMProcessor &proc;

    OSCBox osc1{proc, proc.osc1Params, 0};
    OSCBox osc2{proc, proc.osc2Params, 1};
    OSCBox osc3{proc, proc.osc3Params, 2};
    OSCBox osc4{proc, proc.osc4Params, 3};

    ENVBox env1{proc, proc.env1Params, 0};
    ENVBox env2{proc, proc.env2Params, 1};
    ENVBox env3{proc, proc.env3Params, 2};
    ENVBox env4{proc, proc.env4Params, 3};

    LFOMain lfo1{proc, proc.lfo1Params, 1};
    LFOMain lfo2{proc, proc.lfo2Params, 2};
    LFOMain lfo3{proc, proc.lfo3Params, 3};
    LFOMain lfo4{proc, proc.lfo4Params, 4};

    FilterBox filter{"  flt", proc};
    ModBox modsrc{"  mod", proc};
    TimbreBox timbre{"  timbre", proc};
    AlgoBox algoBox{"   algo", proc};
    GlobalBox global{"  global", proc};
    MainMatrixBox matrix{"  Mod Matrix", proc};
    VolumeBox volumeBox{proc};
    LevelBox levelBox{proc.levelTracker};

    APLNF aplnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainEditor)
};
