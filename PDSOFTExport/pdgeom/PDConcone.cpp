// PDConcone.cpp: implementation of the PDConcone class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDConcone.h"
#include "dbproxy.h"
#include "geassign.h"
#include "acgi.h"
#include <GEINTARR.H>
#include "dbmain.h"
#include "rxboiler.h"
#include <dbsol3d.h>
#include <adscodes.h>
#include <dbapserv.h>

#include <math.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDCONCONE 1
#define PI 3.1415926535897932384626433832795L


#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDConcone, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
                        PDCONCONE, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDConcone, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDCONCONE, Gle);
#endif

void PDConcone::setDefault()
{
  PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
  m_dDiameter1 = m_dDiameter2 = 1.0 ;
  m_ptEnd =AcGePoint3d (0,0,0) ;
  m_ptStart =AcGePoint3d (0,0,1) ;
  m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
#else
  m_dDiameter1 = m_dDiameter2 = 1000 ;
  m_ptEnd =AcGePoint3d (0,0,0) ;
  m_ptStart =AcGePoint3d (1000,0,0) ;
  m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
#endif
}
bool PDConcone::isValidData(double &f)
{
  if(!PDPrimary3D::isValidData(f))
    return false;
  if(!isValidFloat(f=m_dDiameter1))
    return false;
  if(!isValidFloat(f=m_dDiameter2))
    return false;
  if(!isValidFloat(f=m_ptEnd.x) || !isValidFloat(f=m_ptEnd.y) || !isValidFloat(f=m_ptEnd.z))
    return false;
  if(!isValidFloat(f=m_ptStart.x) || !isValidFloat(f=m_ptStart.y) || !isValidFloat(f=m_ptStart.z))
    return false;
  return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDConcone::audit(AcDbAuditInfo* pAuditInfo)
{
  return auditPDPrimary3D(this,pAuditInfo,_T("PDConcone"));
}

PDConcone::PDConcone(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dDiameter1 = m_dDiameter2 = 1.0 ;
	m_ptEnd =AcGePoint3d (0,0,0) ;
	m_ptStart =AcGePoint3d (0,0,1) ;
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
}

PDConcone::PDConcone(const AcGePoint3d &ptStart, const AcGePoint3d &ptEnd, 
                     double Diameter1, double Diameter2, 
                     int Precision/* = PRIMARY3D_DEFAULT_PRECISION*/, 
                     bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_ptStart = ptStart;
	m_ptEnd = ptEnd;
	m_dDiameter1 =fabs(Diameter1) ;
	if(m_dDiameter1==0)
		m_dDiameter1=1;
	m_dDiameter2 =fabs(Diameter2) ;
  
	if(m_ptStart.isEqualTo(m_ptEnd))
		m_ptEnd=m_ptStart+AcGeVector3d(0,0,1);

	if(Precision > 2)
        m_dDividPrecision = Precision;
    else
        m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

#ifdef _USEAMODELER_
    createBody();
#endif
}//added by  linlin 20050929


PDConcone::~PDConcone()
{

}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDConcone::subTransformBy(const AcGeMatrix3d &xform)
#else
Acad::ErrorStatus PDConcone::transformBy(const AcGeMatrix3d &xform)
#endif
{
    assertWriteEnabled();
	m_ptStart.transformBy(xform);
	m_ptEnd.transformBy(xform);

	m_dDiameter1 *= xform.scale();
	m_dDiameter2 *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}
#ifdef _OBJECTARX2010_
Adesk::Boolean PDConcone::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDConcone::worldDraw(AcGiWorldDraw* mode)
#endif
{
	assertReadEnabled();
	//if(acdbHostApplicationServices()->workingDatabase()->useri1() == 1)
	//	return Adesk::kTrue;

#ifdef _USEAMODELER_
#ifdef _OBJECTARX2010_
	PDPrimary3D::subWorldDraw(mode);
#else
	PDPrimary3D::worldDraw(mode);
#endif
#endif
	if (mode->regenAbort()) {
		return Adesk::kTrue;
	}
#ifndef _USEAMODELER_

	AcGePoint3dArray pArray;
	int PreStep = 1;
	int actPrecision = m_dDividPrecision;
	int actPrecision__1 = actPrecision + 1;
	int i;
	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE2 
		|| nRating == PRIMARY3D_SIMPLIZE_RATE1) 
	{ 
		// 优化
		if(m_dDiameter1 > m_dDiameter2)
		{
			actPrecision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptStart)
				* 2.0 / m_dDiameter1));
		}
		else
		{
			actPrecision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptEnd)
				* 2.0 / m_dDiameter2));
		}
		if(actPrecision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{
				//简化为线
				AcGePoint3d pt[2];
				pt[0] = m_ptStart;
				pt[1] = m_ptEnd;
				/*if*/(mode->geometry().polyline(2, pt));
			}
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();

				/////绘制两端的圆
				if(mode -> geometry().circle( m_ptStart, m_dDiameter1 / 2.0, vect))
					return Adesk::kTrue;
				if(mode -> geometry().circle( m_ptEnd, m_dDiameter2 / 2.0, vect))
					return Adesk::kTrue;
				actPrecision = PRIMARY3D_WIREFRAME_LINENUM4;
				actPrecision__1 = actPrecision + 1;

				getVertices( actPrecision,pArray);
				AcGePoint3d pt[2];
				for(i = 0; i < actPrecision; i += PreStep)
				{
					pt[0] = pArray[i];
					pt[1] = pArray[i + actPrecision__1];
					if(mode->geometry().polyline(2, pt))
						return Adesk::kTrue;
				}
			}
			/*return Adesk::kTrue;
			double radius = m_dDiameter1 / 2.0;
			AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
			if(mode -> geometry().circle( m_ptStart, radius, vect))
			return Adesk::kTrue;
			radius = m_dDiameter2 / 2.0;
			mode -> geometry().circle( m_ptEnd, radius, vect);*/
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
			int gsIndex = 0;
			mode->subEntityTraits().setFillType(kAcGiFillAlways);
			if(actPrecision < PRIMARY3D_WIREFRAME_LINENUM * 2)
				actPrecision = PRIMARY3D_WIREFRAME_LINENUM;
			else
			{
				if(actPrecision > m_dDividPrecision)
					actPrecision = m_dDividPrecision;
				PreStep = actPrecision / PRIMARY3D_WIREFRAME_LINENUM;
				actPrecision = PreStep * PRIMARY3D_WIREFRAME_LINENUM;
			}
			getVertices( actPrecision,pArray);
			actPrecision__1 = actPrecision + 1;
			///////////////////////////////////////////////////////
			///////		画两端面						
			///////////////////////////////////////////////////////
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(actPrecision, &pArray[0]))
				return Adesk::kTrue;
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(actPrecision, &pArray[actPrecision__1]))
				return Adesk::kTrue;

			AcGiEdgeData edgeData;
			int edgeDataLen = actPrecision__1 * 3 - 2;
			Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
			for(i = 0; i < edgeDataLen; ++i)
				edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
			int actPrecision___2 = actPrecision * 2;
			for(i = actPrecision___2;i < edgeDataLen; i += PreStep)
			{
				edgeVisibility[i] = kAcGiVisible;
				if(PreStep > 1 && i != edgeDataLen - 1)
				{
					for(int j = 1; j < PreStep; ++j)
						edgeVisibility[i + j] = kAcGiInvisible;
				}
			}
			edgeData.setVisibility(edgeVisibility);
			///////////////////////////////////////////////////////////
			///////		画旋转曲面	
			///////////////////////////////////////////////////////////
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			mode->geometry().mesh(2, actPrecision__1, pArray.asArrayPtr(), &edgeData);
			delete [] edgeVisibility;
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
#endif
		{
			int gsIndex = 0;
			AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();

			/////绘制两端的圆
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptStart, m_dDiameter1 / 2.0, vect))
				return Adesk::kTrue;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptEnd, m_dDiameter2 / 2.0, vect))
				return Adesk::kTrue;
			actPrecision = PRIMARY3D_WIREFRAME_LINENUM;
			actPrecision__1 = actPrecision + 1;

			getVertices( actPrecision,pArray);
			AcGePoint3d pt[2];
			++gsIndex;
			for(i = 0; i < actPrecision; i += PreStep)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + actPrecision__1];
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
	return Adesk::kTrue;
}
/*Adesk::Boolean PDConcone::worldDraw(AcGiWorldDraw* mode)
{
	assertReadEnabled();
#ifdef _USEAMODELER_
    PDPrimary3D::worldDraw(mode);
#endif
    if (mode->regenAbort()) {
        return Adesk::kTrue;
    }
#ifndef _USEAMODELER_

    AcGePoint3dArray pArray;
    AcGeIntArray stdIdx;
    int actPrecision;
    getVertices(getMaxPrecision(mode), pArray, stdIdx, actPrecision);
    int actPrecision__1 = actPrecision + 1;
    int stdIdxLen = stdIdx.length();
    int stdIdxLen_1 = stdIdxLen - 1;
    int i;

	switch(mode->regenType())
    {
	case kAcGiHideOrShadeCommand:
	case kAcGiRenderCommand:
        {
		    mode->subEntityTraits().setFillType(kAcGiFillAlways);
		    ///////////////////////////////////////////////////////
		    ///////		画两端面						
		    ///////////////////////////////////////////////////////
		    mode->geometry().polygon(actPrecision, &pArray[0]);
		    mode->geometry().polygon(actPrecision, &pArray[actPrecision__1]);

            AcGiEdgeData edgeData;
            int edgeDataLen = actPrecision__1 * 3 - 2;
            Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
	        for(i = 0; i < edgeDataLen; ++i)
                edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
            int actPrecision___2 = actPrecision * 2;
            for(i = 0; i < stdIdxLen; ++i)
                edgeVisibility[stdIdx[i] + actPrecision___2] = kAcGiVisible;
            edgeData.setVisibility(edgeVisibility);

		    ///////////////////////////////////////////////////////////
		    ///////		画旋转曲面	
		    ///////////////////////////////////////////////////////////
		    mode->geometry().mesh(2, actPrecision__1, pArray.asArrayPtr(), &edgeData);
            delete [] edgeVisibility;
		}
        break;
	case kAcGiStandardDisplay:
    case kAcGiSaveWorldDrawForR12:
    case kAcGiSaveWorldDrawForProxy:
        {
            int gsIndex = 0;
		    AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();

		    /////绘制两端的圆
            mode->subEntityTraits().setSelectionMarker(++gsIndex);
		        mode -> geometry().circle( m_ptStart, m_dDiameter1 / 2.0, vect);
            mode->subEntityTraits().setSelectionMarker(++gsIndex);
		        mode -> geometry().circle( m_ptEnd, m_dDiameter2 / 2.0, vect);

            AcGePoint3d pt[2];
            for(i = 0; i < stdIdxLen_1; ++i)
            {
                pt[0] = pArray[stdIdx[i]];
                pt[1] = pArray[stdIdx[i] + actPrecision__1];
                mode->subEntityTraits().setSelectionMarker(++gsIndex);
                mode->geometry().polyline(2, pt);
            }
		}
        break;
    default:
        break;
	}
#endif
	return Adesk::kTrue;
}
*/
//////////////////////////////////////////////////////////////////////
/////	对圆台上下圆进行剖分，取得离散点，放到引用数组中		//////
/////	其剖分精度读取父类PDPrimary3D的精度值m_dDividPrecision	//////
//////////////////////////////////////////////////////////////////////
Acad::ErrorStatus PDConcone::getVertices(int Precision, AcGePoint3dArray& vertexArray, 
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

    AcGeVector3d AxE = Ax * m_dDiameter2 / 2.0;
    Ax *= m_dDiameter1 / 2.0;
    double rotateAngPre = PI * 2.0 / Precision;

    vertexArray[0] = m_ptStart + Ax;
    vertexArray[actPrecision__1] = m_ptEnd + AxE;
    AcGeVector3d VecSTemp, VecETemp;
    int i;
    for(i = 1; i < actPrecision; i++)
    {
        VecSTemp = Ax;
        VecETemp = AxE;
        VecSTemp.rotateBy(rotateAngPre * dividDbl[i], vect);
        VecETemp.rotateBy(rotateAngPre * dividDbl[i], vect);
        vertexArray[i] = m_ptStart + VecSTemp;
        vertexArray[i + actPrecision__1] = m_ptEnd + VecETemp;
    }
    vertexArray[actPrecision] = vertexArray[0];
    vertexArray[actPrecision + actPrecision__1] = vertexArray[actPrecision__1];

	return Acad::eOk;
}
Acad::ErrorStatus PDConcone::getVertices(int Precision, AcGePoint3dArray& vertexArray)const
{
	assertReadEnabled();
	int Precision__1 = Precision + 1;
	vertexArray.setLogicalLength(Precision__1 * 2);

	AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
	if (fabs(vect[X]) <1.0/64 && fabs(vect[Y])<1.0/64) 
		Ax = Wy.crossProduct(vect);
	else
		Ax = Wz.crossProduct(vect);
	Ax.normalize();

	AcGeVector3d AxE = Ax * m_dDiameter2 / 2.0;
	Ax *= m_dDiameter1 / 2.0;
	double rotateAngPre = PI * 2.0 / Precision;

	vertexArray[0] = m_ptStart + Ax;
	vertexArray[Precision__1] = m_ptEnd + AxE;
	AcGeVector3d VecSTemp, VecETemp;
	int i;
	VecSTemp = Ax;
	VecETemp = AxE;
	for(i = 1; i < Precision; i++)
	{
		VecSTemp.rotateBy(rotateAngPre, vect);
		VecETemp.rotateBy(rotateAngPre, vect);
		vertexArray[i] = m_ptStart + VecSTemp;
		vertexArray[i + Precision__1] = m_ptEnd + VecETemp;
	}
	vertexArray[Precision] = vertexArray[0];
	vertexArray[Precision + Precision__1] = vertexArray[Precision__1];

	return Acad::eOk;
}
Acad::ErrorStatus PDConcone::dwgOutFields(AcDbDwgFiler* filer)const
{
    assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDCONCONE);

	// Write the data members.
	filer->writeItem(m_ptStart);
	filer->writeItem(m_ptEnd);
	filer->writeItem(m_dDiameter1);
	filer->writeItem(m_dDiameter2);

	return filer->filerStatus();
}

