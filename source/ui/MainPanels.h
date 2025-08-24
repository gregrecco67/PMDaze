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
#include "APColors.h"
#include "APModAdditions.h"
#include <dsp/PMProcessor.h>
#include "EnvelopeComponent.h"
#include "APLevelMeter.h"
#include "BinaryData.h"

inline void gradientRect(juce::Graphics &g, const juce::Rectangle<int> rc, const juce::Colour c1, const juce::Colour c2)
{
    const juce::ColourGradient gradient(c1, static_cast<float>(rc.getX()), static_cast<float>(rc.getY()), c2, static_cast<float>(rc.getRight()),
                                        static_cast<float>(rc.getBottom()), false);

    g.setGradientFill(gradient);
    g.fillRect(rc);
}

//===============================================================================

class OSCBox : public gin::ParamBox
{
  public:
    OSCBox(PMProcessor &proc_, const PMProcessor::OSCParams &params_, const int num_)
        : gin::ParamBox(juce::String("  OSC ") += (num_ + 1)), proc(proc_), oscparams(params_), num(num_)
    {
        addControl(c1 = new APKnob(oscparams.coarse), 0, 0); // coarse
        addControl(f1 = new APKnob(oscparams.fine), 1, 0);   // fine
        addControl(v1 = new APKnob(oscparams.volume), 2, 0); // volume
        addControl(p1 = new APKnob(oscparams.phase), 2, 1);  // phase

        c1->setLookAndFeel(lnfs[num]);
        f1->setLookAndFeel(lnfs[num]);
        v1->setLookAndFeel(lnfs[num]);

        addControl(wave1 = new gin::Select(oscparams.wave));   // saw
        addControl(env1 = new gin::Select(oscparams.env));     // env select
        addControl(fixed1 = new gin::Select(oscparams.fixed)); // fixed
        watchParam(oscparams.fixed);
        watchParam(oscparams.env);
        watchParam(oscparams.coarse);
        watchParam(oscparams.fine);
        addAndMakeVisible(fixedHz1);
        setColour(juce::TextButton::buttonOnColourId, juce::Colours::beige);

        addAndMakeVisible(coarseLabel);
        coarseLabel.setJustificationType(juce::Justification::centredBottom);
        fixedHz1.setJustificationType(juce::Justification::centred);
    }

    ~OSCBox() override
    {
        c1->setLookAndFeel(nullptr);
        f1->setLookAndFeel(nullptr);
        v1->setLookAndFeel(nullptr);
    }

    class APLookAndFeel1 : public APLNF
    {
      public:
        APLookAndFeel1()
        {
            setColour(juce::Slider::rotarySliderFillColourId, APColors::red);
            setColour(juce::Slider::trackColourId, juce::Colours::black);
        }
    };
    class APLookAndFeel2 : public APLNF
    {
      public:
        APLookAndFeel2()
        {
            setColour(juce::Slider::rotarySliderFillColourId, APColors::yellow);
            setColour(juce::Slider::trackColourId, juce::Colours::black);
        }
    };
    class APLookAndFeel3 : public APLNF
    {
      public:
        APLookAndFeel3()
        {
            setColour(juce::Slider::rotarySliderFillColourId, APColors::green);
            setColour(juce::Slider::trackColourId, juce::Colours::black);
        }
    };
    class APLookAndFeel4 : public APLNF
    {
      public:
        APLookAndFeel4()
        {
            setColour(juce::Slider::rotarySliderFillColourId, APColors::blue);
            setColour(juce::Slider::trackColourId, juce::Colours::black);
        }
    };

    void paramChanged() override
    {
        gin::ParamBox::paramChanged();
        if (oscparams.fixed->isOn())
        {
            fixedHz1.setVisible(true);
            fixedHz1.setText(juce::String((oscparams.coarse->getUserValue() + oscparams.fine->getUserValue()) * 100, 2) + juce::String(" Hz"),
                             juce::dontSendNotification);
        }
        else
        {
            fixedHz1.setVisible(false);
        }
        auto coarseString = juce::String(oscparams.coarse->getUserValueInt());
        if (oscparams.fine->getUserValue() > 0.01f)
            coarseString += "+";
        else if (oscparams.fine->getUserValue() < -0.01f)
            coarseString += "-";
        else
            coarseString += "x";
        coarseLabel.setText(coarseString, juce::dontSendNotification);
        const int choice = oscparams.env->getUserValueInt();
        switch (num)
        {
        case 0:
            proc.env1osc1 = (choice == 0);
            proc.env2osc1 = (choice == 1);
            proc.env3osc1 = (choice == 2);
            proc.env4osc1 = (choice == 3);
            break;
        case 1:
            proc.env1osc2 = (choice == 0);
            proc.env2osc2 = (choice == 1);
            proc.env3osc2 = (choice == 2);
            proc.env4osc2 = (choice == 3);
            break;
        case 2:
            proc.env1osc3 = (choice == 0);
            proc.env2osc3 = (choice == 1);
            proc.env3osc3 = (choice == 2);
            proc.env4osc3 = (choice == 3);
            break;
        case 3:
            proc.env1osc4 = (choice == 0);
            proc.env2osc4 = (choice == 1);
            proc.env3osc4 = (choice == 2);
            proc.env4osc4 = (choice == 3);
            break;
        }
    }

