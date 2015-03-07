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
	this->FindBoundary();
	this->UpdatePointStruct();
	t_end = clock();
	
	double t_duration = (double)(t_end - t_start);
	printf("%.0fms\n",t_duration);

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
		//this->Vertex[i].NeighborVertex.sort();
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

		//Find Laplacian  max & min , use for CalculateLaplacianToColor
		double min_lap = min(Vertex[i].LapX,min(Vertex[i].LapY,Vertex[i].LapZ));
		double max_lap = max(Vertex[i].LapX,max(Vertex[i].LapY,Vertex[i].LapZ));

		if(i == 1){ 
			minLap = min_lap;
			maxLap = max_lap;
		}
		else{
			if(minLap > min_lap)  minLap = min_lap;
			if(maxLap < max_lap)  maxLap = max_lap;
		}
	}
}

void MQTriangleMesh::UpdatePointStruct(void){

	//set start coordinate
	double centerX = (boundaryX.first+boundaryX.second)/2;
	double centerY = (boundaryY.first+boundaryY.second)/2;

	double starX = centerX - boundary/2;
	double starY = centerY + boundary/2;

	double maxLap_img = 0;
	double minLap_img = 0;
	//set distance between each point
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
			this->ImagePixel[i][j].X = starX + j*pointDistance;
			this->ImagePixel[i][j].Y = starY - i*pointDistance;
			
			if(j > 0) tri =  this->ImagePixel[i][j-1].Triangle;
			else{
				tri = 0;
			}
			this->PointInTriange(&this->ImagePixel[i][j],tri);
			
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
				//get pixel lap range
				double max_lap = max(ImagePixel[i][j].LapX,max(ImagePixel[i][j].LapY,ImagePixel[i][j].LapZ));
				double min_lap = min(ImagePixel[i][j].LapX,min(ImagePixel[i][j].LapY,ImagePixel[i][j].LapZ));
				if(maxLap_img == 0 && minLap_img ==0){
					maxLap_img = max_lap;
					minLap_img = min_lap;
				}
				else{
					if(minLap_img > min_lap)  minLap_img = min_lap;
					if(maxLap_img < max_lap)  maxLap_img = max_lap;
				}

			}
			else{
				if(checkHole){
					tempHoles.push_back(j);
				}
			}

		}
	}
	
	double normalize_number = 255.0 / (maxLap_img-minLap_img);

	for(int i = 0 ; i < imageSize ; i++){
		for(int j = 0 ; j < imageSize ; j++){
			if(this->ImagePixel[i][j].Triangle == 0) continue;
			this->ImagePixel[i][j].R = (this->ImagePixel[i][j].LapX - minLap_img) *  normalize_number;
			this->ImagePixel[i][j].G = (this->ImagePixel[i][j].LapY - minLap_img) *  normalize_number;
			this->ImagePixel[i][j].B = (this->ImagePixel[i][j].LapZ - minLap_img) *  normalize_number;
			//printf("%f\n",this->ImagePixel[i][j].R);
		}
	}
	
	//fill hole
	bool checknext = false;
	this->convertSample();
	for(int i = 0 ; i < imageSize ; i+= imageSize/4 ){
		for(int j = 0 ; j < imageSize ; j+= imageSize/4){
			for(int y = i ; y < i+imageSize/4 ; y++){
				for(int x = j ; x < j+imageSize/4 ; x++){
					if(ImagePixel[y][x].isHole){
						this->setTexture(imageSize/4,imageSize/4);
						this->generateTexture(5,j,i);
						checknext = true;
						break;
					}
				}
				if(checknext) break;
			}
			checknext = false;
		}	
	}
	printf("UpdatePointStruct Done\n");

}

