#pragma once

#include <maya\MPxCommand.h>
#include <maya\MDagModifier.h>
#include <maya\MObjectArray.h>

#define SCALE_FLAG			"-sc"
#define SCALE_FLAG_LONG		"-scale"

#define SAMPLES_FLAG		"-sm"
#define SAMPLES_FLAG_LONG	"-samples"

#define SUBD_FLAG			"-sd"
#define SUBD_FLAG_LONG		"-subdivs"

#define OVERRIDE_SUBD_FLAG		"-osd"
#define OVERRIDE_SUBD_FLAG_LONG	"-overrideSubdivs"

class CurvatureCombCmd : public MPxCommand
{
public:
	CurvatureCombCmd();
	virtual ~CurvatureCombCmd();

	static void *creator();

	static MSyntax newSyntax();

	virtual MStatus doIt(const MArgList& argList);
	virtual MStatus redoIt();
	virtual MStatus undoIt();

	virtual bool isUndoable() const;

private:
	MDagModifier m_dagMod;

	MObject
		m_node,
		m_transform;

	bool
		m_isCreate;
};