    void resized() override
    {
        gin::ParamBox::resized();
        fixedHz1.setBounds(56, 23 + 70 + 10, 56, 15); // 23/70/10 header, first row, padding
        p1->setBounds(56 * 2, 70 * 1 + 23, 56, 70);
        wave1->setBounds(56 * 3, 93, 56, 70);
        env1->setBounds(56 * 3, 23, 56, 70);
        fixed1->setBounds(56, 128, 56, 35);
        coarseLabel.setBounds(4, 93, 52, 45);
    }

    APLookAndFeel1 lnf1;
    APLookAndFeel2 lnf2;
    APLookAndFeel3 lnf3;
    APLookAndFeel4 lnf4;
    std::array<APLNF *, 4> lnfs{&lnf1, &lnf2, &lnf3, &lnf4};
    PMProcessor &proc;
    gin::ParamComponent::Ptr c1, f1, v1, p1, wave1, env1, fixed1;
    const PMProcessor::OSCParams &oscparams;
    int num{0};
    juce::Label fixedHz1, coarseLabel;
};

class HeaderRect : public juce::Component
{
  public:
    HeaderRect() { setInterceptsMouseClicks(false, false); }
    void paint(juce::Graphics &g) override
    {
        g.setColour(color);
        g.fillRect(getLocalBounds());
    }
    juce::Colour color{juce::Colours::black};
};

//==============================================================================
class ENVBox : public gin::ParamBox, public juce::Timer
{
  public:
    ENVBox(PMProcessor &proc_, const PMProcessor::ENVParams &params_, int num_)
        : gin::ParamBox(juce::String("  ENV ") += (num_ + 1)), proc(proc_), num(num_), envparams(params_), pSelect(proc, *(proc.envSrcIds[num]))
    {
        // in reverse order
        switch (num_)
        {
        case 3:
            addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv4, true));
            break;
        case 2:
            addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv3, true));
            break;
        case 1:
            addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv2, true));
            break;
        case 0:
            addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv1, true));
            break;
        }
        addControl(a1 = new APKnob(envparams.attack), 0, 0);
        addControl(d1 = new APKnob(envparams.decay), 1, 0);
        addControl(s1 = new APKnob(envparams.sustain), 2, 0);
        addControl(r1 = new APKnob(envparams.release), 3, 0);
        addControl(ac1 = new APKnob(envparams.acurve, true), 4, 0);
        addControl(dc1 = new APKnob(envparams.drcurve, true), 5, 0);
        addControl(rpt1 = new gin::Select(envparams.syncrepeat), 4, 1);
        addControl(beats1 = new gin::Select(envparams.duration), 5, 1);
        addControl(rate1 = new APKnob(envparams.time), 5, 1);

        beats1->setVisible(false);
        rate1->setVisible(false);

        watchParam(envparams.syncrepeat);

        addAndMakeVisible(envViz);
        addAndMakeVisible(pSelect);
        pSelect.setText("+Dest", juce::dontSendNotification);

        addAndMakeVisible(o1);
        addAndMakeVisible(o2);
        addAndMakeVisible(o3);
        addAndMakeVisible(o4);
        o1.color = APColors::redMuted;
        o2.color = APColors::yellowMuted;
        o3.color = APColors::greenMuted;
        o4.color = APColors::blueMuted;

        envViz.phaseCallback = [this]() {
            switch (num)
            {
            case 0:
                return proc.synth.getENV1States();
            case 1:
                return proc.synth.getENV2States();
            case 2:
                return proc.synth.getENV3States();
            case 3:
                return proc.synth.getENV4States();
            }
        };
        startTimerHz(3);
    }

    void timerCallback() override
    {
        switch (num)
        {
        case 0:
            osc1 = proc.env1osc1;
            osc2 = proc.env1osc2;
            osc3 = proc.env1osc3;
            osc4 = proc.env1osc4;
            break;
        case 1:
            osc1 = proc.env2osc1;
            osc2 = proc.env2osc2;
            osc3 = proc.env2osc3;
            osc4 = proc.env2osc4;
            break;
        case 2:
            osc1 = proc.env3osc1;
            osc2 = proc.env3osc2;
            osc3 = proc.env3osc3;
            osc4 = proc.env3osc4;
            break;
        case 3:
            osc1 = proc.env4osc1;
            osc2 = proc.env4osc2;
            osc3 = proc.env4osc3;
            osc4 = proc.env4osc4;
            break;
        }
        o1.setVisible(osc1);
        o2.setVisible(osc2);
        o3.setVisible(osc3);
        o4.setVisible(osc4);
        repaint();
    }

    void paramChanged() override
    {
        gin::ParamBox::paramChanged();
        switch (envparams.syncrepeat->getUserValueInt())
        {
        case 0:
            beats1->setVisible(false);
            rate1->setVisible(false);
            break;
        case 1:
            beats1->setVisible(true);
            rate1->setVisible(false);
            break;
        case 2:
            beats1->setVisible(false);
            rate1->setVisible(true);
            break;
        }
        repaint();
    }

    void resized() override
    {
        gin::ParamBox::resized();
        envViz.setBounds(0, 23 + 70, 56 * 4, 70);
        pSelect.setBounds(getWidth() - 75, 4, 56, 16);
        o1.setBounds(50, 4, 16, 16);
        o2.setBounds(68, 4, 16, 16);
        o3.setBounds(86, 4, 16, 16);
        o4.setBounds(104, 4, 16, 16);
    }

    PMProcessor &proc;
    gin::ParamComponent::Ptr a1, d1, s1, r1, ac1, dc1, rpt1, beats1, rate1;
    bool osc1{false}, osc2{false}, osc3{false}, osc4{false};
    HeaderRect o1, o2, o3, o4;

    int num;
    EnvelopeComponent envViz{proc, num + 1};
    const PMProcessor::ENVParams &envparams;
    ParameterSelector pSelect;
};

