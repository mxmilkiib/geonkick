/**
 * File name: processor_vst.h
 * Project: Geonkick (A kick synthesizer)
 *
 * Copyright (C) 2019 Iurie Nistor (http://quamplex.com/geonkick)
 *
 * This file is part of Geonkick.
 *
 * GeonKick is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "GKickVstProcessor.h"
#include "GKickVstEditor.h"
#include "VstIds.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "geonkick_api.h"
#include "kit_state.h"

namespace Steinberg
{

GKickVstProcessor::GKickVstProcessor()
        : geonkickApi{nullptr}
{
        // Temporary, for debugging.
        std::freopen("/home/iurie/redir.txt", "w", stdout);
}

FUnknown* GKickVstProcessor::createInstance(void*)
{
        GEONKICK_LOG_INFO("called");
        return static_cast<Vst::IAudioProcessor*>(new GKickVstProcessor());
}


tresult PLUGIN_API
GKickVstProcessor::initialize(FUnknown* context)
{
        GEONKICK_LOG_INFO("called");
        auto res = Vst::SingleComponentEffect::initialize(context);
        if (res != kResultTrue)
                return kResultFalse;

        addAudioOutput(STR16("AudioOutput"), Vst::SpeakerArr::kStereo);
        addEventInput(STR16("MIDI in"), 1);

        geonkickApi = std::make_unique<GeonkickApi>();
        if (!geonkickApi->init()) {
                GEONKICK_LOG_ERROR("can't init Geonkick API");
                return kResultFalse;
        }
        return kResultTrue;
}

tresult PLUGIN_API
GKickVstProcessor::setBusArrangements(Vst::SpeakerArrangement* inputs,
                                                         int32 numIns,
                                                         Vst::SpeakerArrangement* outputs,
                                                         int32 numOuts)
{
        GEONKICK_LOG_INFO("called");
        if (numIns == 0 && numOuts == 1)
                return Vst::SingleComponentEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
        return kResultFalse;
}

tresult PLUGIN_API
GKickVstProcessor::setupProcessing(Vst::ProcessSetup& setup)
{
        GEONKICK_LOG_INFO("called");
        return Vst::SingleComponentEffect::setupProcessing(setup);
}

tresult PLUGIN_API
GKickVstProcessor::setActive(TBool state)
{
        GEONKICK_LOG_INFO("called");
        return Vst::SingleComponentEffect::setActive(state);
}

tresult PLUGIN_API
GKickVstProcessor::process(Vst::ProcessData& data)
{
        if (data.numSamples > 0) {
                //		SpeakerArrangement arr;
                //		getBusArrangement(kOutput, 0, arr);
                auto events = data.inputEvents;
                auto nEvents = events->getEventCount();
                auto eventIndex = 0;
                Vst::Event event;
                auto res = events->getEvent(eventIndex, event);
                for (decltype(data.numSamples) i = 0; i < data.numSamples; i++) {
                        if (res != kResultOk)
                                res = events->getEvent(eventIndex, event);
                        if (res == kResultOk && event.sampleOffset == i && eventIndex < nEvents) {
                                switch (event.type) {
                                case Vst::Event::kNoteOnEvent:
                                        GEONKICK_LOG_INFO("Vst::Event::kNoteOnEvent");
                                        geonkickApi->setKeyPressed(true,
                                                                   event.noteOn.pitch,
                                                                   127/*event.noteOn.velocity*/);
                                        break;

                                case Vst::Event::kNoteOffEvent:
                                        GEONKICK_LOG_INFO("Vst::Event::kNoteOffEvent");
                                        geonkickApi->setKeyPressed(false,
                                                                   event.noteOff.pitch,
                                                                   127/*event.noteOff.velocity*/);
                                        break;
                                default:
                                        break;
                                }

                                eventIndex++;
                                if (eventIndex < nEvents)
                                        res = events->getEvent(eventIndex, event);
                        }
                        auto val = geonkickApi->getAudioFrame(0);
                        data.outputs[0].channelBuffers32[0][i] = val;
                        data.outputs[0].channelBuffers32[1][i] = val;
                }
	}

        return kResultOk;
}

tresult PLUGIN_API
GKickVstProcessor::setState(IBStream* state)
{
        if (state == nullptr || geonkickApi == nullptr) {
                GEONKICK_LOG_ERROR("wrong arguments or DSP is not ready");
                return kResultFalse;
        }

        // Get the size of data.
        state->seek(0, IBStream::kIBSeekEnd, 0);
        int64 endStream = 0;
        state->tell(&endStream);
        if (endStream < 1) {
                GEONKICK_LOG_ERROR("stream is empty");
                return kResultFalse;
        }
        state->seek(0, IBStream::kIBSeekSet, 0);

        std::string data(endStream, '\0');
        int32 nBytes = 0;
        if (state->read(data.data(), data.size(), &nBytes) == kResultFalse) {
                GEONKICK_LOG_ERROR("error on reading the state");
                return kResultFalse;
        }

        if (static_cast<decltype(nBytes)>(data.size()) != nBytes) {
                GEONKICK_LOG_ERROR("error on reading the state");
                return kResultFalse;
        }
        geonkickApi->setKitState(data);
        return kResultOk;
}

tresult PLUGIN_API
GKickVstProcessor::getState(IBStream* state)
{
        if (state == nullptr || geonkickApi == nullptr) {
                GEONKICK_LOG_ERROR("wrong arguments or DSP is not ready");
                return kResultFalse;
        }

        int32 nBytes = 0;
        auto data = geonkickApi->getKitState()->toJson();
        if (state->write(data.data(), data.size(), &nBytes) == kResultFalse) {
                GEONKICK_LOG_ERROR("error on saving the state");
                return kResultFalse;
        }

        if (static_cast<decltype(nBytes)>(data.size()) != nBytes) {
                GEONKICK_LOG_ERROR("error on saving the state");
                return kResultFalse;
        }
        return kResultOk;

}

IPlugView* PLUGIN_API
GKickVstProcessor::createView(FIDString name)
{
        GEONKICK_LOG_INFO("called: " << name);
        if (name && std::string(name) == std::string("editor"))
                return static_cast<IPlugView*>(new GKickVstEditor(this, geonkickApi.get()));
        return nullptr;
}

tresult PLUGIN_API
GKickVstProcessor::setComponentState(IBStream* state)
{
        GEONKICK_LOG_INFO("called");
        return kResultOk;
}


} // namespace Steinberg
