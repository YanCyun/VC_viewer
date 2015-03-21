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


struct MQTriangleTex{ int T1, T2, T3; };

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
	bool isHole;
	list<int> neighborHole;
	int position;

	MQImagePixel(){
		X = Y = 0.0;
		LapX = LapY = LapZ = 0.0;
		R = G = B = 0.0;
		Triangle = 0;
		isHole = false;
		position = 0;
	}

	~MQImagePixel()	
	{}

	friend bool operator <(const MQImagePixel& p1,const MQImagePixel& p2){ return p1.neighborHole.size() < p2.neighborHole.size();}
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
	vector<MQImagePixel> HolePixels;
	
	pair<double,double> boundaryX;
	pair<double,double> boundaryY;
	pair<double,double> hole_boundaryX;
	pair<double,double> hole_boundaryY;

	map< pair<int,int>, HalfEdge* > Edges; // pair<EdgeStartVertex,EdgeEndVertex>

	list<list<int>> Holes;
	list<list<int>> Holes_uv;
	

	//fill hole data
	int sample_w, sample_h;
	void* sample_data;
	double** sample_red;
	double** sample_green;
	double** sample_blue;

	int texture_w, texture_h;
	void * texture_data;
	double** texture_red;
	double** texture_green;
	double** texture_blue;
	int** original_pos_x;
	int** original_pos_y;

	double** red;
	double** green;
	double** blue;
	vector<int> candidate_x;
	vector<int> candidate_y;


public:

	bool ReadObjFile(const char *FileName);
	void Draw(GLubyte Red, GLubyte Green, GLubyte Blue);
	void Draw2D(void);
	void DrawPoint(void);
	
	void UpdatePointStruct(void);
	void UpdateVertexNeigborVertex(void);
	void UpdateVertexLaplacianCoordinate(void);
	
	void CalculateLaplacianToColor(void);
	void PointInTriange(MQImagePixel *p,int tri);

	void FindBoundary(void);
	void FindHole(void);

	void setTexture(int w , int h);
	void generateTexture(int size);
	void convertSample();
	void initializeTexture(int size);
	void findBestMatch(int i, int j, int size);

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