void MQTriangleMesh::PointInTriange(MQImagePixel *p,int tri)
{

	double a1,a2,a3;
	double sum_a;
	if(tri != 0){
		a1 = (Vertex[TriangleTex[tri].T1].S - p->X) * (Vertex[TriangleTex[tri].T2].T - p->Y) - (Vertex[TriangleTex[tri].T1].T - p->Y) * (Vertex[TriangleTex[tri].T2].S - p->X);//x1y1-y1y2
		a2 = (Vertex[TriangleTex[tri].T2].S - p->X) * (Vertex[TriangleTex[tri].T3].T - p->Y) - (Vertex[TriangleTex[tri].T2].T - p->Y) * (Vertex[TriangleTex[tri].T3].S - p->X);
		a3 = (Vertex[TriangleTex[tri].T3].S - p->X) * (Vertex[TriangleTex[tri].T1].T - p->Y) - (Vertex[TriangleTex[tri].T3].T - p->Y) * (Vertex[TriangleTex[tri].T1].S - p->X);

		if(a1>=0 && a2>=0 && a3>=0){
			p->Triangle = tri;

			sum_a = a1+a2+a3;
			p->LapX = Vertex[TriangleTex[tri].T1].LapX * a2/sum_a + Vertex[TriangleTex[tri].T2].LapX * a3/sum_a + Vertex[TriangleTex[tri].T3].LapX * a1/sum_a; //v1*(a2/(a1+a2+a3)) + v2*(a3/(a1+a2+a3)) + v3 + (a3/(a1+a2+a3)) 
			p->LapY = Vertex[TriangleTex[tri].T1].LapY * a2/sum_a + Vertex[TriangleTex[tri].T2].LapY * a3/sum_a + Vertex[TriangleTex[tri].T3].LapY * a1/sum_a;
			p->LapZ = Vertex[TriangleTex[tri].T1].LapZ * a2/sum_a + Vertex[TriangleTex[tri].T2].LapZ * a3/sum_a + Vertex[TriangleTex[tri].T3].LapZ * a1/sum_a;
			
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
		p->LapX = Vertex[TriangleTex[i].T1].LapX * a2/sum_a + Vertex[TriangleTex[i].T2].LapX * a3/sum_a + Vertex[TriangleTex[i].T3].LapX * a1/sum_a; //v1*(a2/(a1+a2+a3)) + v2*(a3/(a1+a2+a3)) + v3 + (a3/(a1+a2+a3)) 
		p->LapY = Vertex[TriangleTex[i].T1].LapY * a2/sum_a + Vertex[TriangleTex[i].T2].LapY * a3/sum_a + Vertex[TriangleTex[i].T3].LapY * a1/sum_a;
		p->LapZ = Vertex[TriangleTex[i].T1].LapZ * a2/sum_a + Vertex[TriangleTex[i].T2].LapZ * a3/sum_a + Vertex[TriangleTex[i].T3].LapZ * a1/sum_a;

		return ;

	}
	p->Triangle = 0;

}

void MQTriangleMesh::CalculateLaplacianToColor(void)
{
	double normalize_number = 255.0 / (maxLap-minLap);
	for(int i = 1; i <= this->VertexNum; i++){	
		this->Vertex[i].R = (this->Vertex[i].LapX - minLap) *  normalize_number;
		this->Vertex[i].G = (this->Vertex[i].LapY - minLap) *  normalize_number;
		this->Vertex[i].B = (this->Vertex[i].LapZ - minLap) *  normalize_number;
	}

}

void MQTriangleMesh::FindBoundary(void)
{

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

	boundary = max(boundaryX.second-boundaryX.first,boundaryY.second-boundaryY.first) *1.05;

}

void MQTriangleMesh::FindHole(void)
{

	int first,second;
	map<int,int>  singleEdge ;

	//Find Single Edge
	for(int i = 1; i <= this->TriangleNum; i++){
				
		first = this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->oppositeHalfEdge.first;
		second = this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->oppositeHalfEdge.second;
		if(this->Edges.count(pair<int,int>(first,second)) <= 0){
			singleEdge[this->Triangle[i].V2] = this->Triangle[i].V1;
		}
		first = this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->oppositeHalfEdge.first;
		second = this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->oppositeHalfEdge.second;
		if(this->Edges.count(pair<int,int>(first,second)) <= 0){
			singleEdge[this->Triangle[i].V3] = this->Triangle[i].V2;
		}

		first = this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->oppositeHalfEdge.first;
		second = this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->oppositeHalfEdge.second;
		if(this->Edges.count(pair<int,int>(first,second)) <= 0){
			singleEdge[this->Triangle[i].V1] = this->Triangle[i].V3;
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

	printf("Holes size:%d\n",Holes.size());

	list<int> hole_point;
	list<list<int>>::iterator hole_it;
	list<int>::iterator point_it;
	for(hole_it=Holes.begin();hole_it!= Holes.end();hole_it++){
		printf("Hole point size:%d\n",hole_it->size());
		for(point_it = hole_it->begin();point_it != hole_it->end();point_it++){
			if(point_it != hole_it->begin()) printf(",");
			printf("%d",*point_it);
		}
		printf("\n");
	}
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

void MQTriangleMesh::generateTexture(int size,int startX,int startY)
	// generate the texture from the sample using a search window of size x size
{
	int i, j, a=0;

	cout<<"Initializing texture...";
	initializeTexture(size,startX,startY);
	cout<<"done\n";

	cout<<"Performing exhaustive search...\n";

	red = new double*[size];
	green = new double*[size];
	blue = new double*[size];
	for(int x=0; x<size; x++)
	{
		red[x] = new double[size/2+1];
		blue[x] = new double[size/2+1];
		green[x] = new double[size/2+1];
	}

	for(i=0; i<texture_h; i++)
	{
		if(i>=double(a*texture_h)/100)
		{
			cout<<a<<"% done\n";
			a+=10;
		}
		for(j=0; j<texture_w; j++)
		{		
			if(ImagePixel[i+startY][j+startX].isHole){	
				findBestMatch(j, i, size);
				ImagePixel[i+startY][j+startX].R = texture_red[i][j];
				ImagePixel[i+startY][j+startX].G = texture_green[i][j];
				ImagePixel[i+startY][j+startX].B = texture_blue[i][j];
			}
		}
	}
	cout<<"100% done\n";
	cout<<"Texture generation complete\n\n";

	return;
}

void MQTriangleMesh::initializeTexture(int size,int startX, int startY)
	// initialize output texture with random pixels from the sample
{
	int i, j;
	int w, h;

	int valid_w_length = sample_w-size+1;
	int valid_h_length = sample_h-size/2;
	int dw = size/2;
	int dh = size/2;

	//srand(time(NULL));

	for (i=0; i<texture_h; i++)
	{
		for(j=0; j<texture_w; j++)
		{
			if(ImagePixel[i+startY][j+startX].isHole){
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
				texture_red[i][j] = sample_red[i+startY][j+startX];
				texture_green[i][j] = sample_green[i+startY][j+startX];
				texture_blue[i][j] = sample_blue[i+startY][j+startX];
				original_pos_x[i][j] = j+startX;
				original_pos_y[i][j] = i+startY;
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
			// check if neighborhood of candidate lies completely in sample
			if(actualx < size/2 || actualx >= sample_w-size/2 || actualy < size/2 || actualy >= sample_h-size/2)
			{
				//replace with random
				actualx = rand()%(sample_w-size+1)+size/2;
				actualy = rand()%(sample_h-size/2)+size/2;
				while(sample_red[actualy][actualx] == -1){
					actualx = rand()%(sample_w-size+1)+size/2;
					actualy = rand()%(sample_h-size/2)+size/2;
				}
			}
			add = true;
			for(int c=0; c<candidate_x.size(); c++)
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
	for(int c=0; c<candidate_x.size(); c++)
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
	return;
}

void MQTriangleMesh::Draw(GLubyte Red, GLubyte Green, GLubyte Blue)
{
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
	glBegin(GL_TRIANGLES);
	
	for(int i = 1; i <= this->TriangleNum; i++)
	{
		int t1 = this->TriangleTex[i].T1;
		int t2 = this->TriangleTex[i].T2;
		int t3 = this->TriangleTex[i].T3;

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(this->Vertex[t1].R,this->Vertex[t1].G,this->Vertex[t1].B);
		glVertex3f(this->Vertex[t1].S, this->Vertex[t1].T, 0.0);
		
		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(this->Vertex[t2].R,this->Vertex[t2].G,this->Vertex[t2].B);
		glVertex3f(this->Vertex[t2].S, this->Vertex[t2].T, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(this->Vertex[t3].R,this->Vertex[t3].G,this->Vertex[t3].B);
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