#include "stdafx.h"
#include "NetLoad.h"

#include <osg/Geode>

#include <Box.h>
#include <CircularTorus.h>
#include <CombineGeometry.h>
#include <Cone.h>
#include <Cylinder.h>
#include <Ellipsoid.h>
#include <Prism.h>
#include <Pyramid.h>
#include <RectangularTorus.h>
#include <RectCirc.h>
#include <Saddle.h>
#include <SCylinder.h>
#include <Snout.h>
#include <Sphere.h>
#include <Wedge.h>

#ifdef __cplusplus_cli

using namespace System;
using namespace System::Collections::Generic;
using namespace NHibernate;
using namespace DbModel;

osg::Vec4 CvtColor(int color);

inline void Point2Vec3(Point^ pnt, osg::Vec3 &vec)
{
	vec[0] = pnt->X;
	vec[1] = pnt->Y;
	vec[2] = pnt->Z;
}

osg::Node* CreateCylinders(NHibernate::ISession^ session)
{
	osg::Geode *pCylinders = new osg::Geode();
	IList<Cylinder^>^ cylList = session->CreateQuery("from Cylinder")->List<Cylinder^>();
	osg::Vec3 org, height;
	for (int i = 0; i < cylList->Count; ++i) {
		Cylinder^ cyl = cylList->default[i];
		Point2Vec3(cyl->Org, org);
		Point2Vec3(cyl->Height, height);

		osg::ref_ptr<Geometry::Cylinder> geoCyl(new Geometry::Cylinder);
		geoCyl->setOrg(org);
		geoCyl->setHeight(height);
		geoCyl->setRadius(cyl->Radius);
		geoCyl->setColor(CvtColor(cyl->Color));
		geoCyl->draw();

		pCylinders->addDrawable(geoCyl);
	}

	//osg::Group *pCylinders = new osg::Group();
	//IList<Cylinder^>^ cylList = session->CreateQuery("from Cylinder")->List<Cylinder^>();
	//for (int i = 0; i < cylList->Count; ++i) {
	//	Cylinder^ cyl = cylList->default[i];
	//	pCylinders->addChild(BuildCylinder(cyl));
	//}

	return pCylinders;
}

osg::Node* CreateSCylinder(NHibernate::ISession^ session)
{
	osg::Geode *pSCylinder = new osg::Geode();
	IList<SCylinder^>^ cylList = session->CreateQuery("from SCylinder")->List<SCylinder^>();
	osg::Vec3 org, height, bottomNormal;
	for each (SCylinder^ scylinder in cylList)
	{
		Point2Vec3(scylinder->Org, org);
		Point2Vec3(scylinder->Height, height);
		Point2Vec3(scylinder->BottomNormal, bottomNormal);

		osg::ref_ptr<Geometry::SCylinder> geoSCyl(new Geometry::SCylinder);
		geoSCyl->setOrg(org);
		geoSCyl->setHeight(height);
		geoSCyl->setBottomNormal(bottomNormal);
		geoSCyl->setRadius(scylinder->Radius);
		geoSCyl->setColor(CvtColor(scylinder->Color));
		geoSCyl->draw();
		pSCylinder->addDrawable(geoSCyl);
	}
	return pSCylinder;
}

