/*
 * PM Daze - an expressive, semi-modular, phase-modulation synthesizer
 *
 * Copyright 2025, Greg Recco
 *
 * PM Daze is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source code for PM Daze is available at
 * https://github.com/gregrecco67/PMDaze
 */
#pragma once

#include <gin_dsp/gin_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "PMVoice.h"

class PMProcessor;

class PMSynth : public gin::Synthesiser
{
  public:
    explicit PMSynth(PMProcessor &proc_);
    ~PMSynth() override = default;

    void handleMidiEvent(const juce::MidiMessage &m) override;

    inline juce::Array<float> getLiveFilterCutoff() const
    {
        juce::Array<float> values;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                values.add(vav->getFilterCutoffNormalized());
            }
        }
        return values;
    }

    void shutItDown()
    {
        for (auto v : voices)
        {
            if (v->isActive())
            {
                auto vav = dynamic_cast<PMVoice *>(v);
                vav->setFastKill();
            }
        }
    }

    inline std::vector<float> getMSEG1Phases() const
    {
        std::vector<float> values;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                values.push_back(vav->getMSEG1Phase());
            }
        }
        return values;
    }

    inline std::vector<float> getMSEG2Phases() const
    {
        std::vector<float> values;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                values.push_back(vav->getMSEG2Phase());
            }
        }
        return values;
    }

    inline std::vector<float> getMSEG3Phases() const
    {
        std::vector<float> values;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                values.push_back(vav->getMSEG3Phase());
            }
        }
        return values;
    }

    inline std::vector<float> getMSEG4Phases() const
    {
        std::vector<float> values;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                values.push_back(vav->getMSEG4Phase());
            }
        }
        return values;
    }

    inline std::vector<float> getLFO1Phases() const
    {
        std::vector<float> values;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                values.push_back(vav->getLFO1Phase());
            }
        }
        return values;
    }

    inline std::vector<float> getLFO2Phases() const
    {
        std::vector<float> values;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                values.push_back(vav->getLFO2Phase());
            }
        }
        return values;
    }

    inline std::vector<float> getLFO3Phases() const
    {
        std::vector<float> values;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                values.push_back(vav->getLFO3Phase());
            }
        }
        return values;
    }

    inline std::vector<float> getLFO4Phases() const
    {
        std::vector<float> values;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                values.push_back(vav->getLFO4Phase());
            }
        }
        return values;
    }

    inline std::vector<Envelope::EnvelopeState> getENV1States() const
    {
        std::vector<Envelope::EnvelopeState> states;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                states.push_back(vav->getENV1State());
            }
        }
        return states;
    }

    inline std::vector<Envelope::EnvelopeState> getENV2States() const
    {
        std::vector<Envelope::EnvelopeState> states;

        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                states.push_back(vav->getENV2State());
            }
        }
        return states;
    }

    inline std::vector<Envelope::EnvelopeState> getENV3States() const
    {
        std::vector<Envelope::EnvelopeState> states;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                states.push_back(vav->getENV3State());
            }
        }
        return states;
    }

    inline std::vector<Envelope::EnvelopeState> getENV4States() const
    {
        std::vector<Envelope::EnvelopeState> states;
        for (const auto v : voices)
        {
            if (v->isActive())
            {
                const auto vav = dynamic_cast<PMVoice *>(v);
                states.push_back(vav->getENV4State());
            }
        }
        return states;
    }

  private:
    PMProcessor &proc;
};
