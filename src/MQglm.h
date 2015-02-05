/* 
	ripped and modified from glm.h

      original information
      //glm.h
      //Nate Robins, 1997, 2000
      //nate@pobox.com, http://www.pobox.com/~nate
		//
      //Wavefront OBJ model file format reader/writer/manipulator.
		//
      //Includes routines for generating smooth normals with
      //preservation of edges, welding redundant vertices & texture
      //coordinate generation (spheremap and planar projections) + more.
 */


#ifndef MQglm_h
#define MQglm_h


/* MQGLMmaterial: Structure that defines a material in a model.
 */
struct MQGLMmaterial
{
  char* name;                 /* name of material */
  float diffuse[4];           /* diffuse component */
  float ambient[4];           /* ambient component */
  float specular[4];          /* specular component */
  float emmissive[4];         /* emmissive component */
  float shininess;            /* specular exponent */
};

/* MQGLMtriangle: Structure that defines a triangle in a model.
 */
struct MQGLMtriangle
{
  unsigned int vindices[3];           /* array of triangle vertex indices */
  unsigned int nindices[3];           /* array of triangle normal indices */
  unsigned int tindices[3];           /* array of triangle texcoord indices*/
  unsigned int findex;                /* index of triangle facet normal */
};

/* MQGLMgroup: Structure that defines a group in a model.
 */
struct MQGLMgroup
{
  char*         name;           /* name of this group */
  unsigned int  numtriangles;   /* number of triangles in this group */
  unsigned int* triangles;      /* array of triangle indices */
  unsigned int  material;       /* index to material for group */
  MQGLMgroup*	 next;           /* pointer to next group in model */
};

/* MQGLMmodel: Structure that defines a model.
 */
struct MQGLMmodel
{
  char*        pathname;            /* path to this model */
  char*        mtllibname;          /* name of the material library */

  unsigned int numvertices;         /* number of vertices in model */
  double*      vertices;				/* array of vertices  */

  unsigned int numnormals;          /* number of normals in model */
  float*			normals;             /* array of normals */

  unsigned int numtexcoords;        /* number of texcoords in model */
  double*		texcoords;           /* array of texture coordinates */

  unsigned int numfacetnorms;       /* number of facetnorms in model */
  float*			facetnorms;          /* array of facetnorms */

  unsigned int   numtriangles;		/* number of triangles in model */
  MQGLMtriangle* triangles;			/* array of triangles */

  unsigned int   nummaterials;	   /* number of materials in model */
  MQGLMmaterial* materials;			/* array of materials */

  unsigned int   numgroups;			/* number of groups in model */
  MQGLMgroup*    groups;				/* linked list of groups */

  float position[3];				      /* position of the model */
};


/* MQglmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be freed with
 * MQglmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.
 */
MQGLMmodel* 
MQglmReadOBJ(const char* filename);

/* MQglmDelete: Deletes a MQGLMmodel structure.
 *
 * model - initialized MQGLMmodel structure
 */
void
MQglmDelete(MQGLMmodel* model);


#endif	//MQglm_h
