#ifndef MQTriangleMesh_h
#define MQTriangleMesh_h

#include <list>
#include <vector>
#include <cstdlib>
#include <map>
#include <set>
#include <utility>
#include "MQOpenGL.h"
#include "Dtriangle.h"

using namespace std;

class MQVertex
{
public:

	float X, Y, Z;
	float S, T;         //parametric coordinates
	float NX, NY, NZ;   //normal
	int origin_index;
	int origin_uv_index;

	list<int>	NeighborVertex;
	float	LapX, LapY, LapZ;
	float   ori_LapX,ori_LapY,ori_LapZ;
	float  R,G,B; //Laplacian to Color
	float	Lap_length;
	bool evaluate;

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
		evaluate = false;
		origin_index = -1;
		origin_uv_index = -1;
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
	pair<int,int> uv_edge;
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
	float ori_LapX,ori_LapY,ori_LapZ;
	float R,G,B;
	float Lap_length;
	int Triangle;
	bool isHole;
	list<int> neighborHole;
	list<int> neighborLap;
	int position;
	int originX,originY;

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
		originX = originY = -1;
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

struct HoleMesh{
	vector<MQVertex> Vertex;
	vector<MQTriangleTex> TriangleTex;
	vector<int> boundary;
	int VertexNum;
	int TriangleNum;
	int inner_point;
	HoleMesh()
	{
		inner_point = 0;
	}
};


class MQTriangleMesh
{
public:

	string Filename;

	int VertexNum;
	int TexcoordNum;
	int TriangleNum;
	int imageSize;
	int window_size;
	int pca_size;
	int first_point,second_point;
	
	float minLap,maxLap;
	float boundary;
	
	bool draw_boundingbox,draw_boundary,draw_triangle,draw_laplacian;

	vector<int> fillpoint;
	vector<MQVertex>   Vertex;
	vector<MQTriangle> Triangle;
	vector<MQTriangleTex> TriangleTex;
	vector<vector<MQImagePixel>> ImagePixel;
	vector<MQImagePixel*> HolePixels;
	vector<MQImagePixel*> RotateLaplacianPixels;

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
	HoleMesh* baseMesh;
	
	//fill hole data
	int sample_w, sample_h;
	void* sample_data;
	float** sample_red;
	float** sample_green;
	float** sample_blue;
	float*** sample_lap; 

	//int texture_w, texture_h;
	void * texture_data;
	float** texture_red;
	float** texture_green;
	float** texture_blue;
	float*** texture_lap; 
	int** original_pos_x;
	int** original_pos_y;

	float** red;
	float** green;
	float** blue;
	float*** lap_sample; 
	vector<int> candidate_x;
	vector<int> candidate_y;


public:

	bool ReadObjFile(const char *FileName);
	void WriteObjFile();
	void Draw(GLubyte Red, GLubyte Green, GLubyte Blue);
	void Draw2D(void);
	void DrawPoint(void);
	
	void UpdateVertexNormal(void);
	void UpdatePointStruct(void);
	void UpdateVertexNeigborVertex(void);
	void UpdateVertexLaplacianCoordinate(void);
	
	void CalculateLaplacianToColor(void);
	void PointInTriange(MQImagePixel *p,int tri);

	void FindBoundary(void);
	void FindHole(void);
	void CheckHole(void);
	void FillHole(int method);
	void RotateLaplacian(void);

	void setTexture(int window);
	void generateTexture(int size,int method);
	void convertSample();
	void initializeTexture(int size);
	void findBestMatch(int i, int j, int size);
	void TriangulateBaseMesh();
	void RebuildingCoordination();

public:

	MQTriangleMesh()
	{
		VertexNum = 0;
		TriangleNum = 0;
		imageSize = 512;
		window_size = 5;
		pca_size = 5;
		_SrcImgPixDim = 3;
		first_point = second_point = -1;
		baseMesh = NULL;
	}

	virtual ~MQTriangleMesh()
	{}
};


#endif	//MQTriangleMesh_h
