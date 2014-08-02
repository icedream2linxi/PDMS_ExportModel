// PDSOFTExport.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <rxclass.h>
#include <rxregsvc.h>
#include <aced.h>
#include <accmd.h>
#include <acedads.h>
#include <acestext.h>
#include <adscodes.h>
#include <dbobjptr2.h>
#include <migrtion.h>
#include <dbapserv.h>

#include <boost/scope_exit.hpp>

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

#include "TestModel.h"

using namespace Autodesk::AutoCAD::Runtime;
using namespace Autodesk::AutoCAD::ApplicationServices;

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
	acedRegCmds->addCommand(_T("PDSOFT_EXPORT"), _T("PDTestModel"), _T("PDTestModel"), ACRX_CMD_TRANSPARENT | ACRX_CMD_USEPICKSET, PDPRIMARY3D_MODEL);
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

DbModel::Point^ ToPnt(const AcGePoint3d &pnt)
{
	return gcnew DbModel::Point(pnt.x, pnt.y, pnt.z);
}

DbModel::Point^ ToPnt(const AcGeVector3d &vec)
{
	return gcnew DbModel::Point(vec.x, vec.y, vec.z);
}

int GetColor(const AcDbEntity *pEnt)
{
	AcCmColor color = pEnt->color();
	switch (color.colorMethod())
	{
	case AcCmEntityColor::kByColor:
	{
		return (int)color.color();
	}
	case AcCmEntityColor::kByACI:
	{
		int col = acedGetRGB(color.colorIndex());
		char *p = (char*)(void*)&col;
		std::swap(p[0], p[2]);
		return col;
	}
	case AcCmEntityColor::kByLayer:
	{
		AcDbObjectId layerId = pEnt->layerId();
		AcDbSmartObjectPointer<AcDbLayerTableRecord> ltr(layerId, AcDb::kForRead);
		color = ltr->color();
		if (color.isByACI())
		{
			int col = acedGetRGB(color.colorIndex());
			char *p = (char*)(void*)&col;
			std::swap(p[0], p[2]);
			return col;
		}
		else
			return (int)color.color();
	}
	case AcCmEntityColor::kByBlock:
	{
		return 0xffffff;
	}
	default:
		return 0xffffff;
	}
}