//==============================================================================
class TimbreBox : public gin::ParamBox
{
  public:
    TimbreBox(const juce::String &name, const PMProcessor &proc) : gin::ParamBox(name)
    {
        setName(name);
        addControl(new APKnob(proc.timbreParams.algo), 0, 0);
        addControl(new APKnob(proc.globalParams.modIndex), 1, 0);
        addControl(new APKnob(proc.globalParams.modTone), 2, 0);
        addControl(new gin::Select(proc.globalParams.modfm), 0, 1);
        addControl(new APKnob(proc.timbreParams.pitch), 1, 1);
        addControl(new APKnob(proc.globalParams.velSens), 2, 1);
    }
};

class AlgoBox : public gin::ParamBox
{
  public:
    AlgoBox(const juce::String &name, PMProcessor &proc_) : gin::ParamBox(name), proc(proc_)
    {
        setName("algo");
        setTitle(name);
        // addAndMakeVisible(funcImage);
        // funcImage.setImage(juce::ImageCache::getFromMemory(BinaryData::algo_svg, BinaryData::algo_svgSize));
        watchParam(proc.timbreParams.algo);
    }
    PMProcessor &proc;
    // arrows
    bool a43{false}, a42{false}, a41{false}, a32{false}, a31{false}, a21{false};
    // voices
    bool v1{false}, v2{false}, v3{false}, v4{false};

    void paramChanged() override
    {
        gin::ParamBox::paramChanged();
        int algo = proc.timbreParams.algo->getUserValueInt();
        switch (algo)
        {
        case 0:
        {
            a43 = a32 = a21 = true;
            a42 = a41 = a31 = false;
            v1 = true;
            v2 = v3 = v4 = false;
        }
        break;
        case 1:
        {
            a42 = a32 = a21 = true;
            a43 = a41 = a31 = false;
            v1 = true;
            v4 = v3 = v2 = false;
        }
        break;
        case 2:
        {
            a41 = a32 = a21 = true;
            a43 = a42 = a31 = false;
            v1 = true;
            v4 = v3 = v2 = false;
        }
        break;
        case 3:
        {
            a43 = a42 = a31 = a21 = true;
            a41 = a32 = false;
            v1 = true;
            v4 = v3 = v2 = false;
        }
        break;
        case 4:
        {
            a43 = a32 = a31 = true;
            a42 = a41 = a21 = false;
            v1 = v2 = true;
            v4 = v3 = false;
        }
        break;
        case 5:
        {
            a43 = a32 = true;
            a42 = a41 = a31 = a21 = false;
            v1 = v2 = true;
            v4 = v3 = false;
        }
        break;
        case 6:
        {
            a41 = a31 = a21 = true;
            a43 = a42 = a32 = false;
            v1 = true;
            v4 = v3 = v2 = false;
        }
        break;
        case 7:
        {
            a43 = a21 = true;
            a42 = a41 = a32 = a31 = false;
            v1 = v3 = true;
            v2 = v4 = false;
        }
        break;
        case 8:
        {
            a43 = a42 = a41 = true;
            a32 = a31 = a21 = false;
            v1 = v2 = v3 = true;
            v4 = false;
        }
        break;
        case 9:
        {
            a43 = true;
            a42 = a41 = a32 = a31 = a21 = false;
            v1 = v2 = v3 = true;
            v4 = false;
        }
        break;
        case 10:
        {
            a43 = a42 = a41 = a32 = a31 = a21 = false;
            v4 = v3 = v2 = v1 = true;
        }
        break;
        }
    }

