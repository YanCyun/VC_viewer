#include "MQTriangleMesh.h"
#include "MQglm.h"
#include <iostream>
#include <math.h>
#include <algorithm> 
#include <time.h>

void _CalculateNormal(double v[3][3], double out[3])
{
	double v1[3], v2[3];
	int    x = 0, y = 1, z = 2;

	v1[x] = v[2][x] - v[1][x];
	v1[y] = v[2][y] - v[1][y];
	v1[z] = v[2][z] - v[1][z];

	v2[x] = v[0][x] - v[1][x];
	v2[y] = v[0][y] - v[1][y];
	v2[z] = v[0][z] - v[1][z];

	out[x] = v1[y]*v2[z] - v1[z]*v2[y];
	out[y] = v1[z]*v2[x] - v1[x]*v2[z];
	out[z] = v1[x]*v2[y] - v1[y]*v2[x];

	if( (fabs(out[x]) < 1.0e-18) && (fabs(out[y]) < 1.0e-18) && (fabs(out[z]) < 1.0e-18) )
	{
		out[x] = 1.0;
		out[y] = 0.0;
		out[z] = 0.0;
		return;
	}

	double length = sqrt( pow(out[x],2) + pow(out[y],2) + pow(out[z],2) );
	out[x] /= length;
	out[y] /= length;
	out[z] /= length;
}

void MQVertex::Normal2UnitVector()
{
	double length = sqrt( pow(this->NX,2) + pow(this->NY,2) + pow(this->NZ,2) );
	this->NX /= length;
	this->NY /= length;
	this->NZ /= length;
}

void MQTriangle::Normal2UnitVector()
{
	double length = sqrt( pow(this->NX,2) + pow(this->NY,2) + pow(this->NZ,2) );
	this->NX /= length;
	this->NY /= length;
	this->NZ /= length;
}

bool MQTriangleMesh::ReadObjFile(const char *FileName)
{
	

	MQGLMmodel *_model = MQglmReadOBJ(FileName);

	this->VertexNum = (int)(_model->numvertices);
	this->Vertex.resize( (this->VertexNum)+1 );
	for(int i = 1; i <= this->VertexNum; i++)
	{
		this->Vertex[i].X = _model->vertices[ 3*i ];
		this->Vertex[i].Y = _model->vertices[ 3*i+1 ];
		this->Vertex[i].Z = _model->vertices[ 3*i+2 ];
	}

	this->TexcoordNum = (int)(_model->numtexcoords);

	for(int i = 1; i <= this->TexcoordNum; i++)
	{
		this->Vertex[i].S = _model->texcoords[2*i];
		this->Vertex[i].T = _model->texcoords[2*i+1];
	}

	this->TriangleNum = (int)(_model->numtriangles);
	this->Triangle.resize( (this->TriangleNum)+1 );
	this->TriangleTex.resize((this->TriangleNum)+1);
	Edges.clear();
	for(int i = 1; i <= this->TriangleNum; i++)
	{
		this->Triangle[i].V1 = _model->triangles[i-1].vindices[0];
		this->Triangle[i].V2 = _model->triangles[i-1].vindices[1];
		this->Triangle[i].V3 = _model->triangles[i-1].vindices[2];

		this->TriangleTex[i].T1 = _model->triangles[i-1].tindices[0];
		this->TriangleTex[i].T2 = _model->triangles[i-1].tindices[1];
		this->TriangleTex[i].T3 = _model->triangles[i-1].tindices[2];
		
		//HalfEdge Data 
		//Edge1
		this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)] = new HalfEdge();
		this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->face = i;
		this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->nextHalfEdge = pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3);
		this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->oppositeHalfEdge = pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V1);
		//Edge2
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)] = new HalfEdge();
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->face = i;
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->nextHalfEdge = pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1);
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->oppositeHalfEdge = pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V2);
		//Edge3
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)] = new HalfEdge();
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->face = i;
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->nextHalfEdge = pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2);
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->oppositeHalfEdge = pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V3);
	}


	MQglmDelete(_model);

	//calculate normal
	struct _vector3{ double x; double y; double z; };
	vector<_vector3> *_NormalTable = new vector<_vector3>[ (this->VertexNum)+1 ];

	for(int i = 1; i <= this->TriangleNum; i++)
	{
		double v[3][3], out[3];
		v[0][0] = this->Vertex[this->Triangle[i].V1].X;
		v[0][1] = this->Vertex[this->Triangle[i].V1].Y;
		v[0][2] = this->Vertex[this->Triangle[i].V1].Z;
		v[1][0] = this->Vertex[this->Triangle[i].V2].X;
		v[1][1] = this->Vertex[this->Triangle[i].V2].Y;
		v[1][2] = this->Vertex[this->Triangle[i].V2].Z;
		v[2][0] = this->Vertex[this->Triangle[i].V3].X;
		v[2][1] = this->Vertex[this->Triangle[i].V3].Y;
		v[2][2] = this->Vertex[this->Triangle[i].V3].Z;

		_CalculateNormal(v, out);

		this->Triangle[i].NX = out[0];
		this->Triangle[i].NY = out[1];
		this->Triangle[i].NZ = out[2];

		_vector3 _inormal = { out[0], out[1], out[2] };
		_NormalTable[ this->Triangle[i].V1 ].push_back(_inormal);
		_NormalTable[ this->Triangle[i].V2 ].push_back(_inormal);
		_NormalTable[ this->Triangle[i].V3 ].push_back(_inormal);
	}

	for(int i = 1; i <= this->VertexNum; i++)
	{
		double _xsum = 0.0, _ysum = 0.0, _zsum = 0.0;

		for(unsigned int j = 0; j < _NormalTable[i].size(); j++)
		{
			_xsum += _NormalTable[i][j].x;
			_ysum += _NormalTable[i][j].y;
			_zsum += _NormalTable[i][j].z;
		}

		this->Vertex[i].NX = _xsum;
		this->Vertex[i].NY = _ysum;
		this->Vertex[i].NZ = _zsum;
		this->Vertex[i].Normal2UnitVector();
	}

	delete [] _NormalTable;

	clock_t t_start,t_end;
	t_start = clock();
	this->UpdateVertexNeigborVertex();
	this->UpdateVertexLaplacianCoordinate();
	this->CalculateLaplacianToColor();
	this->FindHole();
	this->FindBoundary();
	this->UpdatePointStruct();
	t_end = clock();
	
	double t_duration = (double)(t_end - t_start);
	printf("Read file finish %.3fs\n",t_duration/1000.0f);

	return true;
}

