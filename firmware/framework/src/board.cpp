#include "board.h"

namespace tetrics_module
{
	void board::frame()
	{
	}
	void board::clear()
	{		
	}
	void board::rotate()
	{
	}
	void board::rotateShape(int matrix[4][4][4])
	{
		int x =(currentRotation + 1) % 4;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				currentShape[i][j] = matrix[x][i][j];
			}
			
		}
		
	}
	void board::moveRight()
	{
	}
	void board::moveLeft()
	{
	}
	void board::moveDown()
	{
	}
	int board::getTile(int x, int y)
	{
		return 0;
	}
}