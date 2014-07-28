/////////////////////////////////////////////
// PDPrimary3D custom object.

#if !defined(ARX__PDPRIMARY3D_H__20010918_151458)
#define ARX__PDPRIMARY3D_H__20010918_151458

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dbents.h>
#include <dbcurve.h>
#include <dbsol3d.h>

#ifdef _OBJECTARX14_
#define   bool   int
#define   true   1
#define   false   0
#endif
#include <amodeler.h>
using namespace AModeler;


#pragma warning(disable: 4275 4251)

/**
  PDPrimary3D类的派生关系：
  PDPrimary3D
      PDBox
          PdsDbPrecomp
          PdsDbSupVirBox
      PDBox1
      PDConcone
      PdsDbConnect
          PdsDbBoltConnect
              PdsDbBolt
              PdsDbNut
              PdsDbWasher
      PDCylinder
          PDScylinder
      PDEcone
      PDOval
      PDPrism
      PDPrism1
      PDRevolve
      PDSaddle
      PDSphere
      PDSpolygon
      PDSqucir
      PDSqucone
      PDSqutorus
      PDSqutorus1
      PDTorus
      PDTorus1
      PDWedge
 */

extern Acad::ErrorStatus addToDb(AcDbEntity *pEnt);
//int getOrbitFlag();

#ifndef PI
#define PI 3.1415926535897932384626433832795L
#endif

#define PRIMARY3D_DEFAULT_PRECISION 32
#define PRIMARY3D_WIREFRAME_LINENUM  8
#define PRIMARY3D_WIREFRAME_LINENUM4  4
#define PRIMARY3D_SIMPLIZE_SCALE 3
#define PRIMARY3D_SIMPLIZE_SCALE4 5
//////////////////////////////////////////////////////////////////////////
//简化的等级
#ifndef PRIMARY3D_SIMPLIZE_RATE0
#define  PRIMARY3D_SIMPLIZE_RATE0 0 // 不简化
#define  PRIMARY3D_SIMPLIZE_RATE1 1 // 1级简化
#define  PRIMARY3D_SIMPLIZE_RATE2 2 // 2 级简化
#endif

#define POLYLINEVERTE_FLAG_L   1
#define POLYLINEVERTE_FLAG_A   2
#define POLYLINEVERTE_FLAG_AM   3
#define POLYLINEVERTE_FLAG_LC   4
#define POLYLINEVERTE_FLAG_AC   5
#define POLYLINEVERTE_FLAG_C   6

//长方体六个面的GS标记 
//begin::added by szw 2009.11.18
enum BOX_SelectionMarker{
	BOTTOM_PLANE = 1,
	TOP_PLANE,
	FRONT_PLANE,
	LEFT_PLANE,
	RIGHT_PLANE,
	BACK_PLANE
};
//end::added by szw 2009.11.18

struct t_PolylineVertex
{
   // 1：直线的起点；2：圆弧的起点；3：圆弧的中间一点；4：多义线的最后一点，
   // 该多义线封闭且封闭使用直线；5：多义线的最后一点，该多义线封闭且封闭使
   // 用圆弧，其后还需跟随圆弧上的中间一点；6：圆上的点（必须三个这中点连在一起）
   Adesk::Int16 m_vertexFlag;
   AcGePoint3d m_vertex;
};

//struct shellData
//{
//	Adesk::UInt32 nbVertex;
//	AcGePoint3dArray pVertexList;
//	Adesk::UInt32 faceListSize;
//	std::vector<Adesk::Int32> pFaceList;
//	AcGiEdgeData *pEdgeData;
//	AcGiFaceData *pFaceData;
//	AcGiVertexData *pVertexData;
//	bool bAutoGenerateNormals;
//
//	shellData() : pEdgeData(NULL), pFaceData(NULL), pVertexData(NULL)
//	{
//	}
//
//	shellData(const shellData &op)
//	{
//		if(this == &op)
//			return ;
//		operator=(op);
//	}
//
//	shellData &operator=(const shellData &op)
//	{
//		if(this == &op)
//			return *this;
//		nbVertex = op.nbVertex;
//		pVertexList = op.pVertexList;
//		faceListSize = op.faceListSize;
//		pFaceList.clear();
//		pFaceList.insert(pFaceList.begin(), op.pFaceList.begin(), op.pFaceList.end());
//		//if(pEdgeData != NULL)
//		//	delete pEdgeData;
//		pEdgeData = op.pEdgeData;
//		//if(pFaceData != NULL)
//		//	delete pFaceData;
//		pFaceData = op.pFaceData;
//		//if(pVertexData != NULL)
//		//	delete pVertexData;
//		pVertexData = op.pVertexData;
//		bAutoGenerateNormals = op.bAutoGenerateNormals;
//		return *this;
//	}
//};

class PDPrimary3D : public AcDbCurve
{
public:

   ACRX_DECLARE_MEMBERS(PDPrimary3D);

   // Constructor / Destructor
   PDPrimary3D(bool HasSnap = false);
   virtual ~PDPrimary3D();

   //{{AFX_ARX_METHODS(PDPrimary3D)
#ifdef _OBJECTARX2010_
   virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* mode);
   virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d& xform);
   virtual void              subList() const;
   virtual Acad::ErrorStatus subGetGeomExtents(AcDbExtents& extents) const;
#else
   virtual Adesk::Boolean worldDraw(AcGiWorldDraw* mode);
   virtual Acad::ErrorStatus transformBy(const AcGeMatrix3d& xform);
   virtual void              list() const;
   virtual Acad::ErrorStatus getGeomExtents(AcDbExtents& extents) const;
