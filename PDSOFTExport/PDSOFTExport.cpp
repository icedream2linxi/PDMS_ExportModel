// PDSOFTExport.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <rxclass.h>
#include <rxregsvc.h>
#include <accmd.h>

#include "PDBox.h"
#include "PDBox1.h"
#include "PDCylinder.h"
#include "PDConcone.h"
#include "PDEcone.h"
#include "PDSqucone.h"
#include "PDWedge.h"
#include "PDOval.h"
#include "PDSphere.h"
#include "PDTorus.h"
#include "PDSqutorus.h"
#include "PDSaddle.h"
#include "PDPrism.h"
#include "PDScylinder.h"
#include "PDPrism1.h"
#include "PDTorus1.h"
#include "PDSqutorus1.h"
#include "PDSqucir.h"
#include "PDSPolygon.h"
#include "PDRevolve.h"

void Export();

void InitApplication()
{
#if defined(VIEWER_VERSION) && defined(SUPPORT_VERSION)
	PdSupport::rxInit();
#endif

	//PdsProcessAPI::initClass();
	//PdsStdPartAPI::initClass();

	//PdsDbStructPart::rxInit();
	//PdsDbDesignPart::rxInit();
	//PdsDbStaff::rxInit();

	////attachedpart
	//PdsDbAttachedPart::rxInit();
	//PdsDbStiffener::rxInit();
	//PdsDbComSectConnector::rxInit();
	//PdsDbFilledPlate::rxInit();
	//PdsDbBaffle::rxInit();
	//PdsDbLace::rxInit();

	//PdsPtnAPI::initClass();

	PDPrimary3D::rxInit();
	PDBox::rxInit();
	PDBox1::rxInit();
	PDCylinder::rxInit();
	PDConcone::rxInit();
	PDEcone::rxInit();
	PDSqucone::rxInit();
	PDWedge::rxInit();
	PDSphere::rxInit();
	PDOval::rxInit();
	PDTorus::rxInit();
	PDSqutorus::rxInit();
	PDSaddle::rxInit();
	PDPrism::rxInit();
	PDScylinder::rxInit();
	PDPrism1::rxInit();
	PDTorus1::rxInit();
	PDSqutorus1::rxInit();
	PDSqucir::rxInit();
	PDSpolygon::rxInit();
	PDRevolve::rxInit();

	//PdsDbConnect::rxInit();
	//PdsDbBoltConnect::rxInit();
	//PdsDbBolt::rxInit();
	//PdsDbNut::rxInit();
	//PdsDbWasher::rxInit();
	//PdsDbPrecomp::rxInit();
	//PdsDbSupVirBox::rxInit();

	acrxBuildClassHierarchy();

	acedRegCmds->addCommand(_T("PDSOFT_EXPORT"), _T("PDExport"), _T("PDExport"), ACRX_CMD_TRANSPARENT | ACRX_CMD_USEPICKSET, Export);
}

void UnloadApplication()
{
#if defined(VIEWER_VERSION) && defined(SUPPORT_VERSION)
	deleteAcRxClass(PdSupport::desc());
#endif

	//PdsPtnAPI::unloadClass();

	//deleteAcRxClass(PdsDbLace::desc());
	//deleteAcRxClass(PdsDbBaffle::desc());
	//deleteAcRxClass(PdsDbFilledPlate::desc());
	//deleteAcRxClass(PdsDbComSectConnector::desc());
	//deleteAcRxClass(PdsDbStiffener::desc());
	//deleteAcRxClass(PdsDbAttachedPart::desc());

	//deleteAcRxClass(PdsDbStaff::desc());
	//deleteAcRxClass(PdsDbDesignPart::desc());
	//deleteAcRxClass(PdsDbStructPart::desc());

	//PdsStdPartAPI::unloadClass();
	//PdsProcessAPI::unloadClass();

	acedRegCmds->removeGroup(_T("PDSOFT_EXPORT"));

	//deleteAcRxClass(PdsDbSupVirBox::desc());
	//deleteAcRxClass(PdsDbPrecomp::desc());
	//deleteAcRxClass(PdsDbWasher::desc());
	//deleteAcRxClass(PdsDbNut::desc());
	//deleteAcRxClass(PdsDbBolt::desc());
	//deleteAcRxClass(PdsDbBoltConnect::desc());
	//deleteAcRxClass(PdsDbConnect::desc());

	deleteAcRxClass(PDRevolve::desc());
	deleteAcRxClass(PDSpolygon::desc());
	deleteAcRxClass(PDSqucir::desc());
	deleteAcRxClass(PDSqutorus1::desc());
	deleteAcRxClass(PDTorus1::desc());
	deleteAcRxClass(PDPrism1::desc());
	deleteAcRxClass(PDScylinder::desc());
	deleteAcRxClass(PDPrism::desc());
	deleteAcRxClass(PDSaddle::desc());
	deleteAcRxClass(PDSqutorus::desc());
	deleteAcRxClass(PDTorus::desc());
	deleteAcRxClass(PDOval::desc());
	deleteAcRxClass(PDSphere::desc());
	deleteAcRxClass(PDWedge::desc());
	deleteAcRxClass(PDSqucone::desc());
	deleteAcRxClass(PDEcone::desc());
	deleteAcRxClass(PDConcone::desc());
	deleteAcRxClass(PDCylinder::desc());
	deleteAcRxClass(PDBox1::desc());
	deleteAcRxClass(PDBox::desc());
	deleteAcRxClass(PDPrimary3D::desc());
}

extern "C" AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* ptr) {
	static void *pSvc, *moduleHandle;
	switch (msg) {
	case AcRx::kInitAppMsg:
		acrxDynamicLinker->unlockApplication(ptr);
		moduleHandle = ptr;
		pSvc = acrxRegisterService(_T("PDP_Mreality"));
		InitApplication();
		break;
	case AcRx::kUnloadAppMsg:
		UnloadApplication();
	case AcRx::kDependencyMsg:
		// Since we only have one service registered, this should
		// always be true,  but it doesn't hurt to be sure.
		// So,  we've got a dependency. Lock ourself so we can't be
		// unloaded.
		//
		if (pSvc == ptr)
			acrxDynamicLinker->lockApplication(moduleHandle);
		break;

		// The kNoDependencyMsg gets sent when a service this app has registered
		// has it's dependency count decremented from 1 to 0 (via a call to
		// the service's removeDependency() method).  When this message is sent,
		// the pkt argument is a pointer to the AcRxService object that has had
		// the dependency count zero'd.  As with the kDependency method, this
		// allows an app with multiple services registered to tell which one is
		// sending the message.
		//
	case AcRx::kNoDependencyMsg:
		// No more dependency,  so unlock to make it possible to be
		// unloaded.
		//
		if (pSvc == ptr)
			acrxDynamicLinker->unlockApplication(moduleHandle);
		break;
	default:
		break;
	}
	return AcRx::kRetOK;
}

void Export()
{
	acutPrintf(L"PDSOFT Export ...\n");
}