Acad::ErrorStatus PDConcone::dwgInFields(AcDbDwgFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgInFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgInFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Read version number.
	Adesk::UInt16 version;
	filer->readItem(&version);
	if (version > VERSION_PDCONCONE)
		return Acad::eMakeMeProxy;

	// Read the data members.
	switch (version){
	case (1):
		filer->readItem(&m_ptStart);
		filer->readItem(&m_ptEnd);
		filer->readItem(&m_dDiameter1);
		filer->readItem(&m_dDiameter2);
		break;
	}

#ifdef _USEAMODELER_
    if(m_3dGeom.isNull())
        createBody();
#endif

	return filer->filerStatus();
}

Acad::ErrorStatus PDConcone::dxfOutFields(AcDbDxfFiler* filer) const
{
    assertReadEnabled();
    Acad::ErrorStatus es;

	// Call dxfOutFields from father class: PDPrimary3D
    if ((es = PDPrimary3D::dxfOutFields(filer)) != Acad::eOk){
        return es;
	}

	// Write subclass marker.
    filer->writeItem(AcDb::kDxfSubclass, _T("PDConcone"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDCONCONE);

	// Write data members.
    filer->writeItem(AcDb::kDxfXCoord, m_ptStart);
	filer->writeItem(AcDb::kDxfXCoord+1, m_ptEnd);
    filer->writeItem(AcDb::kDxfReal,m_dDiameter1);
	filer->writeItem(AcDb::kDxfReal+1,m_dDiameter2);

    return filer->filerStatus();
}

Acad::ErrorStatus PDConcone::dxfInFields(AcDbDxfFiler* filer)
{
    assertWriteEnabled();
    Acad::ErrorStatus es = Acad::eOk;
    struct resbuf rb;

	// Call dxfInFields from father class: PDPrimary3D
    if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
        || !filer->atSubclassData(_T("PDConcone"))){
        return filer->filerStatus();
    }

	// Read version number.
	Adesk::UInt16 version;
	filer->readItem(&rb);
	if (rb.restype != AcDb::kDxfInt16) {
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("nError: expected object version group code %d"),
		                 AcDb::kDxfInt16);
		return filer->filerStatus();
	} 
	else{
		version = rb.resval.rint;
		if (version > VERSION_PDCONCONE)
			return Acad::eMakeMeProxy;
	}

	// Read the data members.
	switch (version){
	case (1):
		while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){
			switch (rb.restype) {
			case AcDb::kDxfXCoord:
				m_ptStart = asPnt3d(rb.resval.rpoint);
				break;

			case AcDb::kDxfXCoord+1:
				m_ptEnd = asPnt3d(rb.resval.rpoint);
				break;

			case AcDb::kDxfReal:
				m_dDiameter1 = rb.resval.rreal;
				break;

			case AcDb::kDxfReal+1:
				m_dDiameter2 = rb.resval.rreal;
				break;

			default:
            // An unrecognized group. Push it back so that
            // the subclass can read it again.
				filer->pushBackItem();
				es = Acad::eEndOfFile;
				break;
			}
        }
		break;
    }

