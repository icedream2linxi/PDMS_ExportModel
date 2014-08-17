// MFC_OSG.cpp : implementation of the cOSG class
//
#include "stdafx.h"
#include "MFC_OSG.h"
#include <osgGA/StateSetManipulator>
#include <osg/MatrixTransform>
#include <osgDB/WriteFile>
#include <osg/Multisample>
#include <osgGA/AnimationPathManipulator>

#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <Standard_ConstructionError.hxx>

#include "GeometryUtility.h"
//#include <Geometry.hpp>
#include "ManipulatorTravel.h"
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

using namespace System;
using namespace System::Collections::Generic;
using namespace NHibernate;
using namespace DbModel;

#define MULTI_SAMPLES 0

class AxesCallback : public osg::NodeCallback
{
public:
	AxesCallback(cOSG* mOsg) :_mOsg(mOsg){}
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
		{
			osg::Camera* camera = dynamic_cast<osg::Camera*>(node);
			if (camera)
			{
				osg::Vec3 eye, center, up;
				//_mOsg->getManipulator()->getMatrix().getLookAt(eye,center,up,30);
				_mOsg->getViewer()->getCamera()->getViewMatrixAsLookAt(eye, center, up, 30);
				osg::Matrixd matrix;
				matrix.makeLookAt(eye - center, osg::Vec3(0, 0, 0), up); // always look at (0, 0, 0)
				camera->setViewMatrix(matrix);
				//camera->setProjectionMatrixAsOrtho(-1.5, _mOsg/50, -1.5, _height/50)
			}
		}
		traverse(node, nv);
	}
private:
	cOSG* _mOsg;
};

cOSG::cOSG(HWND hWnd) :
   m_hWnd(hWnd)
   , mHints(new osg::TessellationHints)
{
	mHints->setDetailRatio(0.5f);
}

cOSG::~cOSG()
{
    mViewer->setDone(true);
    Sleep(1000);
    mViewer->stopThreading();

    delete mViewer;
}

void cOSG::InitOSG(std::string modelname)
{
    // Store the name of the model to load
    m_ModelName = modelname;

    // Init different parts of OSG
    InitManipulators();
    InitSceneGraph();
    InitCameraConfig();
}

void cOSG::InitManipulators(void)
{
    // Create a trackball manipulator
    trackball = new osgGA::TrackballManipulator();

    // Create a Manipulator Switcher
    keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

    // Add our trackball manipulator to the switcher
    keyswitchManipulator->addMatrixManipulator( '1', "Trackball", trackball.get());

	//osg::ref_ptr<osgGA::AnimationPathManipulator> apm = new osgGA::AnimationPathManipulator();
	//keyswitchManipulator->addMatrixManipulator('2', "Path", apm);

	//keyswitchManipulator->addMatrixManipulator('3', "Flight", new osgGA::FlightManipulator());

    // Init the switcher to the first manipulator (in this case the only manipulator)
    keyswitchManipulator->selectMatrixManipulator(0);  // Zero based index Value
}

void cOSG::InitSceneGraph(void)
{
    // Init the main Root Node/Group
    mRoot  = new osg::Group;

#if MULTI_SAMPLES > 0
	osg::Multisample *ms = new osg::Multisample;
	ms->setHint(osg::Multisample::NICEST);
	mRoot->getOrCreateStateSet()->setAttributeAndModes(ms, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
#endif

	path modelFile(m_ModelName);
	if (modelFile.extension() == ".db") {
		mModel = InitOSGFromDb();
		if (mModel == NULL)
			return;
		//osg::ref_ptr<osg::StateSet> stateSet = mModel->getOrCreateStateSet();
		//stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
		//stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		//stateSet->setMode(GL_LIGHTING, )
	}
	else if (modelFile.extension() == ".lst")
	{
		ifstream fin(m_ModelName);
		osg::ref_ptr<osg::Group> group = new osg::Group();
		string line;
		path parentPath = modelFile.parent_path();
		while (getline(fin, line))
		{
			path fileName(line);
			path filePath = parentPath / fileName;
			osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filePath);
			group->addChild(node);
		}
		mModel = group;
	}
	else {
		// Load the Model from the model name
		mModel = osgDB::readNodeFile(m_ModelName);
	}

	if (!mModel) return;

	// Optimize the model
	osgUtil::Optimizer optimizer;
	optimizer.optimize(mModel.get());
	if (mPoints != NULL)
		optimizer.optimize(mPoints);
	optimizer.reset();

	// Add the model to the scene
	mRoot->addChild(mModel.get());
	if (mPoints != NULL)
		mRoot->addChild(mPoints);
}

