#pragma once

#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/api/win32/GraphicsWindowWin32>
#include <osgGA/TrackballManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgDB/DatabasePager>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <string>

class cOSG
{
public:
    cOSG(HWND hWnd);
    ~cOSG();

    void InitOSG(std::string filename);
    void InitManipulators(void);
    void InitSceneGraph(void);
    void InitCameraConfig(void);
    void SetupWindow(void);
    void SetupCamera(void);
    void PreFrameUpdate(void);
    void PostFrameUpdate(void);
    void Done(bool value) { mDone = value; }
    bool Done(void) { return mDone; }
    //static void Render(void* ptr);
	void SaveAs(const TCHAR *filename);

    osgViewer::Viewer* getViewer() { return mViewer; }

private:
	void InitAxis(double width, double height);
	osg::Group *InitOSGFromDb();
	osg::Node *CreateCylinders(NHibernate::ISession^ session);
	osg::Geode *CreateBoxs(NHibernate::ISession^ session);
	osg::Node* CreateCircularTorus(NHibernate::ISession^ session);
	void CreatePoint(const osg::Vec3 &pos, int idx = 0);
	osg::Node* CreateSnout(NHibernate::ISession^ session);
	osg::Node* CreateDish(NHibernate::ISession^ session);
	osg::Node* CreateCone(NHibernate::ISession^ session);
	osg::Node* CreatePyramid(NHibernate::ISession^ session);
	osg::Node* CreateRectangularTorus(NHibernate::ISession^ session);
	osg::Node* CreateWedge(NHibernate::ISession^ session);
	osg::Node* CreatePrism(NHibernate::ISession^ session);
	osg::Node* CreateSphere(NHibernate::ISession^ session);
	osg::Node* CreateEllipsoid(NHibernate::ISession^ session);
	osg::Node* CreateSCylinder(NHibernate::ISession^ session);
	osg::Node* CreateSaddle(NHibernate::ISession^ session);
	osg::Node* CreateRectCirc(NHibernate::ISession^ session);
	osg::Node* CreateCombineGeometry(NHibernate::ISession^ session);
private:
    bool mDone;
    std::string m_ModelName;
    HWND m_hWnd;
    osgViewer::Viewer* mViewer;
    osg::ref_ptr<osg::Group> mRoot;
    osg::ref_ptr<osg::Node> mModel;
	osg::ref_ptr<osg::Geode> mPoints;
    osg::ref_ptr<osgGA::TrackballManipulator> trackball;
    osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator;
	osg::ref_ptr<osg::TessellationHints> mHints;
	osg::ref_ptr<osg::Camera> Axescamera;
};

class CRenderingThread : public OpenThreads::Thread
{
public:
    CRenderingThread( cOSG* ptr );
    virtual ~CRenderingThread();

    virtual void run();

protected:
    cOSG* _ptr;
    bool _done;
};
