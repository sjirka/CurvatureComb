#include "CurvatureCombToolCmd.h"
#include "CurvatureCombNode.h"

#include <maya\MArgList.h>
#include <maya\MFnDagNode.h>
#include <maya\MPlug.h>
#include <maya\MFnNumericAttribute.h>

CurvatureCombToolCmd::CurvatureCombToolCmd(){
	setCommandString("setCurvatureComb");
}

CurvatureCombToolCmd::~CurvatureCombToolCmd(){
}

void* CurvatureCombToolCmd::creator() {
	return new CurvatureCombToolCmd;
}

bool CurvatureCombToolCmd::isUndoable() const {
	return true;
}

MStatus CurvatureCombToolCmd::finalize() {
	MStatus status;
	MArgList command;
	status = command.addArg(commandString());
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = MPxToolCommand::doFinalize(command);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	return MS::kSuccess;
}

MStatus CurvatureCombToolCmd::doIt(const MArgList& args) {
	return redoIt();
}

MStatus CurvatureCombToolCmd::undoIt() {
	MStatus status;

	status = m_dagMod.undoIt();
	CHECK_MSTATUS_AND_RETURN_IT(status)

	return MStatus::kSuccess;
}

MStatus CurvatureCombToolCmd::redoIt() {
	MStatus status;

	for (auto &activeNode : m_activeNodes) {
		MFnDagNode fnNode(activeNode.node, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);		

		if (m_attr == CurvatureAttribute::kScale) {
			double newScale = activeNode.initialValue + m_delta;
			MPlug pScale = fnNode.findPlug(CurvatureCombNode::aScale, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			status = m_dagMod.newPlugValueDouble(pScale, newScale);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}
		else if(m_attr == CurvatureAttribute::kSamples){
			double newSamples = activeNode.initialValue + m_delta/100;
			MPlug pSamples = fnNode.findPlug(CurvatureCombNode::aSamples, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			MFnNumericAttribute fnSamples(CurvatureCombNode::aSamples);
			double min;
			fnSamples.getMin(min);

			if (min <= newSamples) {
				status = m_dagMod.newPlugValueInt(pSamples, (int)newSamples);
				CHECK_MSTATUS_AND_RETURN_IT(status);
			}
		}
		else if (m_attr == CurvatureAttribute::kSubdivisions) {
			double newSubdivs = activeNode.initialValue + m_delta / 200;
			MPlug pSubdivs = fnNode.findPlug(CurvatureCombNode::aSubdivs, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			MFnNumericAttribute fnSamples(CurvatureCombNode::aSubdivs);
			double min, max;
			fnSamples.getMin(min);
			fnSamples.getSoftMax(max);

			if (min <= newSubdivs && max >= newSubdivs) {
				status = m_dagMod.newPlugValueInt(pSubdivs, (int)newSubdivs);
				CHECK_MSTATUS_AND_RETURN_IT(status);
				MPlug pOverrideSubdivs = fnNode.findPlug(CurvatureCombNode::aOverrideSubdivs, &status);
				status = m_dagMod.newPlugValueBool(pOverrideSubdivs, true);
				CHECK_MSTATUS_AND_RETURN_IT(status);
			}
		}
	}

	return m_dagMod.doIt();
}

void CurvatureCombToolCmd::setActiveNodes(std::vector <CurvatureCombAttr> &activeNodes) {
	m_activeNodes = activeNodes;
}

void CurvatureCombToolCmd::setDelta(short delta) {
	m_delta = delta;
}

void CurvatureCombToolCmd::setAttr(CurvatureAttribute attr) {
	m_attr = attr;
}