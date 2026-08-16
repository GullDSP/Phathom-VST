//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------
#pragma once
#include <math.h>
#include <vector>
#include "vstgui/plugin-bindings/vst3editor.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "base/source/fstreamer.h"

enum PhathomControllerParamID {
	kZoomFactor = 201
};

enum PhathomParamID {

	kBypass = 101,
	kInGain,
	kMix,
	kDrive,
	kHiSwitch,
	kLoSwitch,

	kUIInAmp,
	kUIOutAmp,
	kOutLevel,
	kDryEq,
	kPhaseFlip,
	kNonLinType,
	kLastParam

};

inline std::vector<Steinberg::Vst::Parameter*> registerParameters(Steinberg::Vst::ParameterContainer& parameters) {

	std::vector<Steinberg::Vst::Parameter*> myParams;

	int flagsSwitchedParams = Steinberg::Vst::ParameterInfo::kCanAutomate;
	int flagsNormalParams = Steinberg::Vst::ParameterInfo::kCanAutomate;
	int flagsUIParams = Steinberg::Vst::ParameterInfo::kIsHidden | Steinberg::Vst::ParameterInfo::kIsReadOnly;

	myParams.push_back(new Steinberg::Vst::Parameter(STR16("Bypass"), kBypass, STR16(""),0,1, Steinberg::Vst::ParameterInfo::kIsBypass | Steinberg::Vst::ParameterInfo::kCanAutomate));

	Steinberg::Vst::StringListParameter* pDistSwitchParam =
		new Steinberg::Vst::StringListParameter(STR16("Dist Type"), kNonLinType, STR16(""), flagsSwitchedParams);
	pDistSwitchParam->appendString(STR16("Clank"));
	pDistSwitchParam->appendString(STR16("Fuzz"));
	pDistSwitchParam->setNormalized(0);
	myParams.push_back(pDistSwitchParam);

	myParams.push_back(new Steinberg::Vst::Parameter(STR16("Mix"), kMix, STR16(""), 0.5, 0, flagsNormalParams));
	myParams.push_back(new Steinberg::Vst::Parameter(STR16("Input Gain"), kInGain, STR16(""), 0.5, 0, flagsNormalParams));
	myParams.push_back(new Steinberg::Vst::Parameter(STR16("Drive"), kDrive, STR16(""), 0.5, 0, flagsNormalParams));

	Steinberg::Vst::StringListParameter* pEqSwitchParam =
		new Steinberg::Vst::StringListParameter(STR16("Dry EQ"), kDryEq, STR16(""), flagsSwitchedParams);
	pEqSwitchParam->appendString(STR16("Off"));
	pEqSwitchParam->appendString(STR16("On"));
	pEqSwitchParam->setNormalized(0.0);

	myParams.push_back(pEqSwitchParam);
	Steinberg::Vst::StringListParameter* pHiSwitchParam =
		new Steinberg::Vst::StringListParameter(STR16("Hi Shape"), kHiSwitch, STR16(""), flagsSwitchedParams);
	pHiSwitchParam->appendString(STR16("Normal"));
	pHiSwitchParam->appendString(STR16("Cut"));
	pHiSwitchParam->appendString(STR16("Boost"));
	pHiSwitchParam->setNormalized(0);
	myParams.push_back(pHiSwitchParam);

	Steinberg::Vst::StringListParameter* pLoSwitchParam =
		new Steinberg::Vst::StringListParameter(STR16("Lo Shape"), kLoSwitch, STR16(""), flagsSwitchedParams);
	pLoSwitchParam->appendString(STR16("Normal"));
	pLoSwitchParam->appendString(STR16("Cut"));
	pLoSwitchParam->appendString(STR16("Boost"));
	pLoSwitchParam->setNormalized(0);
	myParams.push_back(pLoSwitchParam);

	myParams.push_back(new Steinberg::Vst::Parameter(STR16("Output Level"), kOutLevel, STR16(""), 0.5, 0, flagsNormalParams));

	Steinberg::Vst::StringListParameter* pPhaseSwitch =
		new Steinberg::Vst::StringListParameter(STR16("Phase"), kPhaseFlip, STR16(""), flagsSwitchedParams);
	pPhaseSwitch->appendString(STR16("Normal"));
	pPhaseSwitch->appendString(STR16("Invert"));
	pPhaseSwitch->setNormalized(0);
	myParams.push_back(pPhaseSwitch);

	//myParams.push_back(new Steinberg::Vst::Parameter(STR16("Response"), kResponse, STR16(""), 0.5, 0, Steinberg::Vst::ParameterInfo::kNoFlags));


	myParams.push_back(new Steinberg::Vst::Parameter(STR16("UIInLvl"), kUIInAmp, STR16(""), 0.0, 0, flagsUIParams));
	myParams.push_back(new Steinberg::Vst::Parameter(STR16("UIOutLvl"), kUIOutAmp, STR16(""), 0.0, 0, flagsUIParams));

	// Add to Steinberg type list for host
	for (auto& param : myParams) {

		if (param->getInfo().id != kBypass) {
			param->setPrecision(1);
		}
		
		parameters.addParameter(param);

	}

	return myParams;
}

