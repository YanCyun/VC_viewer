#include "MQPCA.h"
#include <math.h>
#include "MQUtility.h"
#include <algorithm>
#include <iostream>


/*********************************************************************/
/* [original source information]
   Principal Components Analysis or the Karhunen-Loeve expansion is a
   classical method for dimensionality reduction or exploratory data
   analysis.  One reference among many is: F. Murtagh and A. Heck,
   Multivariate Data Analysis, Kluwer Academic, Dordrecht, 1987 
   (hardbound, paperback and accompanying diskette).

   This program is public-domain.  If of importance, please reference 
   the author.  Please also send comments of any kind to the author:

   F. Murtagh
   Schlossgartenweg 1          or        35 St. Helen's Road
   D-8045 Ismaning                       Booterstown, Co. Dublin
   W. Germany                            Ireland

   Phone:        + 49 89 32006298 (work)
                 + 49 89 965307 (home)
   Telex:        528 282 22 eo d
   Fax:          + 49 89 3202362
   Earn/Bitnet:  fionn@dgaeso51,  fim@dgaipp1s,  murtagh@stsci
   Span:         esomc1::fionn
   Internet:     murtagh@scivax.stsci.edu
   

   A Fortran version of this program is also available.     
    
   F. Murtagh, Munich, 6 June 1989                                   */   
/*********************************************************************/


float SIGN(float a, float b)
{
	return ( (b < 0) ? -fabs(a) : fabs(a) );
}

void MQPCA::erhand(char* err_msg)
/* Error handler */
{
    fprintf(stderr,"Run-time error:\n");
    fprintf(stderr,"%s\n", err_msg);
    fprintf(stderr,"Exiting to system.\n");
    exit(1);
}

float* MQPCA::vectorx(int n)
/* Allocates a float vector with range [1..n]. */
{
    float *v;

    v = (float *) malloc ((unsigned) n*sizeof(float));
    if (!v) erhand("Allocation failure in vector().");
    return v-1;
}

void MQPCA::covcol(float** data, int n, int m, float* datamean, float** symmat)
/* Create m * m covariance matrix from given n * m data matrix. */
{
int i, j, j1, j2;

/* Determine mean of column vectors of input data matrix */

for (j = 1; j <= m; j++)
    {
    datamean[j] = 0.0;
    for (i = 1; i <= n; i++)
        {
        datamean[j] += data[i][j];
        }
    datamean[j] /= (float)n;
    }

/* Center the column vectors. */

for (i = 1; i <= n; i++)
    {
    for (j = 1; j <= m; j++)
        {
        data[i][j] -= datamean[j];
        }
    }

/* Calculate the m * m covariance matrix. */
for (j1 = 1; j1 <= m; j1++)
    {
    for (j2 = j1; j2 <= m; j2++)
        {
        symmat[j1][j2] = 0.0;
        for (i = 1; i <= n; i++)
            {
            symmat[j1][j2] += data[i][j1] * data[i][j2];
            }
        symmat[j2][j1] = symmat[j1][j2];
        }
    }
}

float** MQPCA::matrix(int n, int m)
/* Allocate a float matrix with range [1..n][1..m]. */
{
    int i;
    float **mat;

    /* Allocate pointers to rows. */
    mat = (float **) malloc((unsigned) (n)*sizeof(float*));
    if (!mat) erhand("Allocation failure 1 in matrix().");
    mat -= 1;

    /* Allocate rows and set pointers to them. */
    for (i = 1; i <= n; i++)
        {
        mat[i] = (float *) malloc((unsigned) (m)*sizeof(float));
        if (!mat[i]) erhand("Allocation failure 2 in matrix().");
        mat[i] -= 1;
        }

     /* Return pointer to array of pointers to rows. */
     return mat;
}

void MQPCA::free_vector(float* v, int n)
/* Free a float vector allocated by vector(). */
{
   free((char*) (v+1));
}

void MQPCA::free_matrix(float** mat, int n, int m)
/* Free a float matrix allocated by matrix(). */
{
   int i;

   for (i = n; i >= 1; i--)
       {
       free ((char*) (mat[i]+1));
       }
   free ((char*) (mat+1));
}

