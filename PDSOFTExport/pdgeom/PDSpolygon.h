// PDSpolygon.h: interface for the PDSpolygon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(PDSPOLYGON_H)
#define PDSPOLYGON_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDSpolygon : public PDPrimary3D
{
public:
	ACRX_DECLARE_MEMBERS(PDSpolygon); 

  PDSpolygon();

  // Vh、OutPtArr、InPtArrArr为局部坐标系的坐标
	PDSpolygon(const AcGePoint3d& oriPt, const AcGeVector3d& V1, const AcGeVector3d& V2, 
             const AcGeVector3d& Vh, double H, const AcGePoint3dArray& OutPtArr, 
             const std::vector<AcGePoint3dArray>& InPtArrArr, bool HasSnap = false, int CorrType = 1/* 0: 左手系，1: 右手系*/);

  // Vh、PtArr为局部坐标系的坐标
	PDSpolygon(int OutPtNum, const std::vector<int>& InPtNumArr, const AcGePoint3d& oriPt, 
             const AcGeVector3d& V1, const AcGeVector3d& V2, const AcGeVector3d& Vh, 
             double H, const AcGePoint3dArray& PtArr, bool HasSnap = false, int CorrType = 1/* 0: 左手系，1: 右手系*/);

	// Vh、OutPtArr、InPtArrArr为局部坐标系的坐标
	PDSpolygon(const AcGePoint3d& oriPt, const AcGeVector3d& V1, const AcGeVector3d& V2, 
			   const AcGeVector3d& Vh, double H, 
			   const std::vector<t_PolylineVertex> &OutPtArr,
			   const std::vector< std::vector<t_PolylineVertex> > &InPtArrArr, 
			   int Precision = PRIMARY3D_DEFAULT_PRECISION, bool HasSnap = false, int CorrType = 1/* 0: 左手系，1: 右手系*/);

	virtual ~PDSpolygon();
public:
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
#else
	virtual Adesk::Boolean worldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus transformBy(const AcGeMatrix3d &xform);
	virtual void list() const;
	virtual Acad::ErrorStatus   getGeomExtents(AcDbExtents& extents) const;
	virtual Acad::ErrorStatus explode(AcDbVoidPtrArray& entitySet) const;
	virtual Acad::ErrorStatus getOsnapPoints(AcDb::OsnapMode osnapMode,
		int gsSelectionMark,
		const AcGePoint3d&    pickPoint,
		const AcGePoint3d&    lastPoint,
		const AcGeMatrix3d&   viewXform,
		AcGePoint3dArray&     snapPoints,
		AcDbIntArray&         geomIds) const;
#endif

	virtual Acad::ErrorStatus	dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus	dwgOutFields(AcDbDwgFiler* filer) const;
	virtual Acad::ErrorStatus	dxfInFields(AcDbDxfFiler* filer);
	virtual Acad::ErrorStatus	dxfOutFields(AcDbDxfFiler* filer) const;


  // 空间任意平面坐标系原点
	const AcGePoint3d &getOri() const;
  // X轴方向
  const AcGeVector3d &getV1() const;
  // Y轴方向
  const AcGeVector3d &getV2() const;
  // 拉伸方向(世界坐标系坐标)
  const AcGeVector3d &getVh() const;
  // 拉伸高度
  double                    getH() const;
  Acad::ErrorStatus         setH(double val);
  // 外环的顶点（世界坐标系的坐标）
  const std::vector<t_PolylineVertex> &getOut() const;
  // 内环的顶点（世界坐标系的坐标）
	const std::vector< std::vector<t_PolylineVertex> > &getIn() const;

  // Vh、OutPtArr、InPtArrArr为局部坐标系的坐标
  Acad::ErrorStatus         setVal(const AcGePoint3d& oriPt, const AcGeVector3d& V1, const AcGeVector3d& V2, 
                                   const AcGeVector3d& Vh, double H, const AcGePoint3dArray& OutPtArr, 
                                   const std::vector<AcGePoint3dArray>& InPtArrArr, int CorrType = 1/* 0: ｚ左手系，1: 右手系*/);
  // Vh、OutPtArr、InPtArrArr为局部坐标系的坐标
  Acad::ErrorStatus         setVal(const AcGePoint3d& oriPt, const AcGeVector3d& V1, const AcGeVector3d& V2, 
	  const AcGeVector3d& Vh, double H, const std::vector<t_PolylineVertex>& OutPtArr,
	  const std::vector< std::vector<t_PolylineVertex> >& InPtArrArr, int CorrType = 1/* 0: ｚ左手系，1: 右手系*/);

    virtual void createBody();

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

protected:
    Adesk::Boolean drawPolylineVertex(AcGiWorldDraw* mode, 
                                     const std::vector<t_PolylineVertex> &vertexArr, 
                                     int &gsIndex, 
                                     int Precision) const;

private:	// 存储的数据
  // 所有点和方向均为世界坐标系的坐标
	AcGePoint3d               m_ptOri;      // 空间任意平面坐标系原点
    AcGeVector3d              m_vecV1;      // X轴方向
    AcGeVector3d              m_vecV2;      // Y轴方向
    AcGeVector3d              m_vecVh;      // 拉伸方向
    double                    m_dH;         // 拉伸高度
    AcGePoint3dArray          m_ptarrOut;   // 外环的顶点	//此数据不使用了，用m_vertexArrOut替代
	std::vector<AcGePoint3dArray>  m_ptarrarrIn; // 内环的顶点	//此数据不使用了，用m_vertexArrArrIn替代

	// version 2
	std::vector<t_PolylineVertex>  m_vertexArrOut; // 外环的顶点（世界坐标系的坐标）
	std::vector< std::vector<t_PolylineVertex> >  m_vertexArrArrIn; // 内环的顶点（世界坐标系的坐标）
	
private:	// 不存储的数据
	double GetMaxLen();
	double m_dMaxLength;

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
MAKE_ACDBOPENOBJECT_FUNCTION(PDSpolygon);

#endif // !defined(PDSPOLYGON_H)
