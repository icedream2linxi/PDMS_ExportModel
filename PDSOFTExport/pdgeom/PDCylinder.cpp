// PDCylinder.cpp: implementation of the PDCylinder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDCylinder.h"
#include "dbproxy.h"
#include "geassign.h"
#include "acgi.h"
#include <GEINTARR.H>
#include <dbsol3d.h>
#include <dbapserv.h>
//#include <gedwgio.h>
//#include <gedxfio.h>
#include <gecylndr.h>
#include <math.h>
#include <dbacis.h>
//#include <gs.h>
//#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define PI 3.1415926535897932384626433832795L
#define  SHORT_LENGTH_CYL 0.001
#define VERSION_PDCYLINDER 1

// tries to get the gsView from autocad. It's fine to do this in non AutoCAD
// applications because if AutoCAD doesn't exist, we simply don't do anything
//AcGsView *GetGsView(AcGiViewportDraw *mode)
//{
//    // see if we are running inside of autocad
//    HMODULE hMod = ::GetModuleHandle(_T("acad.exe"));
//    // if we are
//    if(hMod)
//    {
//        // prepare a function pointer to get the AcGsView.
//        typedef AcGsView* (*exp_acgsGetGsView)(int, bool);
//        // try to get the function pointer to acgsGetGsView
//        exp_acgsGetGsView funcPtr = (exp_acgsGetGsView)::GetProcAddress(hMod, "?acgsGetGsView@@YAPAVAcGsView@@H_N@Z");
//        // If this was successful, we can assume that we can use it.
//        if(funcPtr)
//            // all ok, return the AcGsView
//            return funcPtr(mode->viewport().acadWindowId(), false);
//    }
//
//    return NULL;
//}

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDCylinder, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
                        PDCYLINDER, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDCylinder, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDCYLINDER, Gle);
#endif

void PDCylinder::setDefault()
{
  PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
  m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
  m_dDiameter = 1;
  m_ptEnd =AcGePoint3d (0, 0, 0) ;
  m_ptStart =AcGePoint3d (0, 0, 1) ;
#else
  m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
  m_dDiameter = 100;
  m_ptEnd =AcGePoint3d (0, 0, 0) ;
  m_ptStart =AcGePoint3d (1000, 0, 0) ;
#endif
}
bool PDCylinder::isValidData(double &f)
{
  if(!PDPrimary3D::isValidData(f))
    return false;
  if(!isValidFloat(f=m_dDiameter))
    return false;
  if(!isValidFloat(f=m_ptEnd.x) || !isValidFloat(f=m_ptEnd.y) || !isValidFloat(f=m_ptEnd.z))
    return false;
  if(!isValidFloat(f=m_ptStart.x) || !isValidFloat(f=m_ptStart.y) || !isValidFloat(f=m_ptStart.z))
    return false;
  return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDCylinder::audit(AcDbAuditInfo* pAuditInfo)
{
  return auditPDPrimary3D(this,pAuditInfo,_T("PDCylinder"));
}

PDCylinder::PDCylinder(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dDiameter = 1;
	m_ptEnd =AcGePoint3d (0, 0, 0) ;
	m_ptStart =AcGePoint3d (0, 0, 1) ;
#ifdef _USEAMODELER_
    createBody();
#endif
}

PDCylinder::PDCylinder(const AcGePoint3d &ptStart, const AcGePoint3d &ptEnd, 
                       double Diameter, int Precision/* = PRIMARY3D_DEFAULT_PRECISION*/, 
                       bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_ptStart = ptStart;
	m_ptEnd = ptEnd;
	if(m_ptStart.isEqualTo(m_ptEnd))
		m_ptEnd=m_ptStart+AcGeVector3d(0,0,1);

	m_dDiameter =fabs(Diameter) ;
	if(m_dDiameter==0)
		m_dDiameter=1;
    if(Precision > 2)
	    m_dDividPrecision = Precision;
    else
        m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

    //calPts();
#ifdef _USEAMODELER_
    createBody();
#endif
}//added by  linlin 20050929

PDCylinder::~PDCylinder()
{

}

Acad::ErrorStatus PDCylinder::dwgOutFields(AcDbDwgFiler* pFiler) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	// Call dwgOutFields from PDPrimary3D
	if ((es = PDPrimary3D::dwgOutFields(pFiler)) != Acad::eOk) {
		return es;
	}

	// Write version number.
	pFiler->writeItem((Adesk::UInt16) VERSION_PDCYLINDER);

	// Write the data members.
	// TODO: here you can file datamembers not
	//       created by the ObjectARX Add-In.
	pFiler->writeItem(m_ptStart);
	pFiler->writeItem(m_ptEnd);
	pFiler->writeItem(m_dDiameter);

    //AcGeDwgIO::outFields(pFiler, m_stdPts);
    //AcGeDwgIO::outFields(pFiler, m_verticesPts);

	return pFiler->filerStatus();
}


Acad::ErrorStatus PDCylinder::dwgInFields(AcDbDwgFiler* pFiler)
{
	assertWriteEnabled();
	Acad::ErrorStatus es;

	// Call dwgInFields from PDPrimary3D
	if ((es = PDPrimary3D::dwgInFields(pFiler)) != Acad::eOk) {
		return es;
	}

	// Read version number.
	Adesk::UInt16 version;
	pFiler->readItem(&version);
	if (version > VERSION_PDCYLINDER)
		return Acad::eMakeMeProxy;

	// Read the data members.
	switch (version)
	{
	case (1):
		// TODO: here you can file datamembers not
		//       created by the ObjectARX Add-In.
		pFiler->readItem(&m_ptStart);
		pFiler->readItem(&m_ptEnd);
		pFiler->readItem(&m_dDiameter);
		
		break;
	}

    //AcGeDwgIO::inFields(pFiler, m_stdPts);
    //AcGeDwgIO::inFields(pFiler, m_verticesPts);

#ifdef _USEAMODELER_
    if(m_3dGeom.isNull())
        createBody();
#endif

	return pFiler->filerStatus();
}

