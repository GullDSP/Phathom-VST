//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#include <vstgui/vstgui.h>
#include <vstgui/vstgui_uidescription.h>


namespace VSTGUI {
	/// <summary>
	/// Class to allow drawing a decorative circle. Has no functions
	/// Only Size and color are settable
	/// </summary>
	class Circle : public CView {

	public:

		Circle(const CRect& rect);

		void draw(CDrawContext* pContext) override;

		void setFillColor(CColor newColor) {
			fillColor = newColor;
			setDirty(true);
		}
		CColor fillColor = VSTGUI::kBlueCColor;
	private:
	

		CLASS_METHODS(Circle, CView)

	};


}