osg::Node* CreateCone(NHibernate::ISession^ session)
{
	osg::Geode *pCones = new osg::Geode();
	IList<DbModel::Cone^>^ coneList = session->CreateQuery("from Cone")->List<DbModel::Cone^>();
	osg::Vec3 center, height, offset;
	for (int i = 0; i < coneList->Count; ++i)
	{
		DbModel::Cone^ cone = coneList->default[i];
		Point2Vec3(cone->Org, center);
		Point2Vec3(cone->Height, height);

		if (osg::equivalent(cone->TopRadius, 0.0, Geometry::GetEpsilon()))
		{
			osg::ref_ptr<Geometry::Cone> geoCone(new Geometry::Cone);
			geoCone->setOrg(center);
			geoCone->setHeight(height);
			geoCone->setOffset(offset);
			geoCone->setRadius(cone->BottomRadius);
			geoCone->setColor(CvtColor(cone->Color));
			geoCone->draw();
			pCones->addDrawable(geoCone);
		}
		else if (osg::equivalent(cone->BottomRadius, 0.0, Geometry::GetEpsilon()))
		{
			osg::ref_ptr<Geometry::Cone> geoCone(new Geometry::Cone);
			geoCone->setOrg(center);
			geoCone->setHeight(height);
			geoCone->setOffset(offset);
			geoCone->setRadius(cone->TopRadius);
			geoCone->setColor(CvtColor(cone->Color));
			geoCone->draw();
			pCones->addDrawable(geoCone);
		}
		else
		{
			osg::ref_ptr<Geometry::Snout> geoSnout(new Geometry::Snout);
			geoSnout->setOrg(center);
			geoSnout->setHeight(height);
			geoSnout->setOffset(offset);
			geoSnout->setBottomRadius(cone->BottomRadius);
			geoSnout->setTopRadius(cone->TopRadius);
			geoSnout->setColor(CvtColor(cone->Color));
			geoSnout->draw();
			pCones->addDrawable(geoSnout);
		}
	}
	return pCones;
}

osg::Geode * CreateBoxs(NHibernate::ISession^ session)
{
	osg::Geode *pBoxs = new osg::Geode();
	IList<Box^>^ boxList = session->CreateQuery("from Box")->List<Box^>();
	osg::Vec3 org, xlen, ylen, zlen;
	for (int i = 0; i < boxList->Count; ++i) {
		Box^ box = boxList->default[i];
		Point2Vec3(box->Org, org);
		Point2Vec3(box->XLen, xlen);
		Point2Vec3(box->YLen, ylen);
		Point2Vec3(box->ZLen, zlen);

		osg::ref_ptr<Geometry::Box> geoBox(new Geometry::Box);
		geoBox->setOrg(org);
		geoBox->setXLen(xlen);
		geoBox->setYLen(ylen);
		geoBox->setZLen(zlen);
		geoBox->setColor(CvtColor(box->Color));
		geoBox->draw();

		pBoxs->addDrawable(geoBox);
	}

	return pBoxs;
}

osg::Node* CreateCircularTorus(NHibernate::ISession^ session)
{
	osg::Geode *pCts = new osg::Geode();
	IList<CircularTorus^>^ ctList = session->CreateQuery("from CircularTorus")->List<CircularTorus^>();
	osg::Vec3 center, startPnt, normal;
	for (int i = 0; i < ctList->Count; ++i) {
		CircularTorus^ ct = ctList->default[i];
		Point2Vec3(ct->Center, center);
		Point2Vec3(ct->StartPnt, startPnt);
		Point2Vec3(ct->Normal, normal);

		osg::ref_ptr<Geometry::CircularTorus> geoCt(new Geometry::CircularTorus);
		geoCt->setCenter(center);
		geoCt->setStartPnt(startPnt);
		geoCt->setNormal(normal);
		geoCt->setStartRadius(ct->StartRadius);
		geoCt->setEndRadius(ct->EndRadius);
		geoCt->setAngle(ct->Angle);
		geoCt->setColor(CvtColor(ct->Color));
		geoCt->draw();
		pCts->addDrawable(geoCt);
	}
	return pCts;
}

