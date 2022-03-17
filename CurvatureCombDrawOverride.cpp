#include "CurvatureCombDrawOverride.h"

#include <maya\MFnDagNode.h>
#include <maya\MHWGeometryUtilities.h>
#include <maya\M3dView.h>
#include <maya\MFnCamera.h>

CurvatureCombDrawOverride::CurvatureCombDrawOverride(const MObject& obj) : MHWRender::MPxDrawOverride(obj, CurvatureCombDrawOverride::draw)
{
}

MHWRender::DrawAPI CurvatureCombDrawOverride::supportedDrawAPIs() const {
	return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

MUserData* CurvatureCombDrawOverride::prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath, const MHWRender::MFrameContext& frameContext, MUserData* oldData){
	MStatus status;

	MFnDagNode fnNode(objPath);
	MPlug pUpdate = fnNode.findPlug(CurvatureCombNode::aUpdate, false, &status);
	CHECK_MSTATUS(status);
	MObject oUpdate = pUpdate.asMObject();

	MHWRender::DisplayStatus displayStatus = MHWRender::MGeometryUtilities::displayStatus(objPath);
	M3dView view;

	m_profileColor = (displayStatus == MHWRender::kDormant) ? view.colorAtIndex(12, M3dView::kDormantColors) : view.colorAtIndex(8, M3dView::kActiveColors);
	m_combColor = (displayStatus == MHWRender::kDormant) ? view.colorAtIndex(6, M3dView::kDormantColors) : view.colorAtIndex(18, M3dView::kActiveColors);

	CHECK_MSTATUS(status);

	return oldData;
}

void CurvatureCombDrawOverride::addUIDrawables(const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data) {
	MStatus status;
	MFnDagNode fnNode(objPath, &status);

	CurvatureCombNode *nodePtr = dynamic_cast<CurvatureCombNode*>(fnNode.userNode());

	std::map <unsigned int, CurvatureGeometry> *geoData = nodePtr->getGeoData();
	
	MDagPath camPath = frameContext.getCurrentCameraPath();
	MFnCamera fnCam(camPath);

	MUuid camId = SNode::getUuid(camPath.node(), &status);

	drawManager.beginDrawable();

	for (auto &geo : *geoData) {
		for (auto &data : geo.second.geoViewData[camId.asString().asChar()].geoData) {
			drawManager.setColor(m_profileColor);
			drawManager.lineStrip(data.profilePoints, false);

			drawManager.setColor(m_combColor);
			for (unsigned int i = 0; i < data.samplePoints.length(); i++)
				drawManager.line(data.samplePoints[i], data.profilePoints[i]);
		}
	}
	
	drawManager.endDrawable();
}