void MQTriangleMesh::UpdateVertexNeigborVertex(void)
{
	for(int i = 1; i <= this->VertexNum; i++)
		this->Vertex[i].NeighborVertex.clear();

	for(int i = 1; i <= this->TriangleNum; i++)
	{
		int v1 = this->Triangle[i].V1;
		int v2 = this->Triangle[i].V2;
		int v3 = this->Triangle[i].V3;

		this->Vertex[v1].NeighborVertex.push_back(v2);
		this->Vertex[v1].NeighborVertex.push_back(v3);
		this->Vertex[v2].NeighborVertex.push_back(v3);
		this->Vertex[v2].NeighborVertex.push_back(v1);
		this->Vertex[v3].NeighborVertex.push_back(v1);
		this->Vertex[v3].NeighborVertex.push_back(v2);
	}

	for(int i = 1; i <= this->VertexNum; i++)
	{
		this->Vertex[i].NeighborVertex.sort();
		this->Vertex[i].NeighborVertex.unique();
	}
}

void MQTriangleMesh::UpdateVertexLaplacianCoordinate(void)
{
	list<int>::iterator it, begin, end;
	minLap = maxLap = 0.0;
	
	for(int i = 1; i <= this->VertexNum; i++)
	{
		double sumX = 0.0, sumY = 0.0, sumZ = 0.0;

		begin = this->Vertex[i].NeighborVertex.begin();
		end = this->Vertex[i].NeighborVertex.end();
		for(it = begin; it != end; it++)
		{
			sumX += this->Vertex[*it].X;
			sumY += this->Vertex[*it].Y;
			sumZ += this->Vertex[*it].Z;
		}

		double degree = this->Vertex[i].NeighborVertex.size();

		double mCenterX = sumX/degree;
		double mCenterY = sumY/degree;
		double mCenterZ = sumZ/degree;

		this->Vertex[i].LapX = this->Vertex[i].X - mCenterX;
		this->Vertex[i].LapY = this->Vertex[i].Y - mCenterY;
		this->Vertex[i].LapZ = this->Vertex[i].Z - mCenterZ;

		this->Vertex[i].Lap_length = sqrt(pow(this->Vertex[i].LapX,2)+pow(this->Vertex[i].LapY,2)+pow(this->Vertex[i].LapZ,2));

		//Find Laplacian  max & min , use for CalculateLaplacianToColor
		double min_lap = min(Vertex[i].LapX,min(Vertex[i].LapY,Vertex[i].LapZ));
		double max_lap = max(Vertex[i].LapX,max(Vertex[i].LapY,Vertex[i].LapZ));

		if(i == 1){ 
			minLap = maxLap = this->Vertex[i].Lap_length;
			//minLap = min_lap;
			//maxLap = max_lap;
		}
		else{
			if(minLap > this->Vertex[i].Lap_length) minLap = this->Vertex[i].Lap_length;
			if(maxLap < this->Vertex[i].Lap_length) maxLap = this->Vertex[i].Lap_length;
			//if(minLap > min_lap)  minLap = min_lap;
			//if(maxLap < max_lap)  maxLap = max_lap;
		}
	}
}

