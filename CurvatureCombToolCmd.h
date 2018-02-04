#pragma once

#include <maya\MPxToolCommand.h>
#include <maya\MDagModifier.h>
#include <vector>

struct CurvatureCombAttr {
	CurvatureCombAttr(MObject& pluginNode, double attrValue) {
		node = pluginNode;
		initialValue = attrValue;
	}

	MObject node;
	double initialValue;
};

class CurvatureCombToolCmd : public MPxToolCommand
{
public:
	CurvatureCombToolCmd();
	virtual ~CurvatureCombToolCmd();

	static void* CurvatureCombToolCmd::creator();

	virtual bool CurvatureCombToolCmd::isUndoable() const;

	virtual MStatus CurvatureCombToolCmd::doIt(const MArgList& args);
	virtual MStatus CurvatureCombToolCmd::redoIt();
	virtual MStatus CurvatureCombToolCmd::undoIt();

	virtual MStatus CurvatureCombToolCmd::finalize();

	enum CurvatureAttribute {
		kScale,
		kSamples,
		kSubdivisions
	};

	void setActiveNodes(std::vector <CurvatureCombAttr> &activeNodes);
	void setDelta(short delta);
	void setAttr(CurvatureAttribute attr);

private:
	MDagModifier m_dagMod;
	CurvatureAttribute m_attr;

	std::vector <CurvatureCombAttr> m_activeNodes;
	short m_delta = 0;
};