void MQPCA::tred2(float** a, int n, float* d, float* e)
/* Householder reduction of matrix a to tridiagonal form.
   Algorithm: Martin et al., Num. Math. 11, 181-195, 1968.
   Ref: Smith et al., Matrix Eigensystem Routines -- EISPACK Guide
        Springer-Verlag, 1976, pp. 489-494.
        W H Press et al., Numerical Recipes in C, Cambridge U P,
        1988, pp. 373-374.  */
{
int l, k, j, i;
float scale, hh, h, g, f;

for (i = n; i >= 2; i--)
    {
    l = i - 1;
    h = scale = 0.0;
    if (l > 1)
       {
       for (k = 1; k <= l; k++)
           scale += fabs(a[i][k]);
       if (scale == 0.0)
          e[i] = a[i][l];
       else
          {
          for (k = 1; k <= l; k++)
              {
              a[i][k] /= scale;
              h += a[i][k] * a[i][k];
              }
          f = a[i][l];
          g = f>0 ? -sqrt(h) : sqrt(h);
          e[i] = scale * g;
          h -= f * g;
          a[i][l] = f - g;
          f = 0.0;
          for (j = 1; j <= l; j++)
              {
              a[j][i] = a[i][j]/h;
              g = 0.0;
              for (k = 1; k <= j; k++)
                  g += a[j][k] * a[i][k];
              for (k = j+1; k <= l; k++)
                  g += a[k][j] * a[i][k];
              e[j] = g / h;
              f += e[j] * a[i][j];
              }
          hh = f / (h + h);
          for (j = 1; j <= l; j++)
              {
              f = a[i][j];
              e[j] = g = e[j] - hh * f;
              for (k = 1; k <= j; k++)
                  a[j][k] -= (f * e[k] + g * a[i][k]);
              }
         }
    }
    else
        e[i] = a[i][l];
    d[i] = h;
    }
d[1] = 0.0;
e[1] = 0.0;
for (i = 1; i <= n; i++)
    {
    l = i - 1;
    if (d[i])
       {
       for (j = 1; j <= l; j++)
           {
           g = 0.0;
           for (k = 1; k <= l; k++)
               g += a[i][k] * a[k][j];
           for (k = 1; k <= l; k++)
               a[k][j] -= g * a[k][i];
           }
       }
       d[i] = a[i][i];
       a[i][i] = 1.0;
       for (j = 1; j <= l; j++)
           a[j][i] = a[i][j] = 0.0;
    }
}

void MQPCA::tqli(float* d, float* e, int n, float** z)
{
int m, l, iter, i, k;
float s, r, p, g, f, dd, c, b;

for (i = 2; i <= n; i++)
    e[i-1] = e[i];
e[n] = 0.0f;
for (l = 1; l <= n; l++)
    {
    iter = 0;
    do
      {
      for (m = l; m <= n-1; m++)
          {
          dd = fabs(d[m]) + fabs(d[m+1]);
          if (fabs(e[m]) + dd == dd) break;
          }
          if (m != l)
             {
             if (iter++ == 30) erhand("No convergence in TLQI.");
             g = (d[l+1] - d[l]) / (2.0f * e[l]);
             r = sqrt((g * g) + 1.0f);
             g = d[m] - d[l] + e[l] / (g + SIGN(r, g));
             s = c = 1.0f;
             p = 0.0f;
             for (i = m-1; i >= l; i--)
                 {
                 f = s * e[i];
                 b = c * e[i];
                 if (fabs(f) >= fabs(g))
                    {
                    c = g / f;
                    r = sqrt((c * c) + 1.0f);
                    e[i+1] = f * r;
                    c *= (s = 1.0f/r);
                    }
                 else
                    {
                    s = f / g;
                    r = sqrt((s * s) + 1.0f);
                    e[i+1] = g * r;
                    s *= (c = 1.0f/r);
                    }
                 g = d[i+1] - p;
                 r = (d[i] - g) * s + 2.0f * c * b;
                 p = s * r;
                 d[i+1] = g + p;
                 g = c * r - b;
                 for (k = 1; k <= n; k++)
                     {
                     f = z[k][i+1];
                     z[k][i+1] = s * z[k][i] + c * f;
                     z[k][i] = c * z[k][i] - s * f;
                     }
                 }
                 d[l] = d[l] - p;
                 e[l] = g;
                 e[m] = 0.0f;
             }
          }  while (m != l);
      }
}

MQPCA::MQPCA()
:DataItemDimension(0)
{

}

MQPCA::~MQPCA()
{
	this->free_matrix(this->data_, this->n_, this->m_);
	this->free_vector(this->datamean_, this->m_);
	this->free_matrix(this->symmat_, this->m_, this->m_);
	this->free_vector(this->evals_, this->m_);
}

void MQPCA::SetDataItemDimension(const int Dim)
{
	this->DataItemDimension = Dim;
}

