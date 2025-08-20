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

// #define MIPP_ALIGNED_LOADS
#include "PMProcessor.h"
#include "PMVoice.h"

// inline std::array<float, 2> PMVoice::panWeights(const float in) { // -1
// to 1 	return { std::sqrt((in + 1.f) * 0.5f), std::sqrt(1.f - ((in + 1.f) *
// 0.5f)) };
// }

float PMVoice::Averager::in;

//==============================================================================
PMVoice::PMVoice(PMProcessor &p)
    : proc(p), mseg1(proc.mseg1Data), mseg2(proc.mseg2Data), mseg3(proc.mseg3Data), mseg4(proc.mseg4Data), env1(p.convex), env2(p.convex),
      env3(p.convex), env4(p.convex)
{
    mseg1.reset();
    mseg2.reset();
    mseg3.reset();
    mseg4.reset();
    filter.setNumChannels(2);
}

void PMVoice::noteStarted()
{
    antipop = 0.f; // ramp up

    curNote = getCurrentlyPlayingNote();

    proc.modMatrix.setPolyValue(*this, proc.randSrc1Poly, static_cast<float>(dist(gen)));
    proc.modMatrix.setPolyValue(*this, proc.randSrc2Poly, static_cast<float>(dist(gen)));

    if (MTS_ShouldFilterNote(proc.client, static_cast<char>(curNote.initialNote), static_cast<char>(curNote.midiChannel)))
    {
        return;
    }

    fastKill = false;
    startVoice();

    const auto note = getCurrentlyPlayingNote();
    if (glideInfo.fromNote >= 0 && (glideInfo.glissando || glideInfo.portamento))
    {
        noteSmoother.setTime(glideInfo.rate);
        noteSmoother.setValueUnsmoothed(glideInfo.fromNote / 127.0f);
        noteSmoother.setValue(note.initialNote / 127.0f);
    }
    else
    {
        noteSmoother.setValueUnsmoothed(note.initialNote / 127.0f);
    }

    proc.modMatrix.setPolyValue(*this, proc.modSrcVelocity, note.noteOnVelocity.asUnsignedFloat());
    proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
    proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());

    juce::ScopedValueSetter<bool> svs(disableSmoothing, true);

    filter.reset();

    fnz1 = proc.filterParams.frequency->getUserValue(); // in midi note #
    const float q = gin::Q / (1.0f - (proc.filterParams.resonance->getUserValue() / 100.0f) * 0.99f);
    fqz1 = q;

    lfo1.reset();
    lfo2.reset();
    lfo3.reset();
    lfo4.reset();

    updateParams(0);
    snapParams();
    updateParams(0);
    snapParams();

    lfo1.noteOn();
    lfo2.noteOn();
    lfo3.noteOn();
    lfo4.noteOn();

    env1.noteOn();
    env2.noteOn();
    env3.noteOn();
    env4.noteOn();

    mseg1.noteOn();
    mseg2.noteOn();
    mseg3.noteOn();
    mseg4.noteOn();
}

void PMVoice::noteRetriggered()
{
    // antipop = 0.f;
    const auto note = getCurrentlyPlayingNote();
    curNote = getCurrentlyPlayingNote();

    proc.modMatrix.setPolyValue(*this, proc.randSrc1Poly, static_cast<float>(dist(gen)));
    proc.modMatrix.setPolyValue(*this, proc.randSrc2Poly, static_cast<float>(dist(gen)));

    if (glideInfo.fromNote >= 0 && (glideInfo.glissando || glideInfo.portamento))
    {
        noteSmoother.setTime(glideInfo.rate);
        noteSmoother.setValue(note.initialNote / 127.0f);
    }
    else
    {
        noteSmoother.setValueUnsmoothed(note.initialNote / 127.0f);
    }

    proc.modMatrix.setPolyValue(*this, proc.modSrcVelocity, note.noteOnVelocity.asUnsignedFloat());
    proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, note.initialTimbre.asUnsignedFloat());
    proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());

    updateParams(0);

    env1.noteOn();
    env2.noteOn();
    env3.noteOn();
    env4.noteOn();

    lfo1.reset();
    lfo2.reset();
    lfo3.reset();
    lfo4.reset();

    lfo1.noteOn();
    lfo2.noteOn();
    lfo3.noteOn();
    lfo4.noteOn();

    mseg1.noteOn();
    mseg2.noteOn();
    mseg3.noteOn();
    mseg4.noteOn();
}

