// MFC_OSG_MDIView.cpp : implementation of the CMFC_OSG_MDIView class
//

#include "stdafx.h"
#include "MFC_OSG_MDI.h"
#include "MFC_OSG_MDIDoc.h"
#include "MFC_OSG_MDIView.h"
#include <Box.h>
#include <ViewCenterManipulator.h>
#include <DynamicLOD.h>

#include <osgGA/GUIEventHandler>
#include <osg/PositionAttitudeTransform>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int g_testBoxCount = 50;

class UserOperation : public osg::Referenced
{
public:
	virtual bool execute(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) const = 0;
};

class ResetPosOperation : public UserOperation
{
public:
	virtual bool execute(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) const
	{
		osgViewer::View* view = dynamic_cast<osgViewer::View*>(aa.asView());
		view->setCameraManipulator(new ViewCenterManipulator);
		osg::ref_ptr<osgGA::CameraManipulator> manipulator = (osgGA::CameraManipulator*)view->getCameraManipulator();

		manipulator->home(ea, aa);
		osg::Vec3d homeEye, homeCenter, homeUp;
		manipulator->getHomePosition(homeEye, homeCenter, homeUp);
		osg::Vec3d dir = osg::Vec3d(1, -1, 1);
		dir.normalize();
		osg::Matrix vm = osg::Matrix::lookAt(homeCenter + dir*(homeCenter.y() - homeEye.y()), homeCenter, manipulator->getUpVector(osg::Matrix::identity()));
		manipulator->setByInverseMatrix(vm);
		return true;
	}
};

class UserEventHandler : public osgGA::GUIEventHandler
{
public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		switch (ea.getEventType())
		{
		case osgGA::GUIEventAdapter::USER:
		{
			const UserOperation* oper = dynamic_cast<const UserOperation*>(ea.getUserData());
			if (oper == nullptr)
			{
				return false;
			}
			return const_cast<UserOperation*>(oper)->execute(ea, aa);
		}
		default:
			return false;
		}
	}
};

IMPLEMENT_DYNCREATE(CMFC_OSG_MDIView, CView)

BEGIN_MESSAGE_MAP(CMFC_OSG_MDIView, CView)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_KEYDOWN()
    ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILE_SAVE_AS, &CMFC_OSG_MDIView::OnFileSaveAs)
	ON_COMMAND(ID_TEST_OSG_BOX_TEST, &CMFC_OSG_MDIView::OnTestOsgBoxTest)
	ON_COMMAND(ID_TEST_GEOMETRY_BOX_TEST, &CMFC_OSG_MDIView::OnTestGeometryBoxTest)
	ON_COMMAND(ID_VIEW_SOUTH_WEST, &CMFC_OSG_MDIView::OnViewSouthWest)
	ON_COMMAND(ID_TEST_GEOMETRY_BOX_TEST2, &CMFC_OSG_MDIView::OnTestGeometryBoxTest2)
	ON_COMMAND(ID_TEST_OSG_BOX_TRANSFORM_TEST, &CMFC_OSG_MDIView::OnTestOsgBoxTransformTest)
END_MESSAGE_MAP()

CMFC_OSG_MDIView::CMFC_OSG_MDIView() :
   mOSG(0L)
{
}

CMFC_OSG_MDIView::~CMFC_OSG_MDIView()
{
}

BOOL CMFC_OSG_MDIView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

void CMFC_OSG_MDIView::OnDraw(CDC* /*pDC*/)
{
    CMFC_OSG_MDIDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;
}

#ifdef _DEBUG
void CMFC_OSG_MDIView::AssertValid() const
{
    CView::AssertValid();
}

void CMFC_OSG_MDIView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CMFC_OSG_MDIDoc* CMFC_OSG_MDIView::GetDocument() const // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFC_OSG_MDIDoc)));
    return (CMFC_OSG_MDIDoc*)m_pDocument;
}
#endif //_DEBUG


