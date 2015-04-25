#include "MQTriangleMesh.h"
#include "MQglm.h"
#include "MQPCA.h"
#include <iostream>
#include <math.h>
#include <algorithm> 
#include <time.h>
#include <fstream>

bool my_sort(MQImagePixel* p1,const MQImagePixel* p2){ return p1->neighborHole.size() < p2->neighborHole.size();}
bool my_sort2(LaplaianLength* p1,const LaplaianLength* p2){ return p1->neighbor_pca < p2->neighbor_pca;}

void _CalculateNormal(float v[3][3], float out[3])
{
	float v1[3], v2[3];
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

	float length = sqrt( pow(out[x],2) + pow(out[y],2) + pow(out[z],2) );
	out[x] /= length;
	out[y] /= length;
	out[z] /= length;
}

void MQVertex::Normal2UnitVector()
{
	float length = sqrt( pow(this->NX,2) + pow(this->NY,2) + pow(this->NZ,2) );
	this->NX /= length;
	this->NY /= length;
	this->NZ /= length;
}

void MQTriangle::Normal2UnitVector()
{
	float length = sqrt( pow(this->NX,2) + pow(this->NY,2) + pow(this->NZ,2) );
	this->NX /= length;
	this->NY /= length;
	this->NZ /= length;
}

bool MQTriangleMesh::ReadObjFile(const char *FileName)
{
	MQGLMmodel *_model = MQglmReadOBJ(FileName);

	if(baseMesh){
		delete baseMesh;
		baseMesh =NULL;		
	}
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
		//Edge2
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)] = new HalfEdge();
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->face = i;
		//Edge3
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)] = new HalfEdge();
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->face = i;
	}
	for(int i = 1; i <= this->TriangleNum; i++)
	{
		//HalfEdge Data 
		//Edge1
		this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->nextHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)];
		this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->oppositeHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V1)];
		//Edge2
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->nextHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)];
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->oppositeHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V2)];
		//Edge3
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->nextHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)];
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->oppositeHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V3)];
	}
	MQglmDelete(_model);
	//calculate normal
	struct _vector3{ float x; float y; float z; };
	vector<_vector3> *_NormalTable = new vector<_vector3>[ (this->VertexNum)+1 ];
	
	for(int i = 1; i <= this->TriangleNum; i++)
	{
		float v[3][3], out[3];
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
		float _xsum = 0.0, _ysum = 0.0, _zsum = 0.0;

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
	this->FindHole();
	this->FindBoundary();
	this->CalculateLaplacianToColor();
	this->UpdatePointStruct();
	//this->TriangulateBaseMesh();
	t_end = clock();

	float t_duration = (float)(t_end - t_start);
	printf("Read file finish: %.2fs\n\n",t_duration/1000.0f);

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

	int n1,n2;
	for(int i = 1; i <= this->VertexNum; i++)
	{
		this->Vertex[i].NeighborVertex.sort();
		this->Vertex[i].NeighborVertex.unique();
	}
}

void MQTriangleMesh::UpdateVertexLaplacianCoordinate(void)
{
	list<int>::iterator it, begin, end;
	for(int i = 1; i <= this->VertexNum; i++)
	{
		float sumX = 0.0, sumY = 0.0, sumZ = 0.0;

		begin = this->Vertex[i].NeighborVertex.begin();
		end = this->Vertex[i].NeighborVertex.end();
		for(it = begin; it != end; it++)
		{
			sumX += this->Vertex[*it].X;
			sumY += this->Vertex[*it].Y;
			sumZ += this->Vertex[*it].Z;
		}

		float degree = this->Vertex[i].NeighborVertex.size();

		float mCenterX = sumX/degree;
		float mCenterY = sumY/degree;
		float mCenterZ = sumZ/degree;

		this->Vertex[i].LapX = this->Vertex[i].X - mCenterX;
		this->Vertex[i].LapY = this->Vertex[i].Y - mCenterY;
		this->Vertex[i].LapZ = this->Vertex[i].Z - mCenterZ;
	}
}