Acad::ErrorStatus PDCylinder::dxfInFields(AcDbDxfFiler* pFiler)
{
	assertWriteEnabled();
	struct resbuf rb;

	if ((PDPrimary3D::dxfInFields(pFiler) != Acad::eOk) ||
	    !pFiler->atSubclassData(_T("PDCylinder")))
		return pFiler->filerStatus();

	// Read version number.
	Adesk::UInt16 version;
	pFiler->readItem(&rb);
	if (rb.restype != AcDb::kDxfInt16) {
		pFiler->pushBackItem();
		pFiler->setError(Acad::eInvalidDxfCode,
            _T("nError: expected object version group code %d"),
		                 AcDb::kDxfInt16);
		return pFiler->filerStatus();
	} else {
		version = rb.resval.rint;
		if (version > VERSION_PDCYLINDER)
			return Acad::eMakeMeProxy;
	}

	// TODO: here you can file datamembers not
	//       created by the ObjectARX Add-In.

	// Read the data members.
	switch (version)
	{
	case (1):
		pFiler->readItem(&rb);
		if (rb.restype != AcDb::kDxfXCoord) {
			pFiler->pushBackItem();
			pFiler->setError(Acad::eInvalidDxfCode,
                _T("nError: expected object version group code %d"),
							 AcDb::kDxfXCoord);
			return pFiler->filerStatus();
		} else {
			m_ptStart.set(rb.resval.rpoint[X], rb.resval.rpoint[Y], rb.resval.rpoint[Z]);
		}

		pFiler->readItem(&rb);
		if (rb.restype != AcDb::kDxfXCoord + 1) {
			pFiler->pushBackItem();
			pFiler->setError(Acad::eInvalidDxfCode,
                _T("nError: expected object version group code %d"),
							 AcDb::kDxfXCoord + 1);
			return pFiler->filerStatus();
		} else {
			m_ptEnd.set(rb.resval.rpoint[X], rb.resval.rpoint[Y], rb.resval.rpoint[Z]);
		}

		pFiler->readItem(&rb);
		if (rb.restype != AcDb::kDxfXReal) {
			pFiler->pushBackItem();
			pFiler->setError(Acad::eInvalidDxfCode,
                _T("nError: expected object version group code %d"),
							 AcDb::kDxfXReal);
			return pFiler->filerStatus();
		} else {
			m_dDiameter = rb.resval.rreal;
		}

		break;
	}

    //AcGeDxfIO::inFields(pFiler, m_stdPts);
    //AcGeDxfIO::inFields(pFiler, m_verticesPts);

#ifdef _USEAMODELER_
    if(m_3dGeom.isNull())
        createBody();
#endif

	return pFiler->filerStatus();
}

Acad::ErrorStatus PDCylinder::dxfOutFields(AcDbDxfFiler* pFiler) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(pFiler)) != Acad::eOk)
		return es;

	// Write subclass marker.
	pFiler->writeItem(AcDb::kDxfSubclass, _T("PDCylinder"));

	// Write version number.
	pFiler->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDCYLINDER);


	// TODO: here you can file datamembers not
	//       created by the ObjectARX Add-In.

	pFiler->writeItem(AcDb::kDxfXCoord, m_ptStart);
	pFiler->writeItem(AcDb::kDxfXCoord + 1, m_ptEnd);
	pFiler->writeItem(AcDb::kDxfXReal, m_dDiameter);

    //AcGeDxfIO::outFields(pFiler, m_stdPts);
    //AcGeDxfIO::outFields(pFiler, m_verticesPts);

	return pFiler->filerStatus();
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDCylinder::subTransformBy(const AcGeMatrix3d &xform)
#else
Acad::ErrorStatus PDCylinder::transformBy(const AcGeMatrix3d &xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptStart.transformBy(xform);
	m_ptEnd.transformBy(xform);
	m_dDiameter *= xform.scale();

    //for(int i = 0; i < m_stdPts.length(); ++i)
    //    m_stdPts[i].transformBy(xform);
    //for(i = 0; i < m_verticesPts.length(); ++i)
    //    m_verticesPts[i].transformBy(xform);

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

Acad::ErrorStatus PDCylinder::setParameters(const AcGePoint3d &ptStart, 
                                            const AcGePoint3d &ptEnd, 
                                            double Diameter, 
                                            int Precision/* = PRIMARY3D_DEFAULT_PRECISION*/)
{
    assertWriteEnabled();
	m_ptStart = ptStart;
	m_ptEnd = ptEnd;
	if(m_ptStart.isEqualTo(m_ptEnd))
		m_ptEnd=m_ptStart+AcGeVector3d(0,0,1);

	m_dDiameter =fabs(Diameter) ;
	if(m_dDiameter==0)
		m_dDiameter=1;

    if(Precision > 2)
	    m_dDividPrecision = Precision;
    else
        m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

    //calPts();
#ifdef _USEAMODELER_
    createBody();
#endif
    return Acad::eOk;
}//added by linlin 20050929

const AcGePoint3d & PDCylinder::getPtStart() const
{
	assertReadEnabled();
	return m_ptStart;
}

const AcGePoint3d & PDCylinder::getPtEnd() const
{
	assertReadEnabled();
	return m_ptEnd;
}

double PDCylinder::getDiameter() const
{
	assertReadEnabled();
	return m_dDiameter;
}

//void PDCylinder::viewportDraw(AcGiViewportDraw* mode)
//{
//	//acutPrintf("\n this is viewportDraw\n");
//	//int PreStep = 1;
//	//int Precision = m_dDividPrecision;
//	//if(getCalMaxPrecision())    // 优化
//	//{
//	//	Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptStart)
//	//		* 2.0 / m_dDiameter));
//	//	if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
//	//	{
//	//		AcGePoint3d pt[2];
//	//		pt[0] = m_ptStart;
//	//		pt[1] = m_ptEnd;
//	//		/*if*/(mode->geometry().polyline(2, pt));
//	//		/*return Adesk::kTrue;
//	//		double radius = m_dDiameter / 2.0;
//	//		AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
//	//		if(mode -> geometry().circle( m_ptStart, radius, vect))
//	//		return Adesk::kTrue;
//	//		mode -> geometry().circle( m_ptEnd, radius, vect);*/	
//	//		return ;
//	//	}
//	//}  
//
//    // Try to obtain an AcGsView for the current viewport...this will fail if it is a 2D wireframe (normal) view or if not running in AutoCAD
//    // (will set pView to NULL.)
//    AcGsView *pView = GetGsView(mode); // This will solve problem 2
//
//    // You must invalidate the 3D graphics cache if your graphics change between 2D and 3D modes. 
//    // Perhaps this can be done within an Editor Reactor which detects when the SHADEMODE
//    // command ends, and place the call to invalidateCachedViewportGeometry() there, but of course
//    // that would require ObjectARX. Instead you can place the following code in the viewportDraw()
//    // function, note that it will cause the function to be called twice anytime an update is needed
//    // (This may be a small price to pay however). 
//
//    if(pView)
//    {
//        pView->invalidateCachedViewportGeometry(); // This will solve problem 4
//
//        // The following code also works for this purpose:
//        //AcGsModel *gsModel=acgsGetGsManager()->getDBModel();
//        //gsModel->invalidate(AcGsModel::kInvalidateViewportCache);
//    }
//
//}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDCylinder::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDCylinder::worldDraw(AcGiWorldDraw* mode)
#endif
{
    assertReadEnabled();
//	double dDist =m_ptStart.distanceTo(m_ptEnd);
//	if(dDist <= SHORT_LENGTH_CYL)
//		return Adesk::kTrue;
    //clock_t start, finish;
    //start = clock();	
	//if(acdbHostApplicationServices()->workingDatabase()->useri1() == 1)
	//{
	//	return Adesk::kTrue;
	//}

#ifdef _USEAMODELER_
#ifdef _OBJECTARX2010_
	PDPrimary3D::subWorldDraw(mode);
#else
	PDPrimary3D::worldDraw(mode);
#endif
#endif
    if (mode->regenAbort()) 
	{
        return Adesk::kTrue;
    }

#ifndef _USEAMODELER_
    int PreStep = 1;
    int Precision = m_dDividPrecision;
	AcGePoint3dArray pArray;
	AcGeVector3dArray vertexNors;
	int Precision__1 = Precision + 1;
	int i;

	double radius = m_dDiameter / 2.0;
	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 
		|| nRating == PRIMARY3D_SIMPLIZE_RATE2) 
    {  
		// 优化
		//简化为四等分
        Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptStart)
                    * 2.0 / m_dDiameter));
        if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{	
			if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
				if(mode -> geometry().circle( m_ptStart, radius, vect))
					return Adesk::kTrue;
				if(mode -> geometry().circle( m_ptEnd, radius, vect))
					return Adesk::kTrue;
				Precision = PRIMARY3D_WIREFRAME_LINENUM4;
				Precision__1 = Precision + 1;
				getVertices(Precision, pArray, vertexNors);
				AcGePoint3d pt[2];
				for(i = 0; i < Precision; i += PreStep)
				{
					pt[0] = pArray[i];
					pt[1] = pArray[i + Precision__1];
					if(mode->geometry().polyline(2, pt))
						return Adesk::kTrue;
				}
			}
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{
				//二级简化
				AcGePoint3d pt[2];
				pt[0] = m_ptStart;
				pt[1] = m_ptEnd;
				mode->geometry().polyline(2, pt);
			}

            //finish = clock();
            //ads_printf("\ntime: %ld.", finish - start);

			return Adesk::kTrue;
        }
    }