    void resized() override
    {
        gin::ParamBox::resized();
        // funcImage.setBounds(0, 26, 110, 140);
    }

    void paint(juce::Graphics &g) override
    {
        gin::ParamBox::paint(g);
        juce::Point ctr4{29, 119};
        juce::Point ctr3{81, 119};
        juce::Point ctr2{81, 67};
        juce::Point ctr1{29, 67};
        // 1
        g.setColour(APColors::redMuted);
        g.fillRoundedRectangle(6, 44, 46, 46, 5);
        juce::ColourGradient gradient{juce::Colours::white.withAlpha(0.6f),
                                      {ctr1.getX() + 0.f, ctr1.getY() + 0.f},
                                      APColors::redMuted,
                                      {ctr1.getX() - 9.f, ctr1.getY() - 9.f},
                                      true};
        g.setGradientFill(gradient);
        g.fillEllipse({ctr1.getX() - 20.f, ctr1.getY() - 20.f, 40.f, 40.f});

        // 2
        g.setColour(APColors::yellowMuted);
        g.fillRoundedRectangle(58, 44, 46, 46, 5);
        if (v2)
        {
            juce::ColourGradient gradient{juce::Colours::white.withAlpha(0.6f),
                                          {ctr2.getX() + 0.f, ctr2.getY() + 0.f},
                                          APColors::yellowMuted,
                                          {ctr2.getX() - 9.f, ctr2.getY() - 9.f},
                                          true};
            g.setGradientFill(gradient);
            g.fillEllipse({ctr2.getX() - 20.f, ctr2.getY() - 20.f, 40.f, 40.f});
        }

        // 4
        g.setColour(APColors::blueMuted);
        g.fillRoundedRectangle(6, 96, 46, 46, 5);
        if (v4)
        {
            juce::ColourGradient gradient{juce::Colours::white.withAlpha(0.6f),
                                          {ctr4.getX() + 0.f, ctr4.getY() + 0.f},
                                          APColors::blueMuted,
                                          {ctr4.getX() - 9.f, ctr4.getY() - 9.f},
                                          true};
            g.setGradientFill(gradient);
            g.fillEllipse({ctr4.getX() - 20.f, ctr4.getY() - 20.f, 40.f, 40.f});
        }

        // 3
        g.setColour(APColors::greenMuted);
        g.fillRoundedRectangle(58, 96, 46, 46, 5);
        if (v3)
        {
            juce::ColourGradient gradient{juce::Colours::white.withAlpha(0.6f),
                                          {ctr3.getX() + 0.f, ctr3.getY() + 0.f},
                                          APColors::greenMuted,
                                          {ctr3.getX() - 9.f, ctr3.getY() - 9.f},
                                          true};
            g.setGradientFill(gradient);
            g.fillEllipse({ctr3.getX() - 20.f, ctr3.getY() - 20.f, 40.f, 40.f});
        }

        // arrows

        // clang-format off
        if (a43)
        {
            g.setColour(APColors::blueMuted);
            g.drawArrow(  juce::Line<float>{ctr4.getX() + 7.f, ctr4.getY()+0.f, ctr3.getX() - 7.f, ctr3.getY()+0.f}, 8.f, 16.f, 16.f);
            juce::Path path;
            path.addArrow(juce::Line<float>{ctr4.getX() + 7.f, ctr4.getY()+0.f, ctr3.getX() - 7.f, ctr3.getY()+0.f}, 10.f, 18.f, 18.f);
            g.setColour(juce::Colours::black);
            g.strokePath(path, juce::PathStrokeType(3.f));
        }
        if (a41)
        {
            g.setColour(APColors::blueMuted);
            g.drawArrow(  juce::Line<float>{ctr4.getX()+0.f, ctr4.getY() - 7.f, ctr1.getX()+0.f, ctr1.getY() + 7.f}, 8.f, 16.f, 16.f);
            juce::Path path;
            path.addArrow(juce::Line<float>{ctr4.getX()+0.f, ctr4.getY() - 7.f, ctr1.getX()+0.f, ctr1.getY() + 7.f}, 10.f, 18.f, 18.f);
            g.setColour(juce::Colours::black);
            g.strokePath(path, juce::PathStrokeType(3.f));
        }
        if (a32)
        {
            g.setColour(APColors::greenMuted);
            g.drawArrow(  juce::Line<float>{ctr3.getX()+0.f, ctr3.getY() - 7.f, ctr2.getX()+0.f, ctr2.getY() + 7.f}, 8.f, 16.f, 16.f);
            juce::Path path;
            path.addArrow(juce::Line<float>{ctr3.getX()+0.f, ctr3.getY() - 7.f, ctr2.getX()+0.f, ctr2.getY() + 7.f}, 10.f, 18.f, 18.f);
            g.setColour(juce::Colours::black);
            g.strokePath(path, juce::PathStrokeType(3.f));
        }
        if (a21) 
        {
            g.setColour(APColors::yellowMuted);
            g.drawArrow(  juce::Line<float>{ctr2.getX() - 7.f, ctr2.getY()+0.f, ctr1.getX() + 7.f, ctr1.getY()+0.f}, 8.f, 16.f, 16.f);
            juce::Path path;
            path.addArrow(juce::Line<float>{ctr2.getX() - 7.f, ctr2.getY()+0.f, ctr1.getX() + 7.f, ctr1.getY()+0.f}, 10.f, 18.f, 18.f);
            g.setColour(juce::Colours::black);
            g.strokePath(path, juce::PathStrokeType(3.f));
        }
        if (a42) 
        {
            g.setColour(APColors::blueMuted);
            g.drawArrow(  juce::Line<float>{ctr4.getX() + 7.f, ctr4.getY() - 7.f, ctr2.getX() - 7.f, ctr2.getY() + 7.f}, 8.f, 16.f, 16.f);
            juce::Path path;
            g.setColour(juce::Colours::black);
            path.addArrow(juce::Line<float>{ctr4.getX() + 7.f, ctr4.getY() - 7.f, ctr2.getX() - 7.f, ctr2.getY() + 7.f}, 10.f, 18.f, 18.f);
            g.strokePath(path, juce::PathStrokeType(3.f));
        }
        if (a31) 
        {
            g.setColour(APColors::greenMuted);
            g.drawArrow(  juce::Line<float>{ctr3.getX() - 7.f, ctr3.getY() - 7.f, ctr1.getX() + 7.f, ctr1.getY() + 7.f}, 8.f, 16.f, 16.f);
            juce::Path path;
            path.addArrow(juce::Line<float>{ctr3.getX() - 7.f, ctr3.getY() - 7.f, ctr1.getX() + 7.f, ctr1.getY() + 7.f}, 10.f, 18.f, 18.f);
            g.setColour(juce::Colours::black);
            g.strokePath(path, juce::PathStrokeType(3.f));
        }
    }