#ifdef _USEAMODELER_
    if(m_3dGeom.isNull())
        createBody();
#endif

    // At this point the es variable must contain eEndOfFile
    // - either from readResBuf() or from pushback. If not,
    // it indicates that an error happened and we should
    // return immediately.
    //
    if (es != Acad::eEndOfFile)
        return Acad::eInvalidResBuf;

    return es;
}

Acad::ErrorStatus PDConcone::setParameters(const AcGePoint3d &ptStart, 
                                           const AcGePoint3d &ptEnd, 
                                           double Diameter1, 
                                           double Diameter2, 
                                           int Precision/* = PRIMARY3D_DEFAULT_PRECISION*/)

{
    assertWriteEnabled();

	m_ptStart = ptStart;
	m_ptEnd = ptEnd;
	m_dDiameter1 =fabs(Diameter1) ;
	if(m_dDiameter1==0)
		m_dDiameter1=1;
	m_dDiameter2 =fabs(Diameter2) ;

	
	if(m_ptStart.isEqualTo(m_ptEnd))
		m_ptEnd=m_ptStart+AcGeVector3d(0,0,1);

    if(Precision > 2)
        m_dDividPrecision = Precision;
    else
        m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

#ifdef _USEAMODELER_
    createBody();
#endif
    return Acad::eOk;
}//added by linlin 20050929

