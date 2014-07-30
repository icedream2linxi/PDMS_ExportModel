// PDTorus.h: interface for the PDTorus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDTORUS_H__3434182C_8E65_4EC1_8708_4FB90733A9B0__INCLUDED_)
#define AFX_PDTORUS_H__3434182C_8E65_4EC1_8708_4FB90733A9B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDTorus : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDTorus);

	PDTorus(bool HasSnap = false);
	PDTorus(const AcGePoint3d &center, const AcGePoint3d &p1, 
            const AcGePoint3d &p2, double dia1, double dia2, int d,
			int prearc = PRIMARY3D_DEFAULT_PRECISION, 
            int precir = PRIMARY3D_DEFAULT_PRECISION, 
            bool HasSnap = false,int CorrType = 1 /*0-左手系，1-右手系*/);
	virtual ~PDTorus();

private:
	virtual void createBody();
	double m_dDiameter1,m_dDiameter2;
	Adesk::UInt32 m_nDirection;
	AcGePoint3d m_ptCenter,m_ptP1,m_ptP2;
	Adesk::UInt32 m_nprecisionArc,m_nprecisionCir;
  AcGeVector3d m_v3; //另一向量用于判断左右手系

public:
    Acad::ErrorStatus getVertices(int precisionArc, int precisionCir, 
                                  AcGePoint3dArray& vertexArray, 
                                  AcGeIntArray &stdArcIdx, AcGeIntArray &stdCirIdx, 
                                  int &actPrecisionArc, int &actPrecisionCir) const;
	Acad::ErrorStatus getVertices(int precisionArc, int precisionCir, AcGePoint3dArray& vertexArray) const;

#ifdef _OBJECTARX2010_
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d &xform);
	virtual void subList() const;// added by linlin 20080812
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
	virtual void list() const;// added by linlin 20080812
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


    // 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
    // 精度不大于实体限定的剖分精度m_dDividPrecision。
    virtual int getMaxPrecision(AcGiWorldDraw *mode) const;

    // 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
    // 精度不大于实体限定的剖分精度m_nprecisionArc、m_nprecisionCir。
    int getMaxPrecisionArc(AcGiWorldDraw *mode) const;
    int getMaxPrecisionCir(AcGiWorldDraw *mode) const;
	AcGePoint3dArray getPtOnCentAix(int Precision);
	const AcGePoint3d &getCenter() const;
	const AcGePoint3d &getP1() const;
	const AcGePoint3d &getP2() const;
	double getDiameter1() const;
	double getDiameter2() const;
	Adesk::UInt32 getDirection() const;
	Adesk::UInt32 getprecisionArc() const;
	Adesk::UInt32 getprecisionCir() const;

	Acad::ErrorStatus setParameters(const AcGePoint3d &center, 
                                    const AcGePoint3d &p1, 
                                    const AcGePoint3d &p2, 
                                    double dia1, double dia2, int d,
                                    int prearc = PRIMARY3D_DEFAULT_PRECISION, 
                                    int precir = PRIMARY3D_DEFAULT_PRECISION,
                                    int CorrType = 1 /*0-左手系，1-右手系*/);

    // 取得圆环起点到圆环中心的距离
    double getStartCenDist() const;
    // 取得圆环终点到圆环中心的距离
    double getEndCenDist() const;
    // 取得圆环中心到圆环起点的矢量
    AcGeVector3d getStartCenVect() const;
    // 取得圆环中心到圆环终点的矢量
    AcGeVector3d getEndCenVect() const;
    // 取得圆环的法矢
    AcGeVector3d getNormal() const;
    // 取得圆环的扫过的角度
    double getAngle() const;

	/*!
	 * 取得终点坐标
	 * \return AcGePoint3d
	 */
	AcGePoint3d getEndPoint() const;

	/*!
	 * 获取起点即m_ptP1端面法向
	 * \return  AcGeVector3d
	 */
	AcGeVector3d getStartPlaneNormal() const;

	/*!
	 * 获取终点即m_ptP2端面法向
	 * \return  AcGeVector3d
	 */
	AcGeVector3d getEndPlaneNormal() const;

    Acad::ErrorStatus explodeToSurface(AcDbVoidPtrArray& entitySet) const;

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
MAKE_ACDBOPENOBJECT_FUNCTION(PDTorus);

#endif // !defined(AFX_PDTORUS_H__3434182C_8E65_4EC1_8708_4FB90733A9B0__INCLUDED_)