    // clang-format on
};

class LFOMain : public gin::ParamBox
{
  public:
    LFOMain(PMProcessor &proc_, const PMProcessor::LFOParams &lfoParams_, int num_)
        : ParamBox(juce::String("  LFO ") += juce::String(num_)), proc(proc_), lfoParams(lfoParams_), num(num_),
          monoSelect(proc, *(proc.monoLfoIds[num - 1])), polySelect(proc, *(proc.polyLfoIds[num - 1]))
    {
        setName("lfo");
        addAndMakeVisible(monoSelect);
        addAndMakeVisible(polySelect);
        monoSelect.setText("+M", juce::dontSendNotification);
        polySelect.setText("+P", juce::dontSendNotification);

        gin::ModSrcId src;
        gin::ModSrcId monoSrc;
        switch (num)
        {
        case 1:
            src = proc.modSrcLFO1;
            monoSrc = proc.modSrcMonoLFO1;
            break;
        case 2:
            src = proc.modSrcLFO2;
            monoSrc = proc.modSrcMonoLFO2;
            break;
        case 3:
            src = proc.modSrcLFO3;
            monoSrc = proc.modSrcMonoLFO3;
            break;
        case 4:
            src = proc.modSrcLFO4;
            monoSrc = proc.modSrcMonoLFO4;
            break;
        }
        addModSource(poly1 = new gin::ModulationSourceButton(proc.modMatrix, src, true));
        addModSource(mono1 = new gin::ModulationSourceButton(proc.modMatrix, monoSrc, false));
        poly1->getProperties().set("polysrc", true);

        addControl(r1 = new APKnob(lfoParams.rate), 0, 0);
        addControl(b1 = new gin::Select(lfoParams.beat), 0, 0);

        addControl(s1 = new gin::Select(lfoParams.sync));

        addControl(w1 = new gin::Select(lfoParams.wave));

        addControl(dp1 = new APKnob(lfoParams.depth, true), 1, 0);

        addControl(o1 = new APKnob(lfoParams.offset, true), 1, 1);

        addControl(dl1 = new APKnob(lfoParams.delay), 2, 0);

        addControl(f1 = new APKnob(lfoParams.fade, true), 2, 1);

        watchParam(lfoParams.sync);
    }