void ExportEntity(NHibernate::ISession^ session, const AcDbEntity *pEnt, const AcGeMatrix3d &mtx)
{
	if (pEnt->isKindOf(PDScylinder::desc()))
	{
		const PDScylinder &pdscylinder = *PDScylinder::cast(pEnt);
		AcGePoint3d org = pdscylinder.getPtStart();
		org.transformBy(mtx);
		AcGeVector3d height = pdscylinder.getPtEnd() - pdscylinder.getPtStart();
		height.transformBy(mtx);
		AcGeVector3d bottomNormal = pdscylinder.getBottomNormal();

		DbModel::SCylinder^ scylinder = gcnew DbModel::SCylinder();
		scylinder->Org = ToPnt(org);
		scylinder->Height = ToPnt(height);
		scylinder->BottomNormal = ToPnt(bottomNormal);
		scylinder->Radius = pdscylinder.getDiameter() / 2.0;
		scylinder->Color = GetColor(pEnt);
		session->Save(scylinder);
	}
	else if (pEnt->isKindOf(PDCylinder::desc()))
	{
		const PDCylinder &pdcyl = *PDCylinder::cast(pEnt);
		AcGePoint3d org = pdcyl.getPtStart();
		org.transformBy(mtx);
		AcGeVector3d height = (pdcyl.getPtEnd() - pdcyl.getPtStart());
		height.transformBy(mtx);

		DbModel::Cylinder^ cyl = gcnew DbModel::Cylinder();
		cyl->Org = ToPnt(org);
		cyl->Height = ToPnt(height);
		cyl->Radius = pdcyl.getDiameter() / 2.0;
		cyl->Color = GetColor(pEnt);
		session->Save(cyl);
	}
	else if (pEnt->isKindOf(PDBox::desc()))
	{
		const PDBox &pdbox = *PDBox::cast(pEnt);
		DbModel::Box^ box = gcnew DbModel::Box();
		AcGePoint3d org = pdbox.getOrign();
		box->Org = ToPnt(org.transformBy(mtx));
		box->XLen = ToPnt((pdbox.getXvec() * pdbox.getlength()).transformBy(mtx));
		box->YLen = ToPnt((pdbox.getYvec() * pdbox.getwidth()).transformBy(mtx));
		box->ZLen = ToPnt((pdbox.getZvec() * pdbox.getheight()).transformBy(mtx));
		box->Color = GetColor(pEnt);
		session->Save(box);
	}
	else if (pEnt->isKindOf(PDBox1::desc()))
	{
		const PDBox1 &pdbox = *PDBox1::cast(pEnt);
		AcGeVector3d xLen = pdbox.getVectLength();
		xLen.transformBy(mtx);
		xLen *= pdbox.getLength();
		AcGeVector3d yLen = pdbox.getVectWidth();
		yLen.transformBy(mtx);
		yLen *= pdbox.getWidth();
		AcGeVector3d zLen = xLen.crossProduct(yLen);
		zLen.normalize();
		zLen *= pdbox.getHeight();
		AcGePoint3d org = pdbox.getpointP();
		org.transformBy(mtx);
		org -= (xLen + yLen + zLen) / 2.0;

		DbModel::Box^ box = gcnew DbModel::Box();
		box->Org = ToPnt(org);
		box->XLen = ToPnt(xLen);
		box->YLen = ToPnt(yLen);
		box->ZLen = ToPnt(zLen);
		box->Color = GetColor(pEnt);
		session->Save(box);
	}
	else if (pEnt->isKindOf(PDConcone::desc()))
	{
		const PDConcone &pdcone = *PDConcone::cast(pEnt);
		AcGePoint3d org = pdcone.getpointStart();
		org.transformBy(mtx);
		AcGeVector3d height = pdcone.getpointEnd() - pdcone.getpointStart();
		height.transformBy(mtx);

		DbModel::Cone^ cone = gcnew DbModel::Cone();
		cone->Org = ToPnt(org);
		cone->Height = ToPnt(height);
		cone->BottomRadius = pdcone.getDiameter1() / 2.0;
		cone->TopRadius = pdcone.getDiameter2() / 2.0;
		cone->Color = GetColor(pEnt);
		session->Save(cone);
	}
	else if (pEnt->isKindOf(PDEcone::desc()))
	{
		const PDEcone &pdcone = *PDEcone::cast(pEnt);
		AcGePoint3d org = pdcone.getpointStart();
		org.transformBy(mtx);
		AcGeVector3d height = pdcone.getpointEnd() - pdcone.getpointStart();
		height.transformBy(mtx);
		double dbHeight = height.length();
		AcGeVector3d offset = pdcone.getVect();
		offset.transformBy(mtx).normalize();

		height.normalize();
		height = offset.crossProduct(height.crossProduct(offset));
		height.normalize();
		height *= dbHeight;

		offset *= fabs(pdcone.getDiameter1() / 2.0 - pdcone.getDiameter2() / 2.0);

		DbModel::Snout^ cone = gcnew DbModel::Snout();
		cone->Org = ToPnt(org);
		cone->Height = ToPnt(height);
		cone->BottomRadius = pdcone.getDiameter1() / 2.0;
		cone->TopRadius = pdcone.getDiameter2() / 2.0;
		cone->Offset = ToPnt(offset);
		cone->Color = GetColor(pEnt);
		session->Save(cone);
	}
	else if (pEnt->isKindOf(PDOval::desc()))
	{
		const PDOval &pdoval = *PDOval::cast(pEnt);
		AcGePoint3d center = pdoval.getEllipseCen();
		center.transformBy(mtx);
		AcGeVector3d aLen = pdoval.getVect();
		aLen.normalize();
		aLen *= pdoval.getlengthA();
		aLen.transformBy(mtx);
		double angle = atan((pdoval.getlengthA() - pdoval.getOvalHeight()) / pdoval.getlengthR());

		DbModel::Ellipsoid^ ellipsoid = gcnew DbModel::Ellipsoid();
		ellipsoid->Center = ToPnt(center);
		ellipsoid->ALen = ToPnt(aLen);
		ellipsoid->BRadius = pdoval.getlengthB();
		ellipsoid->Angle = (M_PI_2 - angle) * 2.0;
		ellipsoid->Color = GetColor(pEnt);
		session->Save(ellipsoid);
	}
	else if (pEnt->isKindOf(PDPrism::desc()))
	{
		const PDPrism &pdprism = *PDPrism::cast(pEnt);
		AcGePoint3d org = pdprism.getBottomCenter();
		org.transformBy(mtx);
		AcGeVector3d height = pdprism.getTopCenter() - pdprism.getBottomCenter();
		height.transformBy(mtx);
		AcGePoint3d bottomStartPnt = pdprism.getBottomStartPnt();
		bottomStartPnt.transformBy(mtx);

		DbModel::Prism^ prism = gcnew DbModel::Prism();
		prism->Org = ToPnt(org);
		prism->Height = ToPnt(height);
		prism->BottomStartPnt = ToPnt(bottomStartPnt);
		prism->EdgeNum = (int)pdprism.getEdgeNum();
		prism->Color = GetColor(pEnt);
		session->Save(prism);
	}
	else if (pEnt->isKindOf(PDPrism1::desc()))
	{
		const PDPrism1 &pdprism = *PDPrism1::cast(pEnt);
		AcGePoint3d org = pdprism.getBottomCenter();
		org.transformBy(mtx);
		AcGeVector3d height = pdprism.getTopCenter() - pdprism.getBottomCenter();
		height.transformBy(mtx);
		AcGePoint3d bottomEdgeCenter = pdprism.getBottomEdgeCenter();
		bottomEdgeCenter.transformBy(mtx);
		AcGeVector3d vect = bottomEdgeCenter - org;
		double angle = M_PI / pdprism.getEdgeNum();
		double radius = vect.length() / cos(angle);
		vect.normalize();
		AcGeMatrix3d xform = AcGeMatrix3d::rotation(angle, height);
		vect.transformBy(xform).normalize();
		AcGePoint3d bottomStartPnt = org + vect * radius;

		DbModel::Prism^ prism = gcnew DbModel::Prism();
		prism->Org = ToPnt(org);
		prism->Height = ToPnt(height);
		prism->BottomStartPnt = ToPnt(bottomStartPnt);
		prism->EdgeNum = (int)pdprism.getEdgeNum();
		prism->Color = GetColor(pEnt);
		session->Save(prism);
	}
	else if (pEnt->isKindOf(PDSqucone::desc()))
	{
		const PDSqucone &squcone = *PDSqucone::cast(pEnt);
		AcGePoint3d org = squcone.getOrign();
		org.transformBy(mtx);
		AcGeVector3d height = squcone.getVectH() * squcone.getHeight();
		height.transformBy(mtx);
		AcGeVector3d xAxis = squcone.getVectL();
		xAxis.transformBy(mtx).normalize();
		AcGeVector3d offset = squcone.getVectV() * squcone.getLean();
		offset.transformBy(mtx);

		DbModel::Pyramid^ pyramid = gcnew DbModel::Pyramid();
		pyramid->Org = ToPnt(org);
		pyramid->Height = ToPnt(height);
		pyramid->Offset = ToPnt(offset);
		pyramid->XAxis = ToPnt(xAxis);
		pyramid->BottomXLen = squcone.getLength1();
		pyramid->BottomYLen = squcone.getWidth1();
		pyramid->TopXLen = squcone.getLength2();
		pyramid->TopYLen = squcone.getWidth2();
		pyramid->Color = GetColor(pEnt);
		session->Save(pyramid);
	}
	else if (pEnt->isKindOf(PDWedge::desc()))
	{
		const PDWedge &pdwedge = *PDWedge::cast(pEnt);
		AcGePoint3d org = pdwedge.getpointP2();
		org.transformBy(mtx);
		AcGeVector3d edge1 = pdwedge.getpointP3() - pdwedge.getpointP2();
		edge1.transformBy(mtx);
		AcGeVector3d edge2 = pdwedge.getpointP1() - pdwedge.getpointP2();
		edge2.transformBy(mtx);
		AcGeVector3d height = pdwedge.getpointP4() - pdwedge.getpointP2();
		height.transformBy(mtx);

		DbModel::Wedge^ wedge = gcnew DbModel::Wedge();
		wedge->Org = ToPnt(org);
		wedge->Edge1 = ToPnt(edge1);
		wedge->Edge2 = ToPnt(edge2);
		wedge->Height = ToPnt(height);
		wedge->Color = GetColor(pEnt);
		session->Save(wedge);
	}
	else if (pEnt->isKindOf(PDSphere::desc()))
	{
		const PDSphere &pdsphere = *PDSphere::cast(pEnt);
		AcGePoint3d center = pdsphere.getCenter();
		center.transformBy(mtx);

		DbModel::Sphere^ sphere = gcnew DbModel::Sphere();
		sphere->Center = ToPnt(center);
		sphere->Radius = pdsphere.getRadius();
		sphere->BottomNormal = ToPnt(-AcGeVector3d::kZAxis);
		sphere->Angle = 2.0 * M_PI;
		sphere->Color = GetColor(pEnt);
		session->Save(sphere);
	}
	else if (pEnt->isKindOf(PDTorus::desc()))
	{
		const PDTorus &pdtorus = *PDTorus::cast(pEnt);
		AcGePoint3d center = pdtorus.getCenter();
		center.transformBy(mtx);
		AcGePoint3d startPnt = pdtorus.getP1();
		startPnt.transformBy(mtx);
		AcGeVector3d normal = pdtorus.getNormal();
		normal.transformBy(mtx).normalize();

		DbModel::CircularTorus^ ct = gcnew DbModel::CircularTorus();
		ct->Center = ToPnt(center);
		ct->StartPnt = ToPnt(startPnt);
		ct->Normal = ToPnt(normal);
		ct->StartRadius = pdtorus.getDiameter1() / 2.0;
		ct->EndRadius = pdtorus.getDiameter2() / 2.0;
		ct->Angle = pdtorus.getAngle();
		ct->Color = GetColor(pEnt);
		session->Save(ct);
	}
	else if (pEnt->isKindOf(PDTorus1::desc()))
	{
		const PDTorus1 &pdtorus = *PDTorus1::cast(pEnt);
		AcGePoint3d center = pdtorus.getCenter();
		center.transformBy(mtx);
		AcGePoint3d startPnt = pdtorus.getP1();
		startPnt.transformBy(mtx);
		AcGeVector3d normal = pdtorus.getP2().asVector();
		normal.transformBy(mtx);
		normal.normalize();

		DbModel::CircularTorus^ ct = gcnew DbModel::CircularTorus();
		ct->Center = ToPnt(center);
		ct->StartPnt = ToPnt(startPnt);
		ct->Normal = ToPnt(normal);
		ct->StartRadius = pdtorus.getDiameter1() / 2.0;
		ct->EndRadius = pdtorus.getDiameter2() / 2.0;
		ct->Angle = pdtorus.getAngle() * M_PI / 180.0;
		ct->Color = GetColor(pEnt);
		session->Save(ct);
	}
	else if (pEnt->isKindOf(PDSqutorus::desc()))
	{
		const PDSqutorus &pdtorus = *PDSqutorus::cast(pEnt);
		AcGePoint3d center = pdtorus.getCenter();
		center.transformBy(mtx);
		AcGePoint3d startPnt = pdtorus.getP1();
		startPnt.transformBy(mtx);
		AcGeVector3d normal = pdtorus.getNormal();
		normal.transformBy(mtx).normalize();

		DbModel::RectangularTorus^ rt = gcnew DbModel::RectangularTorus();
		rt->Center = ToPnt(center);
		rt->StartPnt = ToPnt(startPnt);
		rt->Normal = ToPnt(normal);
		rt->StartWidth = pdtorus.getLength1();
		rt->StartHeight = pdtorus.getWidth1();
		rt->EndWidth = pdtorus.getLength2();
		rt->EndHeight = pdtorus.getWidth2();
		rt->Angle = pdtorus.getAngle();
		rt->Color = GetColor(pEnt);
		session->Save(rt);
	}
	else if (pEnt->isKindOf(PDSqutorus1::desc()))
	{
		const PDSqutorus1 &pdtorus = *PDSqutorus1::cast(pEnt);
		AcGePoint3d center = pdtorus.getCenter();
		center.transformBy(mtx);
		AcGePoint3d startPnt = pdtorus.getP1();
		startPnt.transformBy(mtx);
		AcGeVector3d normal = pdtorus.getNormalP2();
		normal.normalize();

		DbModel::RectangularTorus^ rt = gcnew DbModel::RectangularTorus();
		rt->Center = ToPnt(center);
		rt->StartPnt = ToPnt(startPnt);
		rt->Normal = ToPnt(normal);
		rt->StartWidth = pdtorus.getLength1();
		rt->StartHeight = pdtorus.getWidth1();
		rt->EndWidth = pdtorus.getLength2();
		rt->EndHeight = pdtorus.getWidth2();
		rt->Angle = pdtorus.getAngle() * M_PI / 180.0;
		rt->Color = GetColor(pEnt);
		session->Save(rt);
	}
	else if (pEnt->isKindOf(PDSaddle::desc()))
	{
		const PDSaddle &pdsaddle = *PDSaddle::cast(pEnt);
		AcGePoint3d org = pdsaddle.getOrg();
		org.transformBy(mtx);
		double width = pdsaddle.getWidth();
		double length = pdsaddle.getLength();
		double height = pdsaddle.getHeight();
		double angle = pdsaddle.getAngle();
		AcGeVector3d insX, insY, insZ;
		pdsaddle.getInsDir(insX, insY, insZ);
		insX.transformBy(mtx);
		insY.transformBy(mtx);
		insZ.transformBy(mtx);

		AcGeMatrix3d trans = AcGeMatrix3d::rotation(angle, insZ);
		AcGeVector3d xLen = insX * length;
		AcGeVector3d zLen = insZ * height;

		DbModel::Saddle^ saddle = gcnew DbModel::Saddle();
		saddle->Org = ToPnt(org);
		saddle->XLen = ToPnt(xLen);
		saddle->YLen = width;
		saddle->ZLen = ToPnt(zLen);
		saddle->Radius = pdsaddle.getRadius();
		saddle->Color = GetColor(pEnt);
		session->Save(saddle);
	}
	else if (pEnt->isKindOf(PDSqucir::desc()))
	{
		const PDSqucir &pdsqucir = *PDSqucir::cast(pEnt);
		AcGeVector3d xLen = pdsqucir.getVectSqu();
		xLen *= pdsqucir.getLength();
		xLen.transformBy(mtx);
		AcGeVector3d normal = pdsqucir.getVectCir();
		normal.transformBy(mtx);
		AcGeVector3d yVec = normal.crossProduct(xLen);
		yVec.normalize();
		AcGePoint3d org = pdsqucir.getRectOrg();
		org.transformBy(mtx);
		AcGePoint3d circCenter = pdsqucir.getCircCenter();
		circCenter.transformBy(mtx);

		AcGePoint3d rectCenter = org + xLen / 2.0 + yVec * (pdsqucir.getWidth() / 2.0);
		AcGeVector3d vec = circCenter - rectCenter;
		AcGeVector3d offset = vec.orthoProject(normal);
		AcGeVector3d height = vec - offset;

		DbModel::RectCirc^ rectCirc = gcnew DbModel::RectCirc();
		rectCirc->RectCenter = ToPnt(rectCenter);
		rectCirc->XLen = ToPnt(xLen);
		rectCirc->YLen = pdsqucir.getWidth();
		rectCirc->Height = ToPnt(height);
		rectCirc->Offset = ToPnt(offset);
		rectCirc->Radius = pdsqucir.getRadius();
		rectCirc->Color = GetColor(pEnt);
		session->Save(rectCirc);
	}
	else if (pEnt->isKindOf(PDRevolve::desc()))
	{

	}
	else if (pEnt->isKindOf(PDSpolygon::desc()))
	{

	}
	else if (pEnt->isKindOf(AcDbBlockReference::desc()))
	{
		Acad::ErrorStatus es = Acad::eOk;
		const AcDbBlockReference &blockRef = *AcDbBlockReference::cast(pEnt);
		AcGeMatrix3d blockTransform = blockRef.blockTransform();
		AcDbSmartObjectPointer<AcDbBlockTableRecord> pBtr(blockRef.blockTableRecord(), AcDb::kForRead);
		if ((es = pBtr.openStatus()) != Acad::eOk)
		{
			acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
			return;
		}
		AcDbBlockTableRecordIterator *pBtri = NULL;
		es = pBtr->newIterator(pBtri);
		if (es != Acad::eOk)
		{
			acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
			return;
		}

		BOOST_SCOPE_EXIT(pBtri)
		{
			if (pBtri != NULL)
				delete pBtri;
		}
		BOOST_SCOPE_EXIT_END;

		for (; !pBtri->done(); pBtri->step())
		{
			AcDbObjectId id;
			if ((es = pBtri->getEntityId(id)) != Acad::eOk)
			{
				acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
				continue;
			}
			AcDbSmartObjectPointer<AcDbEntity> pBlockEnt(id, AcDb::kForRead);
			if ((es = pBlockEnt.openStatus()) != Acad::eOk)
			{
				acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
				continue;
			}
			ExportEntity(session, pBlockEnt, blockTransform);
		}
	}
}