void MQTriangleMesh::UpdatePointStruct(void)
{

	//�]�w�_�l�I�b���W��
	float centerX = (boundaryX.first+boundaryX.second)/2;
	float centerY = (boundaryY.first+boundaryY.second)/2;
	float startX = centerX - boundary/2;
	float startY = centerY + boundary/2;

	float maxLap_img = 0;
	float minLap_img = 0;

	//�]�w�C��Pixel�����j
	float pointDistance = boundary/imageSize;

	int tri;

	bool checkHole = false;
	vector<int> tempHoles;
	vector<int>::iterator hole_it;
	/*
	//����JPG�ɬ}
	fstream file;
	file.open("test.txt", ios::in);
	if(!file)     //�ˬd�ɮ׬O�_���\�}��
	{
		cerr << "Can't open file!\n";
		exit(1);     //�b�����`���ΤU�A���_�{��������
	}
	this->ImagePixel.resize(imageSize);
	int amount = 0;
	for(int i = 0 ; i < imageSize ; i++)
	{
		//�i��
		fflush(stdout);
		printf("\rUpdatePixel:%.0f%%",((float)i/imageSize)*100);
		this->ImagePixel[i].clear();
		this->ImagePixel[i].resize(imageSize);
		for(int j = 0 ; j < imageSize ; j++)
		{
			this->ImagePixel[i][j].X = startX + j*pointDistance;
			this->ImagePixel[i][j].Y = startY - i*pointDistance;
			file>>this->ImagePixel[i][j].R;
			file>>this->ImagePixel[i][j].G;
			file>>this->ImagePixel[i][j].B;
			if(this->ImagePixel[i][j].R == -1 )
			{	
				this->ImagePixel[i][j].isHole = true;
				continue;
			}
		
			this->ImagePixel[i][j].Lap_length = sqrt(pow(ImagePixel[i][j].R,2) + pow(ImagePixel[i][j].G,2) + pow(ImagePixel[i][j].B,2));				
		}
	}	
	printf("\rUpdatePixel:100%%\n");
	*/
	
	this->ImagePixel.resize(imageSize);
	for(int i = 0 ; i < imageSize ; i++)
	{
		//�i��
		fflush(stdout);
		printf("\rUpdatePixel:%.0f%%",((float)i/imageSize)*100);
		this->ImagePixel[i].clear();
		this->ImagePixel[i].resize(imageSize);
		tempHoles.clear();
		checkHole = false;
		for(int j = 0 ; j < imageSize ; j++)
		{
			//�]�w�C�ӹ������y��
			this->ImagePixel[i][j].X = startX + j*pointDistance;
			this->ImagePixel[i][j].Y = startY - i*pointDistance;
			
			if(j > 0) tri =  this->ImagePixel[i][j-1].Triangle;
			else	tri = 0;

			this->PointInTriange(&this->ImagePixel[i][j],tri);
			
			//�p�G�����bUV�T���θ�
			if(this->ImagePixel[i][j].Triangle != 0)
			{
				//�o��}������
				if(checkHole)
				{
					for(hole_it = tempHoles.begin();hole_it != tempHoles.end() ; hole_it++)
						ImagePixel[i][*hole_it].isHole = true;
					tempHoles.clear();
				}
				else
				{
					checkHole = true;
				}
				/*
				//�o�칳��laplacian���̤j�̤p��
				float max_lap = max(ImagePixel[i][j].LapX,max(ImagePixel[i][j].LapY,ImagePixel[i][j].LapZ));
				float min_lap = min(ImagePixel[i][j].LapX,min(ImagePixel[i][j].LapY,ImagePixel[i][j].LapZ));
				if(maxLap_img == 0 && minLap_img ==0)
				{
					maxLap_img = minLap_img = ImagePixel[i][j].Lap_length;
					//maxLap_img = max_lap;
					//minLap_img = min_lap;
				}
				else
				{
					if(minLap_img > ImagePixel[i][j].Lap_length) minLap_img = ImagePixel[i][j].Lap_length;
					if(maxLap_img < ImagePixel[i][j].Lap_length) maxLap_img = ImagePixel[i][j].Lap_length;
					//if(minLap_img > min_lap)  minLap_img = min_lap;
					//if(maxLap_img < max_lap)  maxLap_img = max_lap;
				}
				*/
			}
			else
			{
				if(this->ImagePixel[i][j].X >= hole_boundaryX.first && this->ImagePixel[i][j].X <= hole_boundaryX.second &&
					this->ImagePixel[i][j].Y >= hole_boundaryY.first && this->ImagePixel[i][j].Y <= hole_boundaryY.second )
						if(checkHole) tempHoles.push_back(j);
			}
		}
	}
	printf("\rUpdatePixel:100%%\n");
	
	//float normalize_number = 255.0 / (maxLap_img-minLap_img);
	
	for(int i = 0 ; i < imageSize ; i++)
	{
		for(int j = 0 ; j < imageSize ; j++)
		{
			if(this->ImagePixel[i][j].isHole)
			{
				//�P�_�}�P�򪺹����O�_�]���},�O���ܱN�o�[��neighborhole���}�C��
				for(int y = i-1 ; y <= i+1 ;y++)
				{
					for(int x = j-1 ; x <= j+1 ; x++)
					{
						if(y == i && x == j) continue;
						if(this->ImagePixel[y][x].isHole)	this->ImagePixel[i][j].neighborHole.push_back(y*imageSize+x);
					}
				}
				//�N�G���y���ର�@���y��
				ImagePixel[i][j].position = i*imageSize + j;
				this->HolePixels.push_back(&ImagePixel[i][j]);		
			}
			/*
			if(this->ImagePixel[i][j].Triangle == 0) continue;
			//laplacian �ഫ���C��(RGB)
			this->ImagePixel[i][j].R = this->ImagePixel[i][j].G = this->ImagePixel[i][j].B = (this->ImagePixel[i][j].Lap_length - minLap_img) *  normalize_number;
			//this->ImagePixel[i][j].R = (this->ImagePixel[i][j].LapX - minLap_img) *  normalize_number;
			//this->ImagePixel[i][j].G = (this->ImagePixel[i][j].LapY - minLap_img) *  normalize_number;
			//this->ImagePixel[i][j].B = (this->ImagePixel[i][j].LapZ - minLap_img) *  normalize_number;		
			*/
		} 
	}
	//�N�}�������̳y�F�~���h��Ƨ�(��->�h)
	sort(this->HolePixels.begin(),this->HolePixels.end(),my_sort);//�мg�ƧǤ�k

	//���}��������Bounding Box ������x,y�d��
	vector<MQImagePixel*>::iterator holepixel_it;
	for(holepixel_it = HolePixels.begin();holepixel_it != HolePixels.end();holepixel_it++)
	{
		int x = (*holepixel_it)->position % imageSize;
		int y = (*holepixel_it)->position / imageSize;
		
		if(holepixel_it == HolePixels.begin())
		{
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

	int ex_dim = pca_size*pca_size;//�]�w�F�~Lap����
	int range = (pca_size-1)/2;

	//���t�O����
	LaplaianLengths.resize(imageSize);
	for(int i = 0 ; i < imageSize ; i++){
		LaplaianLengths[i].resize(imageSize);
		for(int j = 0 ; j < imageSize ; j++){
			LaplaianLengths[i][j] = new LaplaianLength();
		}
	}

	//PCA��Ʒǳ�
	for(int i = range ; i < imageSize -range; i++)
	{
		for(int j = range ; j < imageSize - range ; j++)
		{
			bool isDone = true;
			LaplaianLengths[i][j]->X = j;
			LaplaianLengths[i][j]->Y = i;
			if(ImagePixel[i][j].Triangle == 0)//if(ImagePixel[i][j].isHole)
			{
				if(ImagePixel[i][j].isHole) LaplaianLengths[i][j]->hole = true;
				else LaplaianLengths[i][j]->boundary = true;
				continue;
			}
			else{
				LaplaianLengths[i][j]->length.push_back(0.0);
			}
			for(int y = i - range ; y <= i+range ; y++)
			{
				for(int x = j -range ; x <= j+range ; x++)
				{					
					if(LaplaianLengths[i][j]->hole || LaplaianLengths[i][j]->boundary)
					{						
						LaplaianLengths[i][j]->length.clear();
						tempLaplaianLength.push_back(LaplaianLengths[i][j]);
						isDone = false;
						break;
					}
					LaplaianLengths[i][j]->length.push_back(ImagePixel[y][x].Lap_length);
				}
				if(!isDone) break;
			}
			if(isDone)	LaplaianLengths[i][j]->pca = true;
		}
	}
	
	vector<LaplaianLength*>::iterator lap_it;
	for(lap_it = tempLaplaianLength.begin(); lap_it != tempLaplaianLength.end();lap_it++)
	{
		for(int i = (*lap_it)->X-range ; i <= (*lap_it)->X+range ; i++){
			for(int j = (*lap_it)->Y-range ; j <= (*lap_it)->Y+range;j++){
				if(LaplaianLengths[i][j]->pca) (*lap_it)->neighbor_pca++;
			}			
		}
	}

	sort(tempLaplaianLength.begin(),tempLaplaianLength.end(),my_sort2);//�ƧǾF�~����PCA���ƶq
	
	LaplaianLength* it;
	while(tempLaplaianLength.size() > 0){
		it = tempLaplaianLength.back();
		it->length.resize(ex_dim+1,0.0f);
		for(int i = it->Y - range ; i <= it->Y + range ; i++)
		{
			for(int j = it->X - range ; j<= it->X + range ; j++)
			{
				if(LaplaianLengths[i][j]->pca)
				{
					for(int x = 1 ; x <= ex_dim ; x++)	it->length[x] += LaplaianLengths[i][j]->length[x];//�N�F�~PCA�̷�index�ۥ[
				}
			}
		}
		for(int x = 1 ; x <= ex_dim ; x++)	{
			it->length[x] /= it->neighbor_pca;//������;
		}
		
		it->pca = true;

		vector<LaplaianLength*>::iterator temp_it;
		for(temp_it = tempLaplaianLength.begin(); temp_it != tempLaplaianLength.end(); temp_it++){
			if((*temp_it) == it) continue;
			if(abs((*temp_it)->X - it->X ) <= 1 && abs((*temp_it)->Y - it->Y ) <= 1) (*temp_it)->neighbor_pca++;
		}
		tempLaplaianLength.pop_back();
		sort(tempLaplaianLength.begin(),tempLaplaianLength.end(),my_sort2);
	}
	
	MQPCA PCA_1;

	PCA_1.SetDataItemDimension(ex_dim);		//�i�DPCA�C����ƪ�����5x5=25
	for(int i = range; i < imageSize - range; ++i)
	{
		//printf("%d",i);
		for(int j = range; j < imageSize - range; ++j)
		{			
			if(LaplaianLengths[i][j]->hole || LaplaianLengths[i][j]->boundary) continue;	
			bool add_result = PCA_1.AddDataItem(LaplaianLengths[i][j]->length);	//�Nappearance space������(��t���p)�[�JPCA���ƾڮw
			if(add_result == false){	
				//cout<<"111";
				cout<<"PCA input data error!"<<endl;
			}
		}
	}
	PCA_1.PrincipalComponentAnalysis();	//�i��PCA���R

	#ifdef MQDebug
		PCA_1.DisplayCovMatrixEvals();	//debug�ΡA�C�XPCA������eigen value��
	#endif
		
		for(int i = range; i < imageSize - range; ++i)
		{
			for(int j = range; j < imageSize - range; ++j)
			{
				if(!LaplaianLengths[i][j]->pca || LaplaianLengths[i][j]->hole || LaplaianLengths[i][j]->boundary) continue;
				
				//_SrcImgPixDim=8(�N��PCA�����᪺����)�Fflvecx���PCA�����᪺�V�q(�]�N�O5x5x3=75->8�������V�q)
				vector<float> flvecx = PCA_1.ProjectDataItem(LaplaianLengths[i][j]->length, _SrcImgPixDim);

				//�NPCA�����᪺�V�qassign��_SrcImg
				for(int k = 1; k <= _SrcImgPixDim; ++k){
					ImagePixel[i][j].pca_data.push_back(flvecx[k]);
				}
			}
		}
	
	for(int x = 0 ; x < 3 ; x++)
	{
		for(int i = range ; i < imageSize - range ; i++)
		{
			for(int j = range ; j < imageSize - range ; j++)
			{
				if(LaplaianLengths[i][j]->hole || LaplaianLengths[i][j]->boundary) continue;

				if(maxLap_img == 0 && minLap_img == 0)
				{
					maxLap_img = minLap_img = ImagePixel[i][j].pca_data[x];
				}
				else
				{
					if(minLap_img > ImagePixel[i][j].pca_data[x]) minLap_img = ImagePixel[i][j].pca_data[x];
					if(maxLap_img < ImagePixel[i][j].pca_data[x]) maxLap_img = ImagePixel[i][j].pca_data[x];
				}
			}
		}		
	}

	float normalize_number = 255.0 / (maxLap_img-minLap_img);
	for(int i = range ; i < imageSize - range ; i++)
	{
		for(int j = range ; j < imageSize - range ; j++)
		{
			if(this->ImagePixel[i][j].Triangle == 0) continue;
			//if(LaplaianLengths[i][j]->hole || LaplaianLengths[i][j]->boundary) continue;
			this->ImagePixel[i][j].R = (this->ImagePixel[i][j].pca_data[0] - minLap_img) *  normalize_number;
			this->ImagePixel[i][j].G = (this->ImagePixel[i][j].pca_data[1] - minLap_img) *  normalize_number;
			this->ImagePixel[i][j].B = (this->ImagePixel[i][j].pca_data[2] - minLap_img) *  normalize_number;	

		}
	}
	
	fflush(stdout);
}

void MQTriangleMesh::FillHole(int method)
{
	this->convertSample();
	this->setTexture(imageSize);
	this->generateTexture(window_size,method);
}

//�P�_�����O�_�bUV�T���Τ�
void MQTriangleMesh::PointInTriange(MQImagePixel *p,int tri)
{

	float a1,a2,a3;
	float sum_a;
	//�p�G�F�~�����bUV�T���Τ�
	if(tri != 0)
	{
		a1 = (Vertex[TriangleTex[tri].T1].S - p->X) * (Vertex[TriangleTex[tri].T2].T - p->Y) - (Vertex[TriangleTex[tri].T1].T - p->Y) * (Vertex[TriangleTex[tri].T2].S - p->X);//x1y1-y1y2
		a2 = (Vertex[TriangleTex[tri].T2].S - p->X) * (Vertex[TriangleTex[tri].T3].T - p->Y) - (Vertex[TriangleTex[tri].T2].T - p->Y) * (Vertex[TriangleTex[tri].T3].S - p->X);
		a3 = (Vertex[TriangleTex[tri].T3].S - p->X) * (Vertex[TriangleTex[tri].T1].T - p->Y) - (Vertex[TriangleTex[tri].T3].T - p->Y) * (Vertex[TriangleTex[tri].T1].S - p->X);

		if(a1>=0 && a2>=0 && a3>=0)
		{

			p->Triangle = tri;
			sum_a = a1+a2+a3;
			p->Lap_length = Vertex[Triangle[tri].V1].Lap_length * a2/sum_a + Vertex[Triangle[tri].V2].Lap_length * a3/sum_a + Vertex[Triangle[tri].V3].Lap_length * a1/sum_a;
			p->LapX = Vertex[Triangle[tri].V1].LapX * a2/sum_a + Vertex[Triangle[tri].V2].LapX * a3/sum_a + Vertex[Triangle[tri].V3].LapX * a1/sum_a; //v1*(a2/(a1+a2+a3)) + v2*(a3/(a1+a2+a3)) + v3 + (a3/(a1+a2+a3)) 
			p->LapY = Vertex[Triangle[tri].V1].LapY * a2/sum_a + Vertex[Triangle[tri].V2].LapY * a3/sum_a + Vertex[Triangle[tri].V3].LapY * a1/sum_a;
			p->LapZ = Vertex[Triangle[tri].V1].LapZ * a2/sum_a + Vertex[Triangle[tri].V2].LapZ * a3/sum_a + Vertex[Triangle[tri].V3].LapZ * a1/sum_a;
			return;

		}
	}
	//�p�G�F�~�����S���bUV�T���Τ��K�a�ড�j�M
	for(int i = 1 ; i <= this->TriangleNum ; i++)
	{
		
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
	float normalize_number = 255.0 / (maxLap-minLap);
	for(int i = 1; i <= this->VertexNum; i++)
	{	
		this->Vertex[i].R  = this->Vertex[i].G = this->Vertex[i].B = (this->Vertex[i].Lap_length - minLap) *  normalize_number;
		//this->Vertex[i].R = (this->Vertex[i].LapX - minLap) *  normalize_number;
		//this->Vertex[i].G = (this->Vertex[i].LapY - minLap) *  normalize_number;
		//this->Vertex[i].B = (this->Vertex[i].LapZ - minLap) *  normalize_number;
	}

}

void MQTriangleMesh::FindBoundary(void)
{
	//���UV��Bounding Box�d��
	for(int i = 1 ; i <= this->TexcoordNum ; i++)
	{
		if(i == 1)
		{
			boundaryX.first = boundaryX.second = this->Vertex[i].S;
			boundaryY.first = boundaryY.second = this->Vertex[i].T;
		}
		else
		{
			if(boundaryX.first > this->Vertex[i].S) boundaryX.first = this->Vertex[i].S;
			if(boundaryX.second < this->Vertex[i].S) boundaryX.second = this->Vertex[i].S;
			if(boundaryY.first > this->Vertex[i].T) boundaryY.first = this->Vertex[i].T;
			if(boundaryY.second < this->Vertex[i].T) boundaryY.second = this->Vertex[i].T;
		}
	}
	//�NBounding Box �d���j1.05��
	boundary = max(boundaryX.second-boundaryX.first,boundaryY.second-boundaryY.first) *1.05;

	///���UV�}��������Bounding Box �d��
	if(Holes.size() > 0)
	{
		list<int>::iterator hole_it;
		list<list<int>>::iterator begin = Holes_uv.begin();
		for(hole_it = begin->begin(); hole_it != begin->end();hole_it++)
		{
			if(hole_it == begin->begin())
			{
				hole_boundaryX.first = hole_boundaryX.second = this->Vertex[*hole_it].S;
				hole_boundaryY.first = hole_boundaryY.second = this->Vertex[*hole_it].T;
			}
			else
			{
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
	map<int,int>  singleEdge ;
	map<int,int>  singleEdge_uv;
	//������(Single Edge)
	for(int i = 1; i <= this->TriangleNum; i++)
	{				
		if(!this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->oppositeHalfEdge)
		{
			singleEdge_uv[this->TriangleTex[i].T2] = this->TriangleTex[i].T1;
			singleEdge[this->Triangle[i].V2] = this->Triangle[i].V1;
		}
		if(!this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->oppositeHalfEdge)
		{
			singleEdge_uv[this->TriangleTex[i].T3] = this->TriangleTex[i].T2;
			singleEdge[this->Triangle[i].V3] = this->Triangle[i].V2;
		}
		if(!this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->oppositeHalfEdge)
		{
			singleEdge_uv[this->TriangleTex[i].T1] = this->TriangleTex[i].T3;
			singleEdge[this->Triangle[i].V1] = this->Triangle[i].V3;
		}
	}
	//�N����s�����}(v)
	map<int,int>::iterator it;
	list<int> hole;
	int fist_point,search_point,temp;
	Holes.clear();
	while(!singleEdge.empty())
	{
		it = singleEdge.begin();
		fist_point = it->first;  //�]�m�}���_�l�I
		hole.push_back(fist_point);
		search_point = singleEdge[fist_point];
		singleEdge.erase(fist_point);
		//�p�Gsearch_point == fist_point ��ܳ򦨤@�Ӭ},�~��N�ѤU������s���t�~���}
		while(search_point != fist_point)
		{
			hole.push_back(search_point);
			temp = search_point;
			search_point = singleEdge[search_point];
			singleEdge.erase(temp);			
		}
		Holes.push_back(hole);	
		hole.clear();
	}
	//�N����s�����}(vt)
	list<int> hole_uv;
	Holes_uv.clear();
	while(!singleEdge_uv.empty())
	{
		it = singleEdge_uv.begin();
		fist_point = it->first;  //�]�m�}���_�l�I
		hole_uv.push_back(fist_point);
		search_point = singleEdge_uv[fist_point];
		singleEdge_uv.erase(fist_point);
		//�p�Gsearch_point == fist_point ��ܳ򦨤@�Ӭ},�~��N�ѤU������s���t�~���}
		while(search_point != fist_point)
		{
			hole_uv.push_back(search_point);
			temp = search_point;
			search_point = singleEdge_uv[search_point];
			singleEdge_uv.erase(temp);			
		}
		Holes_uv.push_back(hole_uv);	
		hole_uv.clear();
	}
	
	vector<float> hole_length;
	list<list<int>>::iterator hole_it;
	list<list<int>>::iterator hole_uv_it;
	list<int>::iterator point_it;

	//�ե��}(hole)����t(boundary)���ƭ�
	for(hole_it = Holes.begin() ; hole_it != Holes.end() ; hole_it++)
	{
		for(point_it = hole_it->begin() ; point_it != hole_it->end() ; point_it++)
		{
			float tempX,tempY,tempZ;
			int total_length;
			tempX = tempY = tempZ = total_length =  0;
			list<int>::iterator neighbor_it;
			for(neighbor_it = Vertex[*point_it].NeighborVertex.begin() ; neighbor_it != Vertex[*point_it].NeighborVertex.end() ; neighbor_it++)
			{
				if(find(hole_it->begin(),hole_it->end(),*neighbor_it) != hole_it->end() && Vertex[*neighbor_it].evaluate == false) continue;		
				float temp_length = 1.0f/sqrt(pow(Vertex[*neighbor_it].LapX - Vertex[*point_it].LapX,2)+pow(Vertex[*neighbor_it].LapY - Vertex[*point_it].LapY,2)+pow(Vertex[*neighbor_it].LapZ - Vertex[*point_it].LapZ,2));
				tempX += Vertex[*neighbor_it].LapX  * temp_length ;
				tempY += Vertex[*neighbor_it].LapY  * temp_length ;
				tempZ += Vertex[*neighbor_it].LapZ  * temp_length ;
				total_length += temp_length;		
			}
			Vertex[*point_it].LapX = tempX/ total_length;
			Vertex[*point_it].LapY = tempY/ total_length ;
			Vertex[*point_it].LapZ = tempZ/ total_length ;
			Vertex[*point_it].evaluate = true;
		} 
	}
	minLap = maxLap = 0.0;
	for(int i = 1; i <= this->VertexNum; i++)
	{
		this->Vertex[i].Lap_length = sqrt(pow(this->Vertex[i].LapX,2)+pow(this->Vertex[i].LapY,2)+pow(this->Vertex[i].LapZ,2));
		//���Laplacian���̤j�̤p�� , �H�K�����ഫ���C��
		if(i == 1)
		{ 
			minLap = maxLap = this->Vertex[i].Lap_length;
		}
		else
		{
			if(minLap > this->Vertex[i].Lap_length) minLap = this->Vertex[i].Lap_length;
			if(maxLap < this->Vertex[i].Lap_length) maxLap = this->Vertex[i].Lap_length;
		}
	}
	
	//�p��UV���}������
	for(hole_uv_it=Holes_uv.begin();hole_uv_it!= Holes_uv.end();hole_uv_it++)
	{
		float start_s = -1.0;
		float start_t = -1.0;
		float length = 0.0;
		for(point_it = hole_uv_it->begin();point_it != hole_uv_it->end();point_it++)
		{
			if(start_s == -1.0)
			{
				start_s = Vertex[*point_it].S;
				start_t = Vertex[*point_it].T;
			}
			else
			{
				length += sqrt(pow(Vertex[*point_it].S - start_s,2) + pow(Vertex[*point_it].T - start_t,2));
				start_s = Vertex[*point_it].S;
				start_t = Vertex[*point_it].T;
			}
		}
		length += sqrt(pow(Vertex[*hole_uv_it->begin()].S - start_s,2) + pow(Vertex[*hole_uv_it->begin()].T - start_t,2));
		hole_length.push_back(length);
	}
	//�R���̪����}(Boundary)���}�C
	float max_hole = *max_element(hole_length.begin(),hole_length.end()); 
	vector<float>::iterator length_it;
	for(hole_it = Holes.begin(),hole_uv_it = Holes_uv.begin(),length_it = hole_length.begin();length_it != hole_length.end();hole_it++,hole_uv_it++,length_it++)
	{
		if(*length_it == max_hole)
		{
			Holes.erase(hole_it);
			Holes_uv.erase(hole_uv_it);
			break;
		}
	}
}

void MQTriangleMesh::CheckHole()
{
	printf("---------------Hole Info---------------\n");
	list<list<int>>::iterator hole_it;
	list<list<int>>::iterator hole_uv_it;
	list<int>::iterator point_it;

	printf("Holes size:%d\n",Holes.size());
	printf("Holes_uv size:%d\n",Holes_uv.size());
	for(hole_it=Holes.begin();hole_it!= Holes.end();hole_it++)
	{
		printf("Hole point size:%d\n",hole_it->size());
		for(point_it = hole_it->begin();point_it != hole_it->end();point_it++)
		{
			if(point_it != hole_it->begin()) printf(",");
			printf("%d",*point_it);
		}
		printf("\n");
	}
	for(hole_uv_it=Holes_uv.begin();hole_uv_it!= Holes_uv.end();hole_uv_it++)
	{
		printf("Hole_uv point size:%d\n",hole_uv_it->size());
		for(point_it = hole_uv_it->begin();point_it != hole_uv_it->end();point_it++)
		{
			if(point_it != hole_uv_it->begin()) printf(",");
			printf("%d",*point_it);
		}
		printf("\n");
	}
	printf("---------------------------------------\n\n");
}

void MQTriangleMesh::setTexture(int window)
{
	texture_red = new float* [window];
	texture_green = new float* [window];
	texture_blue = new float* [window];
	texture_lap = new float **[window];
	original_pos_x = new int* [window];
	original_pos_y = new int* [window];
	for(int i=0; i<window; i++)
	{
		texture_red[i] = new float[window];
		texture_green[i] = new float[window];
		texture_blue[i] = new float[window];
		texture_lap[i] = new float *[window];
		for(int j = 0 ; j < window ; j++)
			texture_lap[i][j] = new float[_SrcImgPixDim];
		original_pos_x[i] = new int[window];
		original_pos_y[i] = new int[window];
	}
}

void MQTriangleMesh::generateTexture(int size,int method)// generate the texture from the sample using a search window of size x size (method 1:�H�� 2:Boounding Box 3:���F�~�̦h 4:������)
{
	int i, j, a=0;
	clock_t t_start,t_end;

	cout<<"Initializing texture...\n";
	initializeTexture(size);
	
	red = new float*[size];
	green = new float*[size];
	blue = new float*[size];
	lap_sample = new float**[size];
	for(int y=0; y<size; y++)
	{
		red[y] = new float[size];
		blue[y] = new float[size];
		green[y] = new float[size];
		lap_sample[y] = new float*[size];
		for(int x = 0; x < size ; x++) lap_sample[y][x] = new float[_SrcImgPixDim];
	}
	t_start = clock();
	if(method == 1)//�H��
	{
		cout<<"Start fill hole for random...\n";
		list<int>::iterator it;
		vector<MQImagePixel>::iterator it_hole;
		int index = 0;
		MQImagePixel* temp;
		int hole_count = HolePixels.size();
		int amount = 0;
		while(HolePixels.size()> 0)
		{
			//�i��
			fflush(stdout);
			printf("\rFillHole:%.0f%%",(1.0f-((float)HolePixels.size()/hole_count))*100);
			do{
				index = rand()%HolePixels.size();
				temp = HolePixels[index];	
			}while(temp->neighborHole.size() > 5);
			i = (int)temp->position / imageSize;
			j = (int)temp->position % imageSize;
			findBestMatch(j, i, size);

			for(it = temp->neighborHole.begin(); it !=temp->neighborHole.end();it++)
				ImagePixel[int(*it/imageSize)][int(*it%imageSize)].neighborHole.remove(temp->position);

			HolePixels.erase(HolePixels.begin()+index);
		}
		printf("\n");
	}
	if(method == 2)//bounding box
	{
		cout<<"Start fill hole for bounding box...\n";
		int startX = pixel_boundaryX.first;
		int endX = pixel_boundaryX.second;
		int startY = pixel_boundaryY.first;
		int endY = pixel_boundaryY.second;
		int dir = 1;
		int fix;
		int total_count = (endX-startX) * (endY - startY);
		int counter = 0;
		bool useX = true;
		bool useY = false;
		while(startX <= endX && startY <= endY)
		{ 
			//�i��
			fflush(stdout);
			printf("\rFillHole:%.0f%%",((float)counter/total_count)*100);
			if(useX)
			{
				if(dir>0)//up,left->right
				{
					fix = startY;
					for(int j = startX; j <= endX ; j+=dir)
					{
						if(counter < total_count)counter++;
						if(ImagePixel[fix][j].isHole)	findBestMatch(j, fix, size);
					}
				}
				else//down,right->left
				{
					fix = endY;
					for(int j = endX; j >= startX ; j+=dir)
					{
						if(counter < total_count)counter++;
						if(ImagePixel[fix][j].isHole)	findBestMatch(j, fix, size);
					}
				}
				if(dir > 0) startY += 1;
				if(dir < 0) endY -= 1 ;

				useX = false;
				useY = true;
			}
			if(useY)
			{
				if(dir>0)//right,up->down
				{
					fix = endX;
					for(int i = startY; i <= endY ; i+=dir)
					{
						if(counter < total_count)counter++;
						if(ImagePixel[i][fix].isHole)	findBestMatch(fix, i, size);
					}
				}
				else//left,down->up
				{
					fix = startX;
					for(int i = endY; i >= startY ; i+=dir)
					{
						if(counter < total_count)counter++;
						if(ImagePixel[i][fix].isHole)	findBestMatch(fix, i, size);
					}
				}
			
				if(dir > 0)	endX -= 1;
				if(dir < 0) startX += 1 ;

				useX = true;
				useY = false;
				dir *= -1;
			}
		}
		printf("\n");
	}
	if(method == 3)//���F�~�̦h����
	{
		cout<<"Start fill hole for most neighborhood have pixel ...\n";
		list<int>::iterator it;
		vector<MQImagePixel*>::iterator it_hole;
		MQImagePixel* temp;
		int hole_count = HolePixels.size();
		while(HolePixels.size()> 0)
		{
			//�i��
			fflush(stdout);
			printf("\rFillHole:%.0f%%",(1.0f-((float)HolePixels.size()/hole_count))*100);
			temp = HolePixels.front();
			i = temp->position / imageSize;
			j = temp->position % imageSize;

			findBestMatch(j, i, size);
			for(it = temp->neighborHole.begin(); it !=temp->neighborHole.end();it++){
				ImagePixel[int(*it/imageSize)][int(*it%imageSize)].neighborHole.remove(temp->position);
			}

			HolePixels.erase(HolePixels.begin());
			sort(this->HolePixels.begin(),this->HolePixels.end(),my_sort);
		}
		printf("\n");
	}
	if(method == 4)//������
	{
		cout<<"Start fill hole for growth...\n";
		for(i=0; i<imageSize; i++)
		{
			//�i��
			fflush(stdout);
			printf("\rFillHole:%.0f%%",((float)(i+1)/imageSize)*100);
			for(j=0; j<imageSize; j++)
			{		
				if(ImagePixel[i][j].isHole && ImagePixel[i][j].R == 0)//����JPG�ϥ� if(ImagePixel[i][j].isHole && ImagePixel[i][j].R == -1)
				{	
					findBestMatch(j, i, size);
					bool filldone = false;
					int temp_x = j;
					int temp_y = i;
					while(true)
					{
						for(int y = temp_y-1;y <= temp_y+1;y++)
						{
							for(int x = temp_x-1;x<= temp_x+1;x++)
							{
								if(ImagePixel[y][x].isHole && ImagePixel[y][x].R == 0)//����JPG�ϥ� if(ImagePixel[i][j].isHole && ImagePixel[i][j].R == -1)
								{
									findBestMatch(x, y, size);
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
		printf("\n");
	}
	t_end = clock();
	float t_duration = (float)(t_end - t_start);
	printf("Fill hole done: %.2fs\n\n",t_duration/1000.0f);
	return;
}

void MQTriangleMesh::initializeTexture(int size)// initialize output texture with random pixels from the sample
{
	int i, j;
	int w, h;
	/*
	int valid_w_length = sample_w-size+1;
	int valid_h_length = sample_h-size/2;
	int dw = size/2;
	int dh = size/2;

	//srand(time(NULL));
	
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
	*/
	for (int i=pca_size/2; i<imageSize-pca_size/2; i++)
	{
		for(int j=pca_size/2; j<imageSize-pca_size/2; j++)
		{
			texture_red[i][j] = ImagePixel[i][j].R;
			texture_green[i][j] = ImagePixel[i][j].G;
			texture_blue[i][j] = ImagePixel[i][j].B;
			original_pos_x[i][j] = j;
			original_pos_y[i][j] = i;

			if(ImagePixel[i][j].Triangle != 0)//if(!ImagePixel[i][j].isHole)//	
				for(int n = 0 ; n < _SrcImgPixDim ; n++)	texture_lap[i][j][n] = ImagePixel[i][j].pca_data[n];
			else	
				for(int n = 0 ; n < _SrcImgPixDim ; n++)	texture_lap[i][j][n] = -1.0f;
		}
	}
	return;
	
}

void MQTriangleMesh::convertSample()// convert the sample from the stream of bytes to 2d array of pixels
{

	int i, j;

	sample_red = new float*[imageSize];
	sample_green = new float*[imageSize];
	sample_blue = new float*[imageSize];
	sample_lap = new float **[imageSize];
	for(i=0; i<imageSize; i++)
	{
		sample_red[i] = new float[imageSize];
		sample_green[i] = new float[imageSize];
		sample_blue[i] = new float[imageSize];
		sample_lap[i] = new float*[imageSize];
		for(int j = 0 ; j < imageSize ; j++) sample_lap[i][j] = new float[_SrcImgPixDim];
	}
	for (i=pca_size/2; i<imageSize-pca_size/2; i++)
	{
		for(j=pca_size/2; j<imageSize-pca_size/2; j++)
		{

			if(ImagePixel[i][j].Triangle == 0)
			{
				sample_red[i][j] = sample_green[i][j] = sample_blue[i][j] = -1.0;
				for(int n = 0 ; n < _SrcImgPixDim ; n++)	sample_lap[i][j][n] = -1.0f;
			}
			else
			{
				sample_red[i][j] = ImagePixel[i][j].R;
				sample_green[i][j] = ImagePixel[i][j].G;
				sample_blue[i][j] = ImagePixel[i][j].B;
				if(!ImagePixel[i][j].isHole)	
					for(int n = 0 ; n < _SrcImgPixDim ; n++)	sample_lap[i][j][n] = ImagePixel[i][j].pca_data[n];
				else	
					for(int n = 0 ; n < _SrcImgPixDim ; n++)	sample_lap[i][j][n] = -1.0f;
			}
		}
	}
	return;
}

void MQTriangleMesh::findBestMatch(int j, int i, int size)// find the best match for the texture image at pixel (i, j) using a window of size x size
{
	int actualw, actualh;
	int actualx, actualy;
	int bestw, besth;
	int ti, tj;
	int x, y;

	bool add;

	float bestd = -1.0f;
	float tempd;
	float r, g, b;

	//make local texture window
	for(y=0, ti = i-size/2; y < size; y++, ti++)
	{
		for(x=0, tj = j-size/2; x < size; x++, tj++)
		{
			if(ti < pca_size/2)	ti += imageSize-pca_size/2;				
			else if(ti >= imageSize-pca_size/2)	ti -= imageSize-pca_size/2;

			if(tj < pca_size/2)	tj += imageSize-pca_size/2;				
			else if(tj >= imageSize-pca_size/2)	tj -= imageSize-pca_size/2;

			//red[y][x] = texture_red[ti][tj];
			//green[y][x] = texture_green[ti][tj];
			//blue[y][x] = texture_blue[ti][tj];
			lap_sample[y][x] = texture_lap[ti][tj];
		}
	}

	for(int si = size/2+pca_size/2 ; si < imageSize- size/2-pca_size/2-1; si++)
	{
		for(int sj = size/2+pca_size/2 ;  sj < imageSize- size/2-pca_size/2-1; sj++)
		{
			if(sample_lap[si][sj][0] == -1.0) continue;
			tempd = 0;
			bool isDone = true;
			for(y = si - size/2,ti = 0 ; y <= si + size/2 ; y++,ti++)
			{
				for(x = sj - size/2,tj=0 ; x <= sj + size/2 ; x++,tj++)
				{
					if(tempd > bestd && bestd != -1.0)	break;
					
					if(lap_sample[ti][tj][0] == -1.0) continue;		
					if(sample_lap[y][x][0] == -1.0){
						isDone = false;
						break;
					}
					//r = red[ti][tj]-sample_red[y][x];
					//g = green[ti][tj]-sample_green[y][x];
					//b = blue[ti][tj]-sample_blue[y][x];
					for(int n = 0 ; n < _SrcImgPixDim ; n++){						
						tempd+= pow(lap_sample[ti][tj][n]-sample_lap[y][x][n],2);
					}
					///tempd += r*r + g*g + b*b;
				}
				if(!isDone) break;
			}	
			if(!isDone) continue;;
			if(tempd < bestd || (bestd == -1.0 && tempd !=0))
			{
				bestw = sj;
				besth = si;
				bestd = tempd;		

			}
		}
	}
	/*
	fstream file;
	file.open("test_out.txt",ios::app);
	if(!file)     //�ˬd�ɮ׬O�_���\�}��
	{
		cerr << "Can't open file!\n";
		exit(1);     //�b�����`���ΤU�A���_�{��������
	}
	file<< "i:" << i << "," << "j:" << j << "\n";
	file<< "w:" << bestw << "," << "h:" << besth << "\n";
	file.close();
	*/
	if(besth <  size/2+pca_size/2 || besth > imageSize- size/2-pca_size/2) besth = size/2+pca_size/2 +rand()%(imageSize-size-pca_size);
	if(bestw <  size/2+pca_size/2 || bestw > imageSize- size/2-pca_size/2) bestw = size/2+pca_size/2 +rand()%(imageSize-size-pca_size);

	texture_red[i][j] = sample_red[besth][bestw];
	texture_green[i][j] = sample_green[besth][bestw];
	texture_blue[i][j] = sample_blue[besth][bestw];
	texture_lap[i][j] = sample_lap[besth][bestw];
	original_pos_x[i][j] = bestw;
	original_pos_y[i][j] = besth;

	ImagePixel[i][j].R = sample_red[besth][bestw];
	ImagePixel[i][j].G = sample_green[besth][bestw];
	ImagePixel[i][j].B = sample_blue[besth][bestw];
	ImagePixel[i][j].originX = bestw;
	ImagePixel[i][j].originY = besth;
	fillpoint.push_back(imageSize*i+j);

	return;
	/*
	//get candidates;
	candidate_x.clear();
	candidate_y.clear();
	for(actualh = i-size/2; actualh <= i+size/2; actualh++)
	{
		for(actualw = j-size/2; actualw <= j+size/2; actualw++)
		{
			x = (actualw+texture_w)%texture_w;
			y = (actualh+texture_h)%texture_h;
			
			if(texture_red[y][x] == -1.0)	continue;
			actualx = original_pos_x[y][x];
			actualy = original_pos_y[y][x];
			
			add = true;
			for(size_t c=0; c<candidate_x.size(); c++)
			{
				if(candidate_x[c] == actualx && candidate_y[c] == actualy)// already on candidate list					
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
		for(y=candidate_y[c]-size/2, ti=0; y<candidate_y[c]+size/2+1; y++, ti++)
		{
			if(y>= imageSize || y< 0) break;
			for(x=candidate_x[c]-size/2, tj=0; x<candidate_x[c]+size/2+1; x++, tj++)
			{
				if(x>= imageSize || x<0) break;
				if(tempd > bestd)	break;
				if(red[ti][tj] == -1) continue;

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
	*/
}

void MQTriangleMesh::TriangulateBaseMesh()
{
	triangulateio in, out, *vorout = NULL;
	float total_S= 0;
	float total_T= 0;
	baseMesh = new HoleMesh();
	baseMesh->VertexNum = Holes_uv.begin()->size();
	baseMesh->Vertex.resize(baseMesh->VertexNum+1);
	list<int>::iterator it = Holes_uv.begin()->begin();
	for(int i = 1 ;i<=baseMesh->VertexNum;i++){
		baseMesh->Vertex[i].S = this->Vertex[*it].S;
		baseMesh->Vertex[i].T = this->Vertex[*it].T;
		it++;
	}
	//�}����I�����(x1,y1,x2,y2,...)��in.pointlist
	//BMesh�O�ۤv�w�q����Ƶ��c
	in.pointlist = new REAL[baseMesh->VertexNum*2];	
	for(int i = 1; i <=baseMesh->VertexNum; i++)
	{
		in.pointlist[(i-1)*2] = baseMesh->Vertex[i].S;
		in.pointlist[(i-1)*2+1] = baseMesh->Vertex[i].T;
	}

	in.pointattributelist = NULL;
	in.pointmarkerlist = NULL;
	//�}����I���ƥ�
	in.numberofpoints = baseMesh->VertexNum;
	in.numberofpointattributes = 0;
	in.trianglelist = NULL;
	in.triangleattributelist = NULL;
	in.trianglearealist = NULL;
	in.numberoftriangles = 0;
	in.numberofcorners = 0;
	in.numberoftriangleattributes = 0;
	//�}����䪺���(�@����Ѩ���I�s���Һc���A�s���q0�}�l)��in.segmentlist
	//VBEdgeList�O�ۤv�w�q����Ƶ��c
	in.segmentlist = new int[ 2* baseMesh->VertexNum];
	for(int i = 1 ; i <=  baseMesh->VertexNum; i++)
	{
		if(i ==  baseMesh->VertexNum)
		{
			in.segmentlist[(i-1)*2]   = i-1;
			in.segmentlist[(i-1)*2+1] = 0;
		}
		else
		{
			in.segmentlist[(i-1)*2]   = i-1;
			in.segmentlist[(i-1)*2+1] = i;
		}
	}
	in.segmentmarkerlist = NULL;
	//�}����䪺�ƥ�
	in.numberofsegments = baseMesh->VertexNum;
	in.holelist = NULL;
	in.numberofholes = 0;
	in.regionlist = NULL;
	in.numberofregions = 0;

	out.pointlist = NULL;
	out.pointattributelist = NULL;
	out.pointmarkerlist = NULL;
	out.numberofpoints = 0;
	out.numberofpointattributes = 0;
	out.trianglelist = NULL;
	out.triangleattributelist = NULL;
	out.neighborlist = NULL;
	out.numberoftriangles = 0;
	out.numberofcorners = 0;
	out.numberoftriangleattributes = 0;
	out.segmentlist = NULL;
	out.segmentmarkerlist = NULL;
	out.numberofsegments = 0;
	out.holelist = NULL;
	out.numberofholes = 0;
	out.regionlist = NULL;
	out.numberofregions = 0;
	out.edgelist = NULL;
	out.edgemarkerlist = NULL;
	out.normlist = NULL;
	out.numberofedges = 0;

	//�T���ưѼƳ]�w
	char *option = "zYq28";

	//�T���ƥD�{��
	triangulate(option, &in, &out, vorout);
	
	//out���T���ƫ᪺���G�F�^���ۤv����Ƶ��cBMesh_1
	baseMesh->TriangleNum = out.numberoftriangles;
	baseMesh->TriangleTex.resize(baseMesh->TriangleNum +1);	
	for(int i = 1; i <= baseMesh->TriangleNum; i++)
	{
		baseMesh->TriangleTex[i].T1 = out.trianglelist[(i-1)*3]+1;
		baseMesh->TriangleTex[i].T2 = out.trianglelist[(i-1)*3+1]+1;
		baseMesh->TriangleTex[i].T3 = out.trianglelist[(i-1)*3+2]+1;
	}
	baseMesh->VertexNum = out.numberofpoints;
	baseMesh->Vertex.resize(baseMesh->VertexNum);
	for(int i = 1; i <= baseMesh->VertexNum; i++)
	{
		baseMesh->Vertex[i].S = out.pointlist[(i-1)*2];
		baseMesh->Vertex[i].T = out.pointlist[(i-1)*2+1];
	}
	//free memory
	delete [] in.pointlist;
	free(out.pointlist);
	free(out.trianglelist);
	free(out.segmentlist);
}

void MQTriangleMesh::Draw(GLubyte Red, GLubyte Green, GLubyte Blue)
{
	
	if(Holes.size()>0)
	{
		glBegin(GL_LINES);
		list<int>::iterator hole_it;
		list<list<int>>::iterator begin = Holes.begin();
		for(hole_it = begin->begin(); hole_it != begin->end();hole_it++)
		{
			if(hole_it == begin->begin())
			{
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(0,0,0);
				glVertex3f(this->Vertex[*hole_it].X, this->Vertex[*hole_it].Y,this->Vertex[*hole_it].Z);
			}
			else
			{
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
	glBegin(GL_TRIANGLES);
	
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
	
	//�T���ƪ��u	
	if(baseMesh){
		for(int i = 1 ; i <= baseMesh->TriangleNum; i++){
			int t1 = baseMesh->TriangleTex[i].T1;
			int t2 = baseMesh->TriangleTex[i].T2;
			int t3 = baseMesh->TriangleTex[i].T3;

			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(0,0,0);
			glVertex3f(baseMesh->Vertex[t1].S, baseMesh->Vertex[t1].T, 0.0);

			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(0,0,0);
			glVertex3f(baseMesh->Vertex[t2].S, baseMesh->Vertex[t2].T, 0.0);

			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(0,0,0);
			glVertex3f(baseMesh->Vertex[t3].S, baseMesh->Vertex[t3].T, 0.0);
		}
	}
	
	glEnd();
	
	//Draw hole bounding box
	/*
	glBegin(GL_QUADS);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(1,0,0);
		glVertex3f(this->hole_boundaryX.first, this->hole_boundaryY.first, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,1,0);
		glVertex3f(this->hole_boundaryX.second, this->hole_boundaryY.first, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,1);
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
	for(hole_it = begin->begin(); hole_it != begin->end();hole_it++)
	{
		if(hole_it == begin->begin())
		{
			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(255,255,255);
			glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
		}
		else
		{
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
	/*
	//Draw Point
	glBegin(GL_POINTS);
	list<int>::iterator hole_it;
	list<list<int>>::iterator begin = Holes_uv.begin();
	for(hole_it = begin->begin(); hole_it != begin->end();hole_it++)
	{
			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(255,255,255);
			glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
	}
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
	glLineWidth(3.0); 
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	if(fillpoint.size()>0){
		glVertex3f(this->ImagePixel[first_point/imageSize][first_point%imageSize].X, this->ImagePixel[first_point/imageSize][first_point%imageSize].Y, 0.0);
		glVertex3f(this->ImagePixel[second_point/imageSize][second_point%imageSize].X, this->ImagePixel[second_point/imageSize][second_point%imageSize].Y, 0.0);
	}
	glEnd();

	glBegin(GL_POINTS);
	for(int i = 0; i < this->imageSize; i++)
	{
		for(int j = 0; j < this->imageSize;j++)
		{
			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(this->ImagePixel[i][j].R,this->ImagePixel[i][j].G,this->ImagePixel[i][j].B);
			glVertex3f(this->ImagePixel[i][j].X, this->ImagePixel[i][j].Y, 0.0);
		}
	}
	glEnd();
	
	
	
}