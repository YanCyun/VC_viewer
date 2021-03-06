#include "MQTriangleMesh.h"
#include "MQglm.h"
#include "MQPCA.h"
#include <iostream>
#include <math.h>
#include <algorithm> 
#include <time.h>
#include <fstream>
#include "Eigen\SparseCore"
#include "Eigen\IterativeLinearSolvers"
#include "Eigen\SparseCholesky"

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

	this->Filename = string(FileName);

	if(baseMesh){
		delete baseMesh;
		baseMesh =NULL;		
	}
	this->VertexNum = (int)(_model->numvertices);
	this->Vertex.clear();
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
	this->Triangle.clear();
	this->Triangle.resize( (this->TriangleNum)+1 );
	this->TriangleTex.clear();
	this->TriangleTex.resize((this->TriangleNum)+1);
	this->Edges.clear();
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
		this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)]->uv_edge = pair<int,int>(this->TriangleTex[i].T1,this->TriangleTex[i].T2);
		//Edge2
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->nextHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)];
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->oppositeHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V2)];
		this->Edges[pair<int,int>(this->Triangle[i].V2,this->Triangle[i].V3)]->uv_edge = pair<int,int>(this->TriangleTex[i].T2,this->TriangleTex[i].T3);
		//Edge3
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->nextHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V2)];
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->oppositeHalfEdge = this->Edges[pair<int,int>(this->Triangle[i].V1,this->Triangle[i].V3)];
		this->Edges[pair<int,int>(this->Triangle[i].V3,this->Triangle[i].V1)]->uv_edge = pair<int,int>(this->TriangleTex[i].T3,this->TriangleTex[i].T1);
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

void MQTriangleMesh::WriteObjFile(void)
{
	fstream file;

	size_t pos = this->Filename.find_last_of("/\\");
	file.open("result/result_"+this->Filename.substr(pos+1),ios::out);

	if(!file)     //檢查檔案是否成功開啟
	{
		cerr << "Can't open file!\n";
		exit(1);     //在不正常情形下，中斷程式的執行
	}

	for(int i = 1 ; i <= this->VertexNum ; i++)
	{
		file<< "v " << Vertex[i].X << " " << Vertex[i].Y << " " << Vertex[i].Z << "\n";
	}
	file<<"\n";
	for(int i = 1 ; i <= this->VertexNum ; i++)
	{
		file<< "vt " << Vertex[i].S << " " << Vertex[i].T << " " << "\n";
	}
	file<<"\n";
	for(int i = 1 ; i <= this->TriangleNum ; i++)
	{
		file<< "f " << Triangle[i].V1 << "/" << TriangleTex[i].T1<< " " << Triangle[i].V2 << "/" << TriangleTex[i].T2<< " " << Triangle[i].V3 << "/" << TriangleTex[i].T3<< " " << "\n";
	}

	file.close();
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

		//內積判斷凹凸(正負)
		float dot = Vertex[i].LapX * Vertex[i].NX + Vertex[i].LapY * Vertex[i].NY + Vertex[i].LapZ * Vertex[i].NZ;
		if(dot < 0) Vertex[i].in_or_out = true;
	}
}