    void paramChanged() override
    {
        gin::ParamBox::paramChanged();
        r1->setVisible(!lfoParams.sync->isOn());
        b1->setVisible(lfoParams.sync->isOn());
    }

    void resized() override
    {
        gin::ParamBox::resized();
        s1->setBounds(0, 93, 56, 35);
        w1->setBounds(0, 128, 56, 35);
        monoSelect.setBounds(3 * 56 - 85, 3, 32, 16);
        polySelect.setBounds(3 * 56 - 61, 3, 32, 16);
    }

    PMProcessor &proc;

    gin::ParamComponent::Ptr s1, w1, r1, b1, dp1, o1, f1, dl1;

    juce::Button *poly1, *mono1;

    int currentLFO{1};
    const PMProcessor::LFOParams &lfoParams;
    int num;
    ParameterSelector monoSelect, polySelect;
};

//==============================================================================
class FilterBox : public gin::ParamBox
{
  public:
    FilterBox(const juce::String &name, PMProcessor &proc_) : gin::ParamBox(name), proc(proc_)
    {
        setName("flt");
        setTitle("  filter");

        const auto &flt = proc.filterParams;

        const auto freq = new APKnob(flt.frequency);
        addControl(freq, 0, 0);
        addControl(new APKnob(flt.resonance), 1, 0);

        addControl(new APKnob(flt.keyTracking), 0, 1);
        addControl(new gin::Select(flt.type), 1, 1);

        freq->setLiveValuesCallback([this]() {
            if (proc.filterParams.keyTracking->getUserValue() != 0.0f ||
                proc.modMatrix.isModulated(gin::ModDstId(proc.filterParams.frequency->getModIndex())))
                return proc.getLiveFilterCutoff();
            return juce::Array<float>();
        });
    }

    PMProcessor &proc;
};

//==============================================================================
class ModBox : public gin::ParamBox
{
  public:
    ModBox(const juce::String &name, PMProcessor &proc_) : gin::ParamBox(name), proc(proc_)
    {
        setName("mod");
        setTitle("  mod sources");
        addControl(modlist = new gin::ModSrcListBox(proc.modMatrix), 0.f, 0.f, 5.f, 4.3f);
        modlist->setRowHeight(20);
    }

    gin::ModSrcListBox *modlist;
    PMProcessor &proc;
};

class LevelBox final : public gin::ParamBox, public juce::Timer
{
  public:
    explicit LevelBox(gin::LevelTracker &level_) : gin::ParamBox("  level"), levelMeter(level_, juce::NormalisableRange<float>{-60, 0}, true)
    {
        // startTimerHz(30);
        addAndMakeVisible(levelMeter);
    }
    void timerCallback() override
    {
        // repaint();
    }
    void paint(juce::Graphics &g) override
    {
        gin::ParamBox::paint(g);
        levelMeter.setBounds(getLocalBounds().withTrimmedTop(23).reduced(10));
        levelMeter.paint(g);
        g.setColour(juce::Colours::black);
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colours::white);
        g.drawText("Level", 0, 0, getWidth(), 20, juce::Justification::centred, true);
        g.setColour(findColour(gin::PluginLookAndFeel::title1ColourId));
        g.fillRect(getWidth() - 1, 0, 1, getHeight());
    }

    void resized() override
    {
        gin::ParamBox::resized();
        levelMeter.setBounds(getLocalBounds().withTrimmedTop(23).reduced(10));
    }
    APLevelMeter levelMeter;
};

//==============================================================================
class GlobalBox : public gin::ParamBox
{
  public:
    GlobalBox(const juce::String &name, PMProcessor &proc_) : gin::ParamBox(name), proc(proc_)
    {
        setName("global");

        addControl(new APKnob(proc.globalParams.glideRate), 1, 1);
        addControl(new APKnob(proc.globalParams.pitchbendRange), 0, 1);

        addControl(new gin::Select(proc.globalParams.mono), 0, 0);
        addControl(new gin::Select(proc.globalParams.legato), 1, 0);
        addControl(new gin::Select(proc.globalParams.glideMode), 2, 0);
        addControl(new gin::Select(proc.globalParams.mpe), 2, 1);
    }

    PMProcessor &proc;
};

//==============================================================================