osg::Node* CreateSnout(NHibernate::ISession^ session)
{
	osg::Geode *pSnouts = new osg::Geode();
	IList<Snout^>^ snoutList = session->CreateQuery("from Snout")->List<Snout^>();
	osg::Vec3 center, height, offset;
	for (int i = 0; i < snoutList->Count; ++i) {
		Snout^ snout = snoutList->default[i];
		Point2Vec3(snout->Org, center);
		Point2Vec3(snout->Height, height);
		Point2Vec3(snout->Offset, offset);

		if (osg::equivalent(snout->TopRadius, 0.0, Geometry::GetEpsilon()))
		{
			osg::ref_ptr<Geometry::Cone> geoCone(new Geometry::Cone);
			geoCone->setOrg(center);
			geoCone->setHeight(height);
			geoCone->setOffset(offset);
			geoCone->setRadius(snout->BottomRadius);
			geoCone->setColor(CvtColor(snout->Color));
			geoCone->draw();
			pSnouts->addDrawable(geoCone);
		}
		else if (osg::equivalent(snout->BottomRadius, 0.0, Geometry::GetEpsilon()))
		{
			osg::ref_ptr<Geometry::Cone> geoCone(new Geometry::Cone);
			geoCone->setOrg(center);
			geoCone->setHeight(height);
			geoCone->setOffset(offset);
			geoCone->setRadius(snout->TopRadius);
			geoCone->setColor(CvtColor(snout->Color));
			geoCone->draw();
			pSnouts->addDrawable(geoCone);
		}
		else
		{
			osg::ref_ptr<Geometry::Snout> geoSnout(new Geometry::Snout);
			geoSnout->setOrg(center);
			geoSnout->setHeight(height);
			geoSnout->setOffset(offset);
			geoSnout->setBottomRadius(snout->BottomRadius);
			geoSnout->setTopRadius(snout->TopRadius);
			geoSnout->setColor(CvtColor(snout->Color));
			geoSnout->draw();
			pSnouts->addDrawable(geoSnout);
		}
	}
	return pSnouts;
}

osg::Node* CreateDish(NHibernate::ISession^ session)
{
	osg::Geode *pDishs = new osg::Geode();
	IList<Dish^>^ dishList = session->CreateQuery("from Dish")->List<Dish^>();
	osg::Vec3 center, height;
	for (int i = 0; i < dishList->Count; ++i) {
		Dish^ dish = dishList->default[i];
		Point2Vec3(dish->Org, center);
		Point2Vec3(dish->Height, height);
		if (dish->IsEllipse) {
			osg::ref_ptr<Geometry::Ellipsoid> geoEllipsoid(new Geometry::Ellipsoid);
			geoEllipsoid->setCenter(center);
			geoEllipsoid->setALen(height);
			geoEllipsoid->setBRadius(dish->Radius);
			geoEllipsoid->setAngle(M_PI);
			geoEllipsoid->setColor(CvtColor(dish->Color));
			geoEllipsoid->draw();
			pDishs->addDrawable(geoEllipsoid);
		}
		else {
			double h = height.length();
			double sphereRadius = (dish->Radius * dish->Radius + h * h) / 2.0 / h;
			double angle = M_PI / 2.0 - asin(2.0 * dish->Radius * h / (dish->Radius * dish->Radius + h * h));
			if (dish->Radius >= h)
			{
				angle = M_PI - angle * 2.0;
			}
			else
			{
				angle = M_PI + angle * 2.0;
			}
			osg::Vec3 bottomNormal = -height;
			bottomNormal.normalize();

			osg::ref_ptr<Geometry::Sphere> geoSphere(new Geometry::Sphere);
			geoSphere->setCenter(center);
			geoSphere->setBottomNormal(bottomNormal);
			geoSphere->setRadius(sphereRadius);
			geoSphere->setAngle(angle);
			geoSphere->setColor(CvtColor(dish->Color));
			geoSphere->draw();
			pDishs->addDrawable(geoSphere);
		}
	}
	return pDishs;
}