void MQTriangleMesh::UpdatePointStruct(void)
{

	//設定起始點在左上角
	float centerX = (boundaryX.first+boundaryX.second)/2;
	float centerY = (boundaryY.first+boundaryY.second)/2;
	float startX = centerX - boundary/2;
	float startY = centerY + boundary/2;

	float maxLap_img = 0;
	float minLap_img = 0;

	//設定每個Pixel的間隔
	float pointDistance = boundary/imageSize;

	int tri;

	bool checkHole = false;
	vector<int> tempHoles;
	vector<int>::iterator hole_it;
	this->ImagePixel.resize(imageSize);
	for(int i = 0 ; i < imageSize ; i++)
	{
		//進度
		fflush(stdout);
		printf("\rUpdatePixel:%.0f%%",((float)i/imageSize)*100);
		this->ImagePixel[i].clear();
		this->ImagePixel[i].resize(imageSize);
		tempHoles.clear();
		checkHole = false;
		for(int j = 0 ; j < imageSize ; j++)
		{
			//設定每個像素的座標
			this->ImagePixel[i][j].X = startX + j*pointDistance;
			this->ImagePixel[i][j].Y = startY - i*pointDistance;
			
			if(j > 0) tri =  this->ImagePixel[i][j-1].Triangle;
			else	tri = 0;

			this->PointInTriangel(&this->ImagePixel[i][j],tri);
			
			//如果像素在UV三角形裡
			if(this->ImagePixel[i][j].Triangle != 0)
			{
				//得到洞的像素
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
	this->HolePixels.clear();
	this->RotateLaplacianPixels.clear();
	for(int i = 0 ; i < imageSize ; i++)
	{
		for(int j = 0 ; j < imageSize ; j++)
		{
			if(this->ImagePixel[i][j].isHole)
			{
				//判斷洞周圍的像素是否也為洞,是的話將她加到neighborhole的陣列裡,補洞會使用到(Random,Neighborhood)
				for(int y = i-1 ; y <= i+1 ;y++)
				{
					for(int x = j-1 ; x <= j+1 ; x++)
					{
						if(y == i && x == j) continue;
						if(this->ImagePixel[y][x].isHole)	this->ImagePixel[i][j].neighborHole.push_back(y*imageSize+x);
						else this->ImagePixel[i][j].neighborLap.push_back(y*imageSize+x);
					}
				}
				//將二維座標轉為一維座標
				ImagePixel[i][j].position = i*imageSize + j;
				this->HolePixels.push_back(&ImagePixel[i][j]);		
			}
		} 
	}
	//將洞的像素依造鄰居的多寡排序(少->多)
	sort(this->HolePixels.begin(),this->HolePixels.end(),my_sort);//覆寫排序方法

	//找到洞的像素的Bounding Box 像素的x,y範圍,補洞會使用到(BoundingBox)
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

	int ex_dim = pca_size*pca_size;//設定鄰居Lap維度
	int range = (pca_size-1)/2;

	//分配記憶體
	LaplaianLengths.resize(imageSize);
	for(int i = 0 ; i < imageSize ; i++){
		LaplaianLengths[i].clear();
		LaplaianLengths[i].resize(imageSize);
		for(int j = 0 ; j < imageSize ; j++){
			LaplaianLengths[i][j] = new LaplaianLength();
		}
	}

	//PCA資料準備
	tempLaplaianLength.clear();
	for(int i = range ; i < imageSize -range; i++)
	{
		fflush(stdout);
		printf("\rPrepare PCA data:%.0f%%",((float)(i-range)/(imageSize-range*2))*100);
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
			for(int y = i - range ; y <= i + range ; y++)
			{
				for(int x = j - range ; x <= j + range ; x++)
				{					
					if(ImagePixel[y][x].Triangle == 0)
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
		for(int i = (*lap_it)->Y-range ; i <= (*lap_it)->Y+range ; i++)
		{
			for(int j = (*lap_it)->X-range ; j <= (*lap_it)->X+range;j++)
			{
				if(LaplaianLengths[i][j]->pca) (*lap_it)->neighbor_pca++;
			}			
		}
	}

	sort(tempLaplaianLength.begin(),tempLaplaianLength.end(),my_sort2);//排序鄰居完成PCA的數量
	
	LaplaianLength* it;
	while(tempLaplaianLength.size() > 0){
		it = tempLaplaianLength.back();
		it->length.resize(ex_dim+1);
		it->length[0] = 0.0f;
		int amount = 0;
		for(int i = it->Y - range ; i <= it->Y + range ; i++)
		{
			for(int j = it->X - range ; j<= it->X + range ; j++)
			{
				if(LaplaianLengths[i][j]->pca)
				{
					for(int x = 1 ; x <= ex_dim ; x++)	it->length[x] += LaplaianLengths[i][j]->length[x];	//將鄰居PCA依照index相加					
				}
			}
		}
		for(int x = 1 ; x <= ex_dim ; x++)	{
			it->length[x] /= it->neighbor_pca;//取平均;
		}
		
		it->pca = true;

		for(int i = it->Y - range ; i <= it->Y + range ; i++)
		{
			for(int j = it->X - range ; j<= it->X + range ; j++)
			{
				if(!LaplaianLengths[i][j]->pca && !LaplaianLengths[i][j]->hole && !LaplaianLengths[i][j]->boundary) LaplaianLengths[i][j]->neighbor_pca++;
			}
		}
		tempLaplaianLength.pop_back();
		sort(tempLaplaianLength.begin(),tempLaplaianLength.end(),my_sort2);
	}
	printf("\rPrepare PCA data:100%%\n");
	
	fflush(stdout);
	printf("\rPCA Analysis");
	MQPCA PCA_1;
	PCA_1.SetDataItemDimension(ex_dim);		//告訴PCA每筆資料的維度5x5=25
	for(int i = range; i < imageSize - range; ++i)
	{
		for(int j = range; j < imageSize - range; ++j)
		{			
			if(!LaplaianLengths[i][j]->pca) continue;
			bool add_result = PCA_1.AddDataItem(LaplaianLengths[i][j]->length);	//將appearance space的像素(邊緣不計)加入PCA的數據庫
			if(add_result == false){	
				cout<<"PCA input data error!"<<endl;
			}
		}
	}
	fflush(stdout);
	printf("\rPCA Analysis.");
	PCA_1.PrincipalComponentAnalysis();	//進行PCA分析
	fflush(stdout);
	printf("\rPCA Analysis..");
	#ifdef MQDebug
		PCA_1.DisplayCovMatrixEvals();	//debug用，列出PCA相關的eigen value等
	#endif
		
		for(int i = range; i < imageSize - range; ++i)
		{
			for(int j = range; j < imageSize - range; ++j)
			{
				if(!LaplaianLengths[i][j]->pca) continue;
				
				//_SrcImgPixDim=3(代表PCA降維後的維度)；flvecx表示PCA降維後的向量(也就是5x5x1=25->3降維的向量)
				vector<float> flvecx = PCA_1.ProjectDataItem(LaplaianLengths[i][j]->length, _SrcImgPixDim);

				//將PCA降維後的向量assign給_SrcImg
				for(int k = 1; k <= _SrcImgPixDim; ++k){
					ImagePixel[i][j].pca_data.push_back(flvecx[k]);
				}
			}
		}
	printf("\rPCA Analysis Done\n");

	for(int x = 0 ; x < 3 ; x++)
	{
		for(int i = range ; i < imageSize - range ; i++)
		{
			for(int j = range ; j < imageSize - range ; j++)
			{
				if(!LaplaianLengths[i][j]->pca) continue;

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
		fflush(stdout);
		printf("\rCalculate Pixel Color:%.0f%%",((float)(i-range)/(imageSize-range*2))*100);
		for(int j = range ; j < imageSize - range ; j++)
		{
			if(this->ImagePixel[i][j].Triangle == 0) continue;
			//if(LaplaianLengths[i][j]->hole || LaplaianLengths[i][j]->boundary) continue;
			this->ImagePixel[i][j].R = (this->ImagePixel[i][j].pca_data[0] - minLap_img) *  normalize_number;
			this->ImagePixel[i][j].G = (this->ImagePixel[i][j].pca_data[1] - minLap_img) *  normalize_number;
			this->ImagePixel[i][j].B = (this->ImagePixel[i][j].pca_data[2] - minLap_img) *  normalize_number;	

		}
	}
	printf("\rCalculate Pixel Color:100%%\n");
	fflush(stdout);
}

void MQTriangleMesh::FillHole(int method)
{
	this->convertSample();
	this->setTexture(imageSize);
	this->generateTexture(window_size,method);
	this->RotateLaplacian();
	this->TriangulateBaseMesh();
	this->RebuildingCoordination();
	this->WriteObjFile();
	printf("Done!\n");
}

void MQTriangleMesh::RotateLaplacian()
{
	cout<<"Start RotateLaplacian\n";
	list<int>::iterator it;
	vector<MQImagePixel>::iterator it_hole;
	int index = 0;
	MQImagePixel* temp;
	int hole_count = RotateLaplacianPixels.size();
	int amount = 0;
	while(RotateLaplacianPixels.size()> 0)
	{
		//進度
		fflush(stdout);
		printf("\rRotateLaplacian:%.0f%%",(1.0f-((float)RotateLaplacianPixels.size()/hole_count))*100);
		do{
			index = rand()%RotateLaplacianPixels.size();
			temp = RotateLaplacianPixels[index];	
		}while(temp->neighborLap.size() <= 3);

		int i = (int)temp->position / imageSize;
		int j = (int)temp->position % imageSize;

		MQImagePixel tempImage = ImagePixel[ImagePixel[i][j].originY][ImagePixel[i][j].originX];
		float tempLapX = 0;
		float tempLapY = 0;
		float tempLapZ = 0;
		int count_Lap = 0;
		
		for(it = temp->neighborLap.begin(); it !=temp->neighborLap.end();it++)
		{
			int distance_y = int(*it/imageSize) - i;
			int distance_x = int(*it%imageSize) - j;
			MQImagePixel ori_tempImage = ImagePixel[ImagePixel[i][j].originY+distance_y][ImagePixel[i][j].originX+distance_x];
			MQImagePixel nei_tempImage = ImagePixel[int(*it/imageSize)][int(*it%imageSize)];

			if(ori_tempImage.ori_lap_length == 0 || nei_tempImage.ori_lap_length == 0)continue;
			
			tempLapX += nei_tempImage.LapX/nei_tempImage.ori_lap_length - ori_tempImage.LapX/ori_tempImage.ori_lap_length; 
			tempLapY += nei_tempImage.LapY/nei_tempImage.ori_lap_length - ori_tempImage.LapY/ori_tempImage.ori_lap_length; 
			tempLapZ += nei_tempImage.LapZ/nei_tempImage.ori_lap_length - ori_tempImage.LapZ/ori_tempImage.ori_lap_length; 
			count_Lap++;
 		}
		
		ImagePixel[i][j].ori_LapX = tempImage.LapX;
		ImagePixel[i][j].ori_LapY = tempImage.LapY;
		ImagePixel[i][j].ori_LapZ = tempImage.LapZ;
		
		ImagePixel[i][j].LapX = tempImage.LapX/tempImage.ori_lap_length + tempLapX/count_Lap;
		ImagePixel[i][j].LapY = tempImage.LapY/tempImage.ori_lap_length + tempLapY/count_Lap;
		ImagePixel[i][j].LapZ = tempImage.LapZ/tempImage.ori_lap_length + tempLapZ/count_Lap;
		ImagePixel[i][j].Lap_length = sqrt(pow(ImagePixel[i][j].LapX,2)+pow(ImagePixel[i][j].LapY,2)+pow(ImagePixel[i][j].LapZ,2));
		
		ImagePixel[i][j].LapX = (ImagePixel[i][j].LapX/ImagePixel[i][j].Lap_length)*tempImage.ori_lap_length;
		ImagePixel[i][j].LapY = (ImagePixel[i][j].LapY/ImagePixel[i][j].Lap_length)*tempImage.ori_lap_length;
		ImagePixel[i][j].LapZ = (ImagePixel[i][j].LapZ/ImagePixel[i][j].Lap_length)*tempImage.ori_lap_length;
		ImagePixel[i][j].ori_lap_length = tempImage.ori_lap_length;
		ImagePixel[i][j].Lap_length = tempImage.Lap_length;
		for(int y = i - 1 ; y <= i + 1 ;y++)
		{
			for(int x = j - 1 ; x <= j + 1 ; x++)
			{
				if(ImagePixel[y][x].Lap_length == 0)
				{
					ImagePixel[y][x].neighborLap.push_back(i*imageSize+j);
				}
			}
		}

		RotateLaplacianPixels.erase(RotateLaplacianPixels.begin()+index);
	}
	printf("\n");

}

//判斷像素是否在UV三角形內
void MQTriangleMesh::PointInTriangel(MQImagePixel *p,int tri)
{

	float a1,a2,a3;
	float sum_a;
	//如果鄰居像素在UV三角形內
	if(tri != 0)
	{
		a1 = (Vertex[TriangleTex[tri].T1].S - p->X) * (Vertex[TriangleTex[tri].T2].T - p->Y) - (Vertex[TriangleTex[tri].T1].T - p->Y) * (Vertex[TriangleTex[tri].T2].S - p->X);//x1y1-y1y2
		a2 = (Vertex[TriangleTex[tri].T2].S - p->X) * (Vertex[TriangleTex[tri].T3].T - p->Y) - (Vertex[TriangleTex[tri].T2].T - p->Y) * (Vertex[TriangleTex[tri].T3].S - p->X);
		a3 = (Vertex[TriangleTex[tri].T3].S - p->X) * (Vertex[TriangleTex[tri].T1].T - p->Y) - (Vertex[TriangleTex[tri].T3].T - p->Y) * (Vertex[TriangleTex[tri].T1].S - p->X);

		if(a1>=0 && a2>=0 && a3>=0)
		{

			p->Triangle = tri;
			sum_a = a1+a2+a3;
			p->LapX = Vertex[Triangle[tri].V1].LapX * a2/sum_a + Vertex[Triangle[tri].V2].LapX * a3/sum_a + Vertex[Triangle[tri].V3].LapX * a1/sum_a; //v1*(a2/(a1+a2+a3)) + v2*(a3/(a1+a2+a3)) + v3 + (a3/(a1+a2+a3)) 
			p->LapY = Vertex[Triangle[tri].V1].LapY * a2/sum_a + Vertex[Triangle[tri].V2].LapY * a3/sum_a + Vertex[Triangle[tri].V3].LapY * a1/sum_a;
			p->LapZ = Vertex[Triangle[tri].V1].LapZ * a2/sum_a + Vertex[Triangle[tri].V2].LapZ * a3/sum_a + Vertex[Triangle[tri].V3].LapZ * a1/sum_a;
			p->ori_lap_length = sqrt(pow(p->LapX,2)+pow(p->LapY,2)+pow(p->LapZ,2));
			p->Lap_length = Vertex[Triangle[tri].V1].Lap_length * a2/sum_a + Vertex[Triangle[tri].V2].Lap_length * a3/sum_a + Vertex[Triangle[tri].V3].Lap_length * a1/sum_a;
			return;

		}
	}
	//如果鄰居像素沒有在UV三角形內便地毯式搜尋
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
		p->LapX = Vertex[Triangle[i].V1].LapX * a2/sum_a + Vertex[Triangle[i].V2].LapX * a3/sum_a + Vertex[Triangle[i].V3].LapX * a1/sum_a; //v1*(a2/(a1+a2+a3)) + v2*(a3/(a1+a2+a3)) + v3 + (a3/(a1+a2+a3)) 
		p->LapY = Vertex[Triangle[i].V1].LapY * a2/sum_a + Vertex[Triangle[i].V2].LapY * a3/sum_a + Vertex[Triangle[i].V3].LapY * a1/sum_a;
		p->LapZ = Vertex[Triangle[i].V1].LapZ * a2/sum_a + Vertex[Triangle[i].V2].LapZ * a3/sum_a + Vertex[Triangle[i].V3].LapZ * a1/sum_a;
		p->ori_lap_length = sqrt(pow(p->LapX,2)+pow(p->LapY,2)+pow(p->LapZ,2));
		p->Lap_length = Vertex[Triangle[tri].V1].Lap_length * a2/sum_a + Vertex[Triangle[tri].V2].Lap_length * a3/sum_a + Vertex[Triangle[tri].V3].Lap_length * a1/sum_a;
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
	}

}

void MQTriangleMesh::FindBoundary(void)
{
	//找到UV的Bounding Box範圍
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
	//將Bounding Box 範圍放大1.05倍
	boundary = max(boundaryX.second-boundaryX.first,boundaryY.second-boundaryY.first) *1.05;

	///找到UV洞的像素的Bounding Box 範圍
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
	//找到單邊(Single Edge)
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
	//將單邊連接成洞(v)	
	map<int,int>::iterator it;
	list<int> hole;
	list<int> hole_uv;
	int first_point,search_point,temp;
	int uv_first_point,uv_search_point,uv_temp;
	Holes_uv.clear();
	Holes.clear();
	while(!singleEdge.empty())
	{
		it = singleEdge.begin();
		
		first_point = it->first;  //設置洞的起始點
		uv_first_point = Edges[pair<int,int>(singleEdge[first_point],first_point)]->uv_edge.second;
		
		hole.push_back(first_point);
		hole_uv.push_back(uv_first_point);
		
		search_point = singleEdge[first_point];
		uv_search_point = singleEdge_uv[uv_first_point];

		singleEdge.erase(first_point);
		singleEdge_uv.erase(uv_first_point);

		//如果search_point == first_point 表示圍成一個洞,繼續將剩下的單邊連成另外的洞
		while(search_point != first_point)
		{
			hole.push_back(search_point);
			hole_uv.push_back(uv_search_point);

			temp = search_point;
			uv_temp = uv_search_point;

			search_point = singleEdge[search_point];
			uv_search_point = singleEdge_uv[uv_search_point];
			singleEdge.erase(temp);	
			singleEdge_uv.erase(uv_temp);
			if(!search_point){
				break;
			}
		}
		//if(search_point){
			Holes.push_back(hole);	
			Holes_uv.push_back(hole_uv);	
		//}
		hole.clear();		
		hole_uv.clear();
	}

	vector<float> hole_length;
	list<list<int>>::iterator hole_it;
	list<list<int>>::iterator hole_uv_it;
	list<int>::iterator point_it;
	list<int>::iterator neighbor_it;
	
	//校正洞(hole)跟邊緣(boundary)的數值
	for(hole_it = Holes.begin() ; hole_it != Holes.end() ; hole_it++)
	{
		list<int> tempholepoint = *hole_it;

		while(tempholepoint.size() > 0)
		{
			
			point_it = tempholepoint.begin();
			if(Vertex[*point_it].NeighborVertex.size() == 3)//洞的點鄰居只有一個不是洞的boundary point
			{
				bool next = true;	
				for(neighbor_it = Vertex[*point_it].NeighborVertex.begin() ; neighbor_it != Vertex[*point_it].NeighborVertex.end() ; neighbor_it++)
				{
					if(Vertex[*neighbor_it].evaluate == true)//如果其中的boundary point已經重新計算過
					{
						next = false;
						break;
					}
				}
				if(next){//如果boudary point 都無重新計算過,將現在的point移到list的最後
					tempholepoint.push_back(*point_it);
					tempholepoint.pop_front();
					continue;
				}
			}
			float tempX,tempY,tempZ;
			int total_length;
			tempX = tempY = tempZ = total_length =  0;
			//重新計算laplacian
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
			tempholepoint.erase(tempholepoint.begin());
		}
		
	}
	minLap = maxLap = 0.0;
	for(int i = 1; i <= this->VertexNum; i++)
	{
		this->Vertex[i].Lap_length = sqrt(pow(this->Vertex[i].LapX,2)+pow(this->Vertex[i].LapY,2)+pow(this->Vertex[i].LapZ,2));
		if(this->Vertex[i].in_or_out) this->Vertex[i].Lap_length *= -1;
		//找到Laplacian的最大最小值 , 以便之後轉換成顏色
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
	
	//計算UV的洞的長度
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
	//刪除最長的洞(Boundary)的陣列
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

void MQTriangleMesh::generateTexture(int size,int method)// generate the texture from the sample using a search window of size x size (method 1:隨機 2:Boounding Box 3:取鄰居最多 4:成長式)
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
	RotateLaplacianPixels = HolePixels;
	if(method == 1)//隨機
	{
		cout<<"Start fill hole(Random)...\n";
		list<int>::iterator it;
		vector<MQImagePixel>::iterator it_hole;
		int index = 0;
		MQImagePixel* temp;
		int hole_count = HolePixels.size();
		int amount = 0;
		while(HolePixels.size()> 0)
		{
			//進度
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
		cout<<"Start fill hole(Bounding Box)...\n";
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
			//進度
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
	if(method == 3)//取鄰居最多像素
	{
		cout<<"Start fill hole(Neighborhood) ...\n";
		list<int>::iterator it;
		vector<MQImagePixel*>::iterator it_hole;
		MQImagePixel* temp;
		int hole_count = HolePixels.size();
		while(HolePixels.size()> 0)
		{
			//進度
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
	if(method == 4)//成長式
	{
		cout<<"Start fill hole(Growth)...\n";
		for(i=0; i<imageSize; i++)
		{
			//進度
			fflush(stdout);
			printf("\rFillHole:%.0f%%",((float)(i+1)/imageSize)*100);
			for(j=0; j<imageSize; j++)
			{		
				if(ImagePixel[i][j].isHole && ImagePixel[i][j].R == 0)//測試JPG使用 if(ImagePixel[i][j].isHole && ImagePixel[i][j].R == -1)
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
								if(ImagePixel[y][x].isHole && ImagePixel[y][x].R == 0)//測試JPG使用 if(ImagePixel[i][j].isHole && ImagePixel[i][j].R == -1)
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
	
	for (int i=pca_size/2; i<imageSize-pca_size/2; i++)
	{
		for(int j=pca_size/2; j<imageSize-pca_size/2; j++)
		{
			
			original_pos_x[i][j] = j;
			original_pos_y[i][j] = i;

			if(ImagePixel[i][j].Triangle == 0)//if(!ImagePixel[i][j].isHole)//	
			{
				texture_red[i][j] = texture_green[i][j] = texture_blue[i][j] = -1.0;
				for(int n = 0 ; n < _SrcImgPixDim ; n++)	texture_lap[i][j][n] = -1.0f;	
			}
			else	
			{
				texture_red[i][j] = ImagePixel[i][j].R;
				texture_green[i][j] = ImagePixel[i][j].G;
				texture_blue[i][j] = ImagePixel[i][j].B;
				for(int n = 0 ; n < _SrcImgPixDim ; n++)	texture_lap[i][j][n] = ImagePixel[i][j].pca_data[n];
			}
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
				for(int n = 0 ; n < _SrcImgPixDim ; n++)	sample_lap[i][j][n] = ImagePixel[i][j].pca_data[n];
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

					for(int n = 0 ; n < _SrcImgPixDim ; n++){						
						tempd+= pow(lap_sample[ti][tj][n]-sample_lap[y][x][n],2);
					}

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
	if(!file)     //檢查檔案是否成功開啟
	{
		cerr << "Can't open file!\n";
		exit(1);     //在不正常情形下，中斷程式的執行
	}
	file<< "i:" << i << "," << "j:" << j << "\n";
	file<< "w:" << bestw << "," << "h:" << besth << "\n";
	file.close();
	*/
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
	//洞邊界點的資料(x1,y1,x2,y2,...)給in.pointlist
	//BMesh是自己定義的資料結構
	in.pointlist = new REAL[baseMesh->VertexNum*2];	
	for(int i = 1; i <=baseMesh->VertexNum; i++)
	{
		in.pointlist[(i-1)*2] = baseMesh->Vertex[i].S;
		in.pointlist[(i-1)*2+1] = baseMesh->Vertex[i].T;
	}

	in.pointattributelist = NULL;
	in.pointmarkerlist = NULL;
	//洞邊界點的數目
	in.numberofpoints = baseMesh->VertexNum;
	in.numberofpointattributes = 0;
	in.trianglelist = NULL;
	in.triangleattributelist = NULL;
	in.trianglearealist = NULL;
	in.numberoftriangles = 0;
	in.numberofcorners = 0;
	in.numberoftriangleattributes = 0;
	//洞邊界邊的資料(一個邊由兩個點編號所構成，編號從0開始)給in.segmentlist
	//VBEdgeList是自己定義的資料結構
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
	//洞邊界邊的數目
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

	//三角化參數設定
	char *option = "pzYq30";

	//三角化主程式
	triangulate(option, &in, &out, vorout);
	
	//out為三角化後的結果；回填到自己的資料結構BMesh_1
	baseMesh->TriangleNum = out.numberoftriangles;
	baseMesh->TriangleTex.resize(baseMesh->TriangleNum + 1);	
	for(int i = 1; i <= baseMesh->TriangleNum; i++)
	{
		baseMesh->TriangleTex[i].T1 = out.trianglelist[(i-1)*3]+1;
		baseMesh->TriangleTex[i].T2 = out.trianglelist[(i-1)*3+1]+1;
		baseMesh->TriangleTex[i].T3 = out.trianglelist[(i-1)*3+2]+1;
	}
	baseMesh->VertexNum = out.numberofpoints;
	baseMesh->Vertex.resize(baseMesh->VertexNum + 1);

	list<int>::iterator hole_it;
	list<int>::iterator hole_uv_it;
	for(int i = 1; i <= baseMesh->VertexNum; i++)
	{
		baseMesh->Vertex[i].S = out.pointlist[(i-1)*2];
		baseMesh->Vertex[i].T = out.pointlist[(i-1)*2+1];
		for(hole_uv_it = Holes_uv.begin()->begin(),hole_it = Holes.begin()->begin(); hole_uv_it != Holes_uv.begin()->end() ; hole_uv_it++,hole_it++)
		{
			if(baseMesh->Vertex[i].S == Vertex[*hole_uv_it].S && baseMesh->Vertex[i].T == Vertex[*hole_uv_it].T)
			{
				baseMesh->boundary.push_back(i);
				baseMesh->Vertex[i].origin_index = *hole_it;
				baseMesh->Vertex[i].origin_uv_index = *hole_uv_it;
			}
		}
	}

	//free memory
	delete [] in.pointlist;
	free(out.pointlist);
	free(out.trianglelist);
	free(out.segmentlist);

	//TriangulateBaseMeshAgain();

}
void MQTriangleMesh::TriangulateBaseMeshAgain()
{
	triangulateio in, out, *vorout = NULL;

	//增加三角化的點
	int t1,t2,t3;

	for(int i  = 1 ; i <= baseMesh->TriangleNum ; i++){

		baseMesh->VertexNum += 1;
		baseMesh->Vertex.resize(baseMesh->VertexNum + 1);

		t1 = baseMesh->TriangleTex[i].T1;
		t2 = baseMesh->TriangleTex[i].T2;
		t3 = baseMesh->TriangleTex[i].T3;

		baseMesh->Vertex[baseMesh->VertexNum].S = (baseMesh->Vertex[t1].S + baseMesh->Vertex[t2].S + baseMesh->Vertex[t3].S)/3;
		baseMesh->Vertex[baseMesh->VertexNum].T = (baseMesh->Vertex[t1].T + baseMesh->Vertex[t2].T + baseMesh->Vertex[t3].T)/3;

	}

	in.pointlist = new REAL[baseMesh->VertexNum*2];
	list<int>::iterator it = Holes_uv.begin()->begin();
	for(int i = 1; i <=Holes_uv.begin()->size(); i++)
	{
		in.pointlist[(i-1)*2] = this->Vertex[*it].S;
		in.pointlist[(i-1)*2+1] = this->Vertex[*it].T;
		it++;
	}

	int index = Holes_uv.begin()->size()+1;
	for(int i = 1 ; i <= baseMesh->VertexNum ; i++){
		if(find(baseMesh->boundary.begin(),baseMesh->boundary.end(),i) != baseMesh->boundary.end()) continue;
		in.pointlist[(index-1)*2] = baseMesh->Vertex[i].S;
		in.pointlist[(index-1)*2+1] = baseMesh->Vertex[i].T;
		index++;
	}

	in.pointattributelist = NULL;
	in.pointmarkerlist = NULL;
	//洞邊界點的數目
	in.numberofpoints = baseMesh->VertexNum;
	in.numberofpointattributes = 0;
	in.trianglelist = NULL;
	in.triangleattributelist = NULL;
	in.trianglearealist = NULL;
	in.numberoftriangles = 0;
	in.numberofcorners = 0;
	in.numberoftriangleattributes = 0;
	//洞邊界邊的資料(一個邊由兩個點編號所構成，編號從0開始)給in.segmentlist
	//VBEdgeList是自己定義的資料結構
	in.segmentlist = new int[ 2* Holes_uv.begin()->size()];
	for(int i = 1 ; i <=  Holes_uv.begin()->size(); i++)
	{
		if(i ==  Holes_uv.begin()->size())
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
	//洞邊界邊的數目
	in.numberofsegments = Holes_uv.begin()->size();
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

	//三角化參數設定
	char *option = "pzYq30";

	//三角化主程式
	triangulate(option, &in, &out, vorout);
	
	//out為三角化後的結果；回填到自己的資料結構BMesh_1
	baseMesh->TriangleNum = out.numberoftriangles;
	baseMesh->TriangleTex.resize(baseMesh->TriangleNum + 1);	
	for(int i = 1; i <= baseMesh->TriangleNum; i++)
	{
		baseMesh->TriangleTex[i].T1 = out.trianglelist[(i-1)*3]+1;
		baseMesh->TriangleTex[i].T2 = out.trianglelist[(i-1)*3+1]+1;
		baseMesh->TriangleTex[i].T3 = out.trianglelist[(i-1)*3+2]+1;
	}
	baseMesh->VertexNum = out.numberofpoints;
	baseMesh->Vertex.resize(baseMesh->VertexNum + 1);

	list<int>::iterator hole_it;
	list<int>::iterator hole_uv_it;
	for(int i = 1; i <= baseMesh->VertexNum; i++)
	{
		baseMesh->Vertex[i].S = out.pointlist[(i-1)*2];
		baseMesh->Vertex[i].T = out.pointlist[(i-1)*2+1];
		for(hole_uv_it = Holes_uv.begin()->begin(),hole_it = Holes.begin()->begin(); hole_uv_it != Holes_uv.begin()->end() ; hole_uv_it++,hole_it++)
		{
			if(baseMesh->Vertex[i].S == Vertex[*hole_uv_it].S && baseMesh->Vertex[i].T == Vertex[*hole_uv_it].T)
			{
				baseMesh->boundary.push_back(i);
				baseMesh->Vertex[i].origin_index = *hole_it;
				baseMesh->Vertex[i].origin_uv_index = *hole_uv_it;
			}
		}
	}
	
	//free memory
	delete [] in.pointlist;
	free(out.pointlist);
	free(out.trianglelist);
	free(out.segmentlist);

}

void MQTriangleMesh::RebuildingCoordination()
{
	//三角化後，計算洞內的點的Laplacian
	for(int i = 1 ; i <= baseMesh->VertexNum ; i++)
	{
		vector<int>::iterator temp_vector =  find(baseMesh->boundary.begin(),baseMesh->boundary.end(),i);
		if(temp_vector == baseMesh->boundary.end())
		{
			bool isDone = false;
			int cornerX = 0;
			int cornerY = 0;
			for(int y = 0 ; y < imageSize ; y++)
			{
				for(int x = 0 ; x < imageSize ; x++)
				{
					if(ImagePixel[y][x].Y > baseMesh->Vertex[i].T) break;
					if(ImagePixel[y][x].X < baseMesh->Vertex[i].S) continue;
					cornerY = y;
					cornerX = x;
					isDone = true;
					break;
				}
				if(isDone) break;
			}

			int minimum_distance = 0;
			float total_area = abs((ImagePixel[cornerY][cornerX].X - ImagePixel[cornerY-1][cornerX-1].X) * (ImagePixel[cornerY][cornerX].Y - ImagePixel[cornerY-1][cornerX-1].Y));
			float tempX,tempY,tempZ;
			tempX = tempY = tempZ = 0.0;

			for(int y = cornerY-1 ; y<= cornerY ; y++)
			{
				for(int x = cornerX -1 ; x <= cornerX ; x++)
				{
					/*
					float part_area;
					if(y == cornerY-1 && x == cornerX -1)	part_area = abs((ImagePixel[y+1][x+1].X - baseMesh->Vertex[i].S) *  (ImagePixel[y+1][x+1].Y  - baseMesh->Vertex[i].T) / total_area);
					if(y == cornerY   && x == cornerX -1)	part_area = abs((ImagePixel[y-1][x+1].X - baseMesh->Vertex[i].S) *  (ImagePixel[y-1][x+1].Y  - baseMesh->Vertex[i].T) / total_area);
					if(y == cornerY-1 && x == cornerX   )	part_area = abs((ImagePixel[y+1][x-1].X - baseMesh->Vertex[i].S) *  (ImagePixel[y+1][x-1].Y  - baseMesh->Vertex[i].T) / total_area);
					if(y == cornerY   && x == cornerX   )	part_area = abs((ImagePixel[y-1][x-1].X - baseMesh->Vertex[i].S) *  (ImagePixel[y-1][x-1].Y  - baseMesh->Vertex[i].T) / total_area);					 

					tempX += part_area * ImagePixel[y][x].LapX;
					tempY += part_area * ImagePixel[y][x].LapY;
					tempZ += part_area * ImagePixel[y][x].LapZ;
					*/
					
					int pixel_distance = pow(baseMesh->Vertex[i].S - ImagePixel[y][x].X,2) + pow(baseMesh->Vertex[i].T - ImagePixel[y][x].Y,2);
					if(y == cornerY-1 && x == cornerX -1)
					{
						minimum_distance = pixel_distance;
						baseMesh->Vertex[i].LapX = ImagePixel[y][x].LapX;
						baseMesh->Vertex[i].LapY = ImagePixel[y][x].LapY;
						baseMesh->Vertex[i].LapZ = ImagePixel[y][x].LapZ;
						baseMesh->Vertex[i].ori_LapX = ImagePixel[y][x].ori_LapX;
						baseMesh->Vertex[i].ori_LapY = ImagePixel[y][x].ori_LapY;
						baseMesh->Vertex[i].ori_LapZ = ImagePixel[y][x].ori_LapZ;
						baseMesh->Vertex[i].Lap_length = ImagePixel[y][x].Lap_length;
					}
					else
					{
						if(pixel_distance < minimum_distance)
						{
							minimum_distance = pixel_distance;
							baseMesh->Vertex[i].LapX = ImagePixel[y][x].LapX;
							baseMesh->Vertex[i].LapY = ImagePixel[y][x].LapY;
							baseMesh->Vertex[i].LapZ = ImagePixel[y][x].LapZ;
							baseMesh->Vertex[i].ori_LapX = ImagePixel[y][x].ori_LapX;
							baseMesh->Vertex[i].ori_LapY = ImagePixel[y][x].ori_LapY;
							baseMesh->Vertex[i].ori_LapZ = ImagePixel[y][x].ori_LapZ;
							baseMesh->Vertex[i].Lap_length = ImagePixel[y][x].Lap_length;
						}
					}
					
				}				
			}			
			/*
			baseMesh->Vertex[i].LapX = tempX;
			baseMesh->Vertex[i].LapY = tempY;
			baseMesh->Vertex[i].LapZ = tempZ;
			baseMesh->Vertex[i].Lap_length = sqrt(pow(baseMesh->Vertex[i].LapX,2) + pow(baseMesh->Vertex[i].LapY,2)+ pow(baseMesh->Vertex[i].LapZ,2));
			*/
			baseMesh->inner_point += 1;
			this->VertexNum += 1;			
			this->Vertex.resize(this->VertexNum+1);
			this->Vertex[VertexNum] = baseMesh->Vertex[i];
			baseMesh->Vertex[i].origin_index = baseMesh->Vertex[i].origin_uv_index = this->VertexNum;
			
		}
	}

	for(int i = 1 ; i <= baseMesh->TriangleNum ; i++)
	{
		baseMesh->Vertex[baseMesh->TriangleTex[i].T1].NeighborVertex.push_back(baseMesh->Vertex[baseMesh->TriangleTex[i].T2].origin_index);
		baseMesh->Vertex[baseMesh->TriangleTex[i].T1].NeighborVertex.push_back(baseMesh->Vertex[baseMesh->TriangleTex[i].T3].origin_index);
		baseMesh->Vertex[baseMesh->TriangleTex[i].T2].NeighborVertex.push_back(baseMesh->Vertex[baseMesh->TriangleTex[i].T1].origin_index);
		baseMesh->Vertex[baseMesh->TriangleTex[i].T2].NeighborVertex.push_back(baseMesh->Vertex[baseMesh->TriangleTex[i].T3].origin_index);
		baseMesh->Vertex[baseMesh->TriangleTex[i].T3].NeighborVertex.push_back(baseMesh->Vertex[baseMesh->TriangleTex[i].T1].origin_index);
		baseMesh->Vertex[baseMesh->TriangleTex[i].T3].NeighborVertex.push_back(baseMesh->Vertex[baseMesh->TriangleTex[i].T2].origin_index);
	}
	for(int i = 1 ; i <= baseMesh->VertexNum ; i++)
	{
		baseMesh->Vertex[i].NeighborVertex.sort();
		baseMesh->Vertex[i].NeighborVertex.unique();
	}

	this->UpdateVertexLaplacianCoordinate();
	
	//solve Ax = b in least square sense
	int	n = this->VertexNum;

	//build sparse matrix A, then matrix x and matrix b(x, y, z)
	Eigen::SparseMatrix<double>	A(n, n);
	Eigen::VectorXd			xx(n), xy(n), xz(n);
	Eigen::VectorXd			bx(n), by(n), bz(n);

	A.reserve(Eigen::VectorXi::Constant(n, 12));

	list<int>::iterator it, begin, end;
	for(int i = 1; i <= n - baseMesh->inner_point; i++)
	{
		double degree = this->Vertex[i].NeighborVertex.size();

		begin = this->Vertex[i].NeighborVertex.begin();
		end = this->Vertex[i].NeighborVertex.end();
		for(it = begin; it != end; it++)
			A.insert(i-1, (*it)-1) = -1.0/degree;

		A.insert(i-1, i-1) = 1.0;
		bx(i-1) = this->Vertex[i].LapX;
		by(i-1) = this->Vertex[i].LapY;
		bz(i-1) = this->Vertex[i].LapZ;
	}
	for(int i = 1 ; i <= baseMesh->VertexNum ; i++)
	{
		if(baseMesh->Vertex[i].origin_index > n-baseMesh->inner_point)
		{

			this->Vertex[baseMesh->Vertex[i].origin_index] = baseMesh->Vertex[i];
			double degree = baseMesh->Vertex[i].NeighborVertex.size();

			begin = baseMesh->Vertex[i].NeighborVertex.begin();
			end = baseMesh->Vertex[i].NeighborVertex.end();
			for(it = begin; it != end; it++)
				A.insert(baseMesh->Vertex[i].origin_index-1, (*it)-1) = -1.0/degree;

			A.insert(baseMesh->Vertex[i].origin_index-1, baseMesh->Vertex[i].origin_index-1) = 1.0;
			bx(baseMesh->Vertex[i].origin_index-1) = baseMesh->Vertex[i].LapX;
			by(baseMesh->Vertex[i].origin_index-1) = baseMesh->Vertex[i].LapY;
			bz(baseMesh->Vertex[i].origin_index-1) = baseMesh->Vertex[i].LapZ;
		}
	}
	//finish filling matrix A
	A.makeCompressed();

	//transpose matrix A
	Eigen::SparseMatrix<double> At = A.transpose();

	//build matrix At*A and matrix At*b(x, y, z)
	Eigen::SparseMatrix<double>	AtA = At*A;
	Eigen::VectorXd			Atbx = At*bx;
	Eigen::VectorXd			Atby = At*by;
	Eigen::VectorXd			Atbz = At*bz;

	//begin solve least square system At*A*x = At*b
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;

	solver.compute(AtA);

	//solve xx
	xx = solver.solve(Atbx);

	//solve xy
	xy = solver.solve(Atby);

	//solve xz
	xz = solver.solve(Atbz);

	//write back solutions
	for(int i = 1; i <= n; i++)
	{
		this->Vertex[i].X = xx(i-1);
		this->Vertex[i].Y = xy(i-1);
		this->Vertex[i].Z = xz(i-1);
	}
	for(int i = 1 ; i <= baseMesh->TriangleNum ; i++)
	{
		this->TriangleNum += 1;
		this->Triangle.resize(this->TriangleNum+1);
		this->TriangleTex.resize(this->TriangleNum+1);
		this->Triangle[this->TriangleNum].V1 = baseMesh->Vertex[baseMesh->TriangleTex[i].T1].origin_index;
		this->Triangle[this->TriangleNum].V2 = baseMesh->Vertex[baseMesh->TriangleTex[i].T2].origin_index;
		this->Triangle[this->TriangleNum].V3 = baseMesh->Vertex[baseMesh->TriangleTex[i].T3].origin_index;
		this->TriangleTex[this->TriangleNum].T1 = baseMesh->Vertex[baseMesh->TriangleTex[i].T1].origin_uv_index;
		this->TriangleTex[this->TriangleNum].T2 = baseMesh->Vertex[baseMesh->TriangleTex[i].T2].origin_uv_index;
		this->TriangleTex[this->TriangleNum].T3 = baseMesh->Vertex[baseMesh->TriangleTex[i].T3].origin_uv_index;		
	}

	this->UpdateVertexNeigborVertex();
	this->UpdateVertexNormal();	
	this->CalculateLaplacianToColor();

}
void MQTriangleMesh::UpdateVertexNormal(void)
{
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

}

void MQTriangleMesh::Draw(GLubyte Red, GLubyte Green, GLubyte Blue)
{
	glLineWidth(1.0f);	
	if(draw_boundary)
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
	}
	glLineWidth(3.0f);	
	if(baseMesh && draw_laplacian)
	{
		glBegin(GL_LINES);
		for(int i = 1; i <= this->VertexNum; i++)
		{
			
			if(i > this->VertexNum - baseMesh->inner_point)
			{
				glNormal3f(this->Vertex[i].NX, this->Vertex[i].NY, this->Vertex[i].NZ);
				glColor3ub(0,0,0);
				glVertex3f(this->Vertex[i].X, this->Vertex[i].Y, this->Vertex[i].Z);

				glNormal3f(this->Vertex[i].NX, this->Vertex[i].NY, this->Vertex[i].NZ);
				glColor3ub(0,0,0);
				glVertex3f(this->Vertex[i].X+this->Vertex[i].LapX*10, this->Vertex[i].Y+this->Vertex[i].LapY*10, this->Vertex[i].Z+this->Vertex[i].LapZ*10);
				
				glNormal3f(this->Vertex[i].NX, this->Vertex[i].NY, this->Vertex[i].NZ);
				glColor3ub(0,255,0);
				glVertex3f(this->Vertex[i].X, this->Vertex[i].Y, this->Vertex[i].Z);

				glNormal3f(this->Vertex[i].NX, this->Vertex[i].NY, this->Vertex[i].NZ);
				glColor3ub(0,255,0);
				glVertex3f(this->Vertex[i].X+this->Vertex[i].ori_LapX*10, this->Vertex[i].Y+this->Vertex[i].ori_LapY*10, this->Vertex[i].Z+this->Vertex[i].ori_LapZ*10);
			}
		}
		glEnd();
	}

	glPolygonMode(GL_FRONT,GL_FILL);
	glColor4ub(Red, Green, Blue,128);
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
	glLineWidth(1.0f);	
	//glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT,GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, 0.0f);
	//Draw hole
	if(draw_boundary)
	{
		glBegin(GL_LINES);
		list<int>::iterator hole_it;	
		list<list<int>>::iterator begin = Holes_uv.begin();
		for(hole_it = begin->begin(); hole_it != begin->end();hole_it++)
		{
			if(hole_it == begin->begin())
			{
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(0,0,255);
				glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
			}
			else
			{
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(0,0,255);
				glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(0,0,255);
				glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
			}
		}
		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(0,0,255);
		glVertex3f(this->Vertex[*begin->begin()].S, this->Vertex[*begin->begin()].T,0);
		glEnd();
	}

	
	//Draw hole bounding box
	if(draw_boundingbox){
		glBegin(GL_QUADS);
			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(255,0,0);
			glVertex3f(this->hole_boundaryX.first, this->hole_boundaryY.first, 0.0);

			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(255,0,0);
			glVertex3f(this->hole_boundaryX.second, this->hole_boundaryY.first, 0.0);

			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(255,0,0);
			glVertex3f(this->hole_boundaryX.second, this->hole_boundaryY.second, 0.0);

			glNormal3f(0.0, 0.0, 1.0);
			glColor3ub(255,0,0);
			glVertex3f(this->hole_boundaryX.first, this->hole_boundaryY.second, 0.0);
		glEnd();
	}

	//Draw Triangle Line	
	if(draw_triangle){
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
		
		//三角化的線	
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
	}
	//Draw uv
	glPolygonMode(GL_FRONT,GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 0.0f);
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
	glLineWidth(1.0f);	
	glPolygonMode(GL_FRONT,GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, 0.0f);
	//Draw hole
	if(draw_boundary)
	{
		glBegin(GL_LINES);
		list<int>::iterator hole_it;	
		list<list<int>>::iterator begin = Holes_uv.begin();
		for(hole_it = begin->begin(); hole_it != begin->end();hole_it++)
		{
			if(hole_it == begin->begin())
			{
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(255,0,0);
				glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
			}
			else
			{
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(255,0,0);
				glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
				glNormal3f(0.0, 0.0, 1.0);
				glColor3ub(255,0,0);
				glVertex3f(this->Vertex[*hole_it].S, this->Vertex[*hole_it].T,0);
			}
		}
		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(255,0,0);
		glVertex3f(this->Vertex[*begin->begin()].S, this->Vertex[*begin->begin()].T,0);
		glEnd();
	}

	glPolygonMode(GL_FRONT,GL_LINE);
	//Draw hole bounding box
	if(draw_boundingbox){
		glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(255,255,255);
		glVertex3f(this->hole_boundaryX.first, this->hole_boundaryY.first, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(255,255,255);
		glVertex3f(this->hole_boundaryX.second, this->hole_boundaryY.first, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(255,255,255);
		glVertex3f(this->hole_boundaryX.second, this->hole_boundaryY.second, 0.0);

		glNormal3f(0.0, 0.0, 1.0);
		glColor3ub(255,255,255);
		glVertex3f(this->hole_boundaryX.first, this->hole_boundaryY.second, 0.0);
		glEnd();
	}

	//Draw Triangle Line	
	if(draw_triangle){
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

		//三角化的線	
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
	}


	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	if(fillpoint.size()>0){
		glVertex3f(this->ImagePixel[first_point/imageSize][first_point%imageSize].X, this->ImagePixel[first_point/imageSize][first_point%imageSize].Y, 0.0);
		glVertex3f(this->ImagePixel[second_point/imageSize][second_point%imageSize].X, this->ImagePixel[second_point/imageSize][second_point%imageSize].Y, 0.0);
	}
	glEnd();
	glPolygonMode(GL_FRONT,GL_POINT);
	glEnable(GL_POLYGON_OFFSET_POINT);
	glPolygonOffset(1.0f, 0.0f);
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