class MainMatrixBox : public gin::ParamBox
{
  public:
    MainMatrixBox(const juce::String &name, PMProcessor &proc_) : gin::ParamBox(name), proc(proc_)
    {
        setName("mtx");

        addControl(new APModMatrixBox(proc, proc.modMatrix), 0.f, 0.f, 5.f, 4.3f);
        addAndMakeVisible(clearAllButton);
        clearAllButton.onClick = [this] { clearAll(); };
    }

    void resized() override
    {
        gin::ParamBox::resized();
        clearAllButton.setBounds(getWidth() - 60, 0, 55, 23);
    }

    void clearAll() const
    {
        auto &pluginParams = proc.getPluginParameters();
        for (auto *param : pluginParams)
        {
            if (param->getModIndex() == -1)
                continue;
            if (proc.modMatrix.isModulated(gin::ModDstId(param->getModIndex())))
            {
                auto modSrcs = proc.modMatrix.getModSources(param);
                for (const auto &modSrc : modSrcs)
                {
                    proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
                }
            }
        }
    }

    juce::TextButton clearAllButton{"Clear All"};

    PMProcessor &proc;
};

//==============================================================================
//==============================================================================

class MacrosBox final : public gin::ParamBox
{
  public:
    explicit MacrosBox(PMProcessor &proc_) : gin::ParamBox("  macros"), proc(proc_)
    {
        setName("macros");

        addControl(new APKnob(proc.macroParams.macro1), 0, 0);
        addControl(new APKnob(proc.macroParams.macro2), 1, 0);
        addControl(new APKnob(proc.macroParams.macro3), 2, 0);

        addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.macroSrc3, true));
        addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.macroSrc2, true));
        addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.macroSrc1, true));

        addAndMakeVisible(midiLearnButton1);
        addAndMakeVisible(midiLearnButton2);
        addAndMakeVisible(midiLearnButton3);
        addChildComponent(clear1);
        addChildComponent(clear2);
        addChildComponent(clear3);

        proc.macroParams.macro1cc->addListener(this);
        proc.macroParams.macro2cc->addListener(this);
        proc.macroParams.macro3cc->addListener(this);

        clear1.onClick = [this]() { cancelAssignment(1); };
        clear2.onClick = [this]() { cancelAssignment(2); };
        clear3.onClick = [this]() { cancelAssignment(3); };

        midiLearnButton1.setMacroNumber(1);
        midiLearnButton2.setMacroNumber(2);
        midiLearnButton3.setMacroNumber(3);
    }

    void cancelAssignment(int macroNumber)
    {
        switch (macroNumber)
        {
        case 1:
            proc.macroParams.macro1cc->setValue(-1.f);
            midiLearnButton1.setCCString("Learn");
            midiLearnButton1.setLearning(false);
            clear1.setVisible(false);
            break;
        case 2:
            proc.macroParams.macro2cc->setValue(-1.f);
            midiLearnButton2.setCCString("Learn");
            midiLearnButton2.setLearning(false);
            clear2.setVisible(false);
            break;
        case 3:
            proc.macroParams.macro3cc->setValue(-1.f);
            midiLearnButton3.setCCString("Learn");
            midiLearnButton3.setLearning(false);
            clear3.setVisible(false);
            break;
        }
    }
    void valueUpdated(gin::Parameter *p) override
    {
        if (p == proc.macroParams.macro1cc)
        {
            const auto ccValue = proc.macroParams.macro1cc->getUserValueInt();
            if (ccValue >= 0)
            {
                midiLearnButton1.setCCString("CC " + proc.macroParams.macro1cc->getUserValueText());
                clear1.setVisible(true);
            }
            else
            {
                midiLearnButton1.setCCString("Learn");
                midiLearnButton1.setLearning(false);
                clear1.setVisible(false);
            }
        }
        if (p == proc.macroParams.macro2cc)
        {
            const auto ccValue = proc.macroParams.macro2cc->getUserValueInt();
            if (ccValue >= 0)
            {
                midiLearnButton2.setCCString("CC " + proc.macroParams.macro2cc->getUserValueText());
                clear2.setVisible(true);
            }
            else
            {
                midiLearnButton2.setCCString("Learn");
                midiLearnButton2.setLearning(false);
                clear2.setVisible(false);
            }
        }
        if (p == proc.macroParams.macro3cc)
        {
            const auto ccValue = proc.macroParams.macro3cc->getUserValueInt();
            if (ccValue >= 0)
            {
                midiLearnButton3.setCCString("CC " + proc.macroParams.macro3cc->getUserValueText());
                clear3.setVisible(true);
            }
            else
            {
                midiLearnButton3.setCCString("Learn");
                midiLearnButton3.setLearning(false);
                clear3.setVisible(false);
            }
        }
    }

    void resized() override
    {
        gin::ParamBox::resized();
        midiLearnButton1.setBounds(0, 93, 56, 35);
        midiLearnButton2.setBounds(56, 93, 56, 35);
        midiLearnButton3.setBounds(112, 93, 56, 35);
        clear1.setBounds(0, 128, 56, 35);
        clear2.setBounds(56, 128, 56, 35);
        clear3.setBounds(112, 128, 56, 35);
    }

    class MIDILearnButton final : public juce::Label
    {
      public:
        explicit MIDILearnButton(PMProcessor &p) : proc(p)
        {
            setEditable(false, false, false);
            setJustificationType(juce::Justification::left);
            setText("Learn", juce::dontSendNotification);
            setLookAndFeel(&midilearnLNF);
        }

        ~MIDILearnButton() override { setLookAndFeel(nullptr); }

        void mouseDown(const juce::MouseEvent & /*ev*/) override
        {
            if (thisMacroNumber == 1)
            {
                if (proc.macroParams.macro1cc->getUserValue() > 0.f)
                {
                    return;
                }
            }
            if (thisMacroNumber == 2)
            {
                if (proc.macroParams.macro2cc->getUserValue() > 0.f)
                {
                    return;
                }
            }
            if (thisMacroNumber == 3)
            {
                if (proc.macroParams.macro3cc->getUserValue() > 0.f)
                {
                    return;
                }
            }
            learning = !learning;
            if (learning)
            {
                proc.macroParams.learning->setUserValue(static_cast<float>(thisMacroNumber));
                setText("Learning", juce::dontSendNotification);
            }
            else
            {
                proc.macroParams.learning->setValue(0.0f);
                setText("Learn", juce::dontSendNotification);
            }
        }

        inline void setMacroNumber(int n) { thisMacroNumber = n; }

        inline void setCCString(const juce::String &s)
        {
            currentAssignment = s;
            setText(s, juce::dontSendNotification);
        }

        inline void setLearning(bool shouldLearn) { learning = shouldLearn; }

        class MIDILearnLNF final : public APLNF
        {
          public:
            MIDILearnLNF() = default;

            void drawLabel(juce::Graphics &g, juce::Label &label) override
            {
                const auto rc = label.getLocalBounds();
                g.setColour(juce::Colours::white);
                g.setFont(14.f);
                g.drawText(label.getText(), rc, juce::Justification::centred);
            }

        } midilearnLNF;

        juce::String currentAssignment;
        PMProcessor &proc;
        bool learning{false};
        int mididCC{-1}, thisMacroNumber{0};
    }; // MIDILearnButton

    PMProcessor &proc;
    MIDILearnButton midiLearnButton1{proc}, midiLearnButton2{proc}, midiLearnButton3{proc};
    juce::TextButton clear1{"Clear", "Clear"}, clear2{"Clear", "Clear"}, clear3{"Clear", "Clear"};
}; // MacrosBox

