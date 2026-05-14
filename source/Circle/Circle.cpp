//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#include "Circle.h"
namespace VSTGUI {

	Circle::Circle(const CRect& rect)
		: CView(rect)

	{

	};


	void Circle::draw(CDrawContext* pContext) {
		// Adjust to relative point
		auto viewPos = getViewSize().getTopLeft();
		CDrawContext::Transform t(*pContext, CGraphicsTransform().translate(viewPos));
		// Create a rect (viewBounds) based on this relative point, add allowance for frame
		CRect viewNaive = getViewSize();
		const CRect viewBounds(1, 1, viewNaive.getWidth() - 1, viewNaive.getHeight() - 1);

	
		pContext->setDrawMode(kDrawFilled);
		// Set the fill color 
		pContext->setFillColor(fillColor);

		// Draw
		pContext->drawEllipse(viewBounds, kDrawFilled);
	
		setDirty(false);
	}



}