class ParameterUnit {
public:
	ParameterUnit(double default_value, int param_id) {
		value = default_value;
		defaultValue = default_value;
		id = param_id;
		dirty = true;
	}
	void set(double param_value) {
		this->value = param_value;
		dirty = true;
	}
	void setId(int param_id) {
		this->id = param_id;
	}
	void setDefault(double default_value) {
		defaultValue = default_value;
	}
	double get() {
		return value;
	}
	int getId() const {
		return id;
	}
	bool isDirty() const {
		return dirty;
	}
	void setDirty() {
		dirty = true;
	}
	void setClean() {
		dirty = false;
	}
private:
	int id = -1;
	double value = 0;
	double defaultValue = 0;
	bool dirty = false;
};

class ParameterHandler {
public:
	~ParameterHandler() {
		for (ParameterUnit* ptr : ParamList) {
			delete ptr;
		}
		ParamList.clear();
	}
	/// <summary>
	/// Ingest parameters from host
	/// </summary>
	/// <param name="host_param_list"></param>
	void getHostParameters(Steinberg::Vst::ParameterContainer& host_param_list) {
		int numParams = host_param_list.getParameterCount();

		// Plugin may be re-initialised while live, so need to delete old object
		for (ParameterUnit* ptr : ParamList) {
			delete ptr;
		}
		ParamList.clear();

		if (!numParams) return;

		for (int i = 0; i < numParams; i++) {
			Steinberg::Vst::Parameter* param = host_param_list.getParameterByIndex(i);
			Steinberg::Vst::ParameterInfo info = param->getInfo();
			int tag = info.id;
			double default_value = info.defaultNormalizedValue;

			ParamList.push_back(new ParameterUnit(default_value, tag));
		}
	}
	void addParameter(int id, int default_value) {
		ParamList.push_back(new ParameterUnit(default_value, id));
	}
	void setParameter(int id, double value) {
		ParameterUnit* pParam = findParameter(id);
		if (pParam) {
			pParam->set(value);
		}
	}
	double getParamValue(int id) {
		ParameterUnit* pParam = findParameter(id);
		return pParam ? pParam->get() : 0;

	}
	bool isParamDirty(int id) {
		ParameterUnit* p = findParameter(id);

		return p ? p->isDirty() : false;
	}
	void setAllClean() {
		for (auto& pParam : ParamList) {
			pParam->setClean();
		}
	}
	void setAllDirty() {
		for (auto& pParam : ParamList) {
			pParam->setDirty();
		}
	}
	int getNumParams() const {
		return ParamList.size();
	}
	Steinberg::tresult getState(Steinberg::IBStreamer& streamer)
	{
		// Write version num

		if (!streamer.writeInt32(200)) {
			return Steinberg::kResultFalse;
		}

		// Write num params being streamed
		if (!streamer.writeInt32(getNumParams())) {
			return Steinberg::kResultFalse;
		}
		for (int i = 0; i < getNumParams(); i++) {
			streamer.writeInt32(getParamByIndex(i)->getId());
			streamer.writeDouble(getParamByIndex(i)->get());
		}

		return Steinberg::kResultOk;
	}
	Steinberg::tresult setState(Steinberg::IBStreamer& streamer)
	{

		int version = 0;
		if (!streamer.readInt32(version)) {
			return Steinberg::kResultFalse;
		}

		version_id = version;

		int num_params = 0;
		if (!streamer.readInt32(num_params)) {
			return Steinberg::kResultFalse;
		}

		int id = 0;
		double value = 0;

		for (int i = 0; i < num_params; i++) {
			if (!streamer.readInt32(id)) {
				return Steinberg::kResultFalse;
			}
			if (!streamer.readDouble(value)) {
				return Steinberg::kResultFalse;
			}
			setParameter(id, value);
		}

		if (version < 200) {
			// If old preset, load using classic fuzz
			setParameter(kNonLinType, 1.0);
		}

		setAllDirty();
		return Steinberg::kResultOk;
	}
	ParameterUnit* getParamByIndex(int index) {
		return ParamList[index];
	}

	ParameterUnit* findParameter(int id) {
		for (auto& pParam : ParamList) {
			if (pParam->getId() == id) {
				return pParam;
			}
		}

		// TODO add error state here
		return nullptr;
	}
private:
	std::vector<ParameterUnit*> ParamList;
	int version_id = 0;

};
