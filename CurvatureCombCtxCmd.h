#pragma once

#include "CurvatureCombCtx.h"

#include <maya\MPxContextCommand.h>

class CurvatureCombCtxCmd : public MPxContextCommand
{
public:
	CurvatureCombCtxCmd();
	virtual ~CurvatureCombCtxCmd();

	static void *creator();
	virtual MPxContext* makeObj();
};