void cOSG::InitCameraConfig(void)
{
    // Local Variable to hold window size data
    RECT rect;

    // Create the viewer for this window
    mViewer = new osgViewer::Viewer();

    // Add a Stats Handler to the viewer
    mViewer->addEventHandler(new osgViewer::StatsHandler);
	mViewer->addEventHandler(new osgGA::StateSetManipulator(mViewer->getCamera()->getOrCreateStateSet()));

    // Get the current window size
    ::GetWindowRect(m_hWnd, &rect);

    // Init the GraphicsContext Traits
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

    // Init the Windata Variable that holds the handle for the Window to display OSG in.
    osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowWin32::WindowData(m_hWnd);

    // Setup the traits parameters
    traits->x = 0;
    traits->y = 0;
    traits->width = rect.right - rect.left;
    traits->height = rect.bottom - rect.top;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->setInheritedWindowPixelFormat = true;
    traits->inheritedWindowData = windata;

#if MULTI_SAMPLES > 0
	osg::ref_ptr<osg::DisplaySettings> &ds = osg::DisplaySettings::instance();
	traits->sampleBuffers = ds->getMultiSamples();
	traits->samples = 4;
#endif

    // Create the Graphics Context
    osg::GraphicsContext* gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    // Init Master Camera for this View
    osg::ref_ptr<osg::Camera> camera = mViewer->getCamera();

    // Assign Graphics Context to the Camera
    camera->setGraphicsContext(gc);

    // Set the viewport for the Camera
    camera->setViewport(new osg::Viewport(traits->x, traits->y, traits->width, traits->height));

    // Set projection matrix and camera attribtues
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.4f, 1.0f));
    camera->setProjectionMatrixAsPerspective(
        30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0, 1000.0);

    // Add the Camera to the Viewer
    //mViewer->addSlave(camera.get());
    mViewer->setCamera(camera.get());

    // Add the Camera Manipulator to the Viewer
    mViewer->setCameraManipulator(keyswitchManipulator.get());
	mViewer->addEventHandler(new osgViewer::RecordCameraPathHandler);

    // Set the Scene Data
    mViewer->setSceneData(mRoot.get());

	InitAxis(traits->width, traits->height);

    // Realize the Viewer
    mViewer->realize();

    // Correct aspect ratio
    /*double fovy,aspectRatio,z1,z2;
    mViewer->getCamera()->getProjectionMatrixAsPerspective(fovy,aspectRatio,z1,z2);
    aspectRatio=double(traits->width)/double(traits->height);
    mViewer->getCamera()->setProjectionMatrixAsPerspective(fovy,aspectRatio,z1,z2);*/
}

void cOSG::PreFrameUpdate()
{
    // Due any preframe updates in this routine
}

void cOSG::PostFrameUpdate()
{
    // Due any postframe updates in this routine
}

