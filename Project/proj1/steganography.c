/************************************************************************
**
** NAME:        steganography.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**				Justin Yokota - Starter Code
**				YOUR NAME HERE
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

/**
 * 呼叫的人要手動清理該空間
 * Determines what color the cell at the given row/col should be. This should not affect Image, and should allocate space for a new Color.
 */
Color *evaluateOnePixel(Image *image, int row, int col)
{
	//YOUR CODE HERE
	Color *c = malloc(sizeof(Color));
	if (c == NULL){
		return NULL;
	}

	uint8_t val = (image->image[row][col].B & 1) == 1 ? 255 : 0;
	c->R = val;
    c->G = val;
    c->B = val;

	return c;
}

/**
 * 呼叫的人要手動清理該空間
 * Given an image, creates a new image extracting the LSB of the B channel.
 */
Image *steganography(Image *image)
{
	//YOUR CODE HERE
	Image *i = malloc(sizeof(Image));
	if (i == NULL){
		return NULL;
	}

	uint32_t rows = image->rows;
	uint32_t cols = image->cols;

	// 建立i field
	i->rows = rows;
	i->cols = cols;

	i->image = malloc(rows * sizeof(Color *));
	if (i->image == NULL) {
		freeImage(i);
		return NULL;
	}

	// 初始化指標避免配置過程不夠 free時出問題
	for (uint32_t row = 0; row < rows; row++) {
    	i->image[row] = NULL;
	}

	for (uint32_t row = 0; row < rows; row++) {
    	i->image[row] = malloc(cols * sizeof(Color));
		if (i->image[row] == NULL) {
			fprintf(stderr, "Error: Could not allocate memory for image[row]\n");
			freeImage(i);
			return NULL;
		}
	}

	for (uint32_t row = 0; row < rows; row++){
		for (uint32_t col = 0; col < cols; col++){
			Color *c = evaluateOnePixel(image, row, col);
			if (c == NULL){
				free(i->image);
				freeImage(i);
				return NULL;
			}
			i->image[row][col] = *c;

			//配置給i後 這些evaOnePixel配置的 要手動清除
			free(c);
		}
	}
	
	return i;
}

/*
Loads a file of ppm P3 format from a file, and prints to stdout (e.g. with printf) a new image, 
where each pixel is black if the LSB of the B channel is 0, 
and white if the LSB of the B channel is 1.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a file of ppm P3 format (not necessarily with .ppm file extension).
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!
*/
int main(int argc, char **argv)
{
	//YOUR CODE HERE
	if (argc < 2){
		fprintf(stderr, "Error: not enough para\n");
		return -1;
	}
	Image *i = readData(argv[1]);
	if (i == NULL){
		fprintf(stderr, "Error: read file failed\n");
		return -1;
	} 

	Image *new_I = steganography(i);
	if (new_I == NULL){
		fprintf(stderr, "Error: steganograph failed\n");
		// func裡面有建立動作 這裡要free
		freeImage(i);
		return -1;
	} 

	writeData(new_I);

	// print出來了 一樣free
	freeImage(i);
	freeImage(new_I);
	return 0;
}
