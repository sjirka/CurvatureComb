#include "CurvatureCombNode.h"

#include <maya\MFnTypedAttribute.h>
#include <maya\MFnNumericAttribute.h>
#include <maya\MFnCompoundAttribute.h>
#include <maya\MFnDagNode.h>
#include <maya\MFnNurbsCurve.h>
#include <maya\M3dView.h>
#include <maya\MPlugArray.h>
#include <maya\MFnMesh.h>
#include <maya\MFnComponentListData.h>
#include <maya\MFnSingleIndexedComponent.h>
#include <maya\MPxManipContainer.h>
#include <maya\MViewport2Renderer.h>
#include <maya\MFnCamera.h>
#include <maya\MFnNurbsCurveData.h>

#include "../_library/SMath.h"

MTypeId CurvatureCombNode::id(0x00127894);

MObject CurvatureCombNode::aUpdate;
MObject CurvatureCombNode::aGeometry;
MObject CurvatureCombNode::aWorldGeometry;
MObject CurvatureCombNode::aSmoothGeometry;
MObject CurvatureCombNode::aComponent;
MObject CurvatureCombNode::aScale;
MObject CurvatureCombNode::aSamples;
MObject CurvatureCombNode::aSubdivs;
MObject CurvatureCombNode::aOverrideSubdivs;

CurvatureCombNode::CurvatureCombNode()
{
}

CurvatureCombNode::~CurvatureCombNode()
{
}

void *CurvatureCombNode::creator() {
	return new CurvatureCombNode;
}

