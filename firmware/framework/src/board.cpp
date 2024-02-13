#include "board.h"
#include<cstdlib>

namespace tetrics_module
{
	void board::frame()
	{
		
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
				currentShape[i][j] = matrix[x][i][j];
			}
			
		}
		
	} 
	void board::rotate()
	{
		switch (shapeIndex)
		{
		case 1:
			rotateShape(I_shape);
			break;
		case 2:
			rotateShape(L_shape);
			break;
		case 3:
			rotateShape(J_shape);
			break;
		case 4:
			rotateShape(Z_shape);
			break;
		case 5:
			rotateShape(T_shape);
			break;
		case 6:
			rotateShape(S_shape);
			break;
		case 7:
			rotateShape(O_shape);
			break;
		default:
			break;
		}
	}
	void board::createShape()
	{
		//treba dodati random biranje i da se zapravo stvori
		currentShapeX = 0;
		currentShapeY = 0;
		currentShapeColor = rand() % 6 + 1;
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
	int board::getTile(int x, int y)
	{
		return 0;
	}
}