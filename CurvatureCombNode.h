#pragma once

#include <maya\MPxLocatorNode.h>
#include <maya\MUserData.h>
#include <maya\MPointArray.h>
#include <maya\MVectorArray.h>
#include <map>
#include <vector>

#include "../_library/SMesh.h"

struct CurvatureData {
	void clear() {
		samplePoints.clear();
		profilePoints.clear();
		sampleNormals.clear();
	};

	void setNumSamples(unsigned int samples){
		samplePoints.setLength(samples);
		sampleNormals.setLength(samples);
		profilePoints.setLength(samples);
	};

	MPointArray
		samplePoints,
		profilePoints;

	MVectorArray
		sampleNormals;
};

struct CurvatureGeometry{
	std::vector <CurvatureData> geoData;

	bool isDirty;
};

class CurvatureCombNode : public MPxLocatorNode
{
public:
	CurvatureCombNode();
	virtual ~CurvatureCombNode();

	static void *creator();
	static MStatus initialize();

	virtual MStatus setDependentsDirty(const MPlug &plug, MPlugArray &plugArray);

	virtual MStatus compute(const MPlug &plug, MDataBlock &datablock);
	virtual void draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus stat);

	MStatus getCurveCurvature(MObject &curve, unsigned int samples, CurvatureGeometry &geometry);
	MStatus getMeshCurvature(SMesh &mesh, CurvatureGeometry &geometry);

	std::map <unsigned int, CurvatureGeometry> *getGeoData();

	static MTypeId id;
	static MString drawClassification;
	static MString registrantId;

	static MObject aUpdate;
	static MObject aGeometry;
	static MObject aWorldGeometry;
	static MObject aSmoothGeometry;
	static MObject aComponent;
	static MObject aScale;
	static MObject aSamples;
	static MObject aSubdivs;
	static MObject aOverrideSubdivs;

private:
	std::map <unsigned int, CurvatureGeometry> m_geoData;

	bool
		m_dirtySamples,
		m_dirtyScale;
};