MStatus CurvatureCombNode::initialize() {
	MStatus status;

	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;
	MFnCompoundAttribute cAttr;

	aUpdate = nAttr.create("update", "update", MFnNumericData::kBoolean, true, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	nAttr.setWritable(false);
	nAttr.setHidden(true);
	nAttr.setConnectable(false);
	addAttribute(aUpdate);

	// Numeric attributes /////////////////////////////////////////////////////////////////////////

	aScale = nAttr.create("crvScale", "crvScale", MFnNumericData::kDouble, 1000.0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	nAttr.setSoftMin(0);
	nAttr.setSoftMax(1000);
	addAttribute(aScale);
	attributeAffects(aScale, aUpdate);

	aSamples = nAttr.create("samples", "samples", MFnNumericData::kInt, 50, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	nAttr.setMin(2);
	nAttr.setSoftMax(100);
	addAttribute(aSamples);
	attributeAffects(aSamples, aUpdate);

	aSubdivs = nAttr.create("subdivs", "subdivs", MFnNumericData::kInt, 2, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	nAttr.setMin(0);
	nAttr.setSoftMax(3);
	addAttribute(aSubdivs);
	attributeAffects(aSubdivs, aUpdate);

	aOverrideSubdivs = nAttr.create("overrideSubdivs", "overrideSubdivs", MFnNumericData::kBoolean, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(aOverrideSubdivs);
	attributeAffects(aOverrideSubdivs, aUpdate);

	// Geometry ///////////////////////////////////////////////////////////////////////////////////

	aGeometry = cAttr.create("geometry", "geometry", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	cAttr.setArray(true);

	aWorldGeometry = tAttr.create("worldGeometry", "worldGeometry", MFnData::kAny, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(aWorldGeometry);
	attributeAffects(aWorldGeometry, aUpdate);
	cAttr.addChild(aWorldGeometry);

	aSmoothGeometry = tAttr.create("smoothGeometry", "smoothGeometry", MFnData::kMesh, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(aSmoothGeometry);
	attributeAffects(aSmoothGeometry, aUpdate);
	cAttr.addChild(aSmoothGeometry);

	aComponent = tAttr.create("component", "component", MFnData::kComponentList, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	addAttribute(aComponent);
	attributeAffects(aComponent, aUpdate);
	cAttr.addChild(aComponent);

	addAttribute(aGeometry);
	attributeAffects(aGeometry, aUpdate);

	return MS::kSuccess;
}

MStatus CurvatureCombNode::setDependentsDirty(const MPlug &plug, MPlugArray &plugArray) {
	MStatus status;

	if (plug == aScale)
		m_dirtyScale = true;
	if (plug == aSamples || plug==aSubdivs || plug==aOverrideSubdivs)
		m_dirtySamples = true;
	if (plug == aGeometry)
		m_geoData.erase(plug.logicalIndex());
	if (plug.isChild())
		m_geoData[plug.parent().logicalIndex()].isDirty = true;

	return MS::kSuccess;
}

MStatus CurvatureCombNode::compute(const MPlug &plug, MDataBlock &datablock) {
	MStatus status;

	if (plug != aUpdate)
		return MS::kInvalidParameter;

	unsigned int samples = datablock.inputValue(aSamples).asInt();
	double scale = datablock.inputValue(aScale).asDouble();

	MArrayDataHandle hGeometryArray = datablock.inputArrayValue(aGeometry, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	MFnDagNode fnNode(thisMObject(), &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MPlug pGeometryArray = fnNode.findPlug(aGeometry, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	unsigned int numViews = M3dView::numberOf3dViews();

	for (unsigned int i = 0; i < hGeometryArray.elementCount(); i++) {
		status = hGeometryArray.jumpToArrayElement(i);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		unsigned int logicalIndex = hGeometryArray.elementIndex();

		if (m_geoData[logicalIndex].isDirty || m_dirtySamples) {
			MDataHandle hGeometryElement = hGeometryArray.inputValue(&status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			MDataHandle hWorldGeometry = hGeometryElement.child(aWorldGeometry);
			MObject worldGeometry = hWorldGeometry.data();

			if(MFn::kNurbsCurveData == worldGeometry.apiType()){
				worldGeometry = hWorldGeometry.asNurbsCurveTransformed();

				for (unsigned int w = 0; w < numViews; w++) {
					M3dView view;
					M3dView::get3dView(w, view);
					MDagPath camPath;
					status = view.getCamera(camPath);
					CHECK_MSTATUS_AND_RETURN_IT(status);
					MFnCamera fnCamera(camPath, &status);
					CHECK_MSTATUS_AND_RETURN_IT(status);
					MUuid camId = fnCamera.uuid(&status);
					CHECK_MSTATUS_AND_RETURN_IT(status);

					MPoint nearPlane, farPlane;
					unsigned int x, y, width, height;
					view.viewport(x, y, width, height);
					view.viewToWorld(width / 2, height / 2, nearPlane, farPlane);

					SPlane camPlane = (fnCamera.isOrtho()) ? SPlane(nearPlane, fnCamera.viewDirection(MSpace::kWorld)) : SPlane::ZERO;

					status = getCurveCurvature(worldGeometry, samples, m_geoData[logicalIndex].geoViewData[camId.asString().asChar()], camPlane);
					CHECK_MSTATUS_AND_RETURN_IT(status);
				}
				
			}
			else if (MFn::kMeshData == worldGeometry.apiType()) {
				worldGeometry = hWorldGeometry.asMeshTransformed();

				MPlug pWorldGeo = pGeometryArray.elementByLogicalIndex(logicalIndex).child(aWorldGeometry);
				MPlugArray connections;
				if (pWorldGeo.connectedTo(connections, true, false)) {
					MObject compData = hGeometryElement.child(aComponent).data();
					MFnComponentListData fnCompData(compData, &status);
					CHECK_MSTATUS_AND_RETURN_IT(status);

					SMesh workMesh(worldGeometry, &status);
					CHECK_MSTATUS_AND_RETURN_IT(status);
					status = workMesh.setActiveEdges(fnCompData[0]);
					CHECK_MSTATUS_AND_RETURN_IT(status);

					MFnMesh fnMesh(connections[0].node(), &status);
					CHECK_MSTATUS_AND_RETURN_IT(status);
					MMeshSmoothOptions smOpt;
					status = fnMesh.getSmoothMeshDisplayOptions(smOpt);
					CHECK_MSTATUS_AND_RETURN_IT(status);

					if (datablock.inputValue(aOverrideSubdivs).asBool() == true)
						smOpt.setDivisions(datablock.inputValue(aSubdivs).asInt());

					status = workMesh.smoothMesh(smOpt);
					CHECK_MSTATUS_AND_RETURN_IT(status);

					for (unsigned int w = 0; w < numViews; w++) {
						M3dView view;
						M3dView::get3dView(w, view);
						MDagPath camPath;
						status = view.getCamera(camPath);
						CHECK_MSTATUS_AND_RETURN_IT(status);
						MFnCamera fnCamera(camPath, &status);
						CHECK_MSTATUS_AND_RETURN_IT(status);
						MUuid camId = fnCamera.uuid(&status);
						CHECK_MSTATUS_AND_RETURN_IT(status);

						MPoint nearPlane, farPlane;
						unsigned int x, y, width, height;
						view.viewport(x, y, width, height);
						view.viewToWorld(width / 2, height / 2, nearPlane, farPlane);

						SPlane camPlane = (fnCamera.isOrtho()) ? SPlane(nearPlane, fnCamera.viewDirection(MSpace::kWorld)) : SPlane::ZERO;

						status = getMeshCurvature(workMesh, m_geoData[logicalIndex].geoViewData[camId.asString().asChar()], camPlane);
						CHECK_MSTATUS_AND_RETURN_IT(status);
					}
				}
			}
		}

		if (m_geoData[logicalIndex].isDirty || m_dirtySamples || m_dirtyScale)
			for(auto &geoViewData : m_geoData[logicalIndex].geoViewData)
				for (auto &geoData : geoViewData.second.geoData)
					for (unsigned int i = 0; i < geoData.samplePoints.length(); i++)
						geoData.profilePoints[i] = geoData.samplePoints[i] + geoData.sampleNormals[i] * scale;

		m_geoData[logicalIndex].isDirty = false;
	}

	m_dirtySamples = false;
	m_dirtyScale = false;

	datablock.setClean(aUpdate);

	return MS::kSuccess;
}

void CurvatureCombNode::draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus stat) {
	MStatus status;

	// Trigger compute
	MFnDagNode fnNode(thisMObject());
	MPlug pSection = fnNode.findPlug(aUpdate, &status);
	CHECK_MSTATUS(status);
	pSection.asMObject();

	view.beginGL();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	MColor profileColor = (stat==M3dView::kDormant) ? view.colorAtIndex(12, M3dView::kDormantColors) : view.colorAtIndex(8, M3dView::kActiveColors);
	MColor combColor = (stat == M3dView::kDormant) ? view.colorAtIndex(6, M3dView::kDormantColors) : view.colorAtIndex(18, M3dView::kActiveColors);

	MDagPath camPath;
	view.getCamera(camPath);
	MUuid camId = SNode::getUuid(camPath.node(), &status);
	CHECK_MSTATUS(status)

	for (auto &geo : m_geoData) {
		for (auto &geoData : geo.second.geoViewData[camId.asString().asChar()].geoData) {
			glColor3f(profileColor.r, profileColor.g, profileColor.b);
			glBegin(GL_LINE_STRIP);
			for (unsigned int i = 0; i < geoData.profilePoints.length(); i++) {
				MPoint comb = geoData.profilePoints[i];
				glVertex3d(comb.x, comb.y, comb.z);
			}
			glEnd();

			glColor3f(combColor.r, combColor.g, combColor.b);
			glBegin(GL_LINES);
			for (unsigned int i = 0; i < geoData.samplePoints.length(); i++) {
				MPoint sample = geoData.samplePoints[i];
				MPoint comb = geoData.profilePoints[i];

				glVertex3d(sample.x, sample.y, sample.z);
				glVertex3d(comb.x, comb.y, comb.z);
			}
			glEnd();
		}
	}

	glPopAttrib();
	view.endGL();
}

MStatus CurvatureCombNode::getCurveCurvature(MObject &curve, unsigned int samples, CurvatureViewGeometry &geometry, SPlane &plane) {
	MStatus status;

	geometry.geoData.clear();
	CurvatureData data;
	data.setNumSamples(samples+1);

	if (curve.apiType() != MFn::kNurbsCurveData && curve.apiType() != MFn::kNurbsCurveGeom)
		return MS::kInvalidParameter;
	
	MFnNurbsCurveData dataCreator;
	MObject duplCurve = dataCreator.create();

	MFnNurbsCurve fnCurve(duplCurve, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	fnCurve.copy(curve, duplCurve, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	if (plane != SPlane::ZERO) {
		MPointArray curveCVs;
		fnCurve.getCVs(curveCVs);
		MPointArray newCVs = plane.project(curveCVs);
		fnCurve.setCVs(newCVs);
	}

	double start, end;
	status = fnCurve.getKnotDomain(start, end);
	CHECK_MSTATUS_AND_RETURN_IT(status)
	double span = (end - start) / samples;

	for (unsigned int i = 0; i < samples + 1; i++) {
		double param = i*span + start;

		status = fnCurve.getPointAtParam(param, data.samplePoints[i]);
		MVector normal = fnCurve.normal(param, MSpace::kObject, &status);
		
		data.sampleNormals[i] = (status != MS::kSuccess) ? MVector::zero : normal.normal() / normal.length() * -1;
	}

	geometry.geoData.push_back(data);

	return MS::kSuccess;
}

MStatus CurvatureCombNode::getMeshCurvature(SMesh &mesh, CurvatureViewGeometry &geometry, SPlane &plane) {
	MStatus status;

	geometry.geoData.clear();

	std::vector <SEdgeLoop> activeLoops;
	mesh.getActiveLoops(activeLoops);

	for (auto &loop : activeLoops) {
		MPointArray loopPoints;
		status = loop.getPoints(loopPoints);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		if (loop.isClosed()) {
			loopPoints.append(loopPoints[1]);
			loopPoints.insert(loopPoints[loopPoints.length() - 3], 0);
		}

		loopPoints = (plane != SPlane::ZERO) ? plane.project(loopPoints) : loopPoints;

		CurvatureData data;
		data.setNumSamples(loopPoints.length()-2);

		for (unsigned int i = 1; i < loopPoints.length()-1; i++) {
			MPointArray triad;
			triad.append(loopPoints[i - 1]);
			triad.append(loopPoints[i]);
			triad.append(loopPoints[i + 1]);

			MPoint center;
			double radius;
			status = SMath::threePointCircle(triad, center, radius);
			data.samplePoints[i-1] = loopPoints[i];
			data.sampleNormals[i-1] = (status == MS::kSuccess) ? (loopPoints[i] - center).normal()*(1 / radius) : MVector::zero;
		}
		if (!loop.isClosed()) {
			// Insert zero value to the begining...
			data.samplePoints.insert(loopPoints[0], 0);
			data.sampleNormals.insert(MVector::zero, 0);
			data.profilePoints.insert(loopPoints[0], 0);

			// ...And to the end
			data.samplePoints.append(loopPoints[loopPoints.length() - 1]);
			data.sampleNormals.append(MVector::zero);
			data.profilePoints.append(loopPoints[0]);
		}

		geometry.geoData.push_back(data);
	}

	return MS::kSuccess;
}

std::map <unsigned int, CurvatureGeometry> *CurvatureCombNode::getGeoData() {
	return &m_geoData;
}