void MQTriangleMesh::UpdatePointStruct(void){

	//set start coordinate in left-top
	double centerX = (boundaryX.first+boundaryX.second)/2;
	double centerY = (boundaryY.first+boundaryY.second)/2;
	double startX = centerX - boundary/2;
	double startY = centerY + boundary/2;

	double maxLap_img = 0;
	double minLap_img = 0;

	//set each point distance 
	double pointDistance = boundary/imageSize;

	int tri;

	bool checkHole = false;
	vector<int> tempHoles;
	vector<int>::iterator hole_it;
	
	this->ImagePixel.resize(imageSize);

	for(int i = 0 ; i < imageSize ; i++){
		this->ImagePixel[i].clear();
		this->ImagePixel[i].resize(imageSize);
		tempHoles.clear();
		checkHole = false;
		for(int j = 0 ; j < imageSize ; j++){
			//set each pixel coordinate
			this->ImagePixel[i][j].X = startX + j*pointDistance;
			this->ImagePixel[i][j].Y = startY - i*pointDistance;
			
			if(j > 0) tri =  this->ImagePixel[i][j-1].Triangle;
			else{
				tri = 0;
			}
			this->PointInTriange(&this->ImagePixel[i][j],tri);
			
			//if this pixel in the triangle 
			if(this->ImagePixel[i][j].Triangle != 0){
				//get hole pixels
				if(checkHole){
					for(hole_it = tempHoles.begin();hole_it != tempHoles.end() ; hole_it++)
						ImagePixel[i][*hole_it].isHole = true;
					tempHoles.clear();
				}
				else{
					checkHole = true;
				}	
				//get pixel laplacian range
				double max_lap = max(ImagePixel[i][j].LapX,max(ImagePixel[i][j].LapY,ImagePixel[i][j].LapZ));
				double min_lap = min(ImagePixel[i][j].LapX,min(ImagePixel[i][j].LapY,ImagePixel[i][j].LapZ));
				if(maxLap_img == 0 && minLap_img ==0){
					maxLap_img = minLap_img = ImagePixel[i][j].Lap_length;
					//maxLap_img = max_lap;
					//minLap_img = min_lap;
				}
				else{
					if(minLap_img > ImagePixel[i][j].Lap_length) minLap_img = ImagePixel[i][j].Lap_length;
					if(maxLap_img < ImagePixel[i][j].Lap_length) maxLap_img = ImagePixel[i][j].Lap_length;
					//if(minLap_img > min_lap)  minLap_img = min_lap;
					//if(maxLap_img < max_lap)  maxLap_img = max_lap;
				}

			}
			else{
				if(this->ImagePixel[i][j].X >= hole_boundaryX.first && this->ImagePixel[i][j].X <= hole_boundaryX.second &&
					this->ImagePixel[i][j].Y >= hole_boundaryY.first && this->ImagePixel[i][j].Y <= hole_boundaryY.second ){
						if(checkHole) tempHoles.push_back(j);
						//ImagePixel[i][j].isHole = true;
				}
			}
		}
	}

	
	double normalize_number = 255.0 / (maxLap_img-minLap_img);
	
	for(int i = 0 ; i < imageSize ; i++){
		for(int j = 0 ; j < imageSize ; j++){
			if(this->ImagePixel[i][j].isHole){
				//hole's index in ImagePixel; 2d to 1d
				ImagePixel[i][j].position = i*imageSize + j;
				this->HolePixels.push_back(ImagePixel[i][j]);					
				for(int y = i-1 ; y <= i+1 ;y++){
					for(int x = j-1 ; x <= j+1 ; x++){
						if(y == i && x == j) continue;
						if(this->ImagePixel[y][x].isHole){
							this->ImagePixel[i][j].neighborHole.push_back(y*imageSize+x);
						}
					}
				}
			}
			if(this->ImagePixel[i][j].Triangle == 0) continue;
			//laplacian to color 
			this->ImagePixel[i][j].R = (this->ImagePixel[i][j].Lap_length - minLap_img) *  normalize_number;
			//this->ImagePixel[i][j].R = (this->ImagePixel[i][j].LapX - minLap_img) *  normalize_number;
			//this->ImagePixel[i][j].G = (this->ImagePixel[i][j].LapY - minLap_img) *  normalize_number;
			//this->ImagePixel[i][j].B = (this->ImagePixel[i][j].LapZ - minLap_img) *  normalize_number;
		}
	}
	//sort by hole pixel neighborhood amount
	sort(this->HolePixels.begin(),this->HolePixels.end());

	//Find hole pixel bounding box coordinate
	vector<MQImagePixel>::iterator holepixel_it;
	for(holepixel_it = HolePixels.begin();holepixel_it != HolePixels.end();holepixel_it++){
		int x = holepixel_it->position % imageSize;
		int y = holepixel_it->position / imageSize;
		if(holepixel_it == HolePixels.begin()){
			pixel_boundaryX.first = pixel_boundaryX.second = x;
			pixel_boundaryY.first = pixel_boundaryY.second = y;
		}
		else{
			if(pixel_boundaryX.first > x) pixel_boundaryX.first = x;
			if(pixel_boundaryX.second < x) pixel_boundaryX.second = x;
			if(pixel_boundaryY.first > y) pixel_boundaryY.first = y;
			if(pixel_boundaryY.second < y) pixel_boundaryY.second = y;
		}
	}
	

}

void MQTriangleMesh::FillHole(int method){

	int window_size = 11;
	this->convertSample();
	this->setTexture(imageSize,imageSize);
	this->generateTexture(window_size,method);
	printf("Fill Done\n\n");

}

void MQTriangleMesh::PointInTriange(MQImagePixel *p,int tri)
{

	double a1,a2,a3;
	double sum_a;
	//if neighborhood pixel in any triangle
	if(tri != 0){
		a1 = (Vertex[TriangleTex[tri].T1].S - p->X) * (Vertex[TriangleTex[tri].T2].T - p->Y) - (Vertex[TriangleTex[tri].T1].T - p->Y) * (Vertex[TriangleTex[tri].T2].S - p->X);//x1y1-y1y2
		a2 = (Vertex[TriangleTex[tri].T2].S - p->X) * (Vertex[TriangleTex[tri].T3].T - p->Y) - (Vertex[TriangleTex[tri].T2].T - p->Y) * (Vertex[TriangleTex[tri].T3].S - p->X);
		a3 = (Vertex[TriangleTex[tri].T3].S - p->X) * (Vertex[TriangleTex[tri].T1].T - p->Y) - (Vertex[TriangleTex[tri].T3].T - p->Y) * (Vertex[TriangleTex[tri].T1].S - p->X);

		if(a1>=0 && a2>=0 && a3>=0){

			p->Triangle = tri;
			sum_a = a1+a2+a3;
			p->Lap_length = Vertex[Triangle[tri].V1].Lap_length * a2/sum_a + Vertex[Triangle[tri].V2].Lap_length * a3/sum_a + Vertex[Triangle[tri].V3].Lap_length * a1/sum_a;
			p->LapX = Vertex[Triangle[tri].V1].LapX * a2/sum_a + Vertex[Triangle[tri].V2].LapX * a3/sum_a + Vertex[Triangle[tri].V3].LapX * a1/sum_a; //v1*(a2/(a1+a2+a3)) + v2*(a3/(a1+a2+a3)) + v3 + (a3/(a1+a2+a3)) 
			p->LapY = Vertex[Triangle[tri].V1].LapY * a2/sum_a + Vertex[Triangle[tri].V2].LapY * a3/sum_a + Vertex[Triangle[tri].V3].LapY * a1/sum_a;
			p->LapZ = Vertex[Triangle[tri].V1].LapZ * a2/sum_a + Vertex[Triangle[tri].V2].LapZ * a3/sum_a + Vertex[Triangle[tri].V3].LapZ * a1/sum_a;
			return;

		}
	}

	for(int i = 1 ; i <= this->TriangleNum ; i++){
		
		a1 = (Vertex[TriangleTex[i].T1].S - p->X) * (Vertex[TriangleTex[i].T2].T - p->Y) - (Vertex[TriangleTex[i].T1].T - p->Y) * (Vertex[TriangleTex[i].T2].S - p->X);//x1y1-y1y2
		if(a1 < 0) continue;
		a2 = (Vertex[TriangleTex[i].T2].S - p->X) * (Vertex[TriangleTex[i].T3].T - p->Y) - (Vertex[TriangleTex[i].T2].T - p->Y) * (Vertex[TriangleTex[i].T3].S - p->X);
		if(a2 < 0) continue;
		a3 = (Vertex[TriangleTex[i].T3].S - p->X) * (Vertex[TriangleTex[i].T1].T - p->Y) - (Vertex[TriangleTex[i].T3].T - p->Y) * (Vertex[TriangleTex[i].T1].S - p->X);
		if(a3 < 0) continue;

		p->Triangle = i;

		sum_a = a1+a2+a3;
		p->Lap_length = Vertex[Triangle[i].V1].Lap_length * a2/sum_a + Vertex[Triangle[i].V2].Lap_length * a3/sum_a + Vertex[Triangle[i].V3].Lap_length * a1/sum_a;
		p->LapX = Vertex[Triangle[i].V1].LapX * a2/sum_a + Vertex[Triangle[i].V2].LapX * a3/sum_a + Vertex[Triangle[i].V3].LapX * a1/sum_a; //v1*(a2/(a1+a2+a3)) + v2*(a3/(a1+a2+a3)) + v3 + (a3/(a1+a2+a3)) 
		p->LapY = Vertex[Triangle[i].V1].LapY * a2/sum_a + Vertex[Triangle[i].V2].LapY * a3/sum_a + Vertex[Triangle[i].V3].LapY * a1/sum_a;
		p->LapZ = Vertex[Triangle[i].V1].LapZ * a2/sum_a + Vertex[Triangle[i].V2].LapZ * a3/sum_a + Vertex[Triangle[i].V3].LapZ * a1/sum_a;

		return ;

	}
	p->Triangle = 0;

}

