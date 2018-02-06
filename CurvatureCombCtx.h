#pragma once
#include "CurvatureCombToolCmd.h"

#include <maya\MPxSelectionContext.h>

class CurvatureCombCtx : public MPxContext
{
public:
	CurvatureCombCtx();
	virtual ~CurvatureCombCtx();

	virtual void toolOnSetup(MEvent &);
	virtual void doEnterRegion();
	virtual void toolOffCleanup();

	virtual MStatus doPress(MEvent &event);
	virtual MStatus doDrag(MEvent &event);
	virtual MStatus doRelease(MEvent &event);

	// VP2
	virtual MStatus doPress(MEvent &event, MHWRender::MUIDrawManager &drawMgr, const MHWRender::MFrameContext &context);
	virtual MStatus doDrag(MEvent &event, MHWRender::MUIDrawManager &drawMgr, const MHWRender::MFrameContext &context);
	virtual MStatus doRelease(MEvent &event, MHWRender::MUIDrawManager &drawMgr, const MHWRender::MFrameContext &context);

private:
	CurvatureCombToolCmd* m_cmdPtr;
	
	bool
		m_drag = false;

	short
		m_startX,
		m_startY;
};

