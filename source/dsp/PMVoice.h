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

#pragma once

#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <numbers>
#include <random>
#include "Envelope.h"
#include "MTS-ESP/libMTSClient.h"
#include "Oscillator.h"
class PMProcessor;

using std::numbers::pi_v;

//==============================================================================
class PMVoice final : public gin::SynthesiserVoice, public gin::ModVoice
{
  public:
    explicit PMVoice(PMProcessor &p);

    void noteStarted() override;
    void noteRetriggered() override;
    void noteStopped(bool allowTailOff) override;

    void notePressureChanged() override;
    void noteTimbreChanged() override;
    void notePitchbendChanged() override {}
    void noteKeyStateChanged() override {}

    void setCurrentSampleRate(double newRate) override;

    void renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples) override;

    float getCurrentNote() override { return noteSmoother.getCurrentValue() * 127.0f; }

    bool isVoiceActive() override { return isActive(); }

    float getFilterCutoffNormalized() const;
    [[nodiscard]] inline float getMSEG1Phase() const { return mseg1.getCurrentPhase(); }
    [[nodiscard]] inline float getMSEG2Phase() const { return mseg2.getCurrentPhase(); }
    [[nodiscard]] inline float getMSEG3Phase() const { return mseg3.getCurrentPhase(); }
    [[nodiscard]] inline float getMSEG4Phase() const { return mseg4.getCurrentPhase(); }
    inline float getLFO1Phase() { return lfo1.getCurrentPhase(); }
    inline float getLFO2Phase() { return lfo2.getCurrentPhase(); }
    inline float getLFO3Phase() { return lfo3.getCurrentPhase(); }
    inline float getLFO4Phase() { return lfo4.getCurrentPhase(); }

    [[nodiscard]] inline Envelope::EnvelopeState getENV1State() const { return env1.getState(); }
    [[nodiscard]] inline Envelope::EnvelopeState getENV2State() const { return env2.getState(); }
    [[nodiscard]] inline Envelope::EnvelopeState getENV3State() const { return env3.getState(); }
    [[nodiscard]] inline Envelope::EnvelopeState getENV4State() const { return env4.getState(); }

    gin::Wave waveForChoice(const int choice);

    struct Averager
    {
        static float in;
        float previous{0.f};
        float process(float x) { return previous = (x * in + previous * (1.0f - in)); }
    };

    float sine(float x);
    float wave(int sel, float x, bool isMod = false);

  private:
    void updateParams(int blockSize);

    PMProcessor &proc;

    gin::Filter filter;
    gin::LFO lfo1, lfo2, lfo3, lfo4;
    gin::MSEG mseg1, mseg2, mseg3, mseg4;
    gin::MSEG::Parameters mseg1Params, mseg2Params, mseg3Params, mseg4Params;
    Envelope env1, env2, env3, env4;
    std::array<Envelope *, 4> envs{&env1, &env2, &env3, &env4};
    std::array<Envelope *, 4> envsByNum{&env1, &env2, &env3, &env4};

    int filterType{0};
    double freq1 = 0.0, freq2 = 0.0, freq3 = 0.0, freq4 = 0.0;
    double freq4factor = 1.0f, freq3factor = 1.0f, freq2factor = 1.0f;
    float vol1 = 0.0f, vol2 = 0.0f, vol3 = 0.0f, vol4 = 0.0f;
    double phase1 = 0.0, phase2 = 0.0, phase3 = 0.0, phase4 = 0.0;
    double phase = 0.0;

    int w1{0}, w2{0}, w3{0}, w4{0};
    Averager a4, a3, a2;
    int algo{0};
    float modIndex{4.f}; // was 25 for [0, 2pi] range, switched now to [0, 1] for faster rollover

    // =================================================
    // above: we need it
    // below: do we?
    // =================================================

    int tilUpdate{0}; // only update envelopes/lfo/mseg every 4th block

    float currentMidiNote = -1;

    static constexpr float baseAmplitude = 0.5f;

    gin::EasedValueSmoother<float> noteSmoother;

    double fna0, fnb1, fnz1, fqa0, fqb1, fqz1; //

    juce::AudioBuffer<float> synthBuffer;

    float antipop{0.f};

    friend class PMSynth;
    juce::MPENote curNote;

    std::random_device rd;
    std::mt19937 gen{rd()};
    std::uniform_real_distribution<> dist{-1.f, 1.f};
    const float maxFreq{20000.f};
};
