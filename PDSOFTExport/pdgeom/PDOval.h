// PDOval.h: interface for the PDOval class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDOVAL_H__2A74E268_7791_41B8_83C0_1E5E27B3555D__INCLUDED_)
#define AFX_PDOVAL_H__2A74E268_7791_41B8_83C0_1E5E27B3555D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDOval : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDOval);

	PDOval(bool HasSnap = false);
	PDOval(double lengthA, double lengthB, double lengthR, 
           const AcGePoint3d pt, const AcGeVector3d vect, 
           Adesk::UInt32 precisionA = PRIMARY3D_DEFAULT_PRECISION, 
           Adesk::UInt32 precisionB = PRIMARY3D_DEFAULT_PRECISION, 
           bool HasSnap = false);
	virtual ~PDOval();

private:
	double m_dlengthA, m_dlengthB, m_dlengthR;
	AcGePoint3d m_ptCenter;
	AcGeVector3d m_vect;
	Adesk::UInt32 m_nprecisionA;  //经线剖分精度
    Adesk::UInt32 m_nprecisionB;  //纬线剖分精度

public:
	/*---------------------------------------------------------------------------
     * 名称: _make_eqovalpt
     * 功能: 完成 "在设备椭球封头上画出点" 的功能
     * 注意: 无
    */
    int   _make_eqovalpt(double lengthR,AcGeVector3d vec,ads_point pt,ads_point normal) ;
    Acad::ErrorStatus getVertices(int precisionA, int precisionB, 
                                  AcGePoint3dArray &vertexArray, 
                                  AcGeIntArray &stdBIdx, 
                                  int &actPrecisionA, int &actPrecisionB) const;
	Acad::ErrorStatus getVertices(int precisionA, int precisionB, 
		AcGePoint3dArray &vertexArray) const;

	/*!
	 * 获取m_vect与Ax所围成的四分之一椭圆上的点
	 * \param [in] precisionA 
	 * \param [in] xAxis 
	 * \param [in/out] vertexArray 
	 * \precondition xAxis 与 m_vect 正交
	 * \return
	 */
	Acad::ErrorStatus getVertices(const int& precisionA, const AcGeVector3d& xAxis, 
								  AcGePoint3dArray & vertexArray) const;
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
	virtual Acad::ErrorStatus subMoveGripPointsAt(const AcDbIntArray& indices,
		const AcGeVector3d& offset);
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
	virtual Acad::ErrorStatus moveGripPointsAt(const AcDbIntArray& indices,
		const AcGeVector3d& offset);
#endif

	virtual Acad::ErrorStatus	dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus	dwgOutFields(AcDbDwgFiler* filer) const;
	virtual Acad::ErrorStatus	dxfInFields(AcDbDxfFiler* filer);
	virtual Acad::ErrorStatus	dxfOutFields(AcDbDxfFiler* filer) const;

	////// adding function///
	Acad::ErrorStatus getgrippoints(AcGePoint3dArray& gripArray) const;

    Acad::ErrorStatus setParameters(double lengthA, double lengthB, 
        double lengthR, const AcGePoint3d pt, const AcGeVector3d vect, 
        Adesk::UInt32 precisionA = PRIMARY3D_DEFAULT_PRECISION, 
        Adesk::UInt32 precisionB = PRIMARY3D_DEFAULT_PRECISION);

	AcGePoint3d getpointCenter() const;
	double getlengthA() const;
	double getlengthB() const;
	double getlengthR() const;
	Adesk::UInt32 getprecisionA() const;
	Adesk::UInt32 getprecisionB() const;
	AcGeVector3d getVect() const;
    // 取得椭球封头的高度
    double getOvalHeight() const;
    // 取得椭圆的圆心
    AcGePoint3d getEllipseCen() const;

    // 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
    // 精度不大于实体限定的剖分精度m_dDividPrecision。
    virtual int getMaxPrecision(AcGiWorldDraw *mode) const;

    // 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
    // 精度不大于实体限定的剖分精度m_nprecisionA、m_nprecisionB。
    int getMaxPrecisionA(AcGiWorldDraw *mode) const;
    int getMaxPrecisionB(AcGiWorldDraw *mode) const;

    virtual void createBody();

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
  virtual bool isValidData(double &f); //检查是否成员数据均是合法数据, 并返回出错数

};
MAKE_ACDBOPENOBJECT_FUNCTION(PDOval);

#endif // !defined(AFX_PDOVAL_H__2A74E268_7791_41B8_83C0_1E5E27B3555D__INCLUDED_)
