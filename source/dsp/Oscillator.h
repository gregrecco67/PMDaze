#pragma once
#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>
#include <cmath>

struct Position;
class PMOscillator // : public gin::StereoOscillator
{
  public:
    PMOscillator(gin::BandLimitedLookupTables &bllt_);
    ~PMOscillator() = default;

    inline void noteOn(float p = -1)
    {
        if (p >= 0.0f)
            phase = p;
        else
            phase = 0.0f;
    }

    inline void setSampleRate(const double sr)
    {
        sampleRate = sr;
        invSampleRate = 1.0f / sampleRate;
    }

    inline void bumpPhase(const float bump)
    {
        phase += bump;
        while (phase < 0.0f)
        {
            phase += 1.0f;
        }
        phase -= std::trunc(phase);
    }

    float w(gin::Wave wave, float phase, float freq, bool isMod = false);
    
    gin::BandLimitedLookupTables &bllt;
    float sampleRate = 44100.0f;
    float invSampleRate = 1.0f / sampleRate;
    float phase = 0.0f;
};
