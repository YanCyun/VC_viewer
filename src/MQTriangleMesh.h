#ifndef MQTriangleMesh_h
#define MQTriangleMesh_h

#include <list>
#include <vector>
#include <cstdlib>
#include <map>
#include <utility>
#include "MQOpenGL.h"

using namespace std;

class MQVertex
{
public:

	double X, Y, Z;
	double S, T;         //parametric coordinates
	double NX, NY, NZ;   //normal

	list<int>	NeighborVertex;
	double	LapX, LapY, LapZ;
	double  R,G,B; //Laplacian to Color

public:

	void Normal2UnitVector();

public:

	MQVertex()
	{
		X = Y = Z = 0.0;
		S = T = 0.0;
		NX = NY = NZ = 0.0;
		LapX = LapY = LapZ = 0.0;
		R = G = B = 0.0;
	}

	~MQVertex()
	{}
};

class MQTriangle
{
public:

	int    V1, V2, V3;
	double NX, NY, NZ;	//normal

public:
	void Normal2UnitVector();

public:

	MQTriangle()
	{
		V1 = V2 = V3 = 0;
		NX = NY = NZ = 0.0;
	}

	~MQTriangle()
	{}
};


class MQTriangleTex
{
public:

	int    T1, T2, T3;

public:

	MQTriangleTex()
	{
		T1 = T2 = T3 = 0;
	}

	~MQTriangleTex()
	{}
};

struct HalfEdge
{
	pair<int,int> oppositeHalfEdge;
	pair<int,int> nextHalfEdge;
	int  face;
};

struct MQImagePixel
{
	double X,Y;
	double LapX,LapY,LapZ;
	double R,G,B;
	int Triangle;

};


class MQTriangleMesh
{
public:

	int VertexNum;
	int TexcoordNum;
	int TriangleNum;
	int imageSize;
	double minLap,maxLap;
	double boundary;
	vector<MQVertex>   Vertex;
	vector<MQTriangle> Triangle;
	vector<MQTriangleTex> TriangleTex;
	vector<vector<MQImagePixel>> ImagePixel;
	pair<float,float> boundaryX;
	pair<float,float> boundaryY;
	map< pair<int,int>, HalfEdge* > Edges; // pair<EdgeStartVertex,EdgeEndVertex>
	list<list<int>> Holes;

public:

	bool ReadObjFile(const char *FileName);
	void Draw(GLubyte Red, GLubyte Green, GLubyte Blue);
	void Draw2D(void);
	void DrawPoint(void);
	
	void UpdatePointStruct(void);
	void UpdateVertexNeigborVertex(void);
	void UpdateVertexLaplacianCoordinate(void);
	
	void CalculateLaplacianToColor(void);
	void PointInTriange(MQImagePixel &p);

	void FindBoundary(void);
	void FindHole(void);

public:

	MQTriangleMesh()
	{
		VertexNum = 0;
		TriangleNum = 0;
		imageSize = 512;
	}

	virtual ~MQTriangleMesh()
	{}
};


#endif	//MQTriangleMesh_h
