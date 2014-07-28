// PDEcone.h: interface for the PDEcone class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDECONE_H__3E5242A3_B1C0_4F47_A689_286BE75A2B1C__INCLUDED_)
#define AFX_PDECONE_H__3E5242A3_B1C0_4F47_A689_286BE75A2B1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDEcone : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDEcone);
	PDEcone(bool HasSnap = false);
	PDEcone(const AcGePoint3d &ptStart, const AcGePoint3d &ptEnd, 
            double Diameter1, double Diameter2, const AcGeVector3d &vect, 
            int Precision = PRIMARY3D_DEFAULT_PRECISION, 
            bool HasSnap = false);
	virtual ~PDEcone();

public:
	/*---------------------------------------------------------------------------
* 名称: _make_eqovalpt
* 功能: 完成 "在设备椭球封头上画出点" 的功能
* 注意: 无
*/
    int   _make_eqovalpt(double lengthR,AcGeVector3d vec,ads_point pt,ads_point normal) ;
    Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray, 
                                  AcGeIntArray &stdIdx, int &actPrecision) const;
	Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray) const;
#ifdef _OBJECTARX2010_
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d &xform);
	virtual void subList() const;
	virtual Acad::ErrorStatus subGetGeomExtents(AcDbExtents& extents) const;
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

	Acad::ErrorStatus setParameters(const AcGePoint3d &ptStart, 
                                    const AcGePoint3d &ptEnd, 
                                    double d1, 
                                    double d2, 
                                    const AcGeVector3d &vect, 
                                    int Precision = PRIMARY3D_DEFAULT_PRECISION);

	AcGePoint3d getpointStart() const;
	AcGePoint3d getpointEnd() const;
	double getDiameter1() const;
	double getDiameter2() const;
	AcGeVector3d getVect() const;

    /*!
     * 取得底面的法矢
     *
     * @param none
     *
     * @return AcGeVector3d  : 底面的法矢
     */
    AcGeVector3d getFaceVect() const;

	/*!
	 * 获取m_ptStart在m_ptEnd所在底面上的投影
	 * \return AcGePoint3d
	 */
	AcGePoint3d getStartPtOrthoInEntPtPlane()const;
	/*!
	 * 获取m_ptEnd在m_ptStart所在底面上的投影
	 * \return AcGePoint3d
	 */
	AcGePoint3d getEndPtOrthoInStartPtPlane()const;

	/*!
	 * 获取圆台高
	 * \return double
	 */
	double getHeight()const;


    // 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
    // 精度不大于实体限定的剖分精度m_dDividPrecision。
    virtual int getMaxPrecision(AcGiWorldDraw *mode) const;

    virtual void createBody();

    Acad::ErrorStatus explodeToSurface(AcDbVoidPtrArray& entitySet) const;

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

private:
    /*!
    * 根据已经输入到类中的参数，修正终点和偏心方向，使其值正确。
    *
    * @param none
    *
    * @return Acad::ErrorStatus  : 永远为Acad::eOk
    */
    Acad::ErrorStatus CalActParameter();

	AcGePoint3d m_ptEnd;
	AcGePoint3d m_ptStart;
	double m_dDiameter1;
	double m_dDiameter2;
	AcGeVector3d m_vect;

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
MAKE_ACDBOPENOBJECT_FUNCTION(PDEcone);

#endif // !defined(AFX_PDECONE_H__3E5242A3_B1C0_4F47_A689_286BE75A2B1C__INCLUDED_)
