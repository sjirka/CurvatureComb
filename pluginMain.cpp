#include "CurvatureCombCmd.h"
#include "CurvatureCombNode.h"
#include "CurvatureCombDrawOverride.h"
#include "CurvatureCombCtxCmd.h"
#include "CurvatureCombToolCmd.h"

#include <maya/MFnPlugin.h>
#include <maya\MDrawRegistry.h>

MString CurvatureCombNode::drawClassification("drawdb/geometry/curvatureComb");
MString CurvatureCombNode::registrantId("CurvatureCombNodePlugin");

MStatus initializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin fnPlugin(obj, "Stepan Jirka", "1.0", "Any");

	status = fnPlugin.registerNode(
		"curvatureComb",
		CurvatureCombNode::id,
		CurvatureCombNode::creator,
		CurvatureCombNode::initialize,
		MPxNode::kLocatorNode,
		&CurvatureCombNode::drawClassification);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
		CurvatureCombNode::drawClassification,
		CurvatureCombNode::registrantId,
		CurvatureCombDrawOverride::Creator);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	status = fnPlugin.registerCommand(
		"curvatureComb",
		CurvatureCombCmd::creator,
		CurvatureCombCmd::newSyntax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.registerContextCommand("curvatureCombCtx", CurvatureCombCtxCmd::creator, "setCurvatureComb", CurvatureCombToolCmd::creator);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin fnPlugin(obj);

	status = fnPlugin.deregisterContextCommand("curvatureCombCtx", "setCurvatureComb");
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.deregisterCommand("curvatureComb");
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
		CurvatureCombNode::drawClassification,
		CurvatureCombNode::registrantId);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.deregisterNode(CurvatureCombNode::id);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}
