/*
 * Audible Planets - an expressive, quasi-Ptolemaic semi-modular synthesizer
 *
 * Copyright 2024, Greg Recco
 *
 * Audible Planets is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Audible Planets is available at
 * https://github.com/gregrecco67/AudiblePlanets
 */

#include "APModAdditions.h"
#include "BinaryData.h"
#include "MainEditor.h"

MainEditor::MainEditor(PMProcessor &proc_) : proc(proc_)
{
    addAndMakeVisible(osc1);
    addAndMakeVisible(osc2);
    addAndMakeVisible(osc3);
    addAndMakeVisible(osc4);

    addAndMakeVisible(env1);
    addAndMakeVisible(env2);
    addAndMakeVisible(env3);
    addAndMakeVisible(env4);
    addAndMakeVisible(filter);
    addAndMakeVisible(modsrc);
    addAndMakeVisible(timbre);
    addAndMakeVisible(matrix);
    addAndMakeVisible(liveViz); // toggle
    liveViz.setLookAndFeel(&aplnf);
    proc.globalParams.pitchbendRange->addListener(this);
    proc.globalParams.mpe->addListener(this);
    addAndMakeVisible(aux);
    addAndMakeVisible(matrix);
    addAndMakeVisible(volumeBox);
    addAndMakeVisible(levelBox);
    addAndMakeVisible(speedSlider);
    speedSlider.setRange(0.0, 2.0);
    speedSlider.setSkewFactor(0.5);
    speedSlider.setValue(0.1);

    addChildComponent(global);
    addChildComponent(macros);

    osc1.setRight(true);
    osc2.setRight(true);
    osc3.setRight(true);
    osc4.setRight(true);

    // osc1.setHeaderLeft(true);
    // osc2.setHeaderLeft(true);
    // osc3.setHeaderLeft(true);
    // osc4.setHeaderLeft(true);

    env1.setRight(true);
    env2.setRight(true);
    env3.setRight(true);
    env4.setRight(true);

    filter.setRight(true);
    timbre.setRight(true);
    aux.setRight(true);
    global.setRight(true);
    macros.setRight(true);
    global.bottom = true;
    macros.bottom = true;

    modsrc.setHeaderRight(false);
    matrix.setHeaderRight(false);

    volumeBox.setRight(true);
    levelBox.setRight(true);
}

MainEditor::~MainEditor()
{
    proc.globalParams.pitchbendRange->removeListener(this);
    proc.globalParams.mpe->removeListener(this);
    liveViz.setLookAndFeel(nullptr);
}

void MainEditor::setGrid(gin::ParamBox *box, float x, float y, float heds, float w, float h)
{
    box->setBounds(static_cast<int>(x * 56.f), static_cast<int>(y * 70.f + 23.f * heds),
                   static_cast<int>(w * 56.f), static_cast<int>(h * 70.f + 23.f));
}

void MainEditor::paint(juce::Graphics &g)
{
    juce::Component::paint(g);
    g.setColour(findColour(gin::PluginLookAndFeel::title1ColourId));
    // matrix.setBounds(16 * 56, 326, 5 * 56, 326);
    g.drawLine(16 * 56, 326, 16 * 56, 652, 1.0f);
}

void MainEditor::resized()
{
    const auto area = getLocalBounds();

    if (area.getWidth() > 1186 || area.getHeight() > 725)
    {
        return;
    }

    setGrid(&osc1, 0, 0, 0, 4, 2);
    setGrid(&osc2, 0, 2, 1, 4, 2);
    setGrid(&osc3, 0, 4, 2, 4, 2);
    setGrid(&osc4, 0, 6, 3, 4, 2);
    setGrid(&env1, 4, 0, 0, 6, 2);
    setGrid(&env2, 4, 2, 1, 6, 2);
    setGrid(&env3, 4, 4, 2, 6, 2);
    setGrid(&env4, 4, 6, 3, 6, 2);
    setGrid(&filter, 10, 0, 0, 2, 2);
    setGrid(&timbre, 12, 0, 0, 3, 2);
    setGrid(&aux, 10, 2, 1, 5, 2);
    setGrid(&global, 10, 4, 2, 3, 2);
    global.setBounds(10 * 56, 4 * 70 + 2 * 23, 3 * 56, 2 * 70 + 24);
    setGrid(&macros, 13, 4, 2, 3, 2);
    macros.setBounds(13 * 56, 4 * 70 + 2 * 23, 3 * 56, 2 * 70 + 24);
    volumeBox.setBounds(15 * 56, 0, 56, 163);
    levelBox.setBounds(15 * 56, 163, 56, 163);
    swap.setBounds(10 * 56 - 5, 4 * 70 + 2 * 23 + 4 * 70 + 20, 45, 25);
    swap.changeWidthToFitText();
    liveViz.setBounds(static_cast<int>(10 * 56.f), static_cast<int>(4 * 70.f + 2 * 23.f), 75, 25);
    speedSlider.setBounds(14 * 56, 4 * 70 + 2 * 23 + 1, 111, 25);

    modsrc.setBounds(16 * 56, 0, 5 * 56, 326);
    matrix.setBounds(16 * 56, 326, 5 * 56, 326);
}
