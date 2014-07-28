// PDEcone.cpp: implementation of the PDEcone class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDEcone.h"
#include <dbproxy.h>
#include <geassign.h>
#include <acgi.h>
#include <dbmain.h>
#include <rxboiler.h>
#include <dbsol3d.h>
#include <dbents.h>
#include <dbregion.h>
#include <geintarr.h>
#include <aced.h>
#include <adscodes.h>

#ifdef _OBJECTARX2000_
#include <migrtion.h>
#include <dbapserv.h>
#endif

#include <math.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDECONE 1
#define PI 3.1415926535897932384626433832795L

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDEcone, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
                        PDECONE, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDEcone, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDECONE, Gle);
#endif

void PDEcone::setDefault()
{
  PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
  m_dDiameter1 = m_dDiameter2 = 1.0;
  m_ptStart = AcGePoint3d (0, 0, 0);
  m_ptEnd = AcGePoint3d (0, 0, 1);
  m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
  m_vect.set(1, 0, 0);
#else
  m_dDiameter1 = m_dDiameter2 = 1000;
  m_ptStart = AcGePoint3d (0, 0, 0);
  m_ptEnd = AcGePoint3d (0, 0, 1000);
  m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
  m_vect.set(1, 0, 0);
#endif
}
bool PDEcone::isValidData(double &f)
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
  if(!isValidFloat(f=m_vect.x) || !isValidFloat(f=m_vect.y) || !isValidFloat(f=m_vect.z))
    return false;
  return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDEcone::audit(AcDbAuditInfo* pAuditInfo)
{
  return auditPDPrimary3D(this,pAuditInfo,_T("PDEcone"));
}

PDEcone::PDEcone(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dDiameter1 = m_dDiameter2 = 1.0;
	m_ptStart = AcGePoint3d (0, 0, 0);
	m_ptEnd = AcGePoint3d (0, 0, 1);
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_vect.set(1, 0, 0);
#ifdef _USEAMODELER_
    createBody();
#endif
}

PDEcone::PDEcone(const AcGePoint3d &ptStart, const AcGePoint3d &ptEnd, 
            double Diameter1, double Diameter2, const AcGeVector3d &vect, 
            int Precision/* = PRIMARY3D_DEFAULT_PRECISION*/, 
            bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_ptStart = ptStart;
	m_ptEnd = ptEnd;
	
	if (m_ptStart.isEqualTo(m_ptEnd))
		m_ptEnd=m_ptStart+AcGeVector3d(0,0,1);

	m_dDiameter1 =fabs(Diameter1) ;
	if(m_dDiameter1==0)
		m_dDiameter1=1;
	m_dDiameter2 =fabs(Diameter2) ;
	
	m_vect = vect.normal();
    if(Precision > 2)
	    m_dDividPrecision = Precision;
    else
        m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

    CalActParameter();
#ifdef _USEAMODELER_
    createBody();
#endif
}//added by linlin 20050929

PDEcone::~PDEcone()
{
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDEcone::subTransformBy(const AcGeMatrix3d &xform)
#else
Acad::ErrorStatus PDEcone::transformBy(const AcGeMatrix3d &xform)
#endif
{
	assertWriteEnabled();
	m_ptStart.transformBy(xform);
	m_ptEnd.transformBy(xform);
	m_vect.transformBy(xform);

	m_dDiameter1 *= xform.scale();
	m_dDiameter2 *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}
#ifdef _OBJECTARX2010_
Adesk::Boolean PDEcone::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDEcone::worldDraw(AcGiWorldDraw* mode)
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
	Adesk::Int16 nRating = getCalMaxPrecision();
	int i;
	int actPrecision__1 = actPrecision +1;
	if(nRating == PRIMARY3D_SIMPLIZE_RATE2 
		|| nRating == PRIMARY3D_SIMPLIZE_RATE1)
	{
		//优化
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
				AcGePoint3d pt[2];
				pt[0] = m_ptStart;
				pt[1] = m_ptEnd;
				/*if*/(mode->geometry().polyline(2, pt));
			} 
			else if(nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				AcGeVector3d faceVect = getFaceVect();
				if(mode->geometry().circle(m_ptStart, m_dDiameter1 / 2.0, faceVect))
					return Adesk::kTrue;
				if(mode->geometry().circle(m_ptEnd, m_dDiameter2 / 2.0, faceVect))
					return Adesk::kTrue;
				actPrecision = PRIMARY3D_WIREFRAME_LINENUM4;
				getVertices(actPrecision,pArray);
				actPrecision__1 = actPrecision + 1;
				AcGePoint3d pt[2];
				for(i = 0; i < actPrecision; i += PreStep)
				{
					pt[0] = pArray[i];
					pt[1] = pArray[i + actPrecision__1];
					if(mode->geometry().polyline(2, pt))
						return Adesk::kTrue;
				}
			}
			return Adesk::kTrue;
		}
	}
