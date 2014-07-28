// PDBox1.h: interface for the PDBox1 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDBOX1_H__0FF0BE75_8A25_48CA_9636_1DAB4CC5E12B__INCLUDED_)
#define AFX_PDBOX1_H__0FF0BE75_8A25_48CA_9636_1DAB4CC5E12B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDBox1 : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDBox1);
	PDBox1(bool HasSnap = false);
	PDBox1(const AcGePoint3d &ptP, const AcGeVector3d &vectL, 
           const AcGeVector3d &vectW, double l,double w,double h, 
           bool HasSnap = false);
	virtual ~PDBox1();
private:
	double m_dLength,m_dWidth,m_dHeight;
	AcGePoint3d m_ptP;
	AcGeVector3d m_VectLength,m_VectWidth;
public:
	Acad::ErrorStatus getVertices(AcGePoint3dArray& vertexArray) const;

#ifdef _OBJECTARX2010_
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d &xform);
	virtual void subList() const;
	virtual Acad::ErrorStatus   subGetGeomExtents(AcDbExtents& extents) const;
	virtual Acad::ErrorStatus subExplode(AcDbVoidPtrArray& entitySet) const;
	virtual Acad::ErrorStatus   subGetOsnapPoints(
		AcDb::OsnapMode     osnapMode,
		Adesk::GsMarker     gsSelectionMark,
		const AcGePoint3d&  pickPoint,
		const AcGePoint3d&  lastPoint,
		const AcGeMatrix3d& viewXform,
		AcGePoint3dArray&   snapPoints,
		AcDbIntArray &   geomIds) const;
	virtual Acad::ErrorStatus subGetGripPoints(
		AcGePoint3dArray& gripPoints,
		AcDbIntArray& osnapModes,
		AcDbIntArray& geomIds) const;
#else
	virtual Adesk::Boolean worldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus transformBy(const AcGeMatrix3d &xform);
	virtual void list() const;
	virtual Acad::ErrorStatus   getGeomExtents(AcDbExtents& extents) const;
	virtual Acad::ErrorStatus explode(AcDbVoidPtrArray& entitySet) const;
	virtual Acad::ErrorStatus getOsnapPoints(  AcDb::OsnapMode osnapMode,
		int gsSelectionMark,
		const AcGePoint3d&    pickPoint,
		const AcGePoint3d&    lastPoint,
		const AcGeMatrix3d&   viewXform,
		AcGePoint3dArray&     snapPoints,
		AcDbIntArray&         geomIds) const;
	virtual Acad::ErrorStatus getGripPoints(
		AcGePoint3dArray& gripPoints,
		AcDbIntArray& osnapModes,
		AcDbIntArray& geomIds) const;
#endif

	virtual Acad::ErrorStatus	dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus	dwgOutFields(AcDbDwgFiler* filer) const;
	virtual Acad::ErrorStatus	dxfInFields(AcDbDxfFiler* filer);
	virtual Acad::ErrorStatus	dxfOutFields(AcDbDxfFiler* filer) const;

    virtual void createBody();

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

public:
    Acad::ErrorStatus setParameters(const AcGePoint3d &ptP, const AcGeVector3d &vectL, 
        const AcGeVector3d &vectW, double l,double w,double h);

	AcGeVector3d getVectLength() const;
	AcGeVector3d getVectWidth() const;
	AcGePoint3d getpointP() const;
	double getLength() const;
	double getWidth() const;
	double getHeight() const;

#ifndef _OBJECTARX2010_
private:
	void *operator new[](unsigned nSize) { return 0; }
	void operator delete[](void *p) {};
	void *operator new[](unsigned nSize, const char *file, int line) { return 0; }
#endif

public:
  //zxb,20090113,增加audit，暂时只check自身成员数据
  virtual Acad::ErrorStatus audit(AcDbAuditInfo* pAuditInfo);
  virtual void setDefault(); //设置缺省的对象数据
  virtual bool isValidData(double &f); //检查是否成员数据均是合法数据, 并返回出错数

};
MAKE_ACDBOPENOBJECT_FUNCTION(PDBox1);

#endif // !defined(AFX_PDBOX1_H__0FF0BE75_8A25_48CA_9636_1DAB4CC5E12B__INCLUDED_)