void MQTriangleMesh::CalculateLaplacianToColor(void)
{
	double normalize_number = 255.0 / (maxLap-minLap);
	for(int i = 1; i <= this->VertexNum; i++){	

		this->Vertex[i].R = (this->Vertex[i].Lap_length - minLap) *  normalize_number;
		//this->Vertex[i].R = (this->Vertex[i].LapX - minLap) *  normalize_number;
		//this->Vertex[i].G = (this->Vertex[i].LapY - minLap) *  normalize_number;
		//this->Vertex[i].B = (this->Vertex[i].LapZ - minLap) *  normalize_number;
	}

}

void MQTriangleMesh::FindBoundary(void)
{
	//Find uv bounding box
	for(int i = 1 ; i <= this->TexcoordNum ; i++){
		if(i == 1){
			boundaryX.first = boundaryX.second = this->Vertex[i].S;
			boundaryY.first = boundaryY.second = this->Vertex[i].T;
		}
		else{
			if(boundaryX.first > this->Vertex[i].S) boundaryX.first = this->Vertex[i].S;
			if(boundaryX.second < this->Vertex[i].S) boundaryX.second = this->Vertex[i].S;
			if(boundaryY.first > this->Vertex[i].T) boundaryY.first = this->Vertex[i].T;
			if(boundaryY.second < this->Vertex[i].T) boundaryY.second = this->Vertex[i].T;
		}
	}
	//scale bounding box x1.05
	boundary = max(boundaryX.second-boundaryX.first,boundaryY.second-boundaryY.first) *1.05;


	//Find uv hole bounding box
	if(Holes.size() > 0){
		list<int>::iterator hole_it;
		list<list<int>>::iterator begin = Holes_uv.begin();
		for(hole_it = begin->begin(); hole_it != begin->end();hole_it++){
			if(hole_it == begin->begin()){
				hole_boundaryX.first = hole_boundaryX.second = this->Vertex[*hole_it].S;
				hole_boundaryY.first = hole_boundaryY.second = this->Vertex[*hole_it].T;
			}
			else{
				if(hole_boundaryX.first > this->Vertex[*hole_it].S) hole_boundaryX.first = this->Vertex[*hole_it].S;
				if(hole_boundaryX.second < this->Vertex[*hole_it].S) hole_boundaryX.second = this->Vertex[*hole_it].S;
				if(hole_boundaryY.first > this->Vertex[*hole_it].T) hole_boundaryY.first = this->Vertex[*hole_it].T;
				if(hole_boundaryY.second < this->Vertex[*hole_it].T) hole_boundaryY.second = this->Vertex[*hole_it].T;
			}
		}
	}
	

}

