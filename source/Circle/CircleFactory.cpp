//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#pragma once

#include "vstgui/vstgui.h"
#include "vstgui/vstgui_uidescription.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"

#include "vstgui/uidescription/uiattributes.h" 

#include "Circle.h"

namespace VSTGUI {



	class CircleFactory : public ViewCreatorAdapter {
	public:


		CircleFactory() {
			UIViewFactory::registerViewCreator(*this);


		}

		IdStringPtr getViewName() const {
			return "Circle Dec";
		}
		IdStringPtr getBaseViewName() const {
			return UIViewCreator::kCView;
		}
		CView* create(const UIAttributes& attributes, const IUIDescription* description) const override {
			CRect size(0, 0, 0, 0);

			return new Circle(size);



		}

		bool apply(CView* view, const UIAttributes& attributes, const IUIDescription* description) const override
		{
			ViewCreatorAdapter::apply(view, attributes, description);



			Circle* myCircle = dynamic_cast<Circle*>(view);
			if (!myCircle) {
				return false;
			}


			CColor newFillColor;
			const std::string* fillColor = attributes.getAttributeValue("draw-fill-color");
			if (fillColor && description->getColor(fillColor->data(), newFillColor)) {
				myCircle->setFillColor(newFillColor);
			}


			return true;
		}

		bool getAttributeNames(StringList& attributeNames) const override {

			attributeNames.emplace_back("draw-fill-color");

			return ViewCreatorAdapter::getAttributeNames(attributeNames);
		}
		AttrType getAttributeType(const string& attributeName) const override {

			if (!strcmp(attributeName.data(), "draw-fill-color")) {

				return AttrType::kColorType;
			}

			return ViewCreatorAdapter::getAttributeType(attributeName);
		}
		bool getPossibleListValues(const string& attributeName,
			ConstStringPtrList& values) const override {



			return ViewCreatorAdapter::getPossibleListValues(attributeName, values);
		}
		bool getAttributeValue(CView* view, const string& attributeName, string& stringValue,
			const IUIDescription* desc) const override
		{

			Circle* myCircle = dynamic_cast<Circle*>(view);

			if (attributeName == "draw-fill-color") {
				stringValue = myCircle->fillColor.toString();
				return true;
			}

			return ViewCreatorAdapter::getAttributeValue(view, attributeName, stringValue, desc);
		}

	private:

	};

	CircleFactory __gNewCirclerFactory;

}