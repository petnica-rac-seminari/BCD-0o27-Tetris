
#include <freertos/mpu_wrappers.h>
#include <cstdlib>
#include "board.h"
#include <freertos/projdefs.h>

namespace tetrics_module
{
	void board::start()
	{
		currentRotation = 0;
		clear();
		createShape();
	}
	bool board::frame(TickType_t currtick)
	{
		TickType_t downDif = pdMS_TO_TICKS(500);
		TickType_t checkDif = pdMS_TO_TICKS(250);
		if (lastTick < currtick - downDif)
		{
			moveDown();
			lastTick = currtick;
		}
		else if (lastTick < currtick - checkDif)
		{
			return checkCollision();
		}
		return true;
	}
	void board::clear()
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				board[i][j] = 0;
			}
		}
	}
	void board::rotateShape(int *matrix)
	{
		currentRotation = (currentRotation + 1) % 4;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				currentShape[j][i] = matrix[j + i * 4 + currentRotation * 16] * currentShapeColor;
			}
		}
	}
	void board::rotate()
	{
		switch (shapeIndex)
		{
		case 0:
			rotateShape((int *)I_shape);
			break;
		case 1:
			rotateShape((int *)L_shape);
			break;
		case 2:
			rotateShape((int *)J_shape);
			break;
		case 3:
			rotateShape((int *)Z_shape);
			break;
		case 4:
			rotateShape((int *)T_shape);
			break;
		case 5:
			rotateShape((int *)S_shape);
			break;
		case 6:
			rotateShape((int *)O_shape);
			break;
		default:
			break;
		}
		for(int i = currentShapeX; i < currentShapeX+3; i++){
			for(int j = currentShapeY; j < currentShapeY+3; j++){
				board[i][j] = currentShape[i-currentShapeX][j-currentShapeY];
			}
		}
	}
	void board::copyMatrix(int *source, int *destination, int rotIndex)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				destination[j + i * 4] = source[j + i * 4 + rotIndex * 16] * currentShapeColor;
			}
		}
	}
	bool board::createShape()
	{
		currentShapeX = 3;
		currentShapeY = 0;
		currentShapeColor = rand() % 6 + 1;
		shapeIndex = rand() % 7;
		switch (shapeIndex)
		{
		case 0:
			copyMatrix((int *)I_shape, (int *)currentShape.data(), 0);
			break;
		case 1:
			copyMatrix((int *)L_shape, (int *)currentShape.data(), 0);
			break;
		case 2:
			copyMatrix((int *)J_shape, (int *)currentShape.data(), 0);
			break;
		case 3:
			copyMatrix((int *)Z_shape, (int *)currentShape.data(), 0);
			break;
		case 4:
			copyMatrix((int *)T_shape, (int *)currentShape.data(), 0);
			break;
		case 5:
			copyMatrix((int *)S_shape, (int *)currentShape.data(), 0);
			break;
		case 6:
			copyMatrix((int *)O_shape, (int *)currentShape.data(), 0);
			break;
		}
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				if (board[i][j] < 0)
					board[i][j] = board[i][j] * (-1);
			}
		}
		for (int i = currentShapeX; i < currentShapeX + 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (board[i][j] != 0 && currentShape[i - currentShapeX][j] < 0)
					return false;
				board[i][j] = currentShape[i - currentShapeX][j];
			}
		}
		return true;
	}
	void board::moveRight()
	{
		for (int i = currentShapeX; i < currentShapeX + 4; i++)
			for (int j = currentShapeY; j < currentShapeY + 4; j++)
				if (board[i][j] < 0 && (i + 1 >= width || board[i + 1][j] > 0))
					return;

		for (int i = currentShapeX + 3; i >= currentShapeX; i--)
			for (int j = currentShapeY; j < currentShapeY + 4; j++)
				if (board[i][j] < 0 && i+1<width) 
				{
					board[i + 1][j] = board[i][j];
					board[i][j] = 0;
				}

		currentShapeX += 1;
	}
	void board::moveLeft()
	{
		for (int i = currentShapeX; i < currentShapeX + 4; i++)
			for (int j = currentShapeY; j < currentShapeY + 4; j++)
				if (board[i][j] < 0 && (i <= 0 || board[i - 1][j] > 0))
					return;
				
		for (int i = currentShapeX; i < currentShapeX + 4; i++)
			for (int j = currentShapeY; j < currentShapeY + 4; j++)
				if (i < width && board[i][j] < 0) {
					board[i - 1][j] = board[i][j];
					board[i][j] = 0;
				}
		currentShapeX -= 1;
	}
	void board::moveDown()
	{
		bool canMove = true;
		for (int i = currentShapeX; i < currentShapeX + 4; i++)
			for (int j = currentShapeY; j < currentShapeY + 4; j++)
				if (board[i][j] < 0 && (j + 1 >= height || board[i][j + 1] > 0))
					return;

		for (int j = currentShapeY + 3; j >= currentShapeY; j--)
			for (int i = currentShapeX; i < currentShapeX + 4; i++)
				if (i < width && board[i][j] < 0)
				{
					board[i][j + 1] = board[i][j];
					board[i][j] = 0;
				}
			
		currentShapeY += 1;
	}
	bool board::checkCollision()
	{
		bool canMove = true;
		for (int i = currentShapeX; i < currentShapeX + 4; i++)
		{
			for (int j = currentShapeY; j < currentShapeY + 4; j++)
			{
				if ((board[i][j] < 0 && j + 1 == height) || (board[i][j] < 0 && board[i][j + 1] > 0))
					canMove = false;
			}
		}
		for (int j = height - 1; j > 0; j--)
		{
			int numOfBlocks = 0;
			for (int i = 0; i < width; i++)
			{
				if (board[i][j] > 0)
					numOfBlocks++;
			}
			if (numOfBlocks == width)
			{
				for (int k = j; k > 0; k--)
				{
					for (int i = 0; i < width; i++)
					{
						if (board[i][k - 1] >= 0)
							board[i][k] = board[i][k - 1];
						else if (k == j)
						{
							board[i][k] = 0;
						}
					}
				}
				j++;
			}
		}
		if (!canMove)
		{
			return createShape();
		}
		return true;
	}
	int board::getTile(int x, int y)
	{
		return 0;
	}
}