void MQTriangleMesh::FindHole(void)
{
	printf("---------------Hole Info---------------\n");

	map<int,int>  singleEdge ;
	map<int,int>  singleEdge_uv;

	//Find Single Edge
	for(int i = 1; i <= this->TriangleNum; i++){				
		if(!this->Edges[this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->oppositeHalfEdge])	{
			singleEdge[this->Triangle[i].V2] = this->Triangle[i].V1;
			singleEdge_uv[this->TriangleTex[i].T2] = this->TriangleTex[i].T1;
		}
		if(!this->Edges[this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->oppositeHalfEdge]){
			singleEdge[this->Triangle[i].V3] = this->Triangle[i].V2;
			singleEdge_uv[this->TriangleTex[i].T3] = this->TriangleTex[i].T2;
		}
		if(!this->Edges[this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->oppositeHalfEdge]){
			singleEdge[this->Triangle[i].V1] = this->Triangle[i].V3;
			singleEdge_uv[this->TriangleTex[i].T1] = this->TriangleTex[i].T3;
		}
	}

	//Attach Edge & Find Hole 
	map<int,int>::iterator it;
	list<int> hole;
	int fist_point,search_point,temp;
	Holes.clear();
	while(!singleEdge.empty()){
		it = singleEdge.begin();
		fist_point = it->first;  //hole start point
		hole.push_back(fist_point);
		search_point = singleEdge[fist_point];
		singleEdge.erase(fist_point);
		//attach edge 
		while(search_point != fist_point){
			hole.push_back(search_point);
			temp = search_point;
			search_point = singleEdge[search_point];
			singleEdge.erase(temp);			
		}
		Holes.push_back(hole);	
		hole.clear();
	}
	list<int> hole_uv;
	Holes_uv.clear();
	while(!singleEdge_uv.empty()){
		it = singleEdge_uv.begin();
		fist_point = it->first;  //hole start point
		hole_uv.push_back(fist_point);
		search_point = singleEdge_uv[fist_point];
		singleEdge_uv.erase(fist_point);
		//attach edge 
		while(search_point != fist_point){
			hole_uv.push_back(search_point);
			temp = search_point;
			search_point = singleEdge_uv[search_point];
			singleEdge_uv.erase(temp);			
		}
		Holes_uv.push_back(hole_uv);	
		hole_uv.clear();
	}

	vector<double> hole_length;
	//list<int> hole_point;
	list<list<int>>::iterator hole_it;
	list<list<int>>::iterator hole_uv_it;
	list<int>::iterator point_it;
	//calculate length with each hole
	for(hole_uv_it=Holes_uv.begin();hole_uv_it!= Holes_uv.end();hole_uv_it++){
		double start_s = -1.0;
		double start_t = -1.0;
		double length = 0.0;
		for(point_it = hole_uv_it->begin();point_it != hole_uv_it->end();point_it++){
			if(start_s == -1.0){
				start_s = Vertex[*point_it].S;
				start_t = Vertex[*point_it].T;
			}
			else{
				length += sqrt(pow(Vertex[*point_it].S - start_s,2) + pow(Vertex[*point_it].T - start_t,2));
				start_s = Vertex[*point_it].S;
				start_t = Vertex[*point_it].T;
			}
		}
		length += sqrt(pow(Vertex[*hole_uv_it->begin()].S - start_s,2) + pow(Vertex[*hole_uv_it->begin()].T - start_t,2));
		hole_length.push_back(length);
	}
	//find maximum lenth hole and erase it;
	double max_hole = *max_element(hole_length.begin(),hole_length.end()); 
	vector<double>::iterator length_it;
	for(hole_it = Holes.begin(),hole_uv_it = Holes_uv.begin(),length_it = hole_length.begin();length_it != hole_length.end();hole_it++,hole_uv_it++,length_it++){
		if(*length_it == max_hole){
			Holes.erase(hole_it);
			Holes_uv.erase(hole_uv_it);
			break;
		}
	}

	printf("Holes size:%d\n",Holes.size());
	printf("Holes_uv size:%d\n",Holes_uv.size());
	//print hole point
	for(hole_it=Holes.begin();hole_it!= Holes.end();hole_it++){
		printf("Hole point size:%d\n",hole_it->size());
		for(point_it = hole_it->begin();point_it != hole_it->end();point_it++){
			if(point_it != hole_it->begin()) printf(",");
			printf("%d",*point_it);
		}
		printf("\n");
	}
	for(hole_uv_it=Holes_uv.begin();hole_uv_it!= Holes_uv.end();hole_uv_it++){
		printf("Hole_uv point size:%d\n",hole_uv_it->size());
		for(point_it = hole_uv_it->begin();point_it != hole_uv_it->end();point_it++){
			if(point_it != hole_uv_it->begin()) printf(",");
			printf("%d",*point_it);
		}
		printf("\n");
	}
	printf("---------------------------------------\n");
}

void MQTriangleMesh::setTexture(int w,int h){

	texture_w = w;
	texture_h = h;
	texture_red = new double* [texture_w];
	texture_green = new double* [texture_w];
	texture_blue = new double* [texture_w];
	original_pos_x = new int* [texture_w];
	original_pos_y = new int* [texture_w];
	for(int i=0; i<texture_w; i++)
	{
		texture_red[i] = new double[texture_h];
		texture_green[i] = new double[texture_h];
		texture_blue[i] = new double[texture_h];
		original_pos_x[i] = new int[texture_w];
		original_pos_y[i] = new int[texture_w];
	}
}

