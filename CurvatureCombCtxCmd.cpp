#include "CurvatureCombCtxCmd.h"

CurvatureCombCtxCmd::CurvatureCombCtxCmd(){
}

CurvatureCombCtxCmd::~CurvatureCombCtxCmd(){
}

void *CurvatureCombCtxCmd::creator() {
	return new CurvatureCombCtxCmd;
}

MPxContext* CurvatureCombCtxCmd::makeObj() {
	return new CurvatureCombCtx;
}