#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>

using namespace std;

typedef unsigned char byte;

class texture
{
public:
	//constructor
	texture(int w, int h);

	//user functions
	int getSampleImage(char* filename);
	void generateTexture(int size);
	void writeTexture(char* filename);

private:
	//private functions
	void convertSample();
	void convertTexture();
	void initializeTexture(int size);
	void findBestMatch(int i, int j, int size);

	//data
	int sample_w, sample_h;
	byte** sample_image;
	void* sample_data;
	byte** sample_red;
	byte** sample_green;
	byte** sample_blue;
	
	int texture_w, texture_h;
	corona::Image* texture_image;
	void * texture_data;
	byte** texture_red;
	byte** texture_green;
	byte** texture_blue;
	int** original_pos_x;
	int** original_pos_y;

	byte** red;
	byte** green;
	byte** blue;
	vector<int> candidate_x;
	vector<int> candidate_y;
};

#endif