int CMFC_OSG_MDIView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    // Let MFC create the window before OSG
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Now that the window is created setup OSG
    mOSG = new cOSG(m_hWnd);

    return 1;
}

void CMFC_OSG_MDIView::OnDestroy()
{
    delete mThreadHandle;
    if(mOSG != 0) delete mOSG;

    //WaitForSingleObject(mThreadHandle, 1000);

    CView::OnDestroy();
}

void CMFC_OSG_MDIView::OnInitialUpdate()
{
    CView::OnInitialUpdate();

    // Get Filename from DocumentOpen Dialog
    CString csFileName = GetDocument()->GetFileName();

    // Init the osg class
    mOSG->InitOSG(csFileName.GetString());

    // Start the thread to do OSG Rendering
    //mThreadHandle = (HANDLE)_beginthread(&cOSG::Render, 0, mOSG); 
    mThreadHandle = new CRenderingThread(mOSG);
	mThreadHandle->start();
	mOSG->getViewer()->addEventHandler(new UserEventHandler);
}

void CMFC_OSG_MDIView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // Pass Key Presses into OSG
    //mOSG->getViewer()->getEventQueue()->keyPress(nChar);

    // Close Window on Escape Key
    if(nChar == VK_ESCAPE)
    {
        GetParent()->SendMessage(WM_CLOSE);
    }
}


BOOL CMFC_OSG_MDIView::OnEraseBkgnd(CDC* pDC)
{
    /* Do nothing, to avoid flashing on MSW */
    return true;
}


void CMFC_OSG_MDIView::OnFileSaveAs()
{
	CFileDialog dlg(FALSE, _T(".ive"), _T("SAM"), OFN_OVERWRITEPROMPT, _T("所有文件(*.*)|*.*||"), this);
	if (dlg.DoModal() != IDOK)
		return;
	mOSG->SaveAs(dlg.GetPathName());
}


void CMFC_OSG_MDIView::OnTestOsgBoxTest()
{
	osg::ref_ptr<osg::Group> boxGroup(new osg::Group);
	const int count = g_testBoxCount;
	double len = 50.0;
	osg::Vec3 orgx, orgxy, orgxyz;
	osg::Vec3 xvec(60, 0, 0), yvec(0, 60, 0), zvec(0, 0, 60);
	for (int i = 0; i < count; ++i)
	{
		orgxy = orgx;
		for (int j = 0; j < count; ++j)
		{
			orgxyz = orgxy;
			for (int k = 0; k < count; ++k)
			{
				osg::Box *box = new osg::Box(orgxyz, len);
				osg::ref_ptr<osg::Geode> geode(new osg::Geode);
				geode->addDrawable(new osg::ShapeDrawable(box));
				boxGroup->addChild(geode);
				orgxyz += zvec;
			}
			orgxy += yvec;
		}
		orgx += xvec;
	}

	mOSG->getRoot()->addChild(boxGroup);
}


void CMFC_OSG_MDIView::OnTestGeometryBoxTest()
{
	osg::ref_ptr<osg::Group> boxGroup(new osg::Group);
	const int count = g_testBoxCount;
	double len = 50.0;
	osg::Vec3 orgx, orgxy, orgxyz;
	osg::Vec3 xvec(60, 0, 0), yvec(0, 60, 0), zvec(0, 0, 60);
	for (int i = 0; i < count; ++i)
	{
		orgxy = orgx;
		for (int j = 0; j < count; ++j)
		{
			orgxyz = orgxy;
			for (int k = 0; k < count; ++k)
			{
				Geometry::Box *box = new Geometry::Box;
				box->setOrg(orgxyz);
				box->setXLen(osg::X_AXIS * len);
				box->setYLen(osg::Y_AXIS * len);
				box->setZLen(osg::Z_AXIS * len);
				box->setColor(osg::Vec4(1, 1, 1, 0));
				box->draw();
				osg::ref_ptr<osg::Geode> geode(new osg::Geode);
				geode->addDrawable(box);
				boxGroup->addChild(geode);
				orgxyz += zvec;
			}
			orgxy += yvec;
		}
		orgx += xvec;
	}

	mOSG->getRoot()->addChild(boxGroup);
}