void ExportModel(NHibernate::ISession^ session)
{
	Acad::ErrorStatus es = Acad::eOk;
	AcDbSmartObjectPointer<AcDbBlockTable> pBt(acdbCurDwg()->blockTableId(), AcDb::kForRead);
	if ((es = pBt.openStatus()) != Acad::eOk)
	{
		acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
		return;
	}

	AcDbObjectId recordId;
	if ((es = pBt->getAt(ACDB_MODEL_SPACE, recordId)) != Acad::eOk)
	{
		acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
		return;
	}

	AcDbSmartObjectPointer<AcDbBlockTableRecord> pBtr(recordId, AcDb::kForRead);
	if ((es = pBtr.openStatus()) != Acad::eOk)
	{
		acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
		return;
	}

	AcDbBlockTableRecordIterator *pBtri = NULL;
	if ((es = pBtr->newIterator(pBtri)) != Acad::eOk)
	{
		acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
		return;
	}
	BOOST_SCOPE_EXIT(pBtri)
	{
		if (pBtri != NULL)
			delete pBtri;
	}
	BOOST_SCOPE_EXIT_END;

	AcGeMatrix3d mtx;
	for (; !pBtri->done(); pBtri->step())
	{
		AcDbObjectId id;
		if ((es = pBtri->getEntityId(id)) != Acad::eOk)
		{
			acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
			continue;
		}
		AcDbSmartObjectPointer<AcDbEntity> pBlockEnt(id, AcDb::kForRead);
		if ((es = pBlockEnt.openStatus()) != Acad::eOk)
		{
			acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
			continue;
		}
		ExportEntity(session, pBlockEnt, mtx);
	}
}

