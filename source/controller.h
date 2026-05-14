//------------------------------------------------------------------------
// Copyright(c) 2026 GullDSP.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "Effect/Parameters.h"
#include "CustomEditor.h"
namespace EFFECT {

//------------------------------------------------------------------------
//  Controller
//------------------------------------------------------------------------
class PhathomController : public Steinberg::Vst::EditControllerEx1,
	public VSTGUI::VST3EditorDelegate
{
public:
//------------------------------------------------------------------------
	PhathomController () = default;
	~PhathomController () SMTG_OVERRIDE = default;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{
		return (Steinberg::Vst::IEditController*)new PhathomController;
	}

	//--- from IPluginBase -----------------------------------------------
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	//--- from EditController --------------------------------------------
	Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;


	/// to track current zoom
	void onZoomChanged(VSTGUI::VST3Editor* editor, double newZoom) override {
		currentZoomFactor = newZoom;
	
	};
	// Called when plugin opened
	void didOpen(VSTGUI::VST3Editor* editor) override {
		activeEditor = dynamic_cast<CustomEditor*>(editor);

		if (activeEditor) {
			activeEditor->setZoomFactor(currentZoomFactor);

		}
	}

	// called when plugin window closed
	void willClose(VSTGUI::VST3Editor* editor) override {
		activeEditor = nullptr; 
	}



 	//---Interface---------
	DEFINE_INTERFACES
		// Here you can add more supported VST3 interfaces
		// DEF_INTERFACE (Vst::IXXX)
	END_DEFINE_INTERFACES (EditController)
    DELEGATE_REFCOUNT (EditController)

//------------------------------------------------------------------------
protected:

	double currentZoomFactor = 1.0;
	CustomEditor* activeEditor = nullptr;

};

//------------------------------------------------------------------------
} // namespace EFFECT