AcGePoint3d PDConcone::getpointStart() const
{
	assertReadEnabled();
	return m_ptStart;
}

AcGePoint3d PDConcone::getpointEnd() const
{
	assertReadEnabled();
	return m_ptEnd;
}

double PDConcone::getDiameter1() const
{
	assertReadEnabled();
	return m_dDiameter1;
}

double PDConcone::getDiameter2() const
{
	assertReadEnabled();
	return m_dDiameter2;
}


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDConcone::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDConcone::getOsnapPoints(
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
		snapPoints.append(m_ptEnd);
		snapPoints.append(m_ptStart);
        for(i = 0; i < stdIdxLen_1; ++i)
        {
            snapPoints.append(pArray[stdIdx[i]]);
            snapPoints.append(pArray[stdIdx[i] + actPrecision__1]);
        }
		break;
    case AcDb::kOsModeMid:
        snapPoints.append(m_ptStart + (m_ptEnd - m_ptStart) / 2.0);
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
                AcGeCircArc3d cir(m_ptStart, vect, m_dDiameter1 / 2.0);
                pt = cir.closestPointTo(lastPoint);
                snapPoints.append(pt);
            }
            else if(gsSelectionMark_int == 2)
            {
                AcGeCircArc3d cir(m_ptEnd, vect, m_dDiameter2 / 2.0);
                pt = cir.closestPointTo(lastPoint);
                snapPoints.append(pt);
            }
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
            else if(gsSelectionMark_int == 3)
            {
				for(int i = 0; i < stdIdxLen - 1; ++i)
				{
					vec = pArray[stdIdx[i] + actPrecision__1] - pArray[stdIdx[i]];
					vec.normalize();
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
                cir.set(m_ptStart, vect, m_dDiameter1 / 2.0);
                pt = cir.projClosestPointTo(pickPoint, viewDir);
                snapPoints.append(pt);
            }
            //上底面
            else if(gsSelectionMark_int == 2)
            {
                cir.set(m_ptEnd, vect, m_dDiameter2 / 2.0);
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
Acad::ErrorStatus PDConcone::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDConcone::getGripPoints(
										AcGePoint3dArray& gripPoints,
										AcDbIntArray& osnapModes,
										AcDbIntArray& geomIds) const
#endif
{
    assertReadEnabled();
/*	getgrippoints(gripPoints);*/
	return Acad::eOk; 
}


Acad::ErrorStatus PDConcone::getgrippoints(AcGePoint3dArray& gripArray) const
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

    for(int i = 0; i < stdIdxLen_1; i++)
    {
        gripArray.append(pArray[stdIdx[i]]);
        gripArray.append(pArray[stdIdx[i] + actPrecision__1]);
    }
	return Acad::eOk; 
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDConcone::subMoveGripPointsAt(
		const AcDbIntArray& indices,
		const AcGeVector3d& offset)
#else
Acad::ErrorStatus PDConcone::moveGripPointsAt(
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
		if(indices[0] < 6){
			r = grippoint[indices[0]].distanceTo(m_ptStart);
			m_dDiameter1 = r*2;
		}
		else{
			r = grippoint[indices[0]].distanceTo(m_ptEnd);
			m_dDiameter2 = r*2;
		}
	}*/
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDConcone::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDConcone::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
  assertReadEnabled();

  AcGePoint3d sp, ep;
  double d1, d2;
  if(m_dDiameter1 >= m_dDiameter2)
    {
      sp = m_ptStart;
      ep = m_ptEnd;
      d1 = m_dDiameter1;
      d2 = m_dDiameter2;
    }
  else
    {
      sp = m_ptEnd;
      ep = m_ptStart;
      d1 = m_dDiameter2;
      d2 = m_dDiameter1;
    }

    double SpEp = sp.distanceTo(ep);
  AcDb3dSolid *pBody;
  pBody = new AcDb3dSolid;
  if(pBody->createFrustum(sp.distanceTo(ep), d1 / 2.0, d1 / 2.0, d2 / 2.0) 
    != Acad::eOk)
    {
      delete pBody;
      return Acad::eNotApplicable;
    }

    AcGeVector3d vect = (ep - sp).normal();

    AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
    if (fabs(vect[X]) <1.0/64 && fabs(vect[Y])<1.0/64) 
        Ax = Wy.crossProduct(vect);
    else
        Ax = Wz.crossProduct(vect);
    Ax.normalize();
    Ay = vect.crossProduct(Ax);
    Ay.normalize();

    AcGeMatrix3d mat;
    mat.setCoordSystem(sp + vect * SpEp / 2.0, Ax, Ay, vect);
    pBody->transformBy(mat);
    pBody->setPropertiesFrom(this);
    entitySet.append(pBody);

	return Acad::eOk;
}

Acad::ErrorStatus PDConcone::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	AcGePoint3d sp, ep;
	double d1, d2;
	if(m_dDiameter1 >= m_dDiameter2)
	{
		sp = m_ptStart;
		ep = m_ptEnd;
		d1 = m_dDiameter1;
		d2 = m_dDiameter2;
	}
	else
	{
		sp = m_ptEnd;
		ep = m_ptStart;
		d1 = m_dDiameter2;
		d2 = m_dDiameter1;
	}

	double SpEp = sp.distanceTo(ep);
	AcDb3dSolid *pBody;
	pBody = new AcDb3dSolid;
	if(pBody->createFrustum(sp.distanceTo(ep), d1 / 2.0, d1 / 2.0, d2 / 2.0) 
		!= Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeVector3d vect = (ep - sp).normal();

	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
	if (fabs(vect[X]) <1.0/64 && fabs(vect[Y])<1.0/64) 
		Ax = Wy.crossProduct(vect);
	else
		Ax = Wz.crossProduct(vect);
	Ax.normalize();
	Ay = vect.crossProduct(Ax);
	Ay.normalize();

	AcGeMatrix3d mat;
	mat.setCoordSystem(sp + vect * SpEp / 2.0, Ax, Ay, vect);
	pBody->transformBy(mat);
	pBody->setPropertiesFrom(this);
	
	p3dSolid = pBody;

	return Acad::eOk;
}

void PDConcone::createBody()
{
    assertWriteEnabled();
//#ifdef _OBJECTARX2004_
    m_3dGeom = Body::cone(Line3d(*(Point3d*)&m_ptStart, *(Point3d*)&m_ptEnd), m_dDiameter1 / 2.0, m_dDiameter2 / 2.0, m_dDividPrecision);
//#endif
}

// 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
// 精度不大于实体限定的剖分精度m_dDividPrecision。
int PDConcone::getMaxPrecision(AcGiWorldDraw *mode) const
{
    assertReadEnabled();
    if(!getCalMaxPrecision())
        return m_dDividPrecision;
    double dia;
    if(m_dDiameter1 > m_dDiameter2)
        dia = m_dDiameter1;
    else
        dia = m_dDiameter2;
    int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptStart)
                    * 2.0 / dia));
    if(pre > m_dDividPrecision)
        return m_dDividPrecision;
    else if(pre > 8)
        return pre;
    else
        return 8;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDConcone::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDConcone::getGeomExtents(AcDbExtents& extents) const
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