#ifndef _ALWAYSSHOWWIRE_
	switch(mode->regenType())
	{
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
#endif
		{
			int gsIndex = 0;
			AcGeVector3d faceVect = getFaceVect();
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode->geometry().circle(m_ptStart, m_dDiameter1 / 2.0, faceVect))
				return Adesk::kTrue;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode->geometry().circle(m_ptEnd, m_dDiameter2 / 2.0, faceVect))
				return Adesk::kTrue;
			actPrecision = PRIMARY3D_WIREFRAME_LINENUM;
			getVertices(actPrecision,pArray);
			actPrecision__1 = actPrecision + 1;

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
			getVertices(actPrecision,pArray);
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
			///////画旋转曲面	
			///////////////////////////////////////////////////////////
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			mode->geometry().mesh(2, actPrecision__1, pArray.asArrayPtr(), &edgeData);
			delete [] edgeVisibility;
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
/*Adesk::Boolean PDEcone::worldDraw(AcGiWorldDraw* mode)
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
	case kAcGiStandardDisplay:
    case kAcGiSaveWorldDrawForR12:
    case kAcGiSaveWorldDrawForProxy:
        {
            int gsIndex = 0;
	        AcGeVector3d faceVect = getFaceVect();

            mode->subEntityTraits().setSelectionMarker(++gsIndex);
		    mode->geometry().circle(m_ptStart, m_dDiameter1 / 2.0, faceVect);
            mode->subEntityTraits().setSelectionMarker(++gsIndex);
		    mode->geometry().circle(m_ptEnd, m_dDiameter2 / 2.0, faceVect);

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
    default:
        break;
	}
#endif
	return Adesk::kTrue;
}
*/
/////////////////////////////////////////////////////////////////
//////	求两端圆并进行剖分，将得到的离散放到引用树组中		/////
/////////////////////////////////////////////////////////////////
Acad::ErrorStatus PDEcone::getVertices(int Precision, AcGePoint3dArray& vertexArray, 
                                       AcGeIntArray &stdIdx, int &actPrecision) const
{
    assertReadEnabled();
    AcGeDoubleArray dividDbl;
    getActDivid(Precision, dividDbl, stdIdx);

    int actPrecision__1 = dividDbl.length();
    actPrecision = actPrecision__1 - 1;
    vertexArray.setLogicalLength(actPrecision__1 * 2);

    AcGeVector3d vect = getFaceVect();

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
    for(i = 1; i < actPrecision; ++i)
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
///////////////////////////////////////////////////////////////////////////////
///  public overloaded constant  getVertices
///  <TODO: insert function description here>
///
///  @param  Precision int     
///  @param  vertexArray AcGePoint3dArray &    
///
///  @return Acad::ErrorStatus 
///
///  @remarks 
///
///  @see 
///
///  @author BeJing ZhongKe Fulong, shinf @date 2007-1-5
/// 北京中科辅龙计算机技术有限公司，PDSOFT产品事业部--研发
///////////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus PDEcone::getVertices(int Precision, AcGePoint3dArray& vertexArray) const
{
	assertReadEnabled();
	int actPrecision__1 = Precision + 1;
	vertexArray.setLogicalLength(actPrecision__1 * 2);

	AcGeVector3d vect = getFaceVect();

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
	VecSTemp = Ax;
	VecETemp = AxE;
	for(i = 1; i < Precision; ++i)
	{
		VecSTemp.rotateBy(rotateAngPre , vect);
		VecETemp.rotateBy(rotateAngPre, vect);
		vertexArray[i] = m_ptStart + VecSTemp;
		vertexArray[i + actPrecision__1] = m_ptEnd + VecETemp;
	}
	vertexArray[Precision] = vertexArray[0];
	vertexArray[Precision + actPrecision__1] = vertexArray[actPrecision__1];

	return Acad::eOk;
}
Acad::ErrorStatus PDEcone::dwgOutFields(AcDbDwgFiler* filer) const
{
    assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDECONE);

	// Write the data members.
	filer->writeItem(m_ptStart);
	filer->writeItem(m_ptEnd);
	filer->writeItem(m_dDiameter1);
	filer->writeItem(m_dDiameter2);
	filer->writeItem(m_vect);

	return filer->filerStatus();
}

