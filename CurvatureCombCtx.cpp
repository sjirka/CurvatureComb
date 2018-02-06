#include "CurvatureCombCtx.h"
#include "CurvatureCombNode.h"

#include <maya\MGlobal.h>
#include <maya\MFnDependencyNode.h>
#include <maya\MSelectionList.h>
#include <maya\MItSelectionList.h>
#include <maya\MDagPath.h>

CurvatureCombCtx::CurvatureCombCtx(){
	setTitleString("Curvature Comb");
	setImage("curvatureComb.xpm", MPxContext::kImage1);
}

CurvatureCombCtx::~CurvatureCombCtx()
{
}

void CurvatureCombCtx::toolOnSetup(MEvent &) {
	setHelpString("Drag with LMB to adjust scale, MMB to adjust samples");
}

void CurvatureCombCtx::doEnterRegion() {
	setHelpString("Drag with LMB to adjust scale, MMB to adjust samples");
}

void CurvatureCombCtx::toolOffCleanup() {
	MPxContext::toolOffCleanup();
}

// VP2
MStatus CurvatureCombCtx::doPress(MEvent &event, MHWRender::MUIDrawManager &drawMgr, const MHWRender::MFrameContext &context) {
	return doPress(event);
}

MStatus CurvatureCombCtx::doDrag(MEvent &event, MHWRender::MUIDrawManager &drawMgr, const MHWRender::MFrameContext &context) {
	return doDrag(event);
}

MStatus CurvatureCombCtx::doRelease(MEvent &event, MHWRender::MUIDrawManager &drawMgr, const MHWRender::MFrameContext &context) {
	return doRelease(event);
}

// Common
MStatus CurvatureCombCtx::doPress(MEvent &event){
	MStatus status;

	m_drag = false;

	MSelectionList selection;
	status = MGlobal::getActiveSelectionList(selection);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MItSelectionList itSelection(selection, MFn::kPluginLocatorNode, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CurvatureCombToolCmd::CurvatureAttribute attr;

	if (event.mouseButton() == MEvent::kLeftMouse)
		attr = CurvatureCombToolCmd::CurvatureAttribute::kScale;
	else if (event.mouseButton() == MEvent::kMiddleMouse && event.modifiers()!=MEvent::shiftKey)
		attr = CurvatureCombToolCmd::CurvatureAttribute::kSamples;
	else if (event.mouseButton() == MEvent::kMiddleMouse && event.modifiers() == MEvent::shiftKey)
		attr = CurvatureCombToolCmd::CurvatureAttribute::kSubdivisions;

	std::vector <CurvatureCombAttr> activeNodes;
	for (itSelection.reset(); !itSelection.isDone(); itSelection.next()) {
		MObject node;
		status = itSelection.getDependNode(node);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MFnDagNode fnNode(node, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		if (fnNode.typeId() != CurvatureCombNode::id)
			continue;

		MPlug pPlug;
		
		if (attr == CurvatureCombToolCmd::CurvatureAttribute::kScale)
			pPlug = fnNode.findPlug(CurvatureCombNode::aScale, &status);
		else if (attr == CurvatureCombToolCmd::CurvatureAttribute::kSamples)
			pPlug = fnNode.findPlug(CurvatureCombNode::aSamples, &status);
		else if (attr == CurvatureCombToolCmd::CurvatureAttribute::kSubdivisions)
			pPlug = fnNode.findPlug(CurvatureCombNode::aSubdivs, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		double initialValue = pPlug.asDouble();
		
		activeNodes.push_back(CurvatureCombAttr(node, initialValue));
	}

	status = event.getPosition(m_startX, m_startY);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	m_cmdPtr = (CurvatureCombToolCmd*)newToolCommand();
	m_cmdPtr->setActiveNodes(activeNodes);
	m_cmdPtr->setAttr(attr);

	return MS::kSuccess;
}

MStatus CurvatureCombCtx::doDrag(MEvent &event) {
	MStatus status;

	m_drag = true;

	short
		currentX,
		currentY;

	event.getPosition(currentX, currentY);
	short delta = (currentX - m_startX) + (currentY - m_startY);

	m_cmdPtr->setDelta(delta);
	m_cmdPtr->redoIt();

	for (unsigned int i = 0; i < M3dView::numberOf3dViews(); i++) {
		M3dView view;
		M3dView::get3dView(i, view);
		view.refresh();
	}

	return MS::kSuccess;
}

MStatus CurvatureCombCtx::doRelease(MEvent &event){
	MStatus status;

	m_cmdPtr->finalize();

	if (!m_drag)
		MGlobal::executeCommand("setToolTo moveSuperContext; select -cl;", false, true);

	return MS::kSuccess;
}