#ifdef _OBJECTARX2010_
void PDConcone::subList() const
#else
void PDConcone::list() const
#endif
{
    assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4738/*"\n 底面圆心(%f,%f,%f)\n"*/,m_ptStart.x,m_ptStart.y,m_ptStart.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4739/*" 顶面圆心(%f,%f,%f)\n"*/,m_ptEnd.x,m_ptEnd.y,m_ptEnd.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4740/*" 底面直径=%.3f\n"*/,m_dDiameter1);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4741/*" 顶面直径=%.3f\n\n"*/,m_dDiameter2);
	return ;
}// added by linlin 20050810




/*---------------------------------------------------------------------------
* 名称: _make_eqovalpt
* 功能: 完成 "在设备椭球封头上画出点" 的功能
* 注意: 无
*/
int  PDConcone:: _make_eqovalpt(double lengthR,AcGeVector3d vec,ads_point pt,ads_point normal) 
{
	double R1,R2;
	R1=m_dDiameter1/2;
	R2=m_dDiameter2/2;
    if(lengthR>=0)
    if((lengthR <=R1 && lengthR<= R2)||(lengthR>=R2 && lengthR>=R1))
		return RTERROR;
	if(lengthR<=0)
		if(-lengthR>m_ptStart.distanceTo(m_ptEnd))
			return RTERROR;

	AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
	AcGePoint3d  point,center1,point1;
    AcGeVector3d L,Lnormal,generatrix;
	double H1,z;
	H1=m_ptStart.distanceTo(m_ptEnd);

	if(lengthR>=0)
	{
		z=H1*(R1-lengthR)/(R1-R2);	
		center1=m_ptStart+vect*z;
	    point =center1+vec*lengthR;
	}
	else
	{
		z=-lengthR;
		if(R1>=R2)
		{
			center1=m_ptStart+vect*z;
			point =center1+vec*(R1-z*(R1-R2)/H1);
  		}
		else
		{
			vect = (m_ptStart - m_ptEnd).normal();
           center1=m_ptEnd+vect*z;
		    point=center1+vec*(R2-z*(R2-R1)/H1);
		}

	}
	
	point1=m_ptStart+vec*R1;
    generatrix=(point-point1).normalize();
    L=generatrix.crossProduct(vec);
	Lnormal=L.crossProduct(generatrix);
	Lnormal.normalize();

    pt[0]=point.x;
    pt[1]=point.y;
    pt[2]=point.z;

    normal[0]=Lnormal.x;
    normal[1]=Lnormal.y;
    normal[2]=Lnormal.z;

	return RTNORM;
	
}//added by linlin 20051011