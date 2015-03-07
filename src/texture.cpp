#include "texture.h"
#include <iostream>
#include <time.h>

using namespace std;

texture::texture(int w, int h)
// initialize the texture to size w x h
{
	texture_w = w;
	texture_h = h;
	texture_red = new byte* [texture_w];
	texture_green = new byte* [texture_w];
	texture_blue = new byte* [texture_w];
	original_pos_x = new int* [texture_w];
	original_pos_y = new int* [texture_w];
	for(int i=0; i<texture_w; i++)
	{
		texture_red[i] = new byte[texture_h];
		texture_green[i] = new byte[texture_h];
		texture_blue[i] = new byte[texture_h];
		original_pos_x[i] = new int[texture_w];
		original_pos_y[i] = new int[texture_w];
	}
}

int texture::getSampleImage(char* filename)
// read in the sample from file
{
	sample_image = corona::OpenImage(filename, corona::PF_R8G8B8);
	if (!sample_image)
	{
		std::cout<<"Error: image couldn't be loaded.\n";
		return 1;
	}
	sample_w = sample_image->getWidth();
	sample_h = sample_image->getHeight();
	sample_data = sample_image->getPixels();
	convertSample();

	return 0;
}

void texture::generateTexture(int size)
// generate the texture from the sample using a search window of size x size
{
	int i, j, a=0;

	cout<<"Initializing texture...";
	initializeTexture(size);
	cout<<"done\n";

	cout<<"Performing exhaustive search...\n";

	red = new byte*[size];
	green = new byte*[size];
	blue = new byte*[size];
	for(int x=0; x<size; x++)
	{
		red[x] = new byte[size/2+1];
		blue[x] = new byte[size/2+1];
		green[x] = new byte[size/2+1];
	}
	
	for(j=0; j<texture_h; j++)
	{
		if(j>=double(a*texture_h)/100)
		{
			cout<<a<<"% done\n";
			a+=10;
		}
		for(i=0; i<texture_w; i++)
		{
			findBestMatch(i, j, size);
		}
	}
	cout<<"100% done\n";
	cout<<"Texture generation complete\n\n";

	return;
}

void texture::writeTexture(char* filename)
// write the texture from the stream of bytes to a file
{
	convertTexture();
	//texture_image = corona::CreateImage(texture_w, texture_h, corona::PF_R8G8B8, texture_data);
	//corona::SaveImage(filename, corona::FF_PNG, texture_image);
	return;
}

void texture::convertSample()
// convert the sample from the stream of bytes to 2d array of pixels
{
	int i, j;
	byte* p = (byte*) sample_data;

	sample_red = new byte*[sample_w];
	sample_green = new byte*[sample_w];
	sample_blue = new byte*[sample_w];
	for(i=0; i<sample_w; i++)
	{
		sample_red[i] = new byte[sample_h];
		sample_green[i] = new byte[sample_h];
		sample_blue[i] = new byte[sample_h];
	}

	for (j=0; j<sample_h; j++)
	{
		for(i=0; i<sample_w; i++)
		{
			sample_red[i][j] = *p++;
			sample_green[i][j] = *p++;
			sample_blue[i][j] = *p++;
		}
	}
	return;
}

void texture::convertTexture()
// convert the texture from 2d array of pixels to stream of bytes for output
{	
	int i, j;
	byte* p = new byte[texture_h*texture_w*3];
	byte* t = p;
	for (j=0; j<texture_h; j++)
	{
		for(i=0; i<texture_w; i++)
		{
			*t = texture_red[i][j];
			*t++;
			*t = texture_green[i][j];
			*t++;
			*t = texture_blue[i][j];
			*t++;
		}
	}
	texture_data = p;
	return;
}

void texture::initializeTexture(int size)
// initialize output texture with random pixels from the sample
{
	int i, j;
	int w, h;

	int valid_w_length = sample_w-size+1;
	int valid_h_length = sample_h-size/2;
	int dw = size/2;
	int dh = size/2;

	srand(time(NULL));

	for (j=0; j<texture_h; j++)
	{
		for(i=0; i<texture_w; i++)
		{
			w = rand() % valid_w_length + dw;
			h = rand() % valid_h_length + dh;
			texture_red[i][j] = sample_red[w][h];
			texture_green[i][j] = sample_green[w][h];
			texture_blue[i][j] = sample_blue[w][h];
			original_pos_x[i][j] = w;
			original_pos_y[i][j] = h;
		}
	}
	return;
}

void texture::findBestMatch(int i, int j, int size)
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
	for(y=0, tj = j-size/2; y < size/2+1; y++, tj++)
	{
		for(x=0, ti = i-size/2; x < size; x++, ti++)
		{
			// break if center of window reached (end of causal neighborhood)
			if(y==size/2 && x==size/2)
				break;
			// otherwise
			if(ti < 0)
				ti += texture_w;
			else if(ti >= texture_w)
				ti -= texture_w;
			if(tj < 0)
				tj += texture_h;
			else if(tj >= texture_h)
				tj -= texture_h;
			red[x][y] = texture_red[ti][tj];
			green[x][y] = texture_green[ti][tj];
			blue[x][y] = texture_blue[ti][tj];
		}
	}
	// get candidates;
	candidate_x.clear();
	candidate_y.clear();
	for(actualh = j-size/2; actualh <= j; actualh++)
	{
		for(actualw = i-size/2; actualw <= i+size/2; actualw++)
		{
			if(actualw==i && actualh==j)
			{
				actualh = j+1;
				break;
			}
			// make sure that x and y are within the bounds of the texture array
			x = (actualw+texture_w)%texture_w;
			y = (actualh+texture_h)%texture_h;
			// get the coordinates for the pixel in the sample image from which
			// the texture pixel came from and shift appropriately
			actualx = original_pos_x[x][y]+i-actualw;
			actualy = original_pos_y[x][y]+j-actualh;
			// check if neighborhood of candidate lies completely in sample
			if(actualx < size/2 || actualx >= sample_w-size/2 || actualy < size/2 || actualy >= sample_h-size/2)
			{
				//replace with random
				actualx = rand()%(sample_w-size+1)+size/2;
				actualy = rand()%(sample_h-size/2)+size/2;
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
		for(y=candidate_y[c]-size/2, tj=0; y<candidate_y[c]+1; y++, tj++)
		{
			for(x=candidate_x[c]-size/2, ti=0; x<candidate_x[c]+size/2+1; x++, ti++)
			{
				if(y==candidate_y[c] && x==candidate_x[c] || tempd > bestd)
				{
					y = candidate_y[c]+1;
					break;
				}
				r = int(red[ti][tj]-sample_red[x][y]);
				g = int(green[ti][tj]-sample_green[x][y]);
				b = int(blue[ti][tj]-sample_blue[x][y]);
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

	texture_red[i][j] = sample_red[bestw][besth];
	texture_green[i][j] = sample_green[bestw][besth];
	texture_blue[i][j] = sample_blue[bestw][besth];
	original_pos_x[i][j] = bestw;
	original_pos_y[i][j] = besth;
	return;
}