void MQTriangleMesh::generateTexture(int size,int method)//method 1:隨機 2:Boounding Box 3:取鄰居最多 4:成長式
	// generate the texture from the sample using a search window of size x size
{
	int i, j, a=0;

	cout<<"\nInitializing texture...";
	initializeTexture(size);
	cout<<"done\n";

	red = new double*[size];
	green = new double*[size];
	blue = new double*[size];
	for(int y=0; y<size; y++)
	{
		red[y] = new double[size];
		blue[y] = new double[size];
		green[y] = new double[size];
	}

	
	
	if(method == 1){//隨機
		cout<<"Start fill hole for random...\n";
		list<int>::iterator it;
		int index = 0;
		MQImagePixel temp;
		while(HolePixels.size()> 0){
			index = rand()%HolePixels.size();
			temp = HolePixels[index];
			while(temp.neighborHole.size() == 8){
				index = rand()%HolePixels.size();
				temp = HolePixels[index];
			}
			i = (int)temp.position / imageSize;
			j = (int)temp.position % imageSize;
			//printf("%d,%d\n",i,j);
			findBestMatch(j, i, size);
		
			ImagePixel[i][j].R = texture_red[i][j];
			ImagePixel[i][j].G = texture_green[i][j];
			ImagePixel[i][j].B = texture_blue[i][j];

			for(it = temp.neighborHole.begin(); it !=temp.neighborHole.end();it++){
				ImagePixel[int(*it/imageSize)][int(*it%imageSize)].neighborHole.remove(temp.position);
			}
			HolePixels.erase(HolePixels.begin()+index);
			sort(this->HolePixels.begin(),this->HolePixels.end());
		}
	}
	if(method == 2){//bounding box
		cout<<"Start fill hole for bounding box...\n";
		int startX = (int)pixel_boundaryX.first;
		int endX = (int)pixel_boundaryX.second;
		int startY = (int)pixel_boundaryY.first;
		int endY = (int)pixel_boundaryY.second;
		int dir = 1;
		int fix;
		bool useX = true;
		bool useY = false;
		while(startX <= endX && startY <= endY){
			if(useX){
				if(dir>0){//left
					fix = startY;
					for(int j = startX; j <= endX ; j+=dir){
						if(ImagePixel[fix][j].isHole){
							findBestMatch(j, fix, size);
							ImagePixel[fix][j].R = texture_red[fix][j];
							ImagePixel[fix][j].G = texture_green[fix][j];
							ImagePixel[fix][j].B = texture_blue[fix][j];
						}
					}
				}
				else{//right
					fix = endY;
					for(int j = endX; j >= startX ; j+=dir){
						if(ImagePixel[fix][j].isHole){
							findBestMatch(j, fix, size);
							ImagePixel[fix][j].R = texture_red[fix][j];
							ImagePixel[fix][j].G = texture_green[fix][j];
							ImagePixel[fix][j].B = texture_blue[fix][j];
						}
					}
				}
				if(dir > 0) startY += 1;
				if(dir < 0) endY -= 1 ;

				useX = false;
				useY = true;
			}
			if(useY){
				if(dir>0){//down
					fix = endX;
					for(int i = startY; i <= endY ; i+=dir){
						if(ImagePixel[i][fix].isHole){
							findBestMatch(fix, i, size);
							ImagePixel[i][fix].R = texture_red[i][fix];
							ImagePixel[i][fix].G = texture_green[i][fix];
							ImagePixel[i][fix].B = texture_blue[i][fix];
						}
					}
				}
				else{//up
					fix = startX;
					for(int i = endY; i >= startY ; i+=dir){
						if(ImagePixel[i][fix].isHole){
							findBestMatch(fix, i, size);
							ImagePixel[i][fix].R = texture_red[i][fix];
							ImagePixel[i][fix].G = texture_green[i][fix];
							ImagePixel[i][fix].B = texture_blue[i][fix];
						}
					}
				}
			
				if(dir > 0) endX -= 1;
				if(dir < 0) startX += 1 ;

				useX = true;
				useY = false;
				dir *= -1;
			}
		}
	}
	if(method == 3){//取鄰居最多像素
		cout<<"\nStart fill hole for most pixel neighborhood...";
		list<int>::iterator it;
		MQImagePixel temp;
		while(HolePixels.size()> 0){
			temp = HolePixels[0];

			i = temp.position / imageSize;
			j = temp.position % imageSize;

			findBestMatch(j, i, size);

			ImagePixel[i][j].R = texture_red[i][j];
			ImagePixel[i][j].G = texture_green[i][j];
			ImagePixel[i][j].B = texture_blue[i][j];

			for(it = temp.neighborHole.begin(); it !=temp.neighborHole.end();it++){
				ImagePixel[int(*it/imageSize)][int(*it%imageSize)].neighborHole.remove(temp.position);
			}
			HolePixels.erase(HolePixels.begin());
			sort(this->HolePixels.begin(),this->HolePixels.end());
		}
	}
	if(method == 4){//成長式
		cout<<"Start fill hole for growth...\n";
		for(i=0; i<texture_h; i++)
		{
			for(j=0; j<texture_w; j++)
			{		
				if(ImagePixel[i][j].isHole && ImagePixel[i][j].R == 0){	

					findBestMatch(j, i, size);
					ImagePixel[i][j].R = texture_red[i][j];
					ImagePixel[i][j].G = texture_green[i][j];
					ImagePixel[i][j].B = texture_blue[i][j];

					bool filldone = false;
					int temp_x = j;
					int temp_y = i;
					while(true){
						for(int y = temp_y-1;y <= temp_y+1;y++){
							for(int x = temp_x-1;x<= temp_x+1;x++){
								if(ImagePixel[y][x].isHole && ImagePixel[y][x].R == 0){
									findBestMatch(x, y, size);
									ImagePixel[y][x].R = texture_red[y][x];
									ImagePixel[y][x].G = texture_green[y][x];
									ImagePixel[y][x].B = texture_blue[y][x];
									temp_x = x;
									temp_y = y;
									filldone = true;
									break;
								}
							}
							if(filldone) break;
						}
						if(filldone) filldone = false;
						else break;
					}
				}
			}
		}
	}
	return;
}

void MQTriangleMesh::initializeTexture(int size)
	// initialize output texture with random pixels from the sample
{
	int i, j;
	int w, h;

	int valid_w_length = sample_w-size+1;
	int valid_h_length = sample_h-size/2;
	int dw = size/2;
	int dh = size/2;

	for (i=0; i<texture_h; i++)
	{
		for(j=0; j<texture_w; j++)
		{
			if(ImagePixel[i][j].isHole){
				w = rand() % valid_w_length + dw;
				h = rand() % valid_h_length + dh;
				while(sample_red[w][h] == -1){
					w = rand() % valid_w_length + dw;
					h = rand() % valid_h_length + dh;
				}
				texture_red[i][j] = sample_red[h][w];
				texture_green[i][j] = sample_green[h][w];
				texture_blue[i][j] = sample_blue[h][w];
				original_pos_x[i][j] = w;
				original_pos_y[i][j] = h;
			}
			else{
				texture_red[i][j] = sample_red[i][j];
				texture_green[i][j] = sample_green[i][j];
				texture_blue[i][j] = sample_blue[i][j];
				original_pos_x[i][j] = j;
				original_pos_y[i][j] = i;
			}
		}
	}
	return;
}

void MQTriangleMesh::convertSample()
// convert the sample from the stream of bytes to 2d array of pixels
{
	int i, j;

	sample_w = imageSize;
	sample_h = imageSize;
	sample_red = new double*[imageSize];
	sample_green = new double*[imageSize];
	sample_blue = new double*[imageSize];
	for(i=0; i<imageSize; i++)
	{
		sample_red[i] = new double[imageSize];
		sample_green[i] = new double[imageSize];
		sample_blue[i] = new double[imageSize];
	}

	for (i=0; i<imageSize; i++)
	{
		for(j=0; j<imageSize; j++)
		{
			if(ImagePixel[i][j].Triangle == 0){
				sample_red[i][j] = sample_green[i][j] = sample_blue[i][j] = -1.0;
			} 
			else{
				sample_red[i][j] = ImagePixel[i][j].R;
				sample_green[i][j] = ImagePixel[i][j].G;
				sample_blue[i][j] = ImagePixel[i][j].B;
			}
		}
	}
	return;
}