osg::Node* CreatePyramid(NHibernate::ISession^ session)
{
	osg::ref_ptr<osg::Geode> pPyramids = new osg::Geode();
	IList<Pyramid^>^ pyramidList = session->CreateQuery("from Pyramid")->List<Pyramid^>();
	osg::Vec3 org, height, xAxis, offset;
	for each (Pyramid^ pyramid in pyramidList)
	{
		Point2Vec3(pyramid->Org, org);
		Point2Vec3(pyramid->Height, height);
		Point2Vec3(pyramid->XAxis, xAxis);
		Point2Vec3(pyramid->Offset, offset);

		osg::ref_ptr<Geometry::Pyramid> geoPyramid(new Geometry::Pyramid);
		geoPyramid->setOrg(org);
		geoPyramid->setHeight(height);
		geoPyramid->setXAxis(xAxis);
		geoPyramid->setOffset(offset);
		geoPyramid->setBottomXLen(pyramid->BottomXLen);
		geoPyramid->setBottomYLen(pyramid->BottomYLen);
		geoPyramid->setTopXLen(pyramid->TopXLen);
		geoPyramid->setTopYLen(pyramid->TopYLen);
		geoPyramid->setColor(CvtColor(pyramid->Color));
		geoPyramid->draw();
		pPyramids->addDrawable(geoPyramid);
	}
	return pPyramids.release();
}

osg::Node* CreateRectangularTorus(NHibernate::ISession^ session)
{
	osg::Geode *pRt = new osg::Geode();
	IList<RectangularTorus^>^ rtList = session->CreateQuery("from RectangularTorus")->List<RectangularTorus^>();
	osg::Vec3 center, startPnt, normal;
	for each (RectangularTorus^ rt in rtList) {
		Point2Vec3(rt->Center, center);
		Point2Vec3(rt->StartPnt, startPnt);
		Point2Vec3(rt->Normal, normal);

		osg::ref_ptr<Geometry::RectangularTorus> geoRt(new Geometry::RectangularTorus);
		geoRt->setCenter(center);
		geoRt->setStartPnt(startPnt);
		geoRt->setNormal(normal);
		geoRt->setStartWidth(rt->StartWidth);
		geoRt->setStartHeight(rt->StartHeight);
		geoRt->setEndWidth(rt->EndWidth);
		geoRt->setEndHeight(rt->EndHeight);
		geoRt->setAngle(rt->Angle);
		geoRt->setColor(CvtColor(rt->Color));
		geoRt->draw();
		pRt->addDrawable(geoRt);
	}
	return pRt;
}

osg::Node* CreateWedge(NHibernate::ISession^ session)
{
	osg::Geode *pWedge = new osg::Geode();
	IList<Wedge^>^ wedgeList = session->CreateQuery("from Wedge")->List<Wedge^>();
	osg::Vec3 org, edge1, edge2, height;
	for each (Wedge^ wedge in wedgeList)
	{
		Point2Vec3(wedge->Org, org);
		Point2Vec3(wedge->Edge1, edge1);
		Point2Vec3(wedge->Edge2, edge2);
		Point2Vec3(wedge->Height, height);

		osg::ref_ptr<Geometry::Wedge> geoWedge(new Geometry::Wedge);
		geoWedge->setOrg(org);
		geoWedge->setEdge1(edge1);
		geoWedge->setEdge2(edge2);
		geoWedge->setHeight(height);
		geoWedge->setColor(CvtColor(wedge->Color));
		geoWedge->draw();
		pWedge->addDrawable(geoWedge);
	}
	return pWedge;
}

osg::Node* CreatePrism(NHibernate::ISession^ session)
{
	osg::Geode *pPrism = new osg::Geode();
	IList<Prism^>^ prismList = session->CreateQuery("from Prism")->List<Prism^>();
	osg::Vec3 org, height, bottomStartPnt;
	for each (Prism^ prism in prismList)
	{
		Point2Vec3(prism->Org, org);
		Point2Vec3(prism->Height, height);
		Point2Vec3(prism->BottomStartPnt, bottomStartPnt);

		osg::ref_ptr<Geometry::Prism> geoPrism(new Geometry::Prism);
		geoPrism->setOrg(org);
		geoPrism->setHeight(height);
		geoPrism->setBottomStartPnt(bottomStartPnt);
		geoPrism->setEdgeNum(prism->EdgeNum);
		geoPrism->setColor(CvtColor(prism->Color));
		geoPrism->draw();
		pPrism->addDrawable(geoPrism);
	}
	return pPrism;
}

