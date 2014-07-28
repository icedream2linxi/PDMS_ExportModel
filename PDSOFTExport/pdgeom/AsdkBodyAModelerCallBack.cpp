#include "stdafx.h"
#include "AsdkBodyAModelerCallBack.h"

void 
AsdkBodyAModelerCallBack::outputTriangle(Edge* edges[], int numSides)
{
    if (!m_pCurWorldDraw)
        return;

    // 
    // assume, the numSide must be < 5
    // 
    if (numSides < 3 || numSides > 4)   
        return;

    AcGePoint3d points[4];
    Adesk::UInt32 face_list_size = 1 + numSides;
    Adesk::Int32* face_list = new Adesk::Int32[face_list_size];
    AcGiEdgeData edge_data;
    Adesk::UInt8* edge_vis_array = new Adesk::UInt8[numSides];
    face_list[0] = numSides;
    for (int i = 0; i < numSides; ++i)
    {
        points[i] = AcGePoint3d(edges[i]->point().x, 
            edges[i]->point().y, edges[i]->point().z);
                                                                                
        face_list[i+1] = i;
        if (edges[i]->next() != edges[(i+1)%numSides] 
            || edges[i]->isFlagOn(BEF)) 
        {
            // 
            // triangle edge 
            // 
            edge_vis_array[i] = kAcGiInvisible;
        }
        else if (edges[i]->isFlagOn(AEF))
        {
            edge_vis_array[i] = kAcGiSilhouette;
        }
        else
        {
            edge_vis_array[i] = kAcGiVisible;
        }
    }
    edge_data.setVisibility(edge_vis_array);

    // 
    // set color
    // Need to figure out wall side to assign correct color
    // 
    m_pCurWorldDraw->geometry().shell(numSides, points, face_list_size, 
        face_list, &edge_data);

    delete [] face_list;
    delete [] edge_vis_array;
}


void 
AsdkBodyAModelerCallBack::outputTriStrip(Edge* /*edgeArray*/[], 
    int /*arrayLength*/, bool /* firstTriangleIsCcw */)
{
    assert(0);
}

void drawAllEdges(const Body& b, AcGiWorldDraw *pWorldDraw)
{
//#ifndef NDEBUG
//    Adesk::UInt16 currentColor = pWorldDraw->subEntityTraits().color();
//    Adesk::UInt16 color = currentColor, lastColorSet = currentColor;
//    const Adesk::UInt16 approxColor = 1;    // red
//    const Adesk::UInt16 bridgeColor = 2;    // yellow
//    const Adesk::UInt16 firstEdgeColor = 3; // green
//    const Adesk::UInt16 secondEdgeColor = 4;    // cyan
//    int iEdge = 0, iFaces = b.faceCount();
//#endif

    for (Face* f = b.faceList(); f != NULL; f = f->next())
    {
        if (f->edgeLoop() == NULL)
            continue;
        
        Edge* e = f->edgeLoop();
        do 
        {
            if (e->isEulerEdge()) 
            {
//#ifndef NDEBUG
//                if (e->isApprox())
//                    color = approxColor;
//                else if (iFaces == 1 && iEdge == 0)
//                    color = firstEdgeColor;
//                else if (iFaces == 1 && iEdge == 1)
//                    color = secondEdgeColor;
//                else if (e->isBridge())
//                    color = bridgeColor;
//                else
//                    color = currentColor;
//
//                if (color != lastColorSet)
//                {
//                    pWorldDraw->subEntityTraits().setColor(color);
//                    lastColorSet = color;
//                }
//                iEdge++;
//#else
                if (e->isBridge())
                    continue;
//#endif
                Point3d p[2];
                p[0] = e->point();
                p[1] = e->next()->point();
                pWorldDraw->geometry().polyline(2, (AcGePoint3d*)p);
            }
        } while ((e = e->next()) != f->edgeLoop());
    }
}