void PMVoice::noteStopped(bool allowTailOff)
{
    env1.noteOff();
    env2.noteOff();
    env3.noteOff();
    env4.noteOff();
    curNote = getCurrentlyPlayingNote();
    proc.modMatrix.setPolyValue(*this, proc.modSrcVelOff, curNote.noteOffVelocity.asUnsignedFloat());
    if (!allowTailOff)
    {
        clearCurrentNote();
        stopVoice();
    }
}

void PMVoice::notePressureChanged()
{
    const auto note = getCurrentlyPlayingNote();
    proc.modMatrix.setPolyValue(*this, proc.modSrcPressure, note.pressure.asUnsignedFloat());
}

void PMVoice::noteTimbreChanged()
{
    const auto note = getCurrentlyPlayingNote();
    proc.modMatrix.setPolyValue(*this, proc.modSrcTimbre, note.timbre.asUnsignedFloat());
}

void PMVoice::setCurrentSampleRate(double newRate)
{
    MPESynthesiserVoice::setCurrentSampleRate(newRate);

    const auto quarter = newRate * 0.25;

    filter.setSampleRate(newRate);
    noteSmoother.setSampleRate(quarter);

    lfo1.setSampleRate(quarter);
    lfo2.setSampleRate(quarter);
    lfo3.setSampleRate(quarter);
    lfo4.setSampleRate(quarter);

    env1.setSampleRate(quarter);
    env2.setSampleRate(quarter);
    env3.setSampleRate(quarter);
    env4.setSampleRate(quarter);

    constexpr Envelope::Params p;
    env1.setParameters(p);
    env2.setParameters(p);
    env3.setParameters(p);
    env4.setParameters(p);

    mseg1.setSampleRate(quarter);
    mseg2.setSampleRate(quarter);
    mseg3.setSampleRate(quarter);
    mseg4.setSampleRate(quarter);

    fnb1 = std::exp(-2.0 * pi * 3500 / quarter);
    fna0 = 1 - fnb1;
    fnz1 = 95; // proc.filterParams.frequency->getUserValue();
    fqb1 = fnb1;
    fqa0 = fna0;
    fqz1 = 0; // proc.filterParams.resonance->getUserValue();
}

// clang-format off
float PMVoice::sine(float x) {
	x *= 2.0f * pi; // take in phase in units of [0, 1] and convert to [-pi, pi]
	x = FastMath<float>::normalizePhase(x);
	auto x2 = x * x;
 	return (((((-2.0366233e-08f * x2 + 2.6998227e-06f) 
		* x2 + -0.00019808741f) * x2 + 0.008332408f) 
		* x2 + -0.16666554f) * x2 + 0.99999958f) * x;
}

float PMVoice::wave(int sel, float x, bool isMod) {
	float out;
	switch(sel) {
		case 0:
			out = sine(x);
			break;
		case 1: // saw 3
			out = 0.638263f * sine(x) + 0.31842f * sine(2.0f * x - (int)(2.0f * x)) + 0.211349f * sine(3.0f * x - (int)(3.0f * x));
			break;
		case 2: // saw 4
			out = 0.638263f * sine(x) + 0.31842f * sine(2.0f * x - (int)(2.0f * x)) + 0.211349f * sine(3.0f * x - (int)(3.0f * x))
				+ 0.158489f * sine(4.0f * x - (int)(4.0f * x));
			break;
		case 3: // square 3
			out = sine(x) + 0.42462f * sine(3.0f * x - (int)(3.0f * x)) + 0.254097f * sine(5.0f * x - (int)(5.0f * x));
			break;
		case 4: // square 4
			out = sine(x) + 0.42462f * sine(3.0f * x - (int)(3.0f * x)) + 0.254097f * sine(5.0f * x - (int)(5.0f * x))
				+ 0.18197f * sine(7.0f * x - (int)(7.0f * x));
			break;
	}
	if (proc.globalParams.modfm->isOn() && isMod) {
		return std::exp(out) * 0.7 - 1; // correct for mod index
	}
	else { return out; }
	
}
// clang-format on

void PMVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples)
{
    updateParams(numSamples);

    synthBuffer.setSize(2, numSamples, false, false, true);

    auto synthBufferL = synthBuffer.getWritePointer(0);
    auto synthBufferR = synthBuffer.getWritePointer(1);

    double invSampleRate = 1.0 / currentSampleRate;

    // the whole enchilada
    for (int i = 0; i < numSamples; i++)
    {
        env1.getNextSample();
        env2.getNextSample();
        env3.getNextSample();
        env4.getNextSample();
        auto e1 = envs[0]->getOutput(); // different envelope can be chosen for each osc
        auto e2 = envs[1]->getOutput(); // so "e1" means "the output of the envelope selected for osc 1"
        auto e3 = envs[2]->getOutput();
        auto e4 = envs[3]->getOutput();

        float out{0}, out1{0}, out2{0}, out3{0}, out4{0};
        phase4 += freq4 * invSampleRate;
        phase4 = phase4 - (int)phase4;
        phase3 += freq3 * invSampleRate;
        phase3 = phase3 - (int)phase3;
        phase2 += freq2 * invSampleRate;
        phase2 = phase2 - (int)phase2;
        phase1 += freq1 * invSampleRate;
        phase1 = phase1 - (int)phase1;

        // clang-format off
		switch(algo) {
		case 0: {
			out =	e1 * v1.p(vol1) * wave(w1, phase1 +
					e2 * v2.p(vol2) * modIndex * a2.p(wave(w2, phase2 + // only LP process ops that are modulators
					e3 * v3.p(vol3) * modIndex * a3.p(wave(w3, phase3 +
					e4 * v4.p(vol4) * modIndex * a4.p(wave(w4, phase4, true)), true)), true)));
			break;
			}
		case 1: {
			out =	e1 * vol1 * wave(w1, phase1 +
					e2 * vol2 * modIndex *  a2.p(wave(w2, phase2 +
					(e3 * vol3 * modIndex * a3.p(wave(w3, phase3, true))) +
					(e4 * vol4 * modIndex * a4.p(wave(w4, phase4, true))), true)));
            break;
			}
		case 2: {
			out =	e1 * vol1 * wave(w1, phase1 +
					(e2 * vol2 * modIndex * a2.p(wave(w2, phase2 + 
					e3 * vol3 * modIndex *  a3.p(wave(w3, phase3, true)), true))) +
					e4 * vol4 * modIndex *  a4.p(wave(w4, phase4, true)), true);
            break;
			}
		case 3: {
			auto p4 = e4 * vol4 * modIndex * a4.p(wave(w4, phase4, true));
			auto p3 = e3 * vol3 * modIndex * a3.p(wave(w3, phase3 + p4, true));
			auto p2 = e2 * vol2 * modIndex * a2.p(wave(w2, phase2 + p4, true));
			out = e1 * vol1 * wave(w1, phase1 + p2 + p3);
            break;
			}
		case 4: {
			auto p43 = e3 * vol3 * modIndex * a3.p(wave(w3, phase3 + e4 * vol4 * modIndex * a4.p(wave(w4, phase4, true)), true));
			out2 = e2 * vol2 * wave(w2, phase2 + p43);
			out1 = e1 * vol1 * wave(w1, phase1 + p43);
            out = (out1 + out2) * 0.5f;
			break;
			}
		case 5: {
			out2 = e2 * vol2 * wave(w2, phase2 + e3 * vol3 * modIndex * a3.p(wave(w3, phase3 + 
				e4 * vol4 * modIndex * a4.p(wave(w4, phase4, true)), true)), true);
			out1 = e1 * vol1 * wave(w1, phase1);
            out = (out1 + out2) * 0.5f;
			break;
			}
		case 6: {
			auto p4 = e4 * vol4 * modIndex * a4.p(wave(w4, phase4, true));
			auto p3 = e3 * vol3 * modIndex * a3.p(wave(w3, phase3, true));
			auto p2 = e2 * vol2 * modIndex * a2.p(wave(w2, phase2, true));
			out1 = e1 * vol1 * wave(w1, phase1 + p2 + p3 + p4);
            out = out1;
			break;
			}
		case 7: {
			out1 = e1 * vol1 * wave(w1, phase1 + e2 * vol2 * modIndex * a2.p(wave(w2, phase2, true)));
			out3 = e3 * vol3 * wave(w3, phase3 + e4 * vol4 * modIndex * a4.p(wave(w4, phase4, true)));
			out = (out1 + out3) * 0.5f;
            break;
			}
		case 8: {
			auto p4 = e4 * vol4 * modIndex * a4.p(wave(w4, phase4, true));
			out3 = e3 * vol3 * wave(w3, phase3 + p4);
			out2 = e2 * vol2 * wave(w2, phase2 + p4);
			out1 = e1 * vol1 * wave(w1, phase1 + p4);
            out = (out1 + out2 + out3) * 0.3333333333333333f;
			break;
			}
		case 9: {
			out3 = e3 * vol3 * wave(w3, phase3 + e4 * vol4 * modIndex * a4.p(wave(w4, phase4, true)), true);
			out2 = e2 * vol2 * wave(w2, phase2);
			out1 = e1 * vol1 * wave(w1, phase1);
            out = (out1 + out2 + out3) * 0.3333333333333333f;
			break;
			}
		case 10: {
			out4 = e4 * vol4 * wave(w4, phase4);
			out3 = e3 * vol3 * wave(w3, phase3);
			out2 = e2 * vol2 * wave(w2, phase2);
			out1 = e1 * vol1 * wave(w1, phase1);
            out = (out1 + out2 + out3 + out4) * 0.25f;
			break;
			}
		default: {
			out = 0;
			break;
			}
		}

		synthBufferL[i] = out;
		synthBufferR[i] = out;
        // clang-format on
    }

    // Get and apply velocity according to keytrack param
    float velocity = currentlyPlayingNote.noteOnVelocity.asUnsignedFloat();
    float ampKeyTrack = getValue(proc.globalParams.velSens);
    synthBuffer.applyGain(gin::velocityToGain(velocity, ampKeyTrack) * baseAmplitude);

    filter.process(synthBuffer);

    bool voiceShouldStop = false;
    if (algo == 0 || algo == 1 || algo == 2 || algo == 4 || algo == 6)
    {
        if (!envsByNum[0]->isActive())
        {
            voiceShouldStop = true;
        }
    }
    else if (algo == 3 || algo == 5)
    {
        if (!envsByNum[1]->isActive() && !envsByNum[0]->isActive())
        {
            voiceShouldStop = true;
        }
    }
    else if (algo == 7)
    {
        if (!envsByNum[2]->isActive() && !envsByNum[0]->isActive())
        {
            voiceShouldStop = true;
        }
    }
    else if (algo == 8 || algo == 9)
    {
        if (!envsByNum[0]->isActive() && !envsByNum[1]->isActive() && !envsByNum[2]->isActive())
        {
            voiceShouldStop = true;
        }
    }
    else if (algo == 10)
    {
        if (!envsByNum[0]->isActive() && !envsByNum[1]->isActive() && !envsByNum[2]->isActive() && !envsByNum[3]->isActive())
        {
            voiceShouldStop = true;
        }
    }

    if (voiceShouldStop)
    {
        clearCurrentNote();
        stopVoice();
    }

    // Copy synth voice to output
    outputBuffer.addFrom(0, startSample, synthBuffer, 0, 0, numSamples);
    outputBuffer.addFrom(1, startSample, synthBuffer, 1, 0, numSamples);

    finishBlock(numSamples);
}