void MQTriangleMesh::findBestMatch(int j, int i, int size)
// find the best match for the texture image at pixel (i, j) using a window of size x size
{
	int actualw, actualh;
	int actualx, actualy;
	int bestw, besth;
	int bestd = 200000*size*size;
	int tempd;
	int ti, tj;
	int r, g, b;
	int x, y;
	bool add;

	//make local texture window
	for(y=0, ti = i-size/2; y < size; y++, ti++)
	{
		for(x=0, tj = j-size/2; x < size; x++, tj++)
		{
			if(ti < 0)
				ti += texture_h;
			else if(ti >= texture_h)
				ti -= texture_h;
			if(tj < 0)
				tj += texture_w;
			else if(tj >= texture_w)
				tj -= texture_w;
			red[y][x] = texture_red[ti][tj];
			green[y][x] = texture_green[ti][tj];
			blue[y][x] = texture_blue[ti][tj];
		}
	}
	//get candidates;
	candidate_x.clear();
	candidate_y.clear();
	for(actualh = i-size/2; actualh <= i+size/2; actualh++)
	{
		for(actualw = j-size/2; actualw <= j+size/2; actualw++)
		{
			x = (actualw+texture_w)%texture_w;
			y = (actualh+texture_h)%texture_h;
			
			if(texture_red[y][x] == -1.0) continue;
			actualx = original_pos_x[y][x];
			actualy = original_pos_y[y][x];
			
			add = true;
			for(size_t c=0; c<candidate_x.size(); c++)
			{
				if(candidate_x[c] == actualx && candidate_y[c] == actualy)
					// already on candidate list
				{
					add = false;
					break;
				}
			}
			if(add)
			{
				candidate_x.push_back(actualx);
				candidate_y.push_back(actualy);
				
			}
		}
	}
	for(size_t c=0; c<candidate_x.size(); c++)
	{
		
		tempd = 0;
		
		for(ti=0; ti < size; ti++)
		{
			for(tj=0; tj<size; tj++)
			{
				if(tempd > bestd)	break;
				if(red[ti][tj] == -1) continue;
				r = int(red[ti][tj]-sample_red[candidate_y[c]][candidate_x[c]]);
				g = int(green[ti][tj]-sample_green[candidate_y[c]][candidate_x[c]]);
				b = int(blue[ti][tj]-sample_blue[candidate_y[c]][candidate_x[c]]);
				tempd += r*r + g*g + b*b;
			}
		}
		if(tempd < bestd)
		{
			bestw = candidate_x[c];
			besth = candidate_y[c];
			bestd = tempd;
			
		}
	}
	texture_red[i][j] = sample_red[besth][bestw];
	texture_green[i][j] = sample_green[besth][bestw];
	texture_blue[i][j] = sample_blue[besth][bestw];
	original_pos_x[i][j] = bestw;
	original_pos_y[i][j] = besth;
	return;
	/*
	// make local texture window
	for(y=0, ti = i-size/2; y < size/2+1; y++, ti++)
	{
		for(x=0, tj = j-size/2; x < size; x++, tj++)
		{
			// break if center of window reached (end of causal neighborhood)
			if(y==size/2 && x==size/2)
				break;
			// otherwise
			if(ti < 0)
				ti += texture_h;
			else if(ti >= texture_h)
				ti -= texture_h;
			if(tj < 0)
				tj += texture_w;
			else if(tj >= texture_w)
				tj -= texture_w;
			red[y][x] = texture_red[ti][tj];
			green[y][x] = texture_green[ti][tj];
			blue[y][x] = texture_blue[ti][tj];
		}
	}
	// get candidates;
	candidate_x.clear();
	candidate_y.clear();
	for(actualh = i-size/2; actualh <= i; actualh++)
	{
		for(actualw = j-size/2; actualw <= j+size/2; actualw++)
		{
			if(actualw==j && actualh==i)
			{
				actualh = i+1;
				break;
			}
			// make sure that x and y are within the bounds of the texture array
			x = (actualw+texture_w)%texture_w;
			y = (actualh+texture_h)%texture_h;
			// get the coordinates for the pixel in the sample image from which
			// the texture pixel came from and shift appropriately

			actualx = original_pos_x[y][x]+j-actualw;
			actualy = original_pos_y[y][x]+i-actualh;
			if(sample_red[actualy][actualx] == -1){
				actualx = original_pos_x[y][x]-(j-actualw);
				actualy = original_pos_y[y][x]-(i-actualh);
			}

			// check if neighborhood of candidate lies completely in sample
			if(actualx < size/2 || actualx >= sample_w-size/2 || actualy < size/2 || actualy >= sample_h-size/2)
			{
				//replace with random
				actualx = rand()%(sample_w-size+1)+size/2;
				actualy = rand()%(sample_h-size/2)+size/2;

			}
			while(sample_red[actualy][actualx] == -1){
				actualx = rand()%(sample_w-size+1)+size/2;
				actualy = rand()%(sample_h-size/2)+size/2;
			}
			add = true;
			for(size_t c=0; c<candidate_x.size(); c++)
			{
				if(candidate_x[c] == actualx && candidate_y[c] == actualy)
					// already on candidate list
				{
					add = false;
					break;
				}
			}
			if(add)
			{
				candidate_x.push_back(actualx);
				candidate_y.push_back(actualy);
			}
		}
	}
	for(size_t c=0; c<candidate_x.size(); c++)
	{
		tempd = 0;
		for(y=candidate_y[c]-size/2, ti=0; y<candidate_y[c]+1; y++, ti++)
		{
			for(x=candidate_x[c]-size/2, tj=0; x<candidate_x[c]+size/2+1; x++, tj++)
			{
				if(y==candidate_y[c] && x==candidate_x[c] || tempd > bestd)
				{
					y = candidate_y[c]+1;
					break;
				}
				r = int(red[ti][tj]-sample_red[y][x]);
				g = int(green[ti][tj]-sample_green[y][x]);
				b = int(blue[ti][tj]-sample_blue[y][x]);
				tempd += r*r + g*g + b*b;
			}
		}
		if(tempd < bestd)
		{
			bestw = candidate_x[c];
			besth = candidate_y[c];
			bestd = tempd;
		}
	}

	texture_red[i][j] = sample_red[besth][bestw];
	texture_green[i][j] = sample_green[besth][bestw];
	texture_blue[i][j] = sample_blue[besth][bestw];
	original_pos_x[i][j] = bestw;
	original_pos_y[i][j] = besth;
	return;*/
}