osg::Group *cOSG::InitOSGFromDb()
{
	DbModel::Util^ util = gcnew DbModel::Util();
	try {
		util->init(gcnew String(m_ModelName.c_str()), false);
		NHibernate::ISession^ session = util->SessionFactory->OpenSession();
		try {
			NHibernate::ITransaction^ tx = session->BeginTransaction();
			try {
				osg::Group* group = new osg::Group;
				group->addChild(CreateCylinders(session));
				group->addChild(CreateSCylinder(session));
				group->addChild(CreateCone(session));
				group->addChild(CreateBoxs(session));
				group->addChild(CreateCircularTorus(session));
				group->addChild(CreateSnout(session));
				group->addChild(CreateDish(session));
				group->addChild(CreatePyramid(session));
				group->addChild(CreateRectangularTorus(session));
				group->addChild(CreateWedge(session));
				group->addChild(CreatePrism(session));
				group->addChild(CreateSphere(session));
				group->addChild(CreateEllipsoid(session));
				group->addChild(CreateSaddle(session));
				group->addChild(CreateRectCirc(session));
				group->addChild(CreateCombineGeometry(session));
				tx->Commit();
				return group;
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

	return NULL;
}

inline void Point2Vec3(Point^ pnt, osg::Vec3 &vec)
{
	vec[0] = pnt->X;
	vec[1] = pnt->Y;
	vec[2] = pnt->Z;
}

osg::Node* cOSG::CreateCylinders(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateSCylinder(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateCone(NHibernate::ISession^ session)
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

osg::Geode * cOSG::CreateBoxs(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateCircularTorus(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateSnout(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateDish(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreatePyramid(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateRectangularTorus(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateWedge(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreatePrism(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateSphere(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateEllipsoid(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateSaddle(NHibernate::ISession^ session)
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

osg::Node* cOSG::CreateRectCirc(NHibernate::ISession^ session)
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
osg::Node* cOSG::CreateCombineGeometry(NHibernate::ISession^ session)
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

void cOSG::CreatePoint(const osg::Vec3 &pos, int idx)
{
	if (mPoints == NULL)
		mPoints = new osg::Geode();
	osg::Sphere *pSphere = new osg::Sphere(pos, 5.0f);
	osg::ShapeDrawable *pShape = new osg::ShapeDrawable(pSphere, mHints);
	osg::Vec4 col(1, 0, 0, 0);
	switch (idx)
	{
	case 0:
		col = osg::Vec4(1, 0, 0, 0);
		break;
	case 1:
		col = osg::Vec4(0, 1, 0, 0);
		break;
	case 2:
		col = osg::Vec4(0, 0, 1, 0);
		break;
	default:
		break;
	}
	pShape->setColor(col);
	mPoints->addDrawable(pShape);
}

void cOSG::InitAxis(double width, double height)
{
	//´´½¨×ø±êÖá
	Axescamera = new osg::Camera;
	Axescamera->setProjectionMatrix(osg::Matrix::ortho2D(-1.5, width / 50, -1.5, height / 50));
	Axescamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	Axescamera->setClearMask(GL_DEPTH_BUFFER_BIT);
	Axescamera->setRenderOrder(osg::Camera::POST_RENDER);
	Axescamera->setUpdateCallback(new AxesCallback(this));
	std::string path = "axes.ive";
	osg::ref_ptr<osg::Node> axes = osgDB::readNodeFile(path);
	Axescamera->addChild(axes);
	mRoot->addChild(Axescamera);
}

void cOSG::SaveAs(const TCHAR *filename)
{
	osgDB::writeNodeFile(*mRoot, filename);
}


/*void cOSG::Render(void* ptr)
{
    cOSG* osg = (cOSG*)ptr;

    osgViewer::Viewer* viewer = osg->getViewer();

    // You have two options for the main viewer loop
    //      viewer->run()   or
    //      while(!viewer->done()) { viewer->frame(); }

    //viewer->run();
    while(!viewer->done())
    {
        osg->PreFrameUpdate();
        viewer->frame();
        osg->PostFrameUpdate();
        //Sleep(10);         // Use this command if you need to allow other processes to have cpu time
    }

    // For some reason this has to be here to avoid issue: 
    // if you have multiple OSG windows up 
    // and you exit one then all stop rendering
    AfxMessageBox("Exit Rendering Thread");

    _endthread();
}*/

CRenderingThread::CRenderingThread( cOSG* ptr )
:   OpenThreads::Thread(), _ptr(ptr), _done(false)
{
}

CRenderingThread::~CRenderingThread()
{
    _done = true;
    while( isRunning() )
        OpenThreads::Thread::YieldCurrentThread();
}

void CRenderingThread::run()
{
    if ( !_ptr )
    {
        _done = true;
        return;
    }

    osgViewer::Viewer* viewer = _ptr->getViewer();
    do
    {
        _ptr->PreFrameUpdate();
        viewer->frame();
        _ptr->PostFrameUpdate();
    } while ( !testCancel() && !viewer->done() && !_done );
}
