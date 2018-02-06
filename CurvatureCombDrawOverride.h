#pragma once

#include <maya\MPxDrawOverride.h>
#include "CurvatureCombNode.h"

class CurvatureCombDrawOverride : public MHWRender::MPxDrawOverride
{
public:
	static MHWRender::MPxDrawOverride* Creator(const MObject& obj){
		return new CurvatureCombDrawOverride(obj);
	}

	virtual ~CurvatureCombDrawOverride() {};

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;
	virtual MUserData* prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath, const MHWRender::MFrameContext& frameContext, MUserData* oldData);
	virtual bool hasUIDrawables() const { return true; }
	virtual void addUIDrawables(const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data);

	static void draw(const MHWRender::MDrawContext& context, const MUserData* data) {};

private:
	CurvatureCombDrawOverride(const MObject& obj);

	MColor
		m_profileColor,
		m_combColor;
	MUuid m_camId;
};

