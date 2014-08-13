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
#include <Geometry.hpp>
#include "ManipulatorTravel.h"
#include <Box.h>
#include <Cylinder.h>

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

	osg::ref_ptr<osgGA::AnimationPathManipulator> apm = new osgGA::AnimationPathManipulator();
	keyswitchManipulator->addMatrixManipulator('2', "Path", apm);

	keyswitchManipulator->addMatrixManipulator('3', "Flight", new osgGA::FlightManipulator());

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
		pSCylinder->addDrawable(Geometry::BuildSCylinder(org, height, bottomNormal, scylinder->Radius, CvtColor(scylinder->Color)));
	}
	return pSCylinder;
}

osg::Node* cOSG::CreateCone(NHibernate::ISession^ session)
{
	osg::Geode *pCones = new osg::Geode();
	IList<Cone^>^ coneList = session->CreateQuery("from Cone")->List<Cone^>();
	osg::Vec3 center, height, offset;
	for (int i = 0; i < coneList->Count; ++i)
	{
		Cone^ cone = coneList->default[i];
		Point2Vec3(cone->Org, center);
		Point2Vec3(cone->Height, height);
		pCones->addDrawable(Geometry::BuildSnout(center, height, offset, cone->BottomRadius, cone->TopRadius, CvtColor(cone->Color)));
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
		pCts->addDrawable(Geometry::BuildCircularTorus(center, startPnt, normal,
			ct->StartRadius, ct->EndRadius, ct->Angle, CvtColor(ct->Color)));
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
		pSnouts->addDrawable(Geometry::BuildSnout(center, height, offset, snout->BottomRadius, snout->TopRadius, CvtColor(snout->Color)));
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
		if (dish->IsEllipse)
			pDishs->addDrawable(Geometry::BuildEllipsoid(center, height, dish->Radius, M_PI, CvtColor(dish->Color)));
		else
			pDishs->addDrawable(Geometry::BuildSphere(center, height, dish->Radius, CvtColor(dish->Color)));
	}
	return pDishs;
}

inline osg::Vec3 ToOsgVec3(Point^ pnt)
{
	return osg::Vec3(pnt->X, pnt->Y, pnt->Z);
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
		pPyramids->addDrawable(Geometry::BuildPyramid(org, height, xAxis, offset,
			pyramid->BottomXLen, pyramid->BottomYLen, pyramid->TopXLen, pyramid->TopYLen, CvtColor(pyramid->Color)));
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
		pRt->addDrawable(Geometry::BuildRectangularTorus(center, startPnt, normal,
			rt->StartWidth, rt->StartHeight, rt->EndWidth, rt->EndHeight, rt->Angle, CvtColor(rt->Color)));
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
		pWedge->addDrawable(Geometry::BuildWedge(org, edge1, edge2, height, CvtColor(wedge->Color)));
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
		pPrism->addDrawable(Geometry::BuildPrism(org, height, bottomStartPnt, prism->EdgeNum, CvtColor(prism->Color)));
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
		pSphere->addDrawable(Geometry::BuildSphere(center, bottomNormal, sphere->Radius, sphere->Angle, CvtColor(sphere->Color)));
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
		pEllipsoid->addDrawable(Geometry::BuildEllipsoid(center, aLen, ellipsoid->BRadius, ellipsoid->Angle, CvtColor(ellipsoid->Color)));
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
		pSaddle->addDrawable(Geometry::BuildSaddle(org, xLen, saddle->YLen, zLen, saddle->Radius, CvtColor(saddle->Color)));
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
		pRectCirc->addDrawable(Geometry::BuildRectCirc(rectCenter, xLen, rectCirc->YLen, height, offset, rectCirc->Radius, CvtColor(rectCirc->Color)));
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
		Geometry::CombineGeometry geocg;
		geocg.color = CvtColor(cg->Color);
		for each (Mesh^ mesh in cg->Meshs)
		{
			std::shared_ptr<Geometry::CombineGeometry::Mesh> geomesh(new Geometry::CombineGeometry::Mesh);
			geomesh->rows = mesh->Rows;
			geomesh->colums = mesh->Colums;
			for each (MeshVertex^ vertex in mesh->Vertexs)
			{
				Point2Vec3(vertex->Pos, pos);
				geomesh->vertexs.push_back(pos);
			}
			geocg.meshs.push_back(geomesh);
		}

		for each (DbModel::Shell^ shell in cg->Shells)
		{
			std::shared_ptr<Geometry::CombineGeometry::Shell> geoshell(new Geometry::CombineGeometry::Shell);
			for each (ShellVertex^ vertex in shell->Vertexs)
			{
				Point2Vec3(vertex->Pos, pos);
				geoshell->vertexs.push_back(pos);
			}

			for each (ShellFace^ face in shell->Faces)
			{
				geoshell->faces.push_back(face->VertexIndex);
			}
			geocg.shells.push_back(geoshell);
		}

		for each (DbModel::Polygon^ polygon in cg->Polygons)
		{
			std::shared_ptr<Geometry::CombineGeometry::Polygon> geopolygon(new Geometry::CombineGeometry::Polygon);
			for each (PolygonVertex^ vertex in polygon->Vertexs)
			{
				Point2Vec3(vertex->Pos, pos);
				geopolygon->vertexs.push_back(pos);
			}
			geocg.polygons.push_back(geopolygon);
		}
		pCg->addDrawable(Geometry::BuildCombineGeometry(geocg));
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