//==============================================================================

class VolumeBox final : public gin::ParamBox
{
  public:
    explicit VolumeBox(PMProcessor &proc_) : gin::ParamBox("  vol"), proc(proc_)
    {
        setName("  vol");
        level = new gin::PluginSlider(proc.globalParams.level, juce::Slider::LinearBarVertical, juce::Slider::NoTextBox);
        addAndMakeVisible(level);
    }

    ~VolumeBox() override { delete level; }

    void paint(juce::Graphics &g) override
    {
        // clang-format off
        gin::ParamBox::paint(g);
        g.setColour(juce::Colours::white.darker(0.2f));
        int w = getWidth();
        int gw = w - 4;
        g.fillRect(    2,  39,  4, 1); // +6 dB
        g.fillRect(w - 6,  39,  4, 1);
        g.fillRect(    2,  54, gw, 2); // 0
        g.fillRect(    2,  70,  4, 1); // -6
        g.fillRect(w - 6,  70,  4, 1);
        g.fillRect(    2,  86,  4, 1); // -12
        g.fillRect(w - 6,  86,  4, 1);
        g.fillRect(    2, 102,  4, 1); // -18
        g.fillRect(w - 6, 102,  4, 1);
        g.fillRect(    2, 118,  4, 1); // -24
        g.fillRect(w - 6, 118,  4, 1);
        g.fillRect(    2, 134,  4, 1); // -30
        g.fillRect(w - 6, 134,  4, 1);
        g.fillRect(    2, 150,  4, 1); // -36
        g.fillRect(w - 6, 150,  4, 1);

        // clang-format on
    }

    void resized() override
    {
        gin::ParamBox::resized();
        level->setBounds(0, 23, 56, 140);
    }

    PMProcessor &proc;
    gin::PluginSlider *level;
};
