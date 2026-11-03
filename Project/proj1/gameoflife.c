/************************************************************************
**
** NAME:        gameoflife.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Justin Yokota - Starter Code
**				YOUR NAME HERE
**
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

uint32_t wrap_len(int dim, int shift, uint32_t length);

//Determines what color the cell at the given row/col should be. This function allocates space for a new Color.
//Note that you will need to read the eight neighbors of the cell in question. The grid "wraps", so we treat the top row as adjacent to the bottom row
//and the left column as adjacent to the right column.
Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule)
{
	//YOUR CODE HERE
	// 建立空間 存放計算好的結果
	Color *c = malloc(sizeof(Color));
	if (c == NULL){
		fprintf(stderr, "Error: Could not allocate memory for color\n");
		return NULL;
	}

	// 算wrapped around 
	// 計算完存指標，idx方便記錄放在哪
	Color *neighbors[8];
	int neighbor_idx = 0;

	for (int dr = -1; dr < 2; dr++){
		for (int dc = -1; dc < 2; dc++){
			// jump itself
			if (dr==0 && dc==0){
				continue;
			}
			// 找到實際查找的座標
			uint32_t wR = wrap_len(row, dr, image->rows);
			uint32_t wC = wrap_len(col, dc, image->cols);

			neighbors[neighbor_idx] = &image->image[wR][wC];
			neighbor_idx++;
		}
	}

	// 迴圈順序 針對RGB，看他的8個bit，8個鄰居  鄰居idx上面算好了
	for (int channel = 0; channel < 3; channel++){
		uint8_t next_color = 0;

		for (int bit = 0; bit < 8; bit++) {
			int alive_neighbors = 0;
			for (int n = 0; n < 8; n++) {
				uint8_t neighbor_val = 0;
				if (channel == 0) {
					neighbor_val = neighbors[n]->R;
				} else if (channel == 1) {
					neighbor_val = neighbors[n]->G;
				} else {
					neighbor_val = neighbors[n]->B;
				}
            	alive_neighbors += ((neighbor_val >> bit) & 1);
			}

			// 根據neighbor決定這個bit下一個狀態，先抓自己該c,b的值 決定生死
			int cur_bit = 0;
			if (channel == 0){
				cur_bit = (image->image[row][col].R >> bit) & 1;
			} else if (channel == 1){
				cur_bit = (image->image[row][col].G >> bit) & 1;
			} else {
				cur_bit = (image->image[row][col].B >> bit) & 1;
			}

			// 根據rule跟生死 決定下個狀態
			int next_bit = (cur_bit == 1) ? ((rule >> (alive_neighbors + 9)) & 1) : ((rule >> alive_neighbors) & 1);
			if (next_bit){
                next_color |= (1 << bit);
            }
		}

		// 儲存結果回去next_color  一次寫入8bit
		if(channel == 0){
			c->R = next_color;
		} else if (channel == 1){
			c->G = next_color;
		} else{
			c->B = next_color;
		}
	}
	return c;
}


//The main body of Life; given an image and a rule, computes one iteration of the Game of Life.
//You should be able to copy most of this from steganography.c
Image *life(Image *image, uint32_t rule)
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
	// 初始化避免配置過程不夠 free時出問題
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
	// calc 這裡要用上面的
	for (uint32_t row = 0; row < rows; row++){
		for (uint32_t col = 0; col < cols; col++){
			Color *c = evaluateOneCell(image, row, col, rule);
			if (c == NULL){
				free(i->image);
				freeImage(i);
				return NULL;
			}
			i->image[row][col] = *c;
			//更新玩image了 把cfree掉
			free(c);
		}
	}
	
	return i;
}

/*
Loads a .ppm from a file, computes the next iteration of the game of life, then prints to stdout the new image.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a .ppm.
argv[2] should contain a hexadecimal number (such as 0x1808). Note that this will be a string.
You may find the function strtol useful for this conversion.
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!

You may find it useful to copy the code from steganography.c, to start.
*/
int main(int argc, char **argv)
{
	//YOUR CODE HERE
	// assume that the rule is valid, only check argc
	if (argc != 3){
		printf("usage: ./gameOfLife filename rule\n"
			"filename is an ASCII PPM file (type P3) with maximum value 255.\n"
			"rule is a hex number beginning with 0x; Life is 0x1808.");
	}

	Image *i = readData(argv[1]);
	if (i == NULL){
		fprintf(stderr, "Error: read file failed\n");
		return -1;
	}

	// 把字串argv[2]轉型成 參數需要的type, base0 代表給c解析 0x 0 等等
	char *endptr;
	long parse_rule = strtol(argv[2], &endptr, 0);
	if(*endptr != '\0' || parse_rule <= 0 || parse_rule > UINT32_MAX){
		fprintf(stderr, "Invalid rule\n");
	}
	uint32_t rule = (uint32_t) parse_rule;
	
	Image *new_I = life(i, rule);
	if (new_I == NULL){
		fprintf(stderr, "Error: life failed\n");
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


/**
 * 計算wrap 並返回
 */
uint32_t wrap_len(int dim, int shift, uint32_t length)
{
	if (length == 0){
		fprintf(stderr, "Error: wrong rows or cols\n");
		return 0;
	}
	
	// shift可能-1  會被轉成unsign 造成計算錯誤
	//int ret = (dim + shift + length) % length;
	int len = (int) length;
	int ret = (dim+shift+len) % len;
	return (uint32_t) ret;
} 