void PMVoice::updateParams(int blockSize)
{
    if (tilUpdate != 0)
    {
        --tilUpdate;
        return;
    } // at 4x os, we don't need this every block
    else
    {
        tilUpdate = 3;
    } // every 4th to match envelope/lfo/mseg

    vol1 = getValue(proc.osc1Params.volume);
    vol2 = getValue(proc.osc2Params.volume);
    vol3 = getValue(proc.osc3Params.volume);
    vol4 = getValue(proc.osc4Params.volume);

    algo = proc.timbreParams.algo->getUserValueInt();
    w1 = proc.osc1Params.wave->getUserValueInt();
    w2 = proc.osc2Params.wave->getUserValueInt();
    w3 = proc.osc3Params.wave->getUserValueInt();
    w4 = proc.osc4Params.wave->getUserValueInt();

    modIndex = getValue(proc.globalParams.modIndex);
    Averager::in = getValue(proc.globalParams.modTone);

    auto note = getCurrentlyPlayingNote();

    proc.modMatrix.setPolyValue(*this, proc.modSrcNote, note.initialNote / 127.0f);

    currentMidiNote = noteSmoother.getCurrentValue() * 127.0f;
    if (glideInfo.glissando)
        currentMidiNote = (float)juce::roundToInt(currentMidiNote);

    float dummy;
    float remainder = std::modf(currentMidiNote, &dummy);
    float baseFreq = static_cast<float>(MTS_NoteToFrequency(proc.client, static_cast<char>(currentMidiNote), note.midiChannel));
    baseFreq *= static_cast<float>(
        std::pow(1.05946309436f, note.totalPitchbendInSemitones * (proc.globalParams.pitchbendRange->getUserValue() / 2.0f) + remainder));
    baseFreq = juce::jlimit(20.0f, 20000.f, baseFreq * getValue(proc.timbreParams.pitch));

    if (proc.osc1Params.fixed->isOn())
    {
        freq1 = ((int)(getValue(proc.osc1Params.coarse) + 0.0001f) + getValue(proc.osc1Params.fine)) * 100.f;
    }
    else
    {
        freq1 = baseFreq * ((int)(getValue(proc.osc1Params.coarse) + 0.0001f) + getValue(proc.osc1Params.fine));
    }
    if (proc.osc2Params.fixed->isOn())
    {
        freq2 = ((int)(getValue(proc.osc2Params.coarse) + 0.0001f) + getValue(proc.osc2Params.fine)) * 100.f;
        freq2factor = freq1 / freq2;
    }
    else
    {
        freq2 = baseFreq * ((int)(getValue(proc.osc2Params.coarse) + 0.0001f) + getValue(proc.osc2Params.fine));
        freq2factor = ((int)(getValue(proc.osc2Params.coarse) + 0.0001f) + getValue(proc.osc2Params.fine));
    }
    if (proc.osc3Params.fixed->isOn())
    {
        freq3 = ((int)(getValue(proc.osc3Params.coarse) + 0.0001f) + getValue(proc.osc3Params.fine)) * 100.f;
        freq3factor = freq1 / freq3;
    }
    else
    {
        freq3 = baseFreq * ((int)(getValue(proc.osc3Params.coarse) + 0.0001f) + getValue(proc.osc3Params.fine));
        freq3factor = ((int)(getValue(proc.osc3Params.coarse) + 0.0001f) + getValue(proc.osc3Params.fine));
    }
    if (proc.osc4Params.fixed->isOn())
    {
        freq4 = ((int)(getValue(proc.osc4Params.coarse) + 0.0001f) + getValue(proc.osc4Params.fine)) * 100.f;
        freq4factor = freq1 / freq4;
    }
    else
    {
        freq4 = baseFreq * ((int)(getValue(proc.osc4Params.coarse) + 0.0001f) + getValue(proc.osc4Params.fine));
        freq4factor = ((int)(getValue(proc.osc4Params.coarse) + 0.0001f) + getValue(proc.osc4Params.fine));
    }

    switch (proc.osc1Params.env->getUserValueInt())
    {
    case 0:
        envs[0] = &env1;
        break;
    case 1:
        envs[0] = &env2;
        break;
    case 2:
        envs[0] = &env3;
        break;
    case 3:
        envs[0] = &env4;
        break;
    }

    switch (proc.osc2Params.env->getUserValueInt())
    {
    case 0:
        envs[1] = &env1;
        break;
    case 1:
        envs[1] = &env2;
        break;
    case 2:
        envs[1] = &env3;
        break;
    case 3:
        envs[1] = &env4;
        break;
    }

    switch (proc.osc3Params.env->getUserValueInt())
    {
    case 0:
        envs[2] = &env1;
        break;
    case 1:
        envs[2] = &env2;
        break;
    case 2:
        envs[2] = &env3;
        break;
    case 3:
        envs[2] = &env4;
        break;
    }

    switch (proc.osc4Params.env->getUserValueInt())
    {
    case 0:
        envs[3] = &env1;
        break;
    case 1:
        envs[3] = &env2;
        break;
    case 2:
        envs[3] = &env3;
        break;
    case 3:
        envs[3] = &env4;
        break;
    }

    switch (int(proc.filterParams.type->getUserValue()))
    {
    case 0:
        filter.setType(gin::Filter::lowpass);
        filter.setSlope(gin::Filter::db12);
        break;
    case 1:
        filter.setType(gin::Filter::lowpass);
        filter.setSlope(gin::Filter::db24);
        break;
    case 2:
        filter.setType(gin::Filter::highpass);
        filter.setSlope(gin::Filter::db12);
        break;
    case 3:
        filter.setType(gin::Filter::highpass);
        filter.setSlope(gin::Filter::db24);
        break;
    case 4:
        filter.setType(gin::Filter::bandpass);
        filter.setSlope(gin::Filter::db12);
        break;
    case 5:
        filter.setType(gin::Filter::bandpass);
        filter.setSlope(gin::Filter::db24);
        break;
    case 6:
        filter.setType(gin::Filter::notch);
        filter.setSlope(gin::Filter::db12);
        break;
    case 7:
        filter.setType(gin::Filter::notch);
        filter.setSlope(gin::Filter::db24);
        break;
    }

    // filter
    float noteNum = getValue(proc.filterParams.frequency);
    noteNum += (currentlyPlayingNote.initialNote - 50) * getValue(proc.filterParams.keyTracking);
    float f = gin::getMidiNoteInHertz(noteNum);
    f = juce::jlimit(4.0f, maxFreq, f);
    float q = gin::Q / (1.0f - (getValue(proc.filterParams.resonance) / 100.0f) * 0.99f);

    // 'f' and 'q' LP-filtered
    fnz1 = fna0 * noteNum + fnb1 * fnz1;
    fqz1 = fqa0 * q + fqb1 * fqz1;
    filter.setParams(juce::jlimit<float>(4.f, maxFreq, gin::getMidiNoteInHertz(fnz1)), fqz1);

    gin::LFO::Parameters params;
    float freq = 0;

    // lfo 1
    if (proc.lfo1Params.sync->getUserValue() > 0.0f)
        freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo1Params.beat->getUserValue())].toSeconds(proc.playhead);
    else
        freq = getValue(proc.lfo1Params.rate);
    params.waveShape = (gin::LFO::WaveShape) int(proc.lfo1Params.wave->getUserValue());
    params.frequency = freq;
    params.phase = getValue(proc.lfo1Params.phase);
    params.offset = getValue(proc.lfo1Params.offset);
    params.depth = getValue(proc.lfo1Params.depth);
    params.delay = getValue(proc.lfo1Params.delay);
    params.fade = getValue(proc.lfo1Params.fade);
    lfo1.setParameters(params);
    lfo1.process(blockSize);
    proc.modMatrix.setPolyValue(*this, proc.modSrcLFO1, lfo1.getOutput());

    // lfo 2
    if (proc.lfo2Params.sync->getUserValue() > 0.0f)
        freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo2Params.beat->getUserValue())].toSeconds(proc.playhead);
    else
        freq = getValue(proc.lfo2Params.rate);
    params.waveShape = (gin::LFO::WaveShape) int(proc.lfo2Params.wave->getUserValue());
    params.frequency = freq;
    params.phase = getValue(proc.lfo2Params.phase);
    params.offset = getValue(proc.lfo2Params.offset);
    params.depth = getValue(proc.lfo2Params.depth);
    params.delay = getValue(proc.lfo2Params.delay);
    params.fade = getValue(proc.lfo2Params.fade);
    lfo2.setParameters(params);
    lfo2.process(blockSize);
    proc.modMatrix.setPolyValue(*this, proc.modSrcLFO2, lfo2.getOutput());

    // lfo 3
    if (proc.lfo3Params.sync->getUserValue() > 0.0f)
        freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo3Params.beat->getUserValue())].toSeconds(proc.playhead);
    else
        freq = getValue(proc.lfo3Params.rate);
    params.waveShape = (gin::LFO::WaveShape) int(proc.lfo3Params.wave->getUserValue());
    params.frequency = freq;
    params.phase = getValue(proc.lfo3Params.phase);
    params.offset = getValue(proc.lfo3Params.offset);
    params.depth = getValue(proc.lfo3Params.depth);
    params.delay = getValue(proc.lfo3Params.delay);
    params.fade = getValue(proc.lfo3Params.fade);
    lfo3.setParameters(params);
    lfo3.process(blockSize);
    proc.modMatrix.setPolyValue(*this, proc.modSrcLFO3, lfo3.getOutput());

    // lfo 4
    if (proc.lfo4Params.sync->getUserValue() > 0.0f)
        freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(proc.lfo4Params.beat->getUserValue())].toSeconds(proc.playhead);
    else
        freq = getValue(proc.lfo4Params.rate);
    params.waveShape = (gin::LFO::WaveShape) int(proc.lfo4Params.wave->getUserValue());
    params.frequency = freq;
    params.phase = getValue(proc.lfo4Params.phase);
    params.offset = getValue(proc.lfo4Params.offset);
    params.depth = getValue(proc.lfo4Params.depth);
    params.delay = getValue(proc.lfo4Params.delay);
    params.fade = getValue(proc.lfo4Params.fade);
    lfo4.setParameters(params);
    lfo4.process(blockSize);
    proc.modMatrix.setPolyValue(*this, proc.modSrcLFO4, lfo4.getOutput());

    Envelope::Params p;
    p.attackTimeMs = getValue(proc.env1Params.attack);
    p.decayTimeMs = getValue(proc.env1Params.decay);
    p.sustainLevel = getValue(proc.env1Params.sustain);
    p.releaseTimeMs = fastKill ? 0.01f : getValue(proc.env1Params.release);
    p.aCurve = getValue(proc.env1Params.acurve);
    p.dRCurve = getValue(proc.env1Params.drcurve);
    int mode = proc.env1Params.syncrepeat->getUserValueInt();
    p.sync = (!(mode == 0));
    p.repeat = (!(mode == 0));
    if (mode == 1)
    {
        p.sync = true;
        p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(proc.env1Params.duration->getUserValue())].toSeconds(proc.playhead);
    }
    if (mode == 2)
    {
        p.sync = true;
        p.syncduration = getValue(proc.env1Params.time);
    }
    env1.setParameters(p);

    p.attackTimeMs = getValue(proc.env2Params.attack);
    p.decayTimeMs = getValue(proc.env2Params.decay);
    p.sustainLevel = getValue(proc.env2Params.sustain);
    p.releaseTimeMs = fastKill ? 0.01f : getValue(proc.env2Params.release);
    p.aCurve = getValue(proc.env2Params.acurve);
    p.dRCurve = getValue(proc.env2Params.drcurve);
    mode = proc.env2Params.syncrepeat->getUserValueInt();
    p.sync = (mode != 0);
    p.repeat = (mode != 0);
    if (mode == 1)
    {
        p.sync = true;
        p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(proc.env2Params.duration->getUserValue())].toSeconds(proc.playhead);
    }
    if (mode == 2)
    {
        p.sync = true;
        p.syncduration = getValue(proc.env2Params.time);
    }
    env2.setParameters(p);

    p.attackTimeMs = getValue(proc.env3Params.attack);
    p.decayTimeMs = getValue(proc.env3Params.decay);
    p.sustainLevel = getValue(proc.env3Params.sustain);
    p.releaseTimeMs = fastKill ? 0.01f : getValue(proc.env3Params.release);
    p.aCurve = getValue(proc.env3Params.acurve);
    p.dRCurve = getValue(proc.env3Params.drcurve);
    mode = proc.env3Params.syncrepeat->getUserValueInt();
    p.sync = (!(mode == 0));
    p.repeat = (!(mode == 0));
    if (mode == 1)
    {
        p.sync = true;
        p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(proc.env3Params.duration->getUserValue())].toSeconds(proc.playhead);
    }
    if (mode == 2)
    {
        p.sync = true;
        p.syncduration = getValue(proc.env3Params.time);
    }
    env3.setParameters(p);

    p.attackTimeMs = getValue(proc.env4Params.attack);
    p.decayTimeMs = getValue(proc.env4Params.decay);
    p.sustainLevel = getValue(proc.env4Params.sustain);
    p.releaseTimeMs = fastKill ? 0.01f : getValue(proc.env4Params.release);
    p.aCurve = getValue(proc.env4Params.acurve);
    p.dRCurve = getValue(proc.env4Params.drcurve);
    mode = proc.env4Params.syncrepeat->getUserValueInt();
    p.sync = (!(mode == 0));
    p.repeat = (!(mode == 0));
    if (mode == 1)
    {
        p.sync = true;
        p.syncduration = gin::NoteDuration::getNoteDurations()[size_t(proc.env4Params.duration->getUserValue())].toSeconds(proc.playhead);
    }
    if (mode == 2)
    {
        p.sync = true;
        p.syncduration = getValue(proc.env4Params.time);
    }
    env4.setParameters(p);

    proc.modMatrix.setPolyValue(*this, proc.modSrcEnv1, env1.getOutput());
    proc.modMatrix.setPolyValue(*this, proc.modSrcEnv2, env2.getOutput());
    proc.modMatrix.setPolyValue(*this, proc.modSrcEnv3, env3.getOutput());
    proc.modMatrix.setPolyValue(*this, proc.modSrcEnv4, env4.getOutput());

    noteSmoother.process(blockSize);

    if (proc.mseg1Params.sync->isOn())
    {
        mseg1Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.mseg1Params.beat))].toSeconds(proc.playhead);
    }
    else
    {
        mseg1Params.frequency = getValue(proc.mseg1Params.rate);
    }
    mseg1Params.depth = getValue(proc.mseg1Params.depth); // proc.mseg1Params.depth->getUserValue();
    mseg1Params.offset = getValue(proc.mseg1Params.offset);
    mseg1Params.loop = proc.mseg1Params.loop->isOn();

    if (proc.mseg2Params.sync->isOn())
    {
        mseg2Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.mseg2Params.beat))].toSeconds(proc.playhead);
    }
    else
    {
        mseg2Params.frequency = getValue(proc.mseg2Params.rate);
    }
    mseg2Params.depth = getValue(proc.mseg2Params.depth);
    mseg2Params.offset = getValue(proc.mseg2Params.offset);
    mseg2Params.loop = proc.mseg2Params.loop->isOn();

    if (proc.mseg3Params.sync->isOn())
    {
        mseg3Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.mseg3Params.beat))].toSeconds(proc.playhead);
    }
    else
    {
        mseg3Params.frequency = getValue(proc.mseg3Params.rate);
    }
    mseg3Params.depth = getValue(proc.mseg3Params.depth); // proc.mseg3Params.depth->getUserValue();
    mseg3Params.offset = getValue(proc.mseg3Params.offset);
    mseg3Params.loop = proc.mseg3Params.loop->isOn();

    if (proc.mseg4Params.sync->isOn())
    {
        mseg4Params.frequency = 1 / gin::NoteDuration::getNoteDurations()[size_t(getValue(proc.mseg4Params.beat))].toSeconds(proc.playhead);
    }
    else
    {
        mseg4Params.frequency = getValue(proc.mseg4Params.rate);
    }
    mseg4Params.depth = getValue(proc.mseg4Params.depth);
    mseg4Params.offset = getValue(proc.mseg4Params.offset);
    mseg4Params.loop = proc.mseg4Params.loop->isOn();

    mseg1.setParameters(mseg1Params);
    mseg2.setParameters(mseg2Params);
    mseg3.setParameters(mseg3Params);
    mseg4.setParameters(mseg4Params);

    mseg1.process(blockSize);
    mseg2.process(blockSize);
    mseg3.process(blockSize);
    mseg4.process(blockSize);

    proc.modMatrix.setPolyValue(*this, proc.modSrcMSEG1, mseg1.getOutput());
    proc.modMatrix.setPolyValue(*this, proc.modSrcMSEG2, mseg2.getOutput());
    proc.modMatrix.setPolyValue(*this, proc.modSrcMSEG3, mseg3.getOutput());
    proc.modMatrix.setPolyValue(*this, proc.modSrcMSEG4, mseg4.getOutput());
}

float PMVoice::getFilterCutoffNormalized() const
{
    const float freq = filter.getFrequency();
    const auto range = proc.filterParams.frequency->getUserRange();
    return range.convertTo0to1(juce::jlimit(range.start, range.end, gin::getMidiNoteFromHertz(freq)));
}

gin::Wave PMVoice::waveForChoice(const int choice)
{
    switch (choice)
    {
    case 0:
        return gin::Wave::sine;
    case 1:
        return gin::Wave::triangle;
    case 2:
        return gin::Wave::square;
    case 3:
        return gin::Wave::sawUp;
    case 4:
        return gin::Wave::pinkNoise;
    case 5:
        return gin::Wave::whiteNoise;
    default:
        return gin::Wave::sine;
    }
}
