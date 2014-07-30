#include "stdafx.h"
#include "TestModel.h"
#include <vector>

#include <gepnt3d.h>
#include <gevec3d.h>
#include <dbmain.h>
#include <migrtion.h>
#include <dbapserv.h>

#include "PDBox.h"
#include "PDBox1.h"
#include "PDCylinder.h"
#include "PDConcone.h"
#include "PDEcone.h"
#include "PDSqucone.h"
#include "PDWedge.h"
#include "PDOval.h"
#include "PDSphere.h"
#include "PDTorus.h"
#include "PDSqutorus.h"
#include "PDSaddle.h"
#include "PDPrism.h"
#include "PDScylinder.h"
#include "PDPrism1.h"
#include "PDTorus1.h"
#include "PDSqutorus1.h"
#include "PDSqucir.h"
#include "PDSPolygon.h"
#include "PDRevolve.h"

using namespace std;

Acad::ErrorStatus addToDb(AcDbEntity *pEnt);
//{
//	Acad::ErrorStatus es = Acad::eOk;
//
//	//	ÅÐ¶ÏpEnt²»Îª¿Õ
//	if (pEnt)
//	{
//		AcDbBlockTable *pBlockTable = NULL;
//		AcDbBlockTableRecord  *pBlockTableRecord = NULL;
//		///////////////		get block table		////////
//		AcDbDatabase *pDb = NULL;
//		if ((es = acdbCurDwg()->getBlockTable(pBlockTable, AcDb::kForRead)) == Acad::eOk)
//		{
//			//////////////		get block table record		/////////////
//			if ((es = pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite)) == Acad::eOk)
//			{
//				////	id is NULL,so add this Entity to tablerecord
//				es = pBlockTableRecord->appendAcDbEntity(pEnt);
//				pBlockTableRecord->close();
//			}
//			pBlockTable->close();
//		}
//		return (es);
//	}
//	else
//		return Acad::eInvalidInput;
//}