#ifndef _ALWAYSSHOWWIRE_
	switch(mode->regenType())
    {
	case kAcGiHideOrShadeCommand:
	case kAcGiRenderCommand:
#endif
		{   
#ifdef _OBJECTARX2010_
			Adesk::GsMarker gsIndex = 0;
#else
			Adesk::Int32 gsIndex = 0;
#endif
			mode->subEntityTraits().setFillType(kAcGiFillAlways);
/*			if (Precision < PRIMARY3D_SIMPLIZE_SCALE4)
			{
				Precision = PRIMARY3D_WIREFRAME_LINENUM4 + 2;
			}
			else */if(Precision < PRIMARY3D_WIREFRAME_LINENUM * 2)
				Precision = PRIMARY3D_WIREFRAME_LINENUM;
			else
			{
				if(Precision > m_dDividPrecision)
					Precision = m_dDividPrecision;
				PreStep = Precision / PRIMARY3D_WIREFRAME_LINENUM;
				Precision = PreStep * PRIMARY3D_WIREFRAME_LINENUM;
			}
			Precision__1 = Precision + 1;
			getVertices(Precision, pArray, vertexNors);

        // 上下顶面采用三角剖分方式。
        {
            //         *2
            //       /  \
            //      /    \
            //    3*------*1
            //    /     /  \
            //   /   /      \
            //  / /          \
            //4*--------------*0(8)
            //  \          / /
            //   \      /   /
            //    \  /     /
            //    5*------*7
            //      \    /
            //       \  /
            //       6*
            AcGeVector3d vect = (m_ptStart - m_ptEnd).normal();
            int Precision_2___4 = (Precision - 2) * 4;
            int Precision____2 = Precision / 2;
            Adesk::Int32 *faceList = new Adesk::Int32[Precision_2___4];
            int forLen = Precision / 4 - 1;
            for(i = 0; i < forLen; ++i)
            {
                faceList[i * 16] = 3;
                faceList[i * 16 + 1] = i;
                faceList[i * 16 + 2] = Precision____2 - i;
                faceList[i * 16 + 3] = i + 1;
                faceList[i * 16 + 4] = 3;
                faceList[i * 16 + 5] = i + 1;
                faceList[i * 16 + 6] = Precision____2 - i;
                faceList[i * 16 + 7] = Precision____2 - i - 1;
                faceList[i * 16 + 8] = 3;
                faceList[i * 16 + 9] = Precision - i;
                faceList[i * 16 + 10] = Precision____2 + i + 1;
                faceList[i * 16 + 11] = Precision____2 + i;
                faceList[i * 16 + 12] = 3;
                faceList[i * 16 + 13] = Precision - i;
                faceList[i * 16 + 14] = Precision - i - 1;
                faceList[i * 16 + 15] = Precision____2 + i + 1;
            }
            faceList[Precision_2___4 - 8] = 3;
            faceList[Precision_2___4 - 7] = Precision / 4;
            faceList[Precision_2___4 - 6] = Precision / 4 - 1;
            faceList[Precision_2___4 - 5] = Precision / 4 + 1;
            faceList[Precision_2___4 - 4] = 3;
            faceList[Precision_2___4 - 3] = Precision / 4 * 3;
            faceList[Precision_2___4 - 2] = Precision / 4 * 3 - 1;
            faceList[Precision_2___4 - 1] = Precision / 4 * 3 + 1;
            AcGiEdgeData edgeData;
            int edgeVisibilitysLen = (Precision - 2) * 3;
            Adesk::UInt8 *edgeVisibilitys = new Adesk::UInt8[edgeVisibilitysLen];
            for(i = 0; i < edgeVisibilitysLen; ++i)
            {
                edgeVisibilitys[i] = kAcGiInvisible;
            }
            edgeData.setVisibility(edgeVisibilitys);
            AcGiVertexData vertexData;
            AcGeVector3d *nors = new AcGeVector3d[Precision__1];
            for(i = 0; i < Precision__1; ++i)
            {
                nors[i] = vect;
            }
            vertexData.setNormals(nors);
            vertexData.setOrientationFlag(kAcGiCounterClockwise);
            // 底面
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
            if(mode->geometry().shell(Precision__1, pArray.asArrayPtr(), Precision_2___4, faceList, &edgeData, NULL, &vertexData))
            {
                delete [] faceList;
                delete [] edgeVisibilitys;
                delete [] nors;
                return Adesk::kTrue;
            }

            forLen = Precision / 4 - 1;
            for(i = 0; i < forLen; ++i)
            {
                faceList[i * 16] = 3;
                faceList[i * 16 + 1] = i;
                faceList[i * 16 + 2] = i + 1;
                faceList[i * 16 + 3] = Precision____2 - i;
                faceList[i * 16 + 4] = 3;
                faceList[i * 16 + 5] = i + 1;
                faceList[i * 16 + 6] = Precision____2 - i - 1;
                faceList[i * 16 + 7] = Precision____2 - i;
                faceList[i * 16 + 8] = 3;
                faceList[i * 16 + 9] = Precision - i;
                faceList[i * 16 + 10] = Precision____2 + i;
                faceList[i * 16 + 11] = Precision____2 + i + 1;
                faceList[i * 16 + 12] = 3;
                faceList[i * 16 + 13] = Precision - i;
                faceList[i * 16 + 14] = Precision____2 + i + 1;
                faceList[i * 16 + 15] = Precision - i - 1;
            }
            faceList[Precision_2___4 - 8] = 3;
            faceList[Precision_2___4 - 7] = Precision / 4;
            faceList[Precision_2___4 - 6] = Precision / 4 + 1;
            faceList[Precision_2___4 - 5] = Precision / 4 - 1;
            faceList[Precision_2___4 - 4] = 3;
            faceList[Precision_2___4 - 3] = Precision / 4 * 3;
            faceList[Precision_2___4 - 2] = Precision / 4 * 3 + 1;
            faceList[Precision_2___4 - 1] = Precision / 4 * 3 - 1;
            vect = -vect;
            for(i = 0; i < Precision__1; ++i)
            {
                nors[i] = vect;
            }
            vertexData.setNormals(nors);
            vertexData.setOrientationFlag(kAcGiCounterClockwise);
            // 顶面
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
            if(mode->geometry().shell(Precision__1, &(pArray.asArrayPtr()[Precision__1]), Precision_2___4, faceList, &edgeData, NULL, &vertexData))
            {
                delete [] faceList;
                delete [] edgeVisibilitys;
                delete [] nors;
                return Adesk::kTrue;
            }

            delete [] faceList;
            delete [] edgeVisibilitys;
            delete [] nors;
/*
            AcGeVector3d vect = (m_ptStart - m_ptEnd).normal();
            AcGePoint3d *pts = new AcGePoint3d[Precision + 2];
            for(i = 0; i < Precision__1; ++i)
            {
                pts[i] = pArray[i];
            }
            pts[Precision__1] = m_ptStart;
            Adesk::Int32 *faceList = new Adesk::Int32[Precision * 4];
            for(i = 0; i < Precision; ++i)
            {
                faceList[i * 4] = 3;
                faceList[i * 4 + 1] = i;
                faceList[i * 4 + 2] = Precision__1;
                faceList[i * 4 + 3] = i + 1;
            }
            AcGiVertexData vertexData;
            AcGeVector3d *nors = new AcGeVector3d[Precision + 2];
            for(i = 0; i < Precision + 2; ++i)
            {
                nors[i] = vect;
            }
            vertexData.setNormals(nors);
            vertexData.setOrientationFlag(kAcGiCounterClockwise);
            if(mode->geometry().shell(Precision + 2, pts, Precision * 4, faceList, NULL, NULL, &vertexData))
            {
                delete [] pts;
                delete [] faceList;
                delete [] nors;
                return Adesk::kTrue;
            }

            for(i = 0; i < Precision__1; ++i)
            {
                pts[i] = pArray[i + Precision__1];
            }
            pts[Precision__1] = m_ptEnd;
            for(i = 0; i < Precision; ++i)
            {
                faceList[i * 4] = 3;
                faceList[i * 4 + 1] = i;
                faceList[i * 4 + 2] = i + 1;
                faceList[i * 4 + 3] = Precision__1;
            }
            vect = -vect;
            for(i = 0; i < Precision + 2; ++i)
            {
                nors[i] = vect;
            }
            vertexData.setNormals(nors);
            if(mode->geometry().shell(Precision + 2, pts, Precision * 4, faceList, NULL, NULL, &vertexData))
            {
                delete [] pts;
                delete [] faceList;
                delete [] nors;
                return Adesk::kTrue;
            }

            delete [] pts;
            delete [] faceList;
            delete [] nors;*/
        }

        // 上下顶面采用shell绘制。
/*        {
            Adesk::Int32 *faceList = new Adesk::Int32[Precision__1];
            faceList[0] = Precision;
            for(i = 0; i < Precision; ++i)
                faceList[i + 1] = i;
            AcGeVector3d vect = (m_ptStart - m_ptEnd).normal();
            AcGeVector3d *nors = new AcGeVector3d[Precision];
            for(i = 0; i < Precision; ++i)
                nors[i] = vect;
            AcGiVertexData vertexData;
            vertexData.setNormals(nors);
            vertexData.setOrientationFlag(kAcGiCounterClockwise);
            if(mode->geometry().shell(Precision, pArray.asArrayPtr(), Precision__1, faceList, NULL, NULL, &vertexData))
            {
                delete [] faceList;
                delete [] nors;
                return Adesk::kTrue;
            }
            vect = -vect;
            for(i = 0; i < Precision; ++i)
                nors[i] = vect;
            vertexData.setNormals(nors);
            vertexData.setOrientationFlag(kAcGiClockwise);
            if(mode->geometry().shell(Precision, &(pArray.asArrayPtr()[Precision__1]), Precision__1, faceList, NULL, NULL, &vertexData))
            {
                delete [] faceList;
                delete [] nors;
                return Adesk::kTrue;
            }
            delete [] faceList;
            delete [] nors;
        }
*/
        // 上下顶面采用polygon绘制。
        //if(mode->geometry().polygon(Precision, pArray.asArrayPtr()))
            //return Adesk::kTrue;
        //if(mode->geometry().polygon(Precision, &(pArray.asArrayPtr()[Precision__1])))
            //return Adesk::kTrue;

        // 柱面采用三角剖分方式。
//         {
            //9   10   11   12   13   14   15   16   17
            //*----*----*----*----*----*----*----*----*
            //|\   |\   |\   |\   |\   |\   |\   |\   |
            //| \  | \  | \  | \  | \  | \  | \  | \  |
            //|  \ |  \ |  \ |  \ |  \ |  \ |  \ |  \ |
            //|   \|   \|   \|   \|   \|   \|   \|   \|
            //*----*----*----*----*----*----*----*----*
            //0    1    2    3    4    5    6    7    8
            Adesk::Int32 *facelist = new Adesk::Int32[Precision * 2 * 4];
            for(i = 0; i < Precision; ++i)
            {
                facelist[i * 8] = 3;
                facelist[i * 8 + 1] = i;
                facelist[i * 8 + 2] = i + 1;
                facelist[i * 8 + 3] = i + Precision + 1;
                facelist[i * 8 + 4] = 3;
                facelist[i * 8 + 5] = i + Precision + 2;
                facelist[i * 8 + 6] = i + Precision + 1;
                facelist[i * 8 + 7] = i + 1;
            }
            AcGiEdgeData edgeData;
            Adesk::UInt8 *edgeVisibilitys = new Adesk::UInt8[Precision * 2 * 3];
            int preStepStep = 0;
            for(i = 0; i < Precision; ++i)
            {
                edgeVisibilitys[i * 6] = kAcGiVisible;
                edgeVisibilitys[i * 6 + 1] = kAcGiInvisible;
                ++preStepStep;
                if(preStepStep == 1)
                    edgeVisibilitys[i * 6 + 2] = kAcGiVisible;
                else
                    edgeVisibilitys[i * 6 + 2] = kAcGiInvisible;
                if(preStepStep == PreStep)
                {
                    preStepStep = 0;
                }
                edgeVisibilitys[i * 6 + 3] = kAcGiVisible;
                edgeVisibilitys[i * 6 + 4] = kAcGiInvisible;
                edgeVisibilitys[i * 6 + 5] = kAcGiInvisible;
            }
            edgeData.setVisibility(edgeVisibilitys);
            AcGiVertexData vertexData;
            vertexData.setNormals(vertexNors.asArrayPtr());
            vertexData.setOrientationFlag(kAcGiCounterClockwise);
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
           mode->geometry().shell(pArray.length(), pArray.asArrayPtr(), Precision * 2 * 4, facelist, &edgeData, NULL, &vertexData);
            delete [] facelist;
            delete [] edgeVisibilitys;
//         }

/*        AcGePoint3d *pts = new AcGePoint3d[Precision * 2];
        AcGeVector3d *nors = new AcGeVector3d[Precision * 2];
        for(i = 0; i < Precision; ++i)
        {
            pts[i] = pArray[i];
            pts[i + Precision] = pArray[i + Precision__1];
            nors[i] = vertexNors[i];
            nors[i + Precision] = vertexNors[i + Precision__1];
        }
        Adesk::Int32 *facelist = new Adesk::Int32[Precision * 2 * 4];
        for(i = 0; i < Precision - 1; ++i)
        {
            facelist[i * 8] = 3;
            facelist[i * 8 + 1] = i;
            facelist[i * 8 + 2] = i + 1;
            facelist[i * 8 + 3] = i + Precision;
            facelist[i * 8 + 4] = 3;
            facelist[i * 8 + 5] = i + Precision;
            facelist[i * 8 + 6] = i + 1;
            facelist[i * 8 + 7] = i + Precision + 1;
        }
        facelist[(Precision - 1) * 8] = 3;
        facelist[(Precision - 1) * 8 + 1] = Precision - 1;
        facelist[(Precision - 1) * 8 + 2] = 0;
        facelist[(Precision - 1) * 8 + 3] = Precision - 1 + Precision;
        facelist[(Precision - 1) * 8 + 4] = 3;
        facelist[(Precision - 1) * 8 + 5] = Precision - 1 + Precision;
        facelist[(Precision - 1) * 8 + 6] = 0;
        facelist[(Precision - 1) * 8 + 7] = Precision;
        AcGiVertexData vertexData;
        vertexData.setNormals(nors);
        vertexData.setOrientationFlag(kAcGiCounterClockwise);
        mode->geometry().shell(Precision * 2, pts, Precision * 2 * 4, facelist, NULL, NULL, &vertexData);
        delete [] pts;
        delete [] nors;
        delete [] facelist;
*/
/*        AcGiEdgeData edgeData;
        int edgeVisibilityLen = Precision__1 * 3 - 2;
        Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeVisibilityLen];
        int Precision___2 = Precision * 2;
		//for(i = 0; i < edgeVisibilityLen; ++i)
		//	edgeVisibility[i] = kAcGiVisible;
        for(i = 0; i < Precision___2; ++i)
            edgeVisibility[i] = kAcGiVisible;
	    for(i = Precision___2; i < edgeVisibilityLen; i += PreStep)
        {
            edgeVisibility[i] = kAcGiVisible;
            if(PreStep > 1 && i != edgeVisibilityLen - 1)
            {
                for(int j = 1; j < PreStep; ++j)
                    edgeVisibility[i + j] = kAcGiInvisible;
            }
        }
        edgeData.setVisibility(edgeVisibility);
    
	    //AcGiFaceData faceData;
	    //Adesk::UInt8* faceVisibility = new Adesk::UInt8[Precision];
	    //for(i = 0; i < Precision; i++)
		   // faceVisibility[i] = kAcGiVisible;//kAcGiSilhouette;
	    //faceData.setVisibility(faceVisibility);
     //   faceData.setNormals(faceNors.asArrayPtr());

        AcGiVertexData vertexData;
        vertexData.setNormals(vertexNors.asArrayPtr());
        vertexData.setOrientationFlag(kAcGiCounterClockwise);
*/   
        //mode->subEntityTraits().setFillType(kAcGiFillAlways);
        //mode->geometry().mesh(2, Precision__1, pArray.asArrayPtr(), &edgeData, NULL, &vertexData);

        //delete [] faceList;
    	//delete [] edgeVisibility;
        //delete [] faceVisibility;
	}
#ifndef _ALWAYSSHOWWIRE_
    break;
	case kAcGiStandardDisplay:
	   case kAcGiSaveWorldDrawForR12:
	   case kAcGiSaveWorldDrawForProxy:
#endif
		{
#ifdef _OBJECTARX2010_
			Adesk::GsMarker gsIndex = 0;
#else
			Adesk::Int32 gsIndex = 0;
#endif
			AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptStart, radius, vect))
				return Adesk::kTrue;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptEnd, radius, vect))
				return Adesk::kTrue;
			Precision = PRIMARY3D_WIREFRAME_LINENUM;
			Precision__1 = Precision + 1;
			getVertices(Precision, pArray, vertexNors);

			AcGePoint3d pt[2];
			++gsIndex;
			for(i = 0; i < Precision; i += PreStep)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + Precision__1];
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
			}
			//modified by szw 2009.11.18 : end
		}
