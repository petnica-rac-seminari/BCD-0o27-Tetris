
#include <freertos/mpu_wrappers.h>
#include<cstdlib>
#include "board.h"
#include <freertos/projdefs.h>

namespace tetrics_module
{
	void board::frame(TickType_t currtick)
	{
		TickType_t downDif = pdMS_TO_TICKS(500);
		TickType_t checkDif = pdMS_TO_TICKS(250);
		if(lastTick < currtick-downDif){
			moveDown();
			lastTick = currtick;
		}else if(lastTick < currtick-checkDif){
			checkCollision();
		}

	}
	void board::clear()
	{		
		for(int i = 0; i < width; i++){
			for(int j = 0; j < height; j++){
				board[i][j] = 0;
			}
		}
	}
	void board::rotateShape(int matrix[4][4][4])
	{
		int x = (currentRotation + 1) % 4;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				currentShape[i][j] = matrix[x][i][j]*currentShapeColor;
			}
			
		}
		
	} 
	void board::rotate()
	{
		switch (shapeIndex){
		case 0:
			rotateShape(I_shape);
			break;
		case 1:
			rotateShape(L_shape);
			break;
		case 2:
			rotateShape(J_shape);
			break;
		case 3:
			rotateShape(Z_shape);
			break;
		case 4:
			rotateShape(T_shape);
			break;
		case 5:
			rotateShape(S_shape);
			break;
		case 6:
			rotateShape(O_shape);
			break;
		default:
			break;
		}
	}
	void board::copyMatrix(int source[4][4][4], int destination[4][4], int rotIndex)
	{
		for(int i = 0; i < 4; i++){
			for(int j = 0; j < 4; j++){
				destination[i][j] = source[rotIndex][i][j]*currentShapeColor;
			}
		}
	}
	void board::createShape()
	{
		currentShapeX = 0;
		currentShapeY = 0;
		currentShapeColor = rand() % 6 + 1;
		shapeIndex = rand() % 7;
		switch(shapeIndex){
			case 0:
				copyMatrix(I_shape,currentShape,0);
				break;
			case 1:
				copyMatrix(L_shape,currentShape,0);
				break;
			case 2:
				copyMatrix(J_shape,currentShape,0);
				break;
			case 3:
				copyMatrix(Z_shape,currentShape,0);
				break;
			case 4:
				copyMatrix(T_shape,currentShape,0);
				break;
			case 5:
				copyMatrix(S_shape,currentShape,0);
				break;
			case 6:
				copyMatrix(O_shape,currentShape,0);
				break;
		}
		for(int i = 0; i < width; i++){
			for(int j = 0; j < height; j++){
				if(board[i][j] < 0) board[i][j] = board[i][j]*(-1);
			}
		}
	}
	void board::moveRight()
	{
		bool canMove = true;
		for(int i = currentShapeX; i < currentShapeX+4; i++){
			for(int j = currentShapeY; j < currentShapeY+4; j++){
				if((board[i][j] < 0 && i+1==width) || (board[i][j] < 0 && board[i+1][j] > 0)){
					canMove = false;
					goto izadji;
				}
			}
		}
		izadji:
		if(!canMove) return;
		for(int i = currentShapeX+3; i >= currentShapeX; i--){
			for(int j = currentShapeY; j < currentShapeY+4; j++){
				if(board[i][j] < 0){
					board[i+1][j] = board[i][j];
					board[i][j] = 0;
				}
			}
		}
		currentShapeX+=1;
	}
	void board::moveLeft()
	{
		bool canMove = true;
		for(int i = currentShapeX; i < currentShapeX+4; i++){
			for(int j = currentShapeY; j < currentShapeY+4; j++){
				if((board[i][j] < 0 && i == 0) || (board[i][j] < 0 && board[i-1][j] > 0)){
					canMove = false;
					goto izadji;
				}
			}
		}
		izadji:
		if(!canMove) return;
		for(int i = 0; i < currentShapeX+4; i++){
			for(int j = currentShapeY; j < currentShapeY+4; j++){
				if(board[i][j] < 0){
					board[i-1][j] = board[i][j];
					board[i][j] = 0;
				}
			}
		}
		currentShapeX-=1;
		
	}
	void board::moveDown()
	{
		bool canMove = true;
		for(int i = currentShapeX; i < currentShapeX+4; i++){
			for(int j = currentShapeY; j < currentShapeY+4; j++){
				if((board[i][j] < 0 && j+1 == height) || (board[i][j] < 0 && board[i][j+1] > 0)){
					canMove = false;
					goto izadji;
				}
			}
		}
		izadji:
		if(!canMove) return;
		for(int j = currentShapeY+3; j >= currentShapeY; j--){
			for(int i = 0; i < currentShapeX+4; i++){
				if(board[i][j] < 0){
					board[i][j+1] = board[i][j];
					board[i][j] = 0;
				}
			}
		}
		currentShapeY+=1;
	}
	void board::checkCollision()
	{
		bool canMove = true;
		for(int i = currentShapeX; i < currentShapeX+4; i++){
			for(int j = currentShapeY; j < currentShapeY+4; j++){
				if((board[i][j] < 0 && j+1 == height) || (board[i][j] < 0 && board[i][j+1] > 0))
					canMove = false;
			}
		}
		for(int j = height-1; j > 0; j--){
			int numOfBlocks = 0;
			for(int i = 0; i < width; i++){
				if(board[i][j]>0) numOfBlocks++;
			}
			if(numOfBlocks == width){
				for(int k = j; k > 0; k--){
					for(int i =0; i < width; i++){
						if(board[i][k-1] >= 0)
							board[i][k] = board[i][k-1];
						else if(k == j){
							board[i][k] = 0;
						}
					}
				}
				j++;
			}
		}
		if(!canMove) {
			createShape();
		}
	}
	int board::getTile(int x, int y)
	{
		return 0;	
	}
}