void PDPRIMARY3D_MODEL()
{
	PDBox *pBox = new PDBox(AcGePoint3d(0, 0, 0), 30., 30., 50., AcGeVector3d(1, 0, 0), AcGeVector3d(0, 1, 0));
	addToDb(pBox);
	pBox->close();

	PDBox1 *pBox1 = new PDBox1(AcGePoint3d(100, 0, 0), AcGeVector3d(1, 0, 0), AcGeVector3d(0, 1, 0), 30., 30., 50.);
	addToDb(pBox1);
	pBox1->close();

	PDPrism *pPsm = new PDPrism(AcGePoint3d(200, 0, 0), AcGePoint3d(200, 0, 50), AcGePoint3d(230, 0, 0), 6);
	addToDb(pPsm);
	pPsm->close();

	PDPrism1 *pPsm1 = new PDPrism1(AcGePoint3d(300, 0, 0), AcGePoint3d(300, 0, 50), AcGePoint3d(330, 0, 0), 6);
	addToDb(pPsm1);
	pPsm1->close();

	PDCylinder *pCyl = new PDCylinder(AcGePoint3d(0, 100, 0), AcGePoint3d(0, 100, 50), 30, 32);
	addToDb(pCyl);
	pCyl->close();

	PDConcone *pCon1 = new PDConcone(AcGePoint3d(100, 100, 0), AcGePoint3d(100, 100, 50), 30, 10);
	addToDb(pCon1);
	pCon1->close();

	PDConcone *pCon2 = new PDConcone(AcGePoint3d(200, 100, 0), AcGePoint3d(200, 100, 50), 30, 0);
	addToDb(pCon2);
	pCon2->close();

	PDEcone *pEco = new PDEcone(AcGePoint3d(300, 100, 0), AcGePoint3d(275, 100, 50), 60, 30, AcGeVector3d(-1, 0, 0), 24);
	addToDb(pEco);
	pEco->close();

	PDScylinder *pScy = new PDScylinder(AcGePoint3d(0, 200, 0), AcGePoint3d(0, 200, 100), 30, AcGeVector3d(-1, 0, -1), 24);
	addToDb(pScy);
	pScy->close();

	PDSqucone *pSqu1 = new PDSqucone(40, 30, 20, 10, 50, AcGePoint3d(100, 200, 0), AcGeVector3d(0, 0, 1), AcGeVector3d(1, 0, 0), AcGeVector3d(-1, 0, 0), 10);
	//PDSqucone* pSqu1 = new PDSqucone(4500,40,5000,40,240,AcGePoint3d(1650,20,1800),AcGeVector3d(-192,0,144),AcGeVector3d(3000,0,4000),AcGeVector3d( 0,0,0),0);
	addToDb(pSqu1);
	pSqu1->close();

	PDSqucone *pSqu2 = new PDSqucone(40, 30, 20, 10, 50, AcGePoint3d(200, 200, 0), AcGeVector3d(0, 0, 1), AcGeVector3d(1, 0, 0), AcGeVector3d(0, 0, 0), 10);
	addToDb(pSqu2);
	pSqu2->close();

	PDWedge *pWed = new PDWedge(AcGePoint3d(300, 200, 0), AcGePoint3d(350, 200, 0), AcGePoint3d(350, 230, 0), 60);
	addToDb(pWed);
	pWed->close();

	PDOval *pOvl = new PDOval(30, 40, 40, AcGePoint3d(0, 300, 0), AcGeVector3d(0, 0, 1), 24, 24);
	addToDb(pOvl);
	pOvl->close();

	PDOval *pOvlSphere = new PDOval(40, 40, 40, AcGePoint3d(-100, 300, 0), AcGeVector3d(0, 0, 1), 24, 24);
	addToDb(pOvlSphere);
	pOvlSphere->close();

	PDSphere *pSph = new PDSphere(AcGePoint3d(100, 300, 0), 30, 24);
	addToDb(pSph);
	pSph->close();

	PDTorus *pTor0 = new PDTorus(AcGePoint3d(200, 300, 0), AcGePoint3d(250, 300, 0), AcGePoint3d(200, 350, 0), 50, 30, 0, 24, 24);
	addToDb(pTor0);
	pTor0->close();

	PDTorus *pTor1 = new PDTorus(AcGePoint3d(400, 300, 0), AcGePoint3d(450, 300, 0), AcGePoint3d(400, 350, 0), 50, 30, 1, 24, 24);
	addToDb(pTor1);
	pTor1->close();

	PDTorus *pTor2 = new PDTorus(AcGePoint3d(0, 500, 0), AcGePoint3d(50, 500, 0), AcGePoint3d(0, 0, 1), 50, 30, 2, 24, 24);
	addToDb(pTor2);
	pTor2->close();

	PDTorus *pTor3 = new PDTorus(AcGePoint3d(200, 500, 0), AcGePoint3d(250, 500, 0), AcGePoint3d(0, 0, 1), 50, 30, 3, 24, 24);
	addToDb(pTor3);
	pTor3->close();

	PDTorus1 *pTor11 = new PDTorus1(AcGePoint3d(400, 500, 0), AcGePoint3d(450, 500, 0), AcGePoint3d(0, 0, 1), 50, 30, 135, 24, 24);
	addToDb(pTor11);
	pTor11->close();

	PDTorus1 *pTor12 = new PDTorus1(AcGePoint3d(600, 500, 0), AcGePoint3d(650, 500, 0), AcGePoint3d(0, 0, 1), 50, 20, 360, 24, 24);
	addToDb(pTor12);
	pTor12->close();

	PDSqutorus *pSqt0 = new PDSqutorus(AcGePoint3d(0, 700, 0), AcGePoint3d(50, 700, 0), AcGePoint3d(0, 750, 0), 40, 40, 20, 10, 0, 24);
	addToDb(pSqt0);
	pSqt0->close();

	PDSqutorus *pSqt1 = new PDSqutorus(AcGePoint3d(200, 700, 0), AcGePoint3d(250, 700, 0), AcGePoint3d(200, 750, 0), 40, 40, 20, 10, 1, 24);
	addToDb(pSqt1);
	pSqt1->close();

	PDSqutorus *pSqt2 = new PDSqutorus(AcGePoint3d(400, 700, 0), AcGePoint3d(450, 700, 0), AcGePoint3d(0, 0, 1), 40, 40, 20, 10, 2, 24);
	addToDb(pSqt2);
	pSqt2->close();

	PDSqutorus *pSqt3 = new PDSqutorus(AcGePoint3d(600, 700, 0), AcGePoint3d(650, 700, 0), AcGePoint3d(0, 0, 1), 40, 40, 20, 10, 3, 24);
	addToDb(pSqt3);
	pSqt3->close();

	PDSqutorus1 *pSqu11 = new PDSqutorus1(AcGePoint3d(0, 900, 0), AcGePoint3d(50, 900, 0), AcGeVector3d(0, 0, 1), 40, 40, 20, 10, 135, 24);
	addToDb(pSqu11);
	pSqu11->close();

	PDSqutorus1 *pSqu12 = new PDSqutorus1(AcGePoint3d(200, 900, 0), AcGePoint3d(250, 900, 0), AcGeVector3d(0, 0, 1), 40, 40, 20, 10, 360, 24);
	addToDb(pSqu12);
	pSqu12->close();

	PDSaddle *pSad = new PDSaddle(80, 60, 60, AcGePoint3d(400, 900, 0), 0, 50, 24);
	addToDb(pSad);
	pSad->close();

	PDSqucir *pSqc = new PDSqucir(AcGePoint3d(600, 900, 0), AcGeVector3d(1, 0, 0), 80, 60, AcGePoint3d(640, 920, 80), AcGeVector3d(0, 0, 1), 40, 7);
	addToDb(pSqc);
	pSqc->close();

	{
		AcGePoint3dArray outPts;
		outPts.append(AcGePoint3d(0, 0, 0));
		outPts.append(AcGePoint3d(80, 0, 0));
		outPts.append(AcGePoint3d(0, 80, 0));
		vector<AcGePoint3dArray> inPtsArr;
		{
			AcGePoint3dArray ptarr;
			ptarr.append(AcGePoint3d(10, 40, 0));
			ptarr.append(AcGePoint3d(20, 40, 0));
			ptarr.append(AcGePoint3d(20, 45, 0));
			ptarr.append(AcGePoint3d(10, 45, 0));
			inPtsArr.push_back(ptarr);
		}
		{
			AcGePoint3dArray ptarr;
			ptarr.append(AcGePoint3d(20, 20, 0));
			ptarr.append(AcGePoint3d(40, 20, 0));
			ptarr.append(AcGePoint3d(40, 80, 0));
			ptarr.append(AcGePoint3d(20, 80, 0));
			inPtsArr.push_back(ptarr);
		}
		AcGePoint3dArray AllPts;
		AllPts.append(AcGePoint3d(0, 0, 0));
		AllPts.append(AcGePoint3d(80, 0, 0));
		AllPts.append(AcGePoint3d(0, 80, 0));
		AllPts.append(AcGePoint3d(10, 40, 0));
		AllPts.append(AcGePoint3d(20, 40, 0));
		AllPts.append(AcGePoint3d(20, 45, 0));
		AllPts.append(AcGePoint3d(10, 45, 0));
		AllPts.append(AcGePoint3d(20, 20, 0));
		AllPts.append(AcGePoint3d(40, 20, 0));
		AllPts.append(AcGePoint3d(40, 30, 0));
		AllPts.append(AcGePoint3d(20, 30, 0));
		vector<int> InPtNumArr;
		InPtNumArr.push_back(4);
		InPtNumArr.push_back(4);
		PDSpolygon *pSpolygon1 = new PDSpolygon(AcGePoint3d(800, 0, 0), AcGeVector3d(1, 0, 0), AcGeVector3d(0, 1, 0),
			AcGeVector3d(0, 0, 1), 80, outPts, inPtsArr);
		addToDb(pSpolygon1);
		pSpolygon1->close();
		PDSpolygon *pSpolygon2 = new PDSpolygon(3, InPtNumArr, AcGePoint3d(800, 400, 0), AcGeVector3d(1, 0, 0),
			AcGeVector3d(0, 1, 0), AcGeVector3d(0, 0, 1), 80, AllPts);
		addToDb(pSpolygon2);
		pSpolygon2->close();

		PDSpolygon *pSpolygon3 = new PDSpolygon(AcGePoint3d(1000, 0, 0), AcGeVector3d(-1, 0, 0), AcGeVector3d(0, 1, 0),
			AcGeVector3d(0, 0, 1), 80, outPts, inPtsArr, true, 0);
		addToDb(pSpolygon3);
		pSpolygon3->close();
		PDSpolygon *pSpolygon4 = new PDSpolygon(3, InPtNumArr, AcGePoint3d(1000, 400, 0), AcGeVector3d(-1, 0, 0),
			AcGeVector3d(0, 1, 0), AcGeVector3d(0, 0, 1), 80, AllPts, true, 0);
		addToDb(pSpolygon4);
		pSpolygon4->close();
	}

	{
		vector<t_PolylineVertex> outPts;
		t_PolylineVertex ptTemp;
		ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
		ptTemp.m_vertex.set(0, 0, 0);
		outPts.push_back(ptTemp);
		ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_AC;
		ptTemp.m_vertex.set(80, 0, 0);
		outPts.push_back(ptTemp);
		ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_AM;
		ptTemp.m_vertex.set(0, 80, 0);
		outPts.push_back(ptTemp);
		vector< vector<t_PolylineVertex> > inPtsArr;
		{
			vector<t_PolylineVertex> ptarr;
			ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
			ptTemp.m_vertex.set(10, 40, 0);
			ptarr.push_back(ptTemp);
			ptTemp.m_vertex.set(20, 40, 0);
			ptarr.push_back(ptTemp);
			ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_AC;
			ptTemp.m_vertex.set(20, 45, 0);
			ptarr.push_back(ptTemp);
			ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_AM;
			ptTemp.m_vertex.set(10, 45, 0);
			ptarr.push_back(ptTemp);
			inPtsArr.push_back(ptarr);
		}
		{
			vector<t_PolylineVertex> ptarr;
			ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
			ptTemp.m_vertex.set(20, 20, 0);
			ptarr.push_back(ptTemp);
			ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_A;
			ptTemp.m_vertex.set(40, 20, 0);
			ptarr.push_back(ptTemp);
			ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_AM;
			ptTemp.m_vertex.set(40, 80, 0);
			ptarr.push_back(ptTemp);
			ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
			ptTemp.m_vertex.set(20, 80, 0);
			ptarr.push_back(ptTemp);
			inPtsArr.push_back(ptarr);
		}
		PDSpolygon *pSpolygon1 = new PDSpolygon(AcGePoint3d(800, 200, 0), AcGeVector3d(1, 0, 0), AcGeVector3d(0, 1, 0),
			AcGeVector3d(0, 0, 1), 80, outPts, inPtsArr);
		addToDb(pSpolygon1);
		pSpolygon1->close();

		PDSpolygon *pSpolygon2 = new PDSpolygon(AcGePoint3d(1000, 200, 0), AcGeVector3d(-1, 0, 0), AcGeVector3d(0, 1, 0),
			AcGeVector3d(0, 0, 1), 80, outPts, inPtsArr, PRIMARY3D_DEFAULT_PRECISION, true, 0);
		addToDb(pSpolygon2);
		pSpolygon2->close();
	}

	{
		AcGePoint3dArray outPts;
		outPts.append(AcGePoint3d(0, 0, 0));
		outPts.append(AcGePoint3d(80, 0, 0));
		outPts.append(AcGePoint3d(0, 80, 0));
		vector<AcGePoint3dArray> inPtsArr;
		{
			AcGePoint3dArray ptarr;
			ptarr.append(AcGePoint3d(10, 40, 0));
			ptarr.append(AcGePoint3d(20, 40, 0));
			ptarr.append(AcGePoint3d(20, 45, 0));
			ptarr.append(AcGePoint3d(10, 45, 0));
			inPtsArr.push_back(ptarr);
		}
		{
			AcGePoint3dArray ptarr;
			ptarr.append(AcGePoint3d(20, 20, 0));
			ptarr.append(AcGePoint3d(40, 20, 0));
			ptarr.append(AcGePoint3d(40, 80, 0));
			ptarr.append(AcGePoint3d(20, 80, 0));
			inPtsArr.push_back(ptarr);
		}
		AcGePoint3dArray AllPts;
		AllPts.append(AcGePoint3d(0, 0, 0));
		AllPts.append(AcGePoint3d(80, 0, 0));
		AllPts.append(AcGePoint3d(0, 80, 0));
		AllPts.append(AcGePoint3d(10, 40, 0));
		AllPts.append(AcGePoint3d(20, 40, 0));
		AllPts.append(AcGePoint3d(20, 45, 0));
		AllPts.append(AcGePoint3d(10, 45, 0));
		AllPts.append(AcGePoint3d(20, 20, 0));
		AllPts.append(AcGePoint3d(40, 20, 0));
		AllPts.append(AcGePoint3d(40, 30, 0));
		AllPts.append(AcGePoint3d(20, 30, 0));
		vector<int> InPtNumArr;
		InPtNumArr.push_back(4);
		InPtNumArr.push_back(4);
		PDSpolygon *pSpolygon1 = new PDSpolygon(AcGePoint3d(1000, 800, 0), AcGeVector3d(0, 1, 0), AcGeVector3d(0, 0, 1),
			AcGeVector3d(0, 0, 1), 80, outPts, inPtsArr);
		addToDb(pSpolygon1);
		pSpolygon1->close();
		PDSpolygon *pSpolygon2 = new PDSpolygon(3, InPtNumArr, AcGePoint3d(1000, 800, 400), AcGeVector3d(0, 1, 0),
			AcGeVector3d(0, 0, 1), AcGeVector3d(0, 0, 1), 80, AllPts);
		addToDb(pSpolygon2);
		pSpolygon2->close();

		PDSpolygon *pSpolygon3 = new PDSpolygon(AcGePoint3d(1000, 1000, 0), AcGeVector3d(0, -1, 0), AcGeVector3d(0, 0, 1),
			AcGeVector3d(0, 0, 1), 80, outPts, inPtsArr, true, 0);
		addToDb(pSpolygon3);
		pSpolygon3->close();
		PDSpolygon *pSpolygon4 = new PDSpolygon(3, InPtNumArr, AcGePoint3d(1000, 1000, 400), AcGeVector3d(0, -1, 0),
			AcGeVector3d(0, 0, 1), AcGeVector3d(0, 0, 1), 80, AllPts, true, 0);
		addToDb(pSpolygon4);
		pSpolygon4->close();
	}

	vector<t_PolylineVertex> RevolvePtArr;
	t_PolylineVertex RevolvePtTemp;
	RevolvePtTemp.m_vertexFlag = 1;
	RevolvePtTemp.m_vertex.set(850, 800, 0);
	RevolvePtArr.push_back(RevolvePtTemp);
	RevolvePtTemp.m_vertexFlag = 5;
	RevolvePtTemp.m_vertex.set(850, 800, 50);
	RevolvePtArr.push_back(RevolvePtTemp);
	RevolvePtTemp.m_vertexFlag = 3;
	RevolvePtTemp.m_vertex.set(900, 800, 25);
	RevolvePtArr.push_back(RevolvePtTemp);
	PDRevolve *pRevolve = new PDRevolve(AcGePoint3d(800, 800, 0), AcGeVector3d(0, 0, 1), RevolvePtArr);
	addToDb(pRevolve);
	pRevolve->close();
}