#ifndef _ALWAYSSHOWWIRE_
       break;
  default:
    break;
	}
#endif
#endif

    //finish = clock();
    //ads_printf("\ntime: %ld.", finish - start);

	return Adesk::kTrue;
}

//Adesk::Boolean PDCylinder::worldDraw(AcGiWorldDraw* mode)
//{
//	assertReadEnabled();
//    //clock_t start, finish;
//    //start = clock();
//
//#ifdef _USEAMODELER_
//    PDPrimary3D::worldDraw(mode);
//#endif
//    if (mode->regenAbort()) {
//        return Adesk::kTrue;
//    }
//#ifndef _USEAMODELER_
//
//    int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptStart)
//        * 2.0 / m_dDiameter));
//    if(pre < 3)
//    {
//        if(!getCalMaxPrecision())   // 不进行优化
//            pre = 8;
//        else
//        {
//            AcGePoint3d pt[2];
//            pt[0] = m_ptStart;
//            pt[1] = m_ptEnd;
//            mode->geometry().polyline(2, pt);
//            return Adesk::kTrue;
//        }
//    }
//    else if(pre < 16)
//        pre = 8;
//    else
//        pre = pre / 8 * 8;
//
//    //calPts();
//
//    AcGePoint3dArray pArray;
//    AcGeIntArray stdIdx;
//    int actPrecision;
//    getVertices(getMaxPrecision(mode), pArray, stdIdx, actPrecision);
//    int actPrecision__1 = actPrecision + 1;
//    int stdIdxLen = stdIdx.length();
//    int stdIdxLen_1 = stdIdxLen - 1;
//    int i;
//
//    double radius = m_dDiameter / 2.0;
//  
//	switch(mode->regenType()){
//	case kAcGiStandardDisplay:
//    case kAcGiSaveWorldDrawForR12:
//    case kAcGiSaveWorldDrawForProxy:
//        {
//            int gsIndex = 0;
//            AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
//            mode->subEntityTraits().setSelectionMarker(++gsIndex);
//		    if(mode -> geometry().circle( m_ptStart, radius, vect))
//                return Adesk::kTrue;
//            mode->subEntityTraits().setSelectionMarker(++gsIndex);
//		    if(mode -> geometry().circle( m_ptEnd, radius, vect))
//                return Adesk::kTrue;
//
//            AcGePoint3d pt[2];
//            for(i = 0; i < stdIdxLen_1; i++)
//            {
//                pt[0] = pArray[stdIdx[i]];
//                pt[1] = pArray[stdIdx[i] + actPrecision__1];
//                //pt[0] = m_stdPts[i];
//                //pt[1] = m_stdPts[i + stdPtArrHalfLen];
//                mode->subEntityTraits().setSelectionMarker(++gsIndex);
//                if(mode->geometry().polyline(2, pt))
//                    return Adesk::kTrue;
//            }
//        }
//        break;
//	case kAcGiHideOrShadeCommand:
//	case kAcGiRenderCommand:
//    {
//        mode->subEntityTraits().setFillType(kAcGiFillAlways);
//        if(mode->geometry().polygon(actPrecision, &pArray[0]))
//            return Adesk::kTrue;
//        if(mode->geometry().polygon(actPrecision, &pArray[actPrecision__1]))
//            return Adesk::kTrue;
//        //mode->geometry().polygon( pArrayHalfLen, &m_verticesPts[0]);
//        //mode->geometry().polygon( pArrayHalfLen, &m_verticesPts[pArrayHalfLen]);
//
//        AcGiEdgeData edgeData;
//        int edgeVisibilityLen = actPrecision__1 * 3 - 2;
//        Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeVisibilityLen];
//	    for(i = 0; i < edgeVisibilityLen; ++i)
//            edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
//        int actPrecision___2 = actPrecision * 2;
//        for(i = 0; i < stdIdxLen; ++i)
//            edgeVisibility[stdIdx[i] + actPrecision___2] = kAcGiVisible;
//        edgeData.setVisibility(edgeVisibility);
//    
//    /*
//	    AcGiFaceData faceData;
//	    Adesk::UInt8* faceVisibility = new Adesk::UInt8[maxPrecision];
//	    for(int k=0; k<maxPrecision; k++)
//		    faceVisibility[k]=kAcGiSilhouette;
//	    faceData.setVisibility(faceVisibility);
//    */
////      AcGePoint3d *vertexList = new AcGePoint3d[pArray.length()];
////      for(i = 0; i < pArray.length(); i++)
////        vertexList[i] = pArray[i];
///*      AcGePoint3d *vertexList = new AcGePoint3d[10];
//      vertexList[0].set(0, 0, 10);
//      vertexList[1].set(0, 10, 0);
//      vertexList[2].set(0, 0, -10);
//      vertexList[3].set(0, -10, 0);
//      vertexList[4].set(0, 0, 10);
//      vertexList[5].set(100, 0, 10);
//      vertexList[6].set(100, 10, 0);
//      vertexList[7].set(100, 0, -10);
//      vertexList[8].set(100, -10, 0);
//      vertexList[9].set(100, 0, 10);
//*/
//        if(mode->geometry().mesh(2, actPrecision__1, pArray.asArrayPtr(), &edgeData))//, NULL&faceData)
//            return Adesk::kTrue;
//        //mode->geometry().mesh(2, pArrayHalfLen, m_verticesPts.asArrayPtr(), &edgeData, NULL);//&faceData);
////	    mode->geometry().mesh(2, pArray.length() / 2,vertexList,&edgeData,NULL);//&faceData);
////      mode->geometry().mesh(2, 5,vertexList);//,&edgeData,NULL);//&faceData);
////      delete [] vertexList;
//    	delete [] edgeVisibility;
//	}
//    break;
//  default:
//    break;
//	}
//#endif
//
//    //finish = clock();
//    //ads_printf("\ntime: %ld.", finish - start);
//
//	return Adesk::kTrue;
//}