bool MQPCA::AddDataItem(const vector<float>& DataItem)
{
	if(int(DataItem.size()-1) != this->DataItemDimension)
		return false;
	else
	{
		this->Data.push_back(DataItem);
		return true;
	}
}

void MQPCA::PrincipalComponentAnalysis(void)
{
	this->m_ = this->DataItemDimension;
	this->n_ = int(this->Data.size());

	this->data_ = this->matrix(this->n_, this->m_);  /* Storage allocation for input data */

	list<vector<float>>::iterator vecit;
	int i = 0;
	for(vecit = this->Data.begin(); vecit != this->Data.end(); ++vecit)
	{
		++i;
		for(int j = 1; j <= this->m_; ++j)
			this->data_[i][j] = (*vecit)[j];
	}

	this->datamean_ = this->vectorx(this->m_);

	this->symmat_ = this->matrix(this->m_, this->m_);  /* Allocation of correlation (etc.) matrix */

	this->covcol(this->data_, this->n_, this->m_, this->datamean_, this->symmat_);

	this->evals_ = this->vectorx(this->m_);     /* Storage alloc. for vector of eigenvalues */

	float* interm = this->vectorx(this->m_);    /* Storage alloc. for 'intermediate' vector */

	this->tred2(this->symmat_, this->m_, this->evals_, interm);  /* Triangular decomposition */

	this->tqli(this->evals_, interm, this->m_, this->symmat_);   /* Reduction of sym. trid. matrix */

	this->free_vector(interm, this->m_);

	//reserve original matrix of eigenvectors
	float** oldsymmat = this->matrix(this->m_, this->m_);

	for(int i = 1; i <= this->m_; ++i)
		for(int j = 1; j <= this->m_; ++j)
			oldsymmat[i][j] = this->symmat_[i][j];

	//rearrange eigenvectors by eigenvalues in decreasing order
	vector<MQComplexItem<float, int>> new_evals_table;

	for(int i = 1; i <= this->m_; ++i)
	{
		MQComplexItem<float, int> _citem(this->evals_[i], i);
		new_evals_table.push_back(_citem);
	}
	sort(new_evals_table.begin(), new_evals_table.end());

	for(int i = 1, j = this->m_; i <= this->m_; ++i, --j)
	{
		this->evals_[i] = new_evals_table[j-1].Part1;

		for(int k = 1; k <= this->m_; ++k)
			this->symmat_[k][i] = oldsymmat[k][new_evals_table[j-1].Part2];
	}

	//free oldsymmat
	this->free_matrix(oldsymmat, this->m_, this->m_);
}

vector<float> MQPCA::ProjectDataItem(const vector<float>& DataItem, const int ProjectDim)
{
	vector<float> result(ProjectDim+1, 0.0f);

	if(int(DataItem.size()-1) != this->DataItemDimension)
		return result;
	else
	{
		for(int i = 1; i <= ProjectDim; ++i)
		{
			for(int j = 1; j <= this->DataItemDimension; ++j)
				result[i] += (DataItem[j] - this->datamean_[j]) * this->symmat_[j][i];
		}

		return result;
	}
}

vector<float> MQPCA::ReconstructDataItem(const vector<float>& DataItem, const int ProjectDim)
{
	vector<float> result(this->DataItemDimension+1, 0.0f);

	if(int(DataItem.size()-1) != this->DataItemDimension)
		return result;
	else
	{
		vector<float> ProjectResult = this->ProjectDataItem(DataItem, ProjectDim);

		for(int i = 1; i <= ProjectDim; ++i)
		{
			for(int j = 1; j <= this->DataItemDimension; ++j)
				result[j] += ProjectResult[i] * this->symmat_[j][i];
		}

		for(int i = 1; i <= this->DataItemDimension; ++i)
			result[i] += this->datamean_[i];

		return result;
	}
}

void MQPCA::DisplayCovMatrixEvals(void)
{
	cout<<"eigenvalues of covariance matrix"<<endl<<endl;
	for(int i = 1; i <= this->DataItemDimension; ++i)
		cout<<scientific<<this->evals_[i]<<endl;

	cout<<fixed<<endl;
}

void MQPCA::DisplayCovMatrixEvecs(void)
{
	cout<<"eigenvectors of covariance matrix"<<endl<<endl;
	for(int i = 1; i <= this->DataItemDimension; ++i)
	{
		cout<<"eigenvector "<<i<<" of "<<this->DataItemDimension<<endl;
		for(int j = 1; j <= this->DataItemDimension; ++j)
			cout<<this->symmat_[j][i]<<" ";

		cout<<endl<<endl;
	}
}
