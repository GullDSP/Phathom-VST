//------------------------------------------------------------------------
// Copyright(c) 2026 GullDSP.
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"
#include "vstgui/plugin-bindings/vst3editor.h"

#define MAX_ZOOM_FACTOR_LIMIT 10
#define MIN_ZOOM_FACTOR_LIMIT 0.1

using namespace Steinberg;

namespace EFFECT {

//------------------------------------------------------------------------
// Controller Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API PhathomController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	setKnobMode(Steinberg::Vst::KnobModes::kLinearMode);

	registerParameters(parameters);
	
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomController::terminate ()
{

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomController::setComponentState (IBStream* state)
{
	if (!state) return kResultFalse;
	Steinberg::IBStreamer streamer(state, kLittleEndian);

	
	int version = 0;
	if (!streamer.readInt32(version)) {
		return kResultFalse;
	}

	int32 num_params = 0;
	if (!streamer.readInt32(num_params)) {
		return kResultFalse;
	}

	int id = 0;
	double value = 0;

	if (num_params == 0) return kResultFalse;

	for (int i = 0; i < num_params; i++) {
		if (!streamer.readInt32(id)) {
			return Steinberg::kResultFalse;
		}
		if (!streamer.readDouble(value)) {
			return Steinberg::kResultFalse;
		}
		setParamNormalized(id, value);
	}


	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomController::setState (IBStream* state)
{
	if (state) {
		Steinberg::IBStreamer streamer(state, kLittleEndian);
		int id = -1;
		double value = 0;
		if (streamer.readInt32(id) && streamer.readDouble(value)){

			if (id == PhathomControllerParamID::kZoomFactor) {

				if ((value > MIN_ZOOM_FACTOR_LIMIT) && (value < MAX_ZOOM_FACTOR_LIMIT)) {
					currentZoomFactor = value;
					if (activeEditor) 	activeEditor->setZoomFactor(currentZoomFactor);

				}

			}

		}


	}

	
	
	return EditControllerEx1::setState(state);;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomController::getState (IBStream* state)
{
	
	if (state) {
		Steinberg::IBStreamer streamer(state, kLittleEndian);
		streamer.writeInt32(PhathomControllerParamID::kZoomFactor);
		streamer.writeDouble(currentZoomFactor);
		
	}

	return EditControllerEx1::getState(state);;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API PhathomController::createView (FIDString name)
{
	
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		auto* view = new CustomEditor (this, "view1", "editor.uidesc");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace EFFECT