//void ExportModel(NHibernate::ISession^ session)
//{
//	AcGeMatrix3d mtx;
//	ads_name ss, entName;
//	int nRet = acedSSGet(L"A", NULL, NULL, NULL, ss);
//	long len = 0;
//	nRet = acedSSLength(ss, &len);
//	for (long i = 0; i < len; ++i)
//	{
//		if ((nRet = acedSSName(ss, i, entName)) != RTNORM)
//			continue;
//		AcDbObjectId id;
//		Acad::ErrorStatus es = Acad::eOk;
//		if ((es = acdbGetObjectId(id, entName)) != Acad::eOk)
//		{
//			acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
//			continue;
//		}
//
//		AcDbSmartObjectPointer<AcDbEntity> pEnt(id, AcDb::kForRead);
//		if ((es = pEnt.openStatus()) != Acad::eOk)
//		{
//			acutPrintf(L"es = %s, function = %s:%d\n", acadErrorStatusText(es), __FUNCTIONW__, __LINE__);
//			continue;
//		}
//		ExportEntity(session, pEnt, mtx);
//	}
//	nRet = acedSSFree(ss);
//}

void DoExport()
{
	DbModel::Util^ util = gcnew DbModel::Util();
	try {
		util->init(gcnew System::String(L"d:/pdsoft.db"), false);
		NHibernate::ISession^ session = util->SessionFactory->OpenSession();
		try {
			NHibernate::ITransaction^ tx = session->BeginTransaction();
			try {
				ExportModel(session);
				tx->Commit();
			}
			catch (System::Exception ^e) {
				tx->Rollback();
				
				System::String^ msg = e->Message;
				msg += L"\n" + e->StackTrace;
				Document^ doc = Application::DocumentManager->MdiActiveDocument;
				doc->Editor->WriteMessage(msg);
			}
		}
		finally {
			session->Close();
		}
	}
	finally {
		util->~Util();
	}
}

public ref class Cmd
{
public:
	[CommandMethod("PDNETExport", CommandFlags::Session)]
	static void NetExport()
	{
		acutPrintf(L"PDSOFT Export ...\n");
		DoExport();
	}
};


void Export()
{
	acutPrintf(L"PDSOFT Export ...\n");
	DoExport();
}