void MQTriangleMesh::Draw(GLubyte Red, GLubyte Green, GLubyte Blue)
{
	
	if(Holes.size()>0){
		glBegin(GL_LINES);
		list<int>::iterator hole_it;
		list<list<int>>::iterator begin = Holes.begin();
		for(hole_it = begin->begin(); hole_it != begin->end();hole_it++){

			if(hole_it == begin->begin()){
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(0,0,0);
				glVertex3f(this->Vertex[*hole_it].X, this->Vertex[*hole_it].Y,this->Vertex[*hole_it].Z);
			}
			else{
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(0,0,0);
				glVertex3f(this->Vertex[*hole_it].X, this->Vertex[*hole_it].Y,this->Vertex[*hole_it].Z);
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(0,0,0);
				glVertex3f(this->Vertex[*hole_it].X, this->Vertex[*hole_it].Y,this->Vertex[*hole_it].Z);
			}
		}
		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,0);
		glVertex3f(this->Vertex[*begin->begin()].X, this->Vertex[*begin->begin()].Y,this->Vertex[*begin->begin()].Z);
		glEnd();
	}
	glColor3ub(Red, Green, Blue);
	glBegin(GL_TRIANGLES);
	for(int i = 1; i <= this->TriangleNum; i++)
	{
		int v1 = this->Triangle[i].V1;
		int v2 = this->Triangle[i].V2;
		int v3 = this->Triangle[i].V3;

		int t1 = this->TriangleTex[i].T1;
		int t2 = this->TriangleTex[i].T2;
		int t3 = this->TriangleTex[i].T3;

		glNormal3f(this->Vertex[v1].NX, this->Vertex[v1].NY, this->Vertex[v1].NZ);
		glTexCoord2f(this->Vertex[t1].S, this->Vertex[t1].T);
		glVertex3f(this->Vertex[v1].X, this->Vertex[v1].Y, this->Vertex[v1].Z);

		glNormal3f(this->Vertex[v2].NX, this->Vertex[v2].NY, this->Vertex[v2].NZ);
		glTexCoord2f(this->Vertex[t2].S, this->Vertex[t2].T);
		glVertex3f(this->Vertex[v2].X, this->Vertex[v2].Y, this->Vertex[v2].Z);

		glNormal3f(this->Vertex[v3].NX, this->Vertex[v3].NY, this->Vertex[v3].NZ);
		glTexCoord2f(this->Vertex[t3].S, this->Vertex[t3].T);
		glVertex3f(this->Vertex[v3].X, this->Vertex[v3].Y, this->Vertex[v3].Z);
	}
	glEnd();
}

void MQTriangleMesh::Draw2D(void)
{
	glPolygonMode(GL_FRONT,GL_LINE);
	
	//Draw Triangle Line
	/*glBegin(GL_TRIANGLES);
	for(int i = 1; i <= this->TriangleNum; i++)
	{
		int t1 = this->TriangleTex[i].T1;
		int t2 = this->TriangleTex[i].T2;
		int t3 = this->TriangleTex[i].T3;

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,0);
		glVertex3f(this->Vertex[t1].S, this->Vertex[t1].T, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,0);
		glVertex3f(this->Vertex[t2].S, this->Vertex[t2].T, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,0);
		glVertex3f(this->Vertex[t3].S, this->Vertex[t3].T, 0.0);
	}
	glEnd();*/
	//Draw hole bounding box
	/*
	glBegin(GL_QUADS);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,0);
		glVertex3f(this->hole_boundaryX.first, this->hole_boundaryY.first, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,0);
		glVertex3f(this->hole_boundaryX.second, this->hole_boundaryY.first, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,0);
		glVertex3f(this->hole_boundaryX.second, this->hole_boundaryY.second, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,0);
		glVertex3f(this->hole_boundaryX.first, this->hole_boundaryY.second, 0.0);

	glEnd();
	*/
	//Draw hole
	/*
	glBegin(GL_LINES);
	list<int>::iterator hole_it;
	list<list<int>>::iterator begin = Holes_uv.begin();
	for(hole_it = begin->begin(); hole_it != begin->end();hole_it++){
		

		if(hole_it == begin->begin()){
			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(255,255,255);
			glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
		}
		else{
			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(255,255,255);
			glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(255,255,255);
			glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
		}
	}
	glNormal3f(0.0, 0.0, 1.0);
	glColor3ub(255,255,255);
	glVertex3f(this->Vertex[*begin->begin()].S, this->Vertex[*begin->begin()].T,0);
	glEnd();
	*/
	//Draw uv
	glPolygonMode(GL_FRONT,GL_FILL);
	glBegin(GL_TRIANGLES);
	
	for(int i = 1; i <= this->TriangleNum; i++)
	{
		int t1 = this->TriangleTex[i].T1;
		int t2 = this->TriangleTex[i].T2;
		int t3 = this->TriangleTex[i].T3;

		int v1 = this->Triangle[i].V1;
		int v2 = this->Triangle[i].V2;
		int v3 = this->Triangle[i].V3;

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(this->Vertex[v1].R,this->Vertex[v1].G,this->Vertex[v1].B);
		glVertex3f(this->Vertex[t1].S, this->Vertex[t1].T, 0.0);
		
		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(this->Vertex[v2].R,this->Vertex[v2].G,this->Vertex[v2].B);
		glVertex3f(this->Vertex[t2].S, this->Vertex[t2].T, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(this->Vertex[v3].R,this->Vertex[v3].G,this->Vertex[v3].B);
		glVertex3f(this->Vertex[t3].S, this->Vertex[t3].T, 0.0);
	}
	glEnd();
	
}

void MQTriangleMesh::DrawPoint(void)
{
	glBegin(GL_POINTS);
	for(int i = 0; i < this->imageSize; i++){
		for(int j = 0; j < this->imageSize;j++){
			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(this->ImagePixel[i][j].R,this->ImagePixel[i][j].G,this->ImagePixel[i][j].B);
			glVertex3f(this->ImagePixel[i][j].X, this->ImagePixel[i][j].Y, 0.0);
		}
	}
	glEnd();

	
}