osg::Node* CreateSphere(NHibernate::ISession^ session)
{
	osg::Geode *pSphere = new osg::Geode();
	IList<Sphere^>^ sphereList = session->CreateQuery("from Sphere")->List<Sphere^>();
	osg::Vec3 center, bottomNormal;
	for each (Sphere^ sphere in sphereList)
	{
		Point2Vec3(sphere->Center, center);
		Point2Vec3(sphere->BottomNormal, bottomNormal);

		osg::ref_ptr<Geometry::Sphere> geoSphere(new Geometry::Sphere);
		geoSphere->setCenter(center);
		geoSphere->setBottomNormal(bottomNormal);
		geoSphere->setRadius(sphere->Radius);
		geoSphere->setAngle(sphere->Angle);
		geoSphere->setColor(CvtColor(sphere->Color));
		geoSphere->draw();
		pSphere->addDrawable(geoSphere);
	}
	return pSphere;
}

osg::Node* CreateEllipsoid(NHibernate::ISession^ session)
{
	osg::Geode *pEllipsoid = new osg::Geode();
	IList<Ellipsoid^>^ ellipsoidList = session->CreateQuery("from Ellipsoid")->List<Ellipsoid^>();
	osg::Vec3 center, aLen;
	for each (Ellipsoid^ ellipsoid in ellipsoidList)
	{
		Point2Vec3(ellipsoid->Center, center);
		Point2Vec3(ellipsoid->ALen, aLen);

		osg::ref_ptr<Geometry::Ellipsoid> geoEllipsoid(new Geometry::Ellipsoid);
		geoEllipsoid->setCenter(center);
		geoEllipsoid->setALen(aLen);
		geoEllipsoid->setBRadius(ellipsoid->BRadius);
		geoEllipsoid->setAngle(ellipsoid->Angle);
		geoEllipsoid->setColor(CvtColor(ellipsoid->Color));
		geoEllipsoid->draw();
		pEllipsoid->addDrawable(geoEllipsoid);
	}
	return pEllipsoid;
}

osg::Node* CreateSaddle(NHibernate::ISession^ session)
{
	osg::Geode *pSaddle = new osg::Geode();
	IList<Saddle^>^ saddleList = session->CreateQuery("from Saddle")->List<Saddle^>();
	osg::Vec3 org, xLen, zLen;
	for each (Saddle^ saddle in saddleList)
	{
		Point2Vec3(saddle->Org, org);
		Point2Vec3(saddle->XLen, xLen);
		Point2Vec3(saddle->ZLen, zLen);

		osg::ref_ptr<Geometry::Saddle> geoSaddle(new Geometry::Saddle);
		geoSaddle->setOrg(org);
		geoSaddle->setXLen(xLen);
		geoSaddle->setYLen(saddle->YLen);
		geoSaddle->setZLen(zLen);
		geoSaddle->setRadius(saddle->Radius);
		geoSaddle->setColor(CvtColor(saddle->Color));
		geoSaddle->draw();
		pSaddle->addDrawable(geoSaddle);
	}
	return pSaddle;
}

osg::Node* CreateRectCirc(NHibernate::ISession^ session)
{
	osg::Geode *pRectCirc = new osg::Geode();
	IList<RectCirc^>^ rectCircList = session->CreateQuery("from RectCirc")->List<RectCirc^>();
	osg::Vec3 rectCenter, xLen, height, offset;
	for each (RectCirc^ rectCirc in rectCircList)
	{
		Point2Vec3(rectCirc->RectCenter, rectCenter);
		Point2Vec3(rectCirc->XLen, xLen);
		Point2Vec3(rectCirc->Height, height);
		Point2Vec3(rectCirc->Offset, offset);

		osg::ref_ptr<Geometry::RectCirc> geoRc(new Geometry::RectCirc);
		geoRc->setRectCenter(rectCenter);
		geoRc->setXLen(xLen);
		geoRc->setYLen(rectCirc->YLen);
		geoRc->setHeight(height);
		geoRc->setOffset(offset);
		geoRc->setRadius(rectCirc->Radius);
		geoRc->setColor(CvtColor(rectCirc->Color));
		geoRc->draw();
		pRectCirc->addDrawable(geoRc);
	}
	return pRectCirc;
}