Acad::ErrorStatus PDEcone::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDECONE)
		return Acad::eMakeMeProxy;

	// Read the data members.
	switch (version)
	{
	case (1):

		filer->readItem(&m_ptStart);
		filer->readItem(&m_ptEnd);
		filer->readItem(&m_dDiameter1);
		filer->readItem(&m_dDiameter2);
		filer->readItem(&m_vect);
		break;
	}

#ifdef _USEAMODELER_
    if(m_3dGeom.isNull())
        createBody();
#endif

	return filer->filerStatus();
}


Acad::ErrorStatus PDEcone::dxfOutFields(AcDbDxfFiler* filer) const
{
    assertReadEnabled();
    Acad::ErrorStatus es;

    if ((es = PDPrimary3D::dxfOutFields(filer))
        != Acad::eOk)
    {
        return es;
    }

	// Write subclass marker.
    filer->writeItem(AcDb::kDxfSubclass, _T("PDEcone"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDECONE);

	// Write data members.
    filer->writeItem(AcDb::kDxfXCoord, m_ptStart);
	filer->writeItem(AcDb::kDxfXCoord+1, m_ptEnd);
    filer->writeItem(AcDb::kDxfNormalX, m_vect);
	filer->writeItem(AcDb::kDxfReal,m_dDiameter1);
	filer->writeItem(AcDb::kDxfReal+1,m_dDiameter2);

    return filer->filerStatus();
}

Acad::ErrorStatus PDEcone::dxfInFields(AcDbDxfFiler* filer)
{
    assertWriteEnabled();
    Acad::ErrorStatus es = Acad::eOk;
    struct resbuf rb;

    if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
        || !filer->atSubclassData(_T("PDEcone")))
    {
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
	else {
		version = rb.resval.rint;
		if (version > VERSION_PDECONE)
			return Acad::eMakeMeProxy;
	}

	// Read the data members.
	switch (version){
	case (1):
	    while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){

			switch (rb.restype){
			case AcDb::kDxfXCoord:
				m_ptStart = asPnt3d(rb.resval.rpoint);
				break;

			case AcDb::kDxfXCoord+1:
				m_ptEnd = asPnt3d(rb.resval.rpoint);
				break;

			 case AcDb::kDxfNormalX:
			    m_vect = asVec3d(rb.resval.rpoint);
			    break;

			 case AcDb::kDxfReal:
				m_dDiameter1=rb.resval.rreal;
				break;

			 case AcDb::kDxfReal+1:
				m_dDiameter2=rb.resval.rreal;
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

    return filer->filerStatus();
}

Acad::ErrorStatus PDEcone::setParameters(const AcGePoint3d &ptStart, 
                                         const AcGePoint3d &ptEnd, 
                                         double d1, 
                                         double d2, 
                                         const AcGeVector3d &vect, 
                                         int Precision/* = PRIMARY3D_DEFAULT_PRECISION*/)
{
	assertWriteEnabled();
	m_ptStart = ptStart;
	m_ptEnd = ptEnd;
   
	if(m_ptStart.isEqualTo(m_ptEnd))
		m_ptEnd=m_ptStart+AcGeVector3d(0,0,1);

	m_dDiameter1 =fabs(d1) ;
	if(m_dDiameter1==0)
		m_dDiameter1=1;
	m_dDiameter2 =fabs(d2) ;
	
	m_vect = vect.normal();
    if(Precision > 2)
	    m_dDividPrecision = Precision;
    else
        m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

	CalActParameter();

#ifdef _USEAMODELER_
    createBody();
#endif
	return Acad::eOk;
}//added by linlin 20050929

AcGePoint3d PDEcone::getpointStart() const
{
	assertReadEnabled();
	return m_ptStart;
}

AcGePoint3d PDEcone::getpointEnd() const
{
	assertReadEnabled();
	return m_ptEnd;
}

double PDEcone::getDiameter1() const
{
	assertReadEnabled();
	return m_dDiameter1;
}

double PDEcone::getDiameter2() const
{
	assertReadEnabled();
	return m_dDiameter2;
}

AcGeVector3d PDEcone::getVect() const
{
	assertReadEnabled();
	return m_vect;
}

AcGeVector3d PDEcone::getFaceVect() const
{
    assertReadEnabled();
    AcGeVector3d centerVect = m_ptEnd - m_ptStart;
	AcGeVector3d tmpV1 = centerVect.crossProduct(m_vect);
	return m_vect.crossProduct(tmpV1).normal();
}

AcGePoint3d PDEcone::getStartPtOrthoInEntPtPlane()const
{
	assertReadEnabled();

	AcGeVector3d centerVect = m_ptEnd - m_ptStart;
	AcGeVector3d tmpV = centerVect.crossProduct(m_vect);
	tmpV = m_vect.crossProduct(tmpV);
	return m_ptStart.orthoProject(AcGePlane(m_ptEnd, tmpV));
}

AcGePoint3d PDEcone::getEndPtOrthoInStartPtPlane()const
{
	assertReadEnabled();

	AcGeVector3d centerVect = m_ptEnd - m_ptStart;
	AcGeVector3d tmpV = centerVect.crossProduct(m_vect);
	tmpV = m_vect.crossProduct(tmpV);
	return m_ptEnd.orthoProject(AcGePlane(m_ptStart, tmpV));
}

double PDEcone::getHeight() const
{
	assertReadEnabled();
	AcGePoint3d orthoPt = getStartPtOrthoInEntPtPlane();
	return orthoPt.distanceTo(m_ptStart);
}
/*!
* 根据已经输入到类中的参数，修正终点和偏心方向，使其值正确。
*
* @param none
*
* @return Acad::ErrorStatus  : 永远为Acad::eOk
*/
Acad::ErrorStatus PDEcone::CalActParameter()
{
    assertWriteEnabled();
    AcGeVector3d centerVect = (m_ptEnd - m_ptStart).normalize();
    if(centerVect.isZeroLength() && m_vect.isZeroLength())
    {
        m_ptEnd = m_ptStart + AcGeVector3d(0, 0, 1);
        m_vect.set(1, 0, 0);
    }
    else if(centerVect.isZeroLength())
    {
        AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
        if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
            Ax = Wy.crossProduct(m_vect);
        else
            Ax = Wz.crossProduct(m_vect);
        Ax.normalize();
        //Ay = vect.crossProduct(Ax);
        //Ay.normalize();

        m_ptEnd = m_ptStart + Ax;
    }
    else if(m_vect.isZeroLength() || m_vect.isParallelTo(centerVect))
    {
        AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
        if (fabs(centerVect[X]) <1.0/64 && fabs(centerVect[Y])<1.0/64) 
            Ax = Wy.crossProduct(centerVect);
        else
            Ax = Wz.crossProduct(centerVect);
        Ax.normalize();
        //Ay = vect.crossProduct(Ax);
        //Ay.normalize();

        m_vect = Ax;
    }
    return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDEcone::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDEcone::getOsnapPoints(
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
    AcGeVector3d vect = getFaceVect();

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
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
            else if(gsSelectionMark_int == 3)
            {
				AcGeLineSeg3d lnsg;
				AcGePoint3d p1,p2;
				for(int i = 0; i < stdIdxLen - 1; ++i)
				{
					p1 = pArray[stdIdx[i]];
					p2 = pArray[stdIdx[i] + actPrecision__1];
					lnsg.set(p1, p2);
// 					lnsg.set(pArray[stdIdx[i]], pArray[stdIdx[i] + actPrecision__1]);
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
Acad::ErrorStatus PDEcone::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDEcone::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
    assertReadEnabled();
/*	getgrippoints(gripPoints);*/
	return Acad::eOk; 
}



Acad::ErrorStatus PDEcone::getgrippoints(AcGePoint3dArray& gripArray) const
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
Acad::ErrorStatus PDEcone::subMoveGripPointsAt(
	const AcDbIntArray& indices,
	const AcGeVector3d& offset)
#else
Acad::ErrorStatus PDEcone::moveGripPointsAt(
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
Acad::ErrorStatus PDEcone::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDEcone::explode(AcDbVoidPtrArray& entitySet) const
#endif
{ //AutoCAD2002版采用面方式，AutoCAD 2004采用体方式, AutoCAD R14版有问题。
	assertReadEnabled();

#ifdef _OBJECTARX2004_
	Acad::ErrorStatus es = Acad::eOk;
	int i = 0;
	AcGeVector3d faceVect = getFaceVect();

	AcDbCurve *pCurve = NULL;
	AcDbVoidPtrArray curveSegments;
	double pathLen = m_ptStart.distanceTo(m_ptEnd);

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

	//extrudeAlongPath函数在由较大region拉成较小region时容易产生自交问题，所以
	//炸开时选择由小的region拉成大的region
	AcDbLine *pLine = new AcDbLine(ep, sp);
    pLine->setPropertiesFrom(this);
	pCurve = pLine;	

	if(pCurve != NULL)
	{
		if((es = addToDb(pCurve)) == Acad::eOk)
		{
			AcDbObjectId id;
			id = pCurve->objectId();
			pCurve->close();
			if((es = acdbOpenObject((AcDbCurve*&)pCurve, id, AcDb::kForRead)) != Acad::eOk)
				return explodeToSurface(entitySet);
		}
		else
		{
			if(pCurve != NULL)
				delete pCurve;
			return explodeToSurface(entitySet);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	AcDbCircle sCir(ep, faceVect, d2 / 2.0);
	curveSegments.append(&sCir);

	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;

		////求taper Angle
		//AcGeVector3d vec1 = (m_ptStart-m_ptEnd);//.normal();

		////m_dDiameter1和m_dDiameter2的大小问题
		//AcGeVector3d perpVec = faceVect.perpVector()*((m_dDiameter1/2-m_dDiameter2/2));
		//AcGePoint3d point2(m_ptStart.x+perpVec.x, m_ptStart.y+perpVec.y, m_ptStart.z+perpVec.z);
		//AcGeVector3d vec2 = (point2 - m_ptEnd);//.normal();

		////taperAngle为负时表示放大region，反之为缩小
		//double angle = -1*vec1.angleTo(vec2);
		////angle没有到达最大值时仍然有问题，建议不使用extrudeAlongPath函数

		//taperAngle是指两个圆心连线垂直于圆面时的角度，大于0时表示缩小，小于0时表示放大
		es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,//angle);
		-1*atan((d1 / 2.0 - d2 / 2.0) / pathLen));

		if(es != Acad::eOk)
		{
			if(pCurve != NULL)
			{
				pCurve->upgradeOpen();
				pCurve->erase();
				pCurve->close();
			}
			delete pBody;
			for(i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return explodeToSurface(entitySet);
		}
		pBody->setPropertiesFrom(this);
		entitySet.append(pBody);
	}
	else
	{
		if(pCurve != NULL)
		{
			pCurve->upgradeOpen();
			pCurve->erase();
			pCurve->close();
		}
		for(i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		return explodeToSurface(entitySet);
	}

	if(pCurve != NULL)
	{
		pCurve->upgradeOpen();
		pCurve->erase();
		pCurve->close();
	}
	for(i = 0; i < regions.length(); i++)
		delete (AcRxObject*)regions[i];

	return Acad::eOk;
#else
	return explodeToSurface(entitySet);
#endif
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDEcone::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

#ifdef _OBJECTARX2004_
	Acad::ErrorStatus es = Acad::eOk;
	int i = 0;
	AcGeVector3d faceVect = getFaceVect();

	AcDbCurve *pCurve = NULL;
	AcDbVoidPtrArray curveSegments;
	double pathLen = m_ptStart.distanceTo(m_ptEnd);

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

	//extrudeAlongPath函数在由较大region拉成较小region时容易产生自交问题，所以
	//炸开时选择由小的region拉成大的region
	AcDbLine *pLine = new AcDbLine(ep, sp);
    pLine->setPropertiesFrom(this);
	pCurve = pLine;

	if(pCurve != NULL)
	{
		if((es = addToDb(pCurve)) == Acad::eOk)
		{
			AcDbObjectId id;
			id = pCurve->objectId();
			pCurve->close();
			if((es = acdbOpenObject((AcDbCurve*&)pCurve, id, AcDb::kForRead)) != Acad::eOk)
				return Acad::eNotApplicable;
		}
		else
		{
			if(pCurve != NULL)
				delete pCurve;
			return Acad::eNotApplicable;
		}
	}

	AcDbCircle sCir(ep, faceVect, d2 / 2.0);
	curveSegments.append(&sCir);

	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;

		////求taper Angle
		//AcGeVector3d vec1 = (m_ptStart-m_ptEnd);//.normal();

		////m_dDiameter1和m_dDiameter2的大小问题
		//AcGeVector3d perpVec = faceVect.perpVector()*((m_dDiameter1/2-m_dDiameter2/2));
		//AcGePoint3d point2(m_ptStart.x+perpVec.x, m_ptStart.y+perpVec.y, m_ptStart.z+perpVec.z);
		//AcGeVector3d vec2 = (point2 - m_ptEnd);//.normal();

		////taperAngle为负时表示放大region，反之为缩小
		//double angle = -1*vec1.angleTo(vec2);
		////angle没有到达最大值时仍然有问题，建议不使用extrudeAlongPath函数

		//taperAngle是指两个圆心连线垂直于圆面时的角度，大于0时表示缩小，小于0时表示放大
		es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,//angle);
			-1*atan((d1 / 2.0 - d2 / 2.0) / pathLen));

		if(es != Acad::eOk)
		{
			if(pCurve != NULL)
			{
				pCurve->upgradeOpen();
				pCurve->erase();
				pCurve->close();
			}
			delete pBody;
			for(i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return Acad::eNotApplicable;
		}
		pBody->setPropertiesFrom(this);
		p3dSolid = pBody;
	}
	else
	{
		if(pCurve != NULL)
		{
			pCurve->upgradeOpen();
			pCurve->erase();
			pCurve->close();
		}
		for(i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		return Acad::eNotApplicable;
	}

	if(pCurve != NULL)
	{
		pCurve->upgradeOpen();
		pCurve->erase();
		pCurve->close();
	}
	for(i = 0; i < regions.length(); i++)
		delete (AcRxObject*)regions[i];

	return Acad::eOk;
#else
	return Acad::eNotApplicable;
#endif
}

Acad::ErrorStatus PDEcone::explodeToSurface(AcDbVoidPtrArray& entitySet) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = Acad::eOk;
    int i;

    AcGePoint3dArray pArray;
    AcGeIntArray stdIdx;
    int actPrecision;
    getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
    int actPrecision__1 = actPrecision + 1;
    int actPrecision___2__1 = actPrecision * 2 + 1;

    AcGeVector3d faceVect = getFaceVect();

    AcDbVoidPtrArray curSeg, resRegion;
    AcDbCircle *pCir = NULL;
    AcGePoint3d  ucs_org = acdbCurDwg()->ucsorg();
    AcGeVector3d ucs_x = acdbCurDwg()->ucsxdir();
    AcGeVector3d ucs_y = acdbCurDwg()->ucsydir();
    AcGeVector3d ucs_z = ucs_x.crossProduct(ucs_y);
    AcGeMatrix3d ucs_mat;

    ucs_mat.setToWorldToPlane(-faceVect);
//#ifdef _OBJECTARX14_
//    acdbSetCurrentUCS(ucs_mat);
//#else
//    acedSetCurrentUCS(ucs_mat);
//#endif
    pCir = new AcDbCircle(m_ptStart, -faceVect, m_dDiameter1 / 2.0);
    curSeg.append(pCir);
    es = AcDbRegion::createFromCurves(curSeg, resRegion);
    if(es != Acad::eOk)
    {
        for(i = 0; i < resRegion.length(); ++i)
            delete (AcRxObject*)resRegion[i];
    }
    else
    {
        for(i = 0; i < resRegion.length(); ++i)
            ((AcDbEntity *)resRegion[i])->setPropertiesFrom(this);
        entitySet.append(resRegion);
    }
    for(i = 0; i < curSeg.length(); ++i)
        delete (AcRxObject*)curSeg[i];
    
    curSeg.setLogicalLength(0);
    resRegion.setLogicalLength(0);

    ucs_mat.setToWorldToPlane(faceVect);
//#ifdef _OBJECTARX14_
//    acdbSetCurrentUCS(ucs_mat);
//#else
//    acedSetCurrentUCS(ucs_mat);
//#endif
    pCir = new AcDbCircle(m_ptEnd, faceVect, m_dDiameter2 / 2.0);
    curSeg.append(pCir);
    es = AcDbRegion::createFromCurves(curSeg, resRegion);
    if(es != Acad::eOk)
    {
        for(i = 0; i < resRegion.length(); ++i)
            delete (AcRxObject*)resRegion[i];
    }
    else
    {
        for(i = 0; i < resRegion.length(); ++i)
            ((AcDbEntity *)resRegion[i])->setPropertiesFrom(this);
        entitySet.append(resRegion);
    }
    for(i = 0; i < curSeg.length(); ++i)
        delete (AcRxObject*)curSeg[i];

    ucs_mat.setCoordSystem(ucs_org, ucs_x, ucs_y, ucs_z);
//#ifdef _OBJECTARX14_
//    acdbSetCurrentUCS(ucs_mat);
//#else
//    acedSetCurrentUCS(ucs_mat);
//#endif

    AcDbPolygonMesh *pMesh;
    pMesh = new AcDbPolygonMesh(AcDb::kSimpleMesh, 2, actPrecision + 1,
                                pArray, Adesk::kFalse, Adesk::kFalse);
    pMesh->setPropertiesFrom(this);
    entitySet.append(pMesh);

    return Acad::eOk;
}

void PDEcone::createBody()
{
    assertWriteEnabled();
//#ifdef _OBJECTARX2004_
    AcGeVector3d faceVect = getFaceVect();
    m_3dGeom = Body::cone(Line3d(*(Point3d*)&m_ptStart, *(Point3d*)&m_ptEnd), *(Vector3d*)&faceVect, m_dDiameter1 / 2.0, m_dDiameter2 / 2.0, m_dDividPrecision);
//#endif
}

// 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
// 精度不大于实体限定的剖分精度m_dDividPrecision。
int PDEcone::getMaxPrecision(AcGiWorldDraw *mode) const
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
Acad::ErrorStatus PDEcone::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDEcone::getGeomExtents(AcDbExtents& extents) const
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
void PDEcone::subList() const
#else
void PDEcone::list() const
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
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4747/*" 顶面直径=%.3f\n"*/,m_dDiameter2);	
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4748/*" 偏心方向(%f,%f,%f)\n\n"*/,m_vect.x,m_vect.y,m_vect.z);
	return ;
}// added by linlin 20050810



/*---------------------------------------------------------------------------
* 名称: _make_eqovalpt
* 功能: 完成 "在设备椭球封头上画出点" 的功能
* 注意: 无
*/
int  PDEcone:: _make_eqovalpt(double lengthR,AcGeVector3d vec,ads_point pt,ads_point normal) 
{
	double R1,R2;
	R1=m_dDiameter1/2;
	R2=m_dDiameter2/2;
	double ang,H1;
	AcGeVector3d vect1 = (m_ptEnd - m_ptStart).normal();
    AcGeVector3d vect = getFaceVect();
	H1=m_ptStart.distanceTo(m_ptEnd);
	ang=vect.angleTo(vect1);

    if(lengthR>=0)
    if((lengthR <=R1 &&lengthR<= R2)||(lengthR>=R2 && lengthR>=R1))
		return RTERROR;
	if(lengthR<0)
	   if(-lengthR > H1*cos(ang))
			return RTERROR;
		
	AcGePoint3d  point,center1,point1;
    AcGeVector3d L,Lnormal,generatrix;
	double z;

	if(lengthR>=0)
	{
		z=H1*(R1-lengthR)/(R1-R2);	
		center1=m_ptStart+vect1*z;
    	point=center1+vec*lengthR;
	}
	else
	{   
		if (R1>=R2)
		{
			z=-lengthR/cos(ang) ;
			center1=m_ptStart+vect1*z;
			point=center1+vec*(R1-z*(R1-R2)/H1);
		}
		else
		{
			vect=-vect;
			vect1=-vect1;
			ang=vect.angleTo(vect1);
			z=-lengthR/cos(ang) ;
			center1=m_ptEnd+vect1*z;
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
	
}//added by linlin 20051012