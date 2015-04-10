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

	float X, Y, Z;
	float S, T;         //parametric coordinates
	float NX, NY, NZ;   //normal

	list<int>	NeighborVertex;
	float	LapX, LapY, LapZ;
	float  R,G,B; //Laplacian to Color
	float	Lap_length;

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
	float NX, NY, NZ;	//normal

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
	HalfEdge* oppositeHalfEdge;
	HalfEdge* nextHalfEdge;
	int  face;

	HalfEdge()
	{
		oppositeHalfEdge = NULL;
		nextHalfEdge = NULL;
		face = 0;
	}
	~HalfEdge(){}
};

struct MQImagePixel
{
	float X,Y;
	float LapX,LapY,LapZ;
	float R,G,B;
	float Lap_length;
	int Triangle;
	bool isHole;
	list<int> neighborHole;
	int position;

	vector<float> pca_data;

	MQImagePixel()
	{
		X = Y = 0.0;
		LapX = LapY = LapZ = 0.0;
		Lap_length = 0 ;
		R = G = B = 0.0;
		Triangle = 0;
		isHole = false;
		position = 0;
	}

	~MQImagePixel(){}

};

struct LaplaianLength
{
	bool pca;
	bool boundary;
	bool hole;
	vector<float> length;
	int X,Y;
	int neighbor_pca;

	LaplaianLength()
	{
		pca = false;
		boundary = false;
		hole= false;
		X = Y = 0;
		neighbor_pca = 0;
	}
	~LaplaianLength(){}
};


class MQTriangleMesh
{
public:

	int VertexNum;
	int TexcoordNum;
	int TriangleNum;
	int imageSize;
	int window_size;
	

	float minLap,maxLap;
	//vector<float> minLap,maxLap;
	float boundary;
	
	vector<MQVertex>   Vertex;
	vector<MQTriangle> Triangle;
	vector<MQTriangleTex> TriangleTex;
	vector<vector<MQImagePixel>> ImagePixel;
	vector<MQImagePixel*> HolePixels;

	vector<vector<LaplaianLength*>> LaplaianLengths; //PCA data
	vector<LaplaianLength*> tempLaplaianLength;
	int _SrcImgPixDim;
	
	pair<float,float> boundaryX;
	pair<float,float> boundaryY;
	pair<float,float> hole_boundaryX;
	pair<float,float> hole_boundaryY;
	pair<int,int> pixel_boundaryX;
	pair<int,int> pixel_boundaryY;

	map< pair<int,int>, HalfEdge* > Edges; // pair<EdgeStartVertex,EdgeEndVertex>

	list<list<int>> Holes;
	list<list<int>> Holes_uv;
	

	//fill hole data
	int sample_w, sample_h;
	void* sample_data;
	float** sample_red;
	float** sample_green;
	float** sample_blue;

	int texture_w, texture_h;
	void * texture_data;
	float** texture_red;
	float** texture_green;
	float** texture_blue;
	int** original_pos_x;
	int** original_pos_y;

	float** red;
	float** green;
	float** blue;
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
	void CheckHole(void);
	void FillHole(int method);

	void setTexture(int w , int h);
	void generateTexture(int size,int method);
	void convertSample();
	void initializeTexture(int size);
	void findBestMatch(int i, int j, int size);

public:

	MQTriangleMesh()
	{
		VertexNum = 0;
		TriangleNum = 0;
		imageSize = 512;
		window_size = 3;
		_SrcImgPixDim = 3;
	}

	virtual ~MQTriangleMesh()
	{}
};


#endif	//MQTriangleMesh_h
