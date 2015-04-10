#ifndef MQPCA_H_20080505
#define MQPCA_H_20080505


#include <list>
#include <vector>


using namespace std;

class MQPCA
{
public:
	int DataItemDimension;
	list<vector<float>> Data;

public:
	void			SetDataItemDimension(const int Dim);
	bool			AddDataItem(const vector<float>& DataItem);							//index start from 1
	void			PrincipalComponentAnalysis(void);
	vector<float>	ProjectDataItem(const vector<float>& DataItem, const int ProjectDim);		//index start from 1
	vector<float>	ReconstructDataItem(const vector<float>& DataItem, const int ProjectDim);	//index start from 1
	void			DisplayCovMatrixEvals(void);
	void			DisplayCovMatrixEvecs(void);

public:
	MQPCA();
	~MQPCA();

private:
	int		n_, m_;
	float	**data_, *datamean_, **symmat_, *evals_;

private:
	void		erhand(char* err_msg);
	float*	vectorx(int n);
	void		covcol(float** data, int n, int m, float* datamean, float** symmat);
	float**	matrix(int n, int m);
	void		free_vector(float* v, int n);
	void		free_matrix(float** mat, int n, int m);
	void		tred2(float** a, int n, float* d, float* e);
	void		tqli(float* d, float* e, int n, float** z);
};


#endif	//MQPCA_H_20080505