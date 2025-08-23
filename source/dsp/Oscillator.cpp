#include "Oscillator.h"

#include "PMVoice.h"

PMOscillator::PMOscillator(gin::BandLimitedLookupTables &bllt_) : bllt(bllt_)
{
    setSampleRate(sampleRate); // bllt.setSampleRate(sampleRate);
}

float PMOscillator::w(gin::Wave wave, float phase, float freq, bool isMod) {
    float out = bllt.process(wave, freq, phase);
	return isMod ? std::exp(out) * 0.850918 - 1.313035 : out;
}