//#include <fstream>
osg::Node* CreateCombineGeometry(NHibernate::ISession^ session)
{
	osg::Geode *pCg = new osg::Geode();
	IList<CombineGeometry^>^ cgList = session->CreateQuery("from CombineGeometry")->List<CombineGeometry^>();
	osg::Vec3 pos;
	for each (CombineGeometry^ cg in cgList)
	{
		osg::ref_ptr<Geometry::CombineGeometry> geocg(new Geometry::CombineGeometry);
		geocg->setColor(CvtColor(cg->Color));

		std::vector<std::shared_ptr<Geometry::Mesh>> meshs;
		for each (Mesh^ mesh in cg->Meshs)
		{
			std::shared_ptr<Geometry::Mesh> geomesh(new Geometry::Mesh);
			geomesh->rows = mesh->Rows;
			geomesh->colums = mesh->Colums;
			for each (MeshVertex^ vertex in mesh->Vertexs)
			{
				Point2Vec3(vertex->Pos, pos);
				geomesh->vertexs.push_back(pos);
			}
			meshs.push_back(geomesh);
		}
		geocg->setMeshs(meshs);

		std::vector<std::shared_ptr<Geometry::Shell>> shells;
		for each (DbModel::Shell^ shell in cg->Shells)
		{
			std::shared_ptr<Geometry::Shell> geoshell(new Geometry::Shell);
			for each (ShellVertex^ vertex in shell->Vertexs)
			{
				Point2Vec3(vertex->Pos, pos);
				geoshell->vertexs.push_back(pos);
			}

			for each (ShellFace^ face in shell->Faces)
			{
				geoshell->faces.push_back(face->VertexIndex);
			}
			shells.push_back(geoshell);
		}
		geocg->setShells(shells);

		std::vector<std::shared_ptr<Geometry::Polygon>> polygons;
		for each (DbModel::Polygon^ polygon in cg->Polygons)
		{
			std::shared_ptr<Geometry::Polygon> geopolygon(new Geometry::Polygon);
			for each (PolygonVertex^ vertex in polygon->Vertexs)
			{
				Point2Vec3(vertex->Pos, pos);
				geopolygon->vertexs.push_back(pos);
			}
			polygons.push_back(geopolygon);
		}
		geocg->setPolygons(polygons);
		geocg->draw();

		pCg->addDrawable(geocg);
	}
	return pCg;
}

void NetLoad(osg::ref_ptr<osg::Group> &root, const std::string &filePath)
{
	DbModel::Util^ util = gcnew DbModel::Util();
	try {
		util->init(gcnew String(filePath.c_str()), false);
		NHibernate::ISession^ session = util->SessionFactory->OpenSession();
		try {
			NHibernate::ITransaction^ tx = session->BeginTransaction();
			try {
				root->addChild(CreateCylinders(session));
				root->addChild(CreateSCylinder(session));
				root->addChild(CreateCone(session));
				root->addChild(CreateBoxs(session));
				root->addChild(CreateCircularTorus(session));
				root->addChild(CreateSnout(session));
				root->addChild(CreateDish(session));
				root->addChild(CreatePyramid(session));
				root->addChild(CreateRectangularTorus(session));
				root->addChild(CreateWedge(session));
				root->addChild(CreatePrism(session));
				root->addChild(CreateSphere(session));
				root->addChild(CreateEllipsoid(session));
				root->addChild(CreateSaddle(session));
				root->addChild(CreateRectCirc(session));
				root->addChild(CreateCombineGeometry(session));
				tx->Commit();
			}
			catch (Exception ^e) {
				tx->Rollback();
				throw;
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

#endif // __cplusplus_cli