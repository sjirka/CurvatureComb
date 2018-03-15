#include "CurvatureCombCmd.h"
#include "CurvatureCombNode.h"

#include <maya\MSyntax.h>
#include <maya\MArgDatabase.h>
#include <maya\MSelectionList.h>
#include <maya\MDagPath.h>
#include <maya\MPlug.h>
#include <maya\MFnComponentListData.h>
#include <maya\MGlobal.h>
#include <maya\MItSelectionList.h>

#include "../_library/SNode.h"

CurvatureCombCmd::CurvatureCombCmd()
{
}


CurvatureCombCmd::~CurvatureCombCmd()
{
}


void *CurvatureCombCmd::creator() {
	return new CurvatureCombCmd;
}

MSyntax CurvatureCombCmd::newSyntax() {
	MSyntax syntax;

	syntax.addFlag(SCALE_FLAG, SCALE_FLAG_LONG, MSyntax::kDouble);
	syntax.addFlag(SAMPLES_FLAG, SAMPLES_FLAG_LONG, MSyntax::kLong);
	syntax.addFlag(OVERRIDE_SUBD_FLAG, OVERRIDE_SUBD_FLAG_LONG, MSyntax::kBoolean);
	syntax.addFlag(SUBD_FLAG, SUBD_FLAG_LONG, MSyntax::kLong);

	syntax.addArg(MSyntax::kString);

	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 1);

	syntax.enableEdit(true);
	syntax.enableQuery(true);

	return syntax;
}

MStatus CurvatureCombCmd::doIt(const MArgList& argList) {
	MStatus status;

	MArgDatabase argData(syntax(), argList, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	bool isEdit = argData.isEdit();
	bool isQuery = argData.isQuery();
	m_isCreate = (isQuery || isEdit) ? false : true;


	MString name = argData.commandArgumentString(0);

	if (m_isCreate) {
		status = SNode::createDagGroup((name == "") ? "curvatureComb" : name, CurvatureCombNode::id, m_dagMod, m_node, m_transform);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		MFnDagNode fnComb(m_node, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MSelectionList selection;
		argData.getObjects(selection);

		unsigned int counter = 0;
		MItSelectionList itSelection(selection);
		for (itSelection.reset(); !itSelection.isDone(); itSelection.next()) {
			MDagPath path;
			MObject component;

			status = itSelection.getDagPath(path, component);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			status = path.extendToShape();
			CHECK_MSTATUS_AND_RETURN_IT(status);

			MFnDagNode fnGeometry(path, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			if ((path.apiType() != MFn::kMesh && path.apiType() != MFn::kNurbsCurve) ||
				(path.apiType() == MFn::kMesh && component.apiType() != MFn::kMeshEdgeComponent))
				continue;

			// Connect geometry
			MPlug geoArrPlug = fnComb.findPlug(CurvatureCombNode::aGeometry, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			MPlug geoElPlug = geoArrPlug.elementByLogicalIndex(counter++, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			MPlug pGeometry = fnGeometry.findPlug((path.apiType() == MFn::kMesh) ? "worldMesh" : "worldSpace");
			status = m_dagMod.connect(pGeometry.elementByLogicalIndex(path.instanceNumber()), geoElPlug.child(CurvatureCombNode::aWorldGeometry));
			CHECK_MSTATUS_AND_RETURN_IT(status);

			// Connect smooth mesh and set active edges
			if (path.apiType() == MFn::kMesh) {
				status = m_dagMod.connect(fnGeometry.findPlug("outSmoothMesh"), geoElPlug.child(CurvatureCombNode::aSmoothGeometry));

				MFnComponentListData fnCompData;
				MObject compData = fnCompData.create(&status);
				CHECK_MSTATUS_AND_RETURN_IT(status);
				status = fnCompData.add(component);
				CHECK_MSTATUS_AND_RETURN_IT(status);
				status = m_dagMod.newPlugValue(geoElPlug.child(CurvatureCombNode::aComponent), compData);
				CHECK_MSTATUS_AND_RETURN_IT(status);
			}
		}

		if (counter == 0) {
			displayError("Invalid selection. Select curves or mesh edges.");
			return MS::kInvalidParameter;
		}
	}
	else if (isQuery || isEdit) {
		status = SNode::getPluginNode(name, CurvatureCombNode::id, m_node);
		if (status != MS::kSuccess) {
			displayError("invalid object");
			setResult(false);
			return status;
		}
	}

	MFnDagNode fnComb(m_node, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	if (m_isCreate || isEdit) {
		if (argData.isFlagSet(SCALE_FLAG))
			m_dagMod.newPlugValueDouble(fnComb.findPlug(CurvatureCombNode::aScale), argData.flagArgumentDouble(SCALE_FLAG, 0));
		if (argData.isFlagSet(SAMPLES_FLAG))
			m_dagMod.newPlugValueInt(fnComb.findPlug(CurvatureCombNode::aSamples), argData.flagArgumentInt(SAMPLES_FLAG, 0));
		if (argData.isFlagSet(OVERRIDE_SUBD_FLAG))
			m_dagMod.newPlugValueBool(fnComb.findPlug(CurvatureCombNode::aOverrideSubdivs), argData.flagArgumentBool(OVERRIDE_SUBD_FLAG, 0));
		if (argData.isFlagSet(SUBD_FLAG))
			m_dagMod.newPlugValueInt(fnComb.findPlug(CurvatureCombNode::aSubdivs), argData.flagArgumentInt(SUBD_FLAG, 0));
	}
	else if (isQuery) {
		if (argData.isFlagSet(SCALE_FLAG))
			setResult(fnComb.findPlug(CurvatureCombNode::aScale).asDouble());
		else if (argData.isFlagSet(SAMPLES_FLAG))
			setResult(fnComb.findPlug(CurvatureCombNode::aSamples).asInt());
		else if (argData.isFlagSet(OVERRIDE_SUBD_FLAG))
			setResult(fnComb.findPlug(CurvatureCombNode::aOverrideSubdivs).asBool());
		else if (argData.isFlagSet(SUBD_FLAG))
			setResult(fnComb.findPlug(CurvatureCombNode::aSubdivs).asInt());
	}

	return redoIt();
}

MStatus CurvatureCombCmd::redoIt() {
	MStatus status;

	if (m_isCreate) {
		status = m_dagMod.doIt();
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MFnDagNode
			fnTransform(m_transform),
			fnNode(m_node);

		appendToResult(fnTransform.name());
		appendToResult(fnNode.name());
		m_dagMod.commandToExecute("select " + fnNode.fullPathName());
		m_dagMod.commandToExecute("string $ccCtx = `curvatureCombCtx`; setToolTo $ccCtx;");
	}

	m_dagMod.doIt();

	return MS::kSuccess;
}

MStatus CurvatureCombCmd::undoIt() {
	MStatus status;

	status = m_dagMod.undoIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}

bool CurvatureCombCmd::isUndoable() const {
	return true;
}