//void PDCylinder::calPts()
//{
//    if(m_dDividPrecision == m_dDividPrecision_bak && 
//        m_ptStart == m_ptStart_bak && 
//        m_ptEnd == m_ptEnd_bak && 
//        m_dDiameter == m_dDiameter_bak)
//        return ;
//    else
//    {
//        getStandardVertices(m_stdPts);
//        getVertices(m_verticesPts, m_dDividPrecision);
//        m_dDividPrecision_bak = m_dDividPrecision;
//        m_ptStart_bak = m_ptStart;
//        m_ptEnd_bak = m_ptEnd;
//        m_dDiameter_bak = m_dDiameter;
//        return ;
//    }
//}

Acad::ErrorStatus PDCylinder::getVertices(int Precision, AcGePoint3dArray& vertexArray, 
                                          AcGeIntArray &stdIdx, int &actPrecision) const
{
    assertReadEnabled();

    AcGeDoubleArray dividDbl;
    getActDivid(Precision, dividDbl, stdIdx);

    int actPrecision__1 = dividDbl.length();
    actPrecision = actPrecision__1 - 1;
    vertexArray.setLogicalLength(actPrecision__1 * 2);

    AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();

    AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
    if (fabs(vect[X]) <1.0/64 && fabs(vect[Y])<1.0/64) 
        Ax = Wy.crossProduct(vect);
    else
        Ax = Wz.crossProduct(vect);
    Ax.normalize();
    //Ay = vect.crossProduct(Ax);
    //Ay.normalize();

    Ax *= m_dDiameter / 2.0;
    double rotateAngPre = PI * 2.0 / Precision;

    vertexArray[0] = m_ptStart + Ax;
    vertexArray[actPrecision__1] = m_ptEnd + Ax;
    AcGeVector3d VecTemp;
    int i;
    for(i = 1; i < actPrecision; ++i)
    {
        VecTemp = Ax;
        VecTemp.rotateBy(rotateAngPre * dividDbl[i], vect);
        vertexArray[i] = m_ptStart + VecTemp;	
        vertexArray[i + actPrecision__1] = m_ptEnd + VecTemp;
    }
    vertexArray[actPrecision] = vertexArray[0];
    vertexArray[actPrecision + actPrecision__1] = vertexArray[actPrecision__1];

    return Acad::eOk;
}

Acad::ErrorStatus PDCylinder::getVertices(int Precision, 
                                          AcGePoint3dArray &vertexArray, 
                                          AcGeVector3dArray &vertexNors) const
{
    assertReadEnabled();

    int Precision__1 = Precision + 1;
    vertexArray.setLogicalLength(Precision__1 * 2);
    vertexNors.setLogicalLength(Precision__1 * 2);

    AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();

    AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
    if (fabs(vect[X]) <1.0/64 && fabs(vect[Y])<1.0/64) 
        Ax = Wy.crossProduct(vect);
    else
        Ax = Wz.crossProduct(vect);
    Ax.normalize();
    //Ay = vect.crossProduct(Ax);
    //Ay.normalize();

    Ax *= m_dDiameter / 2.0;
    double rotateAngPre = PI * 2.0 / Precision;

    AcGeVector3d VecTemp = Ax;
    AcGeVector3d norTemp = Ax;
    vertexArray[0] = m_ptStart + Ax;
    vertexArray[Precision__1] = m_ptEnd + Ax;
    vertexNors[0] = VecTemp;
    vertexNors[0].normalize();
    vertexNors[Precision__1] = vertexNors[0];
    for(int i = 1; i < Precision; ++i)
    {
        VecTemp.rotateBy(rotateAngPre, vect);
        vertexArray[i] = m_ptStart + VecTemp;
        vertexNors[i] = VecTemp;
        vertexNors[i].normalize();
        vertexArray[i + Precision__1] = m_ptEnd + VecTemp;
        vertexNors[i + Precision__1] = vertexNors[i];
    }
    vertexArray[Precision] = vertexArray[0];
    vertexArray[Precision + Precision__1] = vertexArray[Precision__1];
    vertexNors[Precision] = vertexNors[0];
    vertexNors[Precision + Precision__1] = vertexNors[Precision__1];

    return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDCylinder::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDCylinder::getOsnapPoints(
	AcDb::OsnapMode       osnapMode,
	int                   gsSelectionMark,
	const AcGePoint3d&    pickPoint,
	const AcGePoint3d&    lastPoint,
	const AcGeMatrix3d&   viewXform,
	AcGePoint3dArray&     snapPoints,
	AcDbIntArray&         geomIds) const
#endif
{
    assertReadEnabled();

    if(!hasSnap())
        return Acad::eOk;

	int gsSelectionMark_int = (int)gsSelectionMark;
    if(gsSelectionMark_int == 0)
        return Acad::eOk;

    AcGePoint3dArray pArray;
    AcGeIntArray stdIdx;
    int actPrecision;
    getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
    int actPrecision__1 = actPrecision + 1;
    int stdIdxLen = stdIdx.length();
    int stdIdxLen_1 = stdIdxLen - 1;
    int stdIdxLen____2 = stdIdxLen / 2;

    AcGeVector3d viewDir(viewXform(Z, 0), viewXform(Z, 1),
                viewXform(Z, 2));
    AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();

    int i;
    switch(osnapMode)
    {
    case AcDb::kOsModeEnd:
        snapPoints.append(m_ptStart);
        snapPoints.append(m_ptEnd);
        for(i = 0; i < stdIdxLen_1; ++i)
        {
            snapPoints.append(pArray[stdIdx[i]]);
            snapPoints.append(pArray[stdIdx[i] + actPrecision__1]);
        }
        break;
    case AcDb::kOsModeMid:
        snapPoints.append(m_ptStart + (m_ptEnd - m_ptStart) / 2.0);
        //棱边的中点
        for(i = 0; i < stdIdxLen_1; ++i)
            snapPoints.append(pArray[stdIdx[i]] + 
                              (pArray[stdIdx[i] + actPrecision__1] - pArray[stdIdx[i]]) / 2.0);
        break;
    case AcDb::kOsModeCen:
        if(gsSelectionMark_int == 1)
            snapPoints.append(m_ptStart);
        else if(gsSelectionMark_int == 2)
	        snapPoints.append(m_ptEnd);
        else
	        snapPoints.append(m_ptStart + (m_ptEnd - m_ptStart) / 2.0);
        break;
    case AcDb::kOsModeQuad:
        for(i = 0; i < stdIdxLen____2; i++)
        {
            snapPoints.append(pArray[stdIdx[i * 2]]);
            snapPoints.append(pArray[stdIdx[i * 2] + actPrecision__1]);
        }
        break;
    case AcDb::kOsModeNode:
		break;
    case AcDb::kOsModeIns:
		snapPoints.append(m_ptStart);
		break;
    case AcDb::kOsModePerp:
        {
            AcGeLine3d line;
            AcGeVector3d vec;
            AcGePoint3d pt;
            if(gsSelectionMark_int == 1)
            {
                AcGeCircArc3d cir(m_ptStart, vect, m_dDiameter / 2.0);
                pt = cir.closestPointTo(lastPoint);
                snapPoints.append(pt);
            }
            else if(gsSelectionMark_int == 2)
            {
                AcGeCircArc3d cir(m_ptEnd, vect, m_dDiameter / 2.0);
                pt = cir.closestPointTo(lastPoint);
                snapPoints.append(pt);
            }
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			else if(gsSelectionMark_int == 3)
            {
				for(int i = 0; i < stdIdxLen - 1; ++i)
				{
					vec = pArray[stdIdx[i]] - pArray[stdIdx[i] + actPrecision__1];
					line.set(pArray[stdIdx[i]], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
            }
			//modified by szw 2009.11.18 : end
        }
		break;
    case AcDb::kOsModeTan:
		break;
    case AcDb::kOsModeNear:
        {
            AcGePoint3d pt;
            AcGeCircArc3d cir;
            //下底面
            if(gsSelectionMark_int == 1)
            {
                cir.set(m_ptStart, vect, m_dDiameter / 2.0);
                pt = cir.projClosestPointTo(pickPoint, viewDir);
                snapPoints.append(pt);
            }
            //上底面
            else if(gsSelectionMark_int == 2)
            {
                cir.set(m_ptEnd, vect, m_dDiameter / 2.0);
                pt = cir.projClosestPointTo(pickPoint, viewDir);
                snapPoints.append(pt);
            }
            //棱边
			//重新定义对象最近点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
            else if(gsSelectionMark_int == 3)
            {
				AcGeLineSeg3d lnsg;
				for(int i = 0; i < stdIdxLen - 1; ++i)
				{
					lnsg.set(pArray[stdIdx[i]], pArray[stdIdx[i] + actPrecision__1]);
					pt = lnsg.projClosestPointTo(pickPoint, viewDir);
					snapPoints.append(pt);
				}
            }
			//modified by szw 2009.11.18 : end
        }
        break;
    default:
        break;
    }

  return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDCylinder::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDCylinder::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
    assertReadEnabled();
/*	getgrippoints(gripPoints);*/
	return Acad::eOk; 
}

Acad::ErrorStatus PDCylinder::getgrippoints(AcGePoint3dArray& gripArray) const
{
    assertReadEnabled();

	gripArray.append(m_ptStart);
	gripArray.append(m_ptEnd);

    AcGePoint3dArray pArray;
    AcGeIntArray stdIdx;
    int actPrecision;
    getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
    int actPrecision__1 = actPrecision + 1;
    int stdIdxLen_1 = stdIdx.length() - 1;

    for(int i = 0; i < stdIdxLen_1; ++i)
    {
        gripArray.append(pArray[stdIdx[i]]);
        gripArray.append(pArray[stdIdx[i] + actPrecision__1]);
    }
	return Acad::eOk; 
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDCylinder::subMoveGripPointsAt(
	const AcDbIntArray& indices,
	const AcGeVector3d& offset)
#else
Acad::ErrorStatus PDCylinder::moveGripPointsAt(
	const AcDbIntArray& indices,
	const AcGeVector3d& offset)
#endif
{
	assertWriteEnabled();
/*	if(indices.length()>1){
		m_ptStart.transformBy(offset);
		m_ptEnd.transformBy(offset);
		return Acad::eOk;
	}

	if(indices[0] ==0 )
		m_ptStart.transformBy(offset);

	else if(indices[0] ==1 )
		m_ptEnd.transformBy(offset);

	else{
		AcGePoint3dArray grippoint;
		getgrippoints(grippoint);
		grippoint[indices[0]].transformBy(offset);

		double r;
		if(indices[0] < 6)
			r = grippoint[indices[0]].distanceTo(m_ptStart);
		else
			r = grippoint[indices[0]].distanceTo(m_ptEnd);
		m_dDiameter = r*2;
	}*/
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDCylinder::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDCylinder::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
    assertReadEnabled();

	//if( m_ptEnd.distanceTo( m_ptStart)<1) return  Acad::eNotApplicable;

    double SpEp = m_ptStart.distanceTo(m_ptEnd);
    AcDb3dSolid *pBody;
    double radius = m_dDiameter / 2.0;
    pBody = new AcDb3dSolid;
    if(pBody->createFrustum(SpEp, radius, radius, radius) != Acad::eOk)
    {
        delete pBody;
        return Acad::eNotApplicable;
    }

    AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();

    AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
    if (fabs(vect[X]) <1.0/64 && fabs(vect[Y])<1.0/64) 
        Ax = Wy.crossProduct(vect);
    else
        Ax = Wz.crossProduct(vect);
    Ax.normalize();
    Ay = vect.crossProduct(Ax);
    Ay.normalize();

    AcGeMatrix3d mat;
    mat.setCoordSystem(m_ptStart + vect * SpEp / 2.0, Ax, Ay, vect);
    pBody->transformBy(mat);
    pBody->setPropertiesFrom(this);
    entitySet.append(pBody);
	
	return Acad::eOk;
}

Acad::ErrorStatus PDCylinder::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	//if( m_ptEnd.distanceTo( m_ptStart)<1){
	//	return  Acad::eNotApplicable;
	//}

	double SpEp = m_ptStart.distanceTo(m_ptEnd);
	AcDb3dSolid *pBody;
	double radius = m_dDiameter / 2.0;
	pBody = new AcDb3dSolid;
	if(pBody->createFrustum(SpEp, radius, radius, radius) != Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();

	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
	if (fabs(vect[X]) <1.0/64 && fabs(vect[Y])<1.0/64) 
		Ax = Wy.crossProduct(vect);
	else
		Ax = Wz.crossProduct(vect);
	Ax.normalize();
	Ay = vect.crossProduct(Ax);
	Ay.normalize();

	AcGeMatrix3d mat;
	mat.setCoordSystem(m_ptStart + vect * SpEp / 2.0, Ax, Ay, vect);
	pBody->transformBy(mat);
	pBody->setPropertiesFrom(this);
	
	p3dSolid = pBody;

	return Acad::eOk;
}

void PDCylinder::createBody()
{
    assertWriteEnabled();
#ifdef _OBJECTARX2004_
    m_3dGeom = Body::cylinder(Line3d(*(Point3d*)&m_ptStart, *(Point3d*)&m_ptEnd), m_dDiameter / 2.0, m_dDividPrecision);
#endif
}

// 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
// 精度不大于实体限定的剖分精度m_dDividPrecision。
int PDCylinder::getMaxPrecision(AcGiWorldDraw *mode) const
{
    assertReadEnabled();
    if(!getCalMaxPrecision())
        return m_dDividPrecision;
    int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptStart)
                    * 2.0 / m_dDiameter));
    if(pre > m_dDividPrecision)
        return m_dDividPrecision;
    else if(pre > 8)
        return pre;
    else
        return 8;
}

#ifdef _OBJECTARX2010_
void PDCylinder::subList() const
#else
void PDCylinder::list() const
#endif
{
    assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4743/*"\n 起点圆心(%f,%f,%f)\n"*/,m_ptStart.x,m_ptStart.y,m_ptStart.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4744/*" 终点圆心(%f,%f,%f)\n"*/,m_ptEnd.x,m_ptEnd.y,m_ptEnd.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4745/*" 直径=%.3f\n\n"*/,m_dDiameter);
	return ;   
}// added by linlin 20050810

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDCylinder::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDCylinder::getGeomExtents(AcDbExtents& extents) const
#endif
{
    assertReadEnabled();
    extents.set(m_ptStart, m_ptStart);
    AcGePoint3dArray pArray;
    AcGeIntArray stdIdx;
    int actPrecision;
    getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
    int i;
    for(i = 0; i < actPrecision; ++i)
    {
        extents.addPoint(pArray[i]);
        extents.addPoint(pArray[i + actPrecision + 1]);
    }
    return Acad::eOk;
}
Adesk::Boolean PDCylinder::intersectWithLineSeg(const AcGeLineSeg3d& line,
										AcGePoint3dArray&   points)const
{
	AcGePoint3d p1, p2;
	int noOfInter = -1;
	AcGeCylinder geCylinder(m_dDiameter/2.0, m_ptStart, (m_ptEnd-m_ptStart).normal());
	Adesk::Boolean flg = 
		geCylinder.intersectWith(line, noOfInter, p1, p2);
	if (flg == Adesk::kTrue)
	{
		points.append(p1);
		if (noOfInter == 2)
		{
			points.append(p2);
		}
		return Adesk::kTrue;
	}
	else
	{
		return Adesk::kFalse;// no intersection
	}
	//AcDb3dSolid *pSolid = NULL;
	//AcDbLine dbLine(line.startPoint(), line.endPoint());
	//if ( Acad::eOk == explodeTo3DSolid(pSolid))
	//{
	//	//Acad::ErrorStatus es = pSolid->intersectWith(&dbLine, AcDb::kOnBothOperands, points);
	//	Acad::ErrorStatus es = dbLine.intersectWith(pSolid, AcDb::kOnBothOperands, points);

	//	delete pSolid;
	//	return (es == Acad::eOk);
	//}
	//else
	//	return Adesk::kFalse;
}
