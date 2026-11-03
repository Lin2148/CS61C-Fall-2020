/************************************************************************
**
** NAME:        imageloader.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**              Justin Yokota - Starter Code
**				YOUR NAME HERE
**
**
** DATE:        2020-08-15
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "imageloader.h"

//Opens a .ppm P3 image file, and constructs an Image object. 
//You may find the function fscanf useful.
//Make sure that you close the file with fclose before returning.
Image *readData(char *filename) 
{
	//YOUR CODE HERE
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Error: Could not open file %s\n", filename);
		return NULL;
	}
	// 配置空間
	Image *image = malloc (sizeof(Image));
	if (image == NULL) {
		fprintf(stderr, "Error: Could not allocate memory for image\n");
		fclose(f);
		return NULL;
	}
	

	// Read the header
	char format[3];

	fscanf(f, "%2s", format);
	fscanf(f, "%u %u", &image->cols, &image->rows);
	// 255 no use
	fscanf(f, "%*u");

	uint32_t rows = image->rows;
	uint32_t cols = image->cols;
	image->image = malloc(rows * sizeof(Color *));
	if (image->image == NULL) {
		fprintf(stderr, "Error: Could not allocate memory for image->image\n");
		free(image);
		fclose(f);
		return NULL;
	}
	for (uint32_t i = 0; i < rows; i++) {
    	image->image[i] = malloc(cols * sizeof(Color));
		if (image->image[i] == NULL) {
			fprintf(stderr, "Error: Could not allocate memory for image[i]\n");
			fclose(f);
			return NULL;
		}
	}

	// put data
	for (uint32_t i = 0; i < rows; i++){
		for (uint32_t j = 0; j < cols; j++){
			unsigned int r;
        	unsigned int g;
        	unsigned int b;
			if (fscanf(f, "%u %u %u", &r, &g, &b) != 3) {
				fprintf(stderr, "Error: Invalid pixel data\n");
				fclose(f);
				freeImage(image);
				return NULL;
			}

			image->image[i][j].R = (uint8_t)r;
			image->image[i][j].G = (uint8_t)g;
			image->image[i][j].B = (uint8_t)b;
			//core dump 寫法597byte inside 600 alloc 
			//fscanf(f, "%u %u %u", &image->image[i][j].R, &image->image[i][j].G, &image->image[i][j].B);
		}
	}

	fclose(f);
	return image;
}

//Given an image, prints to stdout (e.g. with printf) a .ppm P3 file with the image's data.
void writeData(Image *image)
{
	//YOUR CODE HERE
	if (image == NULL) {
        return;
    }

	uint32_t rows = image->rows;
	uint32_t cols = image->cols;
	printf("P3\n%u %u\n255\n", cols, rows);

	//輸出color
	for (uint32_t i = 0; i < rows; i++){
		for (uint32_t j = 0; j < cols; j++){
			printf("%3u %3u %3u",  (unsigned int)image->image[i][j].R,  (unsigned int)image->image[i][j].G,  (unsigned int)image->image[i][j].B);
			// 補color中間3空格
			if( j < cols-1){
				printf("   ");
			}	
		}
		printf("\n");
	}
}

//Frees an image
void freeImage(Image *image)
{
	//YOUR CODE HERE
	if (image == NULL) {
        return;
    }
	for (uint32_t i = 0; i < image->rows; i++){
		free(image->image[i]);
	}
	free(image->image);
    free(image);
}