void CMFC_OSG_MDIView::RestPos()
{
	//mOSG->getViewer()->setCameraManipulator(new ViewCenterManipulator);
	//osg::ref_ptr<osgGA::CameraManipulator> manipulator = (osgGA::CameraManipulator*)mOSG->getViewer()->getCameraManipulator();

	////manipulator->home(ea, aa);
	//osg::Vec3d homeEye, homeCenter, homeUp;
	//manipulator->getHomePosition(homeEye, homeCenter, homeUp);
	//osg::Vec3d dir = osg::Vec3d(1, -1, 1);
	//dir.normalize();
	//osg::Matrix vm = osg::Matrix::lookAt(homeCenter + dir*(homeCenter.y() - homeEye.y()), homeCenter, manipulator->getUpVector(osg::Matrix::identity()));
	//manipulator->setByInverseMatrix(vm);
}

void CMFC_OSG_MDIView::OnViewSouthWest()
{
	mOSG->getViewer()->getEventQueue()->userEvent(new ResetPosOperation);
}


void CMFC_OSG_MDIView::OnTestGeometryBoxTest2()
{
	osg::ref_ptr<Geometry::DynamicLOD> boxGroup(new Geometry::DynamicLOD);
	const int count = g_testBoxCount;
	double len = 50.0;
	osg::Vec3 orgx, orgxy, orgxyz;
	osg::Vec3 xvec(60, 0, 0), yvec(0, 60, 0), zvec(0, 0, 60);
	for (int i = 0; i < count; ++i)
	{
		orgxy = orgx;
		for (int j = 0; j < count; ++j)
		{
			orgxyz = orgxy;
			for (int k = 0; k < count; ++k)
			{
				Geometry::Box *box = new Geometry::Box;
				box->setOrg(orgxyz);
				box->setXLen(osg::X_AXIS * len);
				box->setYLen(osg::Y_AXIS * len);
				box->setZLen(osg::Z_AXIS * len);
				box->setColor(osg::Vec4(1, 1, 1, 0));
				box->draw();
				osg::ref_ptr<osg::Geode> geode(new osg::Geode);
				geode->addDrawable(box);
				boxGroup->addChild(geode);
				orgxyz += zvec;
			}
			orgxy += yvec;
		}
		orgx += xvec;
	}

	boxGroup->setUpdateCallback(new Geometry::DynamicLODUpdateCallback);
	mOSG->getRoot()->addChild(boxGroup);
}


void CMFC_OSG_MDIView::OnTestOsgBoxTransformTest()
{
	osg::ref_ptr<osg::Group> boxGroup(new osg::Group);
	const int count = g_testBoxCount;
	double len = 50.0;

	osg::Box *box = new osg::Box(osg::Vec3(0, 0, 0), len);
	osg::ref_ptr<osg::Geode> geode(new osg::Geode);
	geode->addDrawable(new osg::ShapeDrawable(box));

	osg::Vec3 orgx, orgxy, orgxyz;
	osg::Vec3 xvec(60, 0, 0), yvec(0, 60, 0), zvec(0, 0, 60);
	for (int i = 0; i < count; ++i)
	{
		orgxy = orgx;
		for (int j = 0; j < count; ++j)
		{
			orgxyz = orgxy;
			for (int k = 0; k < count; ++k)
			{
				osg::ref_ptr<osg::PositionAttitudeTransform> pat(new osg::PositionAttitudeTransform);
				pat->setPosition(orgxyz);
				pat->addChild(geode);
				boxGroup->addChild(pat);
				orgxyz += zvec;
			}
			orgxy += yvec;
		}
		orgx += xvec;
	}

	mOSG->getRoot()->addChild(boxGroup);
}
