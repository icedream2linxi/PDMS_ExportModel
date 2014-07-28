// PDCylinder.h: interface for the PDCylinder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDCYLINDER_H__6C4AF5BF_06DD_48E3_B6C4_06E5A368FD57__INCLUDED_)
#define AFX_PDCYLINDER_H__6C4AF5BF_06DD_48E3_B6C4_06E5A368FD57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDCylinder : public PDPrimary3D  
{
public:

	ACRX_DECLARE_MEMBERS(PDCylinder);

	PDCylinder(bool HasSnap = false);
	PDCylinder(const AcGePoint3d &ptStart, const AcGePoint3d &ptEnd, 
               double Diameter, int Precision = PRIMARY3D_DEFAULT_PRECISION, 
               bool HasSnap = false);
	virtual ~PDCylinder();

	//{{AFX_ARX_METHODS(PDCylinder)
#ifdef _OBJECTARX2010_
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d& xform);
	virtual void              subList() const;
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
	virtual Acad::ErrorStatus subGetGripPoints( AcGePoint3dArray& gripPoints,
		AcDbIntArray& osnapModes,
		AcDbIntArray& geomIds) const;
	virtual Acad::ErrorStatus subMoveGripPointsAt(const AcDbIntArray& indices,
		const AcGeVector3d& offset);
#else
	virtual Adesk::Boolean worldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus transformBy(const AcGeMatrix3d& xform);
	virtual void              list() const;
	virtual Acad::ErrorStatus   getGeomExtents(AcDbExtents& extents) const;
	virtual Acad::ErrorStatus explode(AcDbVoidPtrArray& entitySet) const;
	virtual Acad::ErrorStatus getOsnapPoints(  AcDb::OsnapMode osnapMode,
		int gsSelectionMark,
		const AcGePoint3d&    pickPoint,
		const AcGePoint3d&    lastPoint,
		const AcGeMatrix3d&   viewXform,
		AcGePoint3dArray&     snapPoints,
		AcDbIntArray&         geomIds) const;
	virtual Acad::ErrorStatus getGripPoints( AcGePoint3dArray& gripPoints,
		AcDbIntArray& osnapModes,
		AcDbIntArray& geomIds) const;
	virtual Acad::ErrorStatus moveGripPointsAt(const AcDbIntArray& indices,
		const AcGeVector3d& offset);
#endif
	//virtual void viewportDraw(AcGiViewportDraw* mode);
	virtual Acad::ErrorStatus dxfOutFields(AcDbDxfFiler* pFiler) const;
	virtual Acad::ErrorStatus dxfInFields(AcDbDxfFiler* pFiler);
	virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler* pFiler) const;
	virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler* pFiler);
    //}}AFX_ARX_METHODS

	//{{AFX_ARX_DATA_ACCESS(PDCylinder)
	Acad::ErrorStatus setParameters(const AcGePoint3d &ptStart, 
                                    const AcGePoint3d &ptEnd, 
                                    double Diameter, 
                                    int Precision = PRIMARY3D_DEFAULT_PRECISION);

	double getDiameter() const;
	AcGePoint3d getPtEnd() const;
	AcGePoint3d getPtStart() const;
	//}}AFX_ARX_DATA_ACCESS
	////// adding function///
	Acad::ErrorStatus getgrippoints(AcGePoint3dArray& gripArray) const;

    // 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
    // 精度不大于实体限定的剖分精度m_dDividPrecision。
    virtual int getMaxPrecision(AcGiWorldDraw *mode) const;

    virtual void createBody();

    //void calPts();
// Added by SongNan 2007/7/27 : begin
	Adesk::Boolean PDCylinder::intersectWithLineSeg(const AcGeLineSeg3d& line,
											AcGePoint3dArray&   points)const;

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

// Added by SongNan 2007/7/27 : end
protected:
//	Adesk::Int32 m_nPrecision;//精度
	//{{AFX_ARX_DATA(PDCylinder)
	double m_dDiameter;   //直径
	AcGePoint3d m_ptEnd;  //终点
	AcGePoint3d m_ptStart; //起点  added by linlin 20050810
	//}}AFX_ARX_DATA

public:
  //zxb,20090113,增加audit，暂时只check自身成员数据
  virtual Acad::ErrorStatus audit(AcDbAuditInfo* pAuditInfo);
  virtual void setDefault(); //设置缺省的对象数据
  virtual bool isValidData(double &f); //检查是否成员数据均是合法数据, 并返回出错数


private:

#ifndef _OBJECTARX2010_
	void *operator new[](unsigned nSize) { return 0; }
	void operator delete[](void *p) {};
	void *operator new[](unsigned nSize, const TCHAR *file, int line) { return 0; }
#endif

    virtual Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray, 
                                          AcGeIntArray &stdIdx, int &actPrecision) const;
    virtual Acad::ErrorStatus getVertices(int Precision, 
                                          AcGePoint3dArray &vertexArray, 
                                          AcGeVector3dArray &vertexNors) const;
};
MAKE_ACDBOPENOBJECT_FUNCTION(PDCylinder);

#endif // !defined(AFX_PDCYLINDER_H__6C4AF5BF_06DD_48E3_B6C4_06E5A368FD57__INCLUDED_)