#endif
   virtual Acad::ErrorStatus dxfOutFields(AcDbDxfFiler* pFiler) const;
   virtual Acad::ErrorStatus dxfInFields(AcDbDxfFiler* pFiler);
   virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler* pFiler) const;
   virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler* pFiler);
   //}}AFX_ARX_METHODS

   //{{AFX_ARX_DATA_ACCESS(PDPrimary3D)
   Acad::ErrorStatus setDividPrecision(const Adesk::Int32 dDividPrecision);
   Adesk::Int32 dividPrecision() const;
   //}}AFX_ARX_DATA_ACCESS

    ////用来控制所有子类是否输出捕捉点////
    bool hasSnap() const;
    void setHasSnap(bool val);

    /** 取得是否计算最大剖分精度
     * @return bool
     *
     * 如果计算最大剖分精度，则可以提高体元的显示速度，但在render命令中
     * 生成的图片可能效果不理想，如果不计算最大剖分精度，则render命令生成
     * 的图片效果好，但体元的显示速度有可能下降。
     */ 
    Adesk::Int16 getCalMaxPrecision() const;
    /** 设置是否计算最大剖分精度
     * @param val - 输入：是否计算最大剖分精度
     * @return void
     *
     * 如果计算最大剖分精度，则可以提高体元的显示速度，但在render命令中
     * 生成的图片可能效果不理想，如果不计算最大剖分精度，则render命令生成
     * 的图片效果好，但体元的显示速度有可能下降。
     */ 
    void setCalMaxPrecision(Adesk::Int16 val);

    // 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
    // 精度不大于实体限定的剖分精度m_dDividPrecision。
    virtual int getMaxPrecision(AcGiWorldDraw *mode) const;
    void getActDivid(int Precision, AcGeDoubleArray &dividDbl, AcGeIntArray &stdIdx) const;

    virtual void createBody();

	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

   // 取得Amodeler 模型的实体
   const Body* getBody() const;
   // 清空Amodeler模型的实体
   Acad::ErrorStatus clearBody();

   Acad::ErrorStatus convertPolyVertexToSegments(const std::vector<t_PolylineVertex> &vertexArr, const AcGeVector3d &norm, AcDbVoidPtrArray &curveSegments) const;
   Acad::ErrorStatus convertPolyVertexToAmodelerData(const std::vector<t_PolylineVertex> &vertexArr, AcGePoint3d* &vertices, PolygonVertexData** &vertexData, 
      int &iVertices, AcGeVector3d &normal) const;
   void getPolylineVertices(AcGePoint3d* vertices, const AcGeDoubleArray &bulge, const AcGeVector3d normal, int isClosed, PolygonVertexData** &vertexData) const;
   double circleRadiusFromBulge(const AcGePoint3d& p1, const AcGePoint3d& p2, double bulge) const;

   static bool isLeftCoord(AcGeVector3d x,AcGeVector3d y, AcGeVector3d z);

   void getSolidInfo(const AcDbObjectId &solidId);
   //void getSolidWorldDrawInfo(const AcDbObjectId &solidId);

protected:
   //{{AFX_ARX_DATA(PDPrimary3D)
   Adesk::Int32 m_dDividPrecision;
   ////用来统一控制所有子类的显示精度////
   //}}AFX_ARX_DATA

    ////用来控制所有子类是否输出捕捉点////
    Adesk::Int16 m_bHasSnap;

    ////////
   
    //! 是否计算最大剖分精度
    /*! 如果计算最大剖分精度，则可以提高体元的显示速度，但在render命令中
     *  生成的图片可能效果不理想，如果不计算最大剖分精度，则render命令生成
     *  的图片效果好，但体元的显示速度有可能下降。
     */ 
    Adesk::Int16 m_bCalMaxPrecision;

    // version 2
    // version 3 不对m_3dGeom进行存储
    Body m_3dGeom;  // Amodeler 模型的实体
	AcDb3dSolid m_3dSolid;	// AcDb3dSolid实体
	// version 4
	std::vector<AcGePoint3dArray> m_ptsArr;	//Brep产生的点

public:
   bool m_no8;      //不插入8等分
   int m_drawType;	//显示形式，0：不采用Body、3dSolid和brep显示，1：Body显示，2：3dSolid显示，3：brep显示

   //std::vector<shellData> m_shelldata;

#ifndef _OBJECTARX2010_
private:
   void *operator new[](unsigned nSize) { return 0; }
   void operator delete[](void *p) {};
   void *operator new[](unsigned nSize, const char *file, int line) { return 0; }
#endif

public:
  virtual void setDefault(); //设置缺省的对象数据
  virtual bool isValidData(double &f); //检查是否成员数据均是合法数据,并返回出错的浮点数
};
MAKE_ACDBOPENOBJECT_FUNCTION(PDPrimary3D);

#include <float.h>
inline bool isValidFloat(double f)
{
  int flt = _fpclass(f);
  return ((flt)&_FPCLASS_NN||(flt)&_FPCLASS_NZ||(flt)&_FPCLASS_PZ||(flt)&_FPCLASS_PN)?true:false;
}

Acad::ErrorStatus auditPDPrimary3D(PDPrimary3D *pent, AcDbAuditInfo* pAuditInfo,const TCHAR *prompt);

#endif // !defined(ARX__PDPRIMARY3D_H__20010918_151458)
