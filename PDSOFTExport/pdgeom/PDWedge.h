// PDWedge.h: interface for the PDWedge class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDWEDGE_H__3CBB93DB_E47B_43D1_A40E_F541EA0D71B7__INCLUDED_)
#define AFX_PDWEDGE_H__3CBB93DB_E47B_43D1_A40E_F541EA0D71B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDWedge : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDWedge);
	PDWedge(bool HasSnap = false);
	PDWedge(AcGePoint3d P1,AcGePoint3d P2,AcGePoint3d P3,double height,
		bool HasSnap = false, int CorrType = 1/* 0: ｚ左手系，1: 右手系*/);
	virtual ~PDWedge();

private:
	AcGePoint3d m_ptP1, m_ptP2, m_ptP3, m_ptP4;
	double m_dMaxLength;
//               O P1
//              /|\
//             / | \
//            OP2O--O P3
//            |\/  /
//            |/\ /
//         P4 O--O

public:
	virtual void createBody();
    Acad::ErrorStatus getVertices(AcGePoint3dArray& vertexArray) const;

	virtual Acad::ErrorStatus	dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus	dwgOutFields(AcDbDwgFiler* filer) const;
	virtual Acad::ErrorStatus	dxfInFields(AcDbDxfFiler* filer);
	virtual Acad::ErrorStatus	dxfOutFields(AcDbDxfFiler* filer) const;
#ifdef _OBJECTARX2010_
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d &xform);
	virtual void subList() const;
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
	virtual Acad::ErrorStatus   subGetGeomExtents(AcDbExtents& extents) const;
#else
	virtual Adesk::Boolean worldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus transformBy(const AcGeMatrix3d &xform);
	virtual void list() const;
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
	virtual Acad::ErrorStatus   getGeomExtents(AcDbExtents& extents) const;
#endif

	AcGePoint3d getpointP1();
	AcGePoint3d getpointP2();
	AcGePoint3d getpointP3();
	AcGePoint3d getpointP4();

    Acad::ErrorStatus setParameters(AcGePoint3d P1,AcGePoint3d P2,AcGePoint3d P3,
		double height,int CorrType = 1/* 0: ｚ左手系，1: 右手系*/);
	double GetMaxLength();

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

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
  virtual bool isValidData(double &f); //检查是否成员数据均是合法数据,并返回出错误的数
};
MAKE_ACDBOPENOBJECT_FUNCTION(PDWedge);

#endif // !defined(AFX_PDWEDGE_H__3CBB93DB_E47B_43D1_A40E_F541EA0D71B7__INCLUDED_)
