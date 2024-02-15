
#include <freertos/mpu_wrappers.h>
#include <cstdlib>
#include "board.h"
#include <freertos/projdefs.h>

namespace tetrics_module
{
	void board::start()
	{		
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
			for (int j = 0; j < height; j++)
				board[i][j] = 0;
	}	
	void board::rotate()
	{		
		std::array<std::array<int, 4>, 4> rotatedShape = getShape(shapeIndex, (currentRotation + 1) % 4);

		for (int i = currentShapeX; i < currentShapeX + 4; i++)
			for (int j = currentShapeY; j < currentShapeY + 4; j++)
				if (rotatedShape[i - currentShapeX][j - currentShapeY] < 0 && (i < 0 || i >= width || j < 0 || j >= height || board[i][j] > 0))
					return;

		currentRotation = (currentRotation + 1) % 4;
		currentShape = rotatedShape;

		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
			{
				if(board[i][j] < 0) 
					board[i][j] = 0;

				if(currentShape[i - currentShapeX][j - currentShapeY] < 0)
					board[i][j] = -currentShapeColor;
			}
	}	
	bool board::createShape()
	{
		currentShapeX = rand() % 7;
		currentShapeY = 0;
		currentRotation = 0;
		currentShapeColor = rand() % 6 + 1;
		shapeIndex = rand() % 7;

		currentShape = getShape(shapeIndex, currentRotation);
		
		for (int i = 0; i < 4; i++)		
			for (int j = 0; j < 4; j++)
			{
				if (currentShape[i][j] < 0 && board[currentShapeX + i][currentShapeY + j] != 0)
					return false;

				if (currentShape[i][j] < 0)
					board[currentShapeX + i][currentShapeY + j] = -currentShapeColor;
			}		
		
		return true;
	}
	void board::moveRight()
	{
		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
				if (board[i][j] < 0 && (i + 1 == width || board[i + 1][j] > 0))
					return;

		for (int i = std::min(currentShapeX + 3, width - 1); i >= std::max(currentShapeX, 0); i--)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
				if (board[i][j] < 0)
				{
					board[i + 1][j] = board[i][j];
					board[i][j] = 0;
				}

		currentShapeX += 1;
	}
	void board::moveLeft()
	{
		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
				if (board[i][j] < 0 && (i == 0 || board[i - 1][j] > 0))
					return;
				
		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
				if (board[i][j] < 0) 
				{
					board[i - 1][j] = board[i][j];
					board[i][j] = 0;
				}

		currentShapeX -= 1;
	}
	void board::moveDown()
	{
		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
				if (board[i][j] < 0 && (j + 1 == height || board[i][j + 1] > 0))
					return;

		for (int j = std::min(currentShapeY + 3, height - 1); j >= std::max(currentShapeY, 0); j--)
			for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
				if (board[i][j] < 0)
				{
					board[i][j + 1] = board[i][j];
					board[i][j] = 0;
				}
			
		currentShapeY += 1;
	}
	bool board::checkCollision()
	{
		bool canMove = true;

		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
				if (board[i][j] < 0 && (j + 1 == height || board[i][j + 1] > 0))
					canMove = false;

		if (canMove)
			return true;

		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
				if (board[i][j] < 0)
					board[i][j] = -board[i][j];
	
		for (int j = height - 1; j >= 0; j--)
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
					for (int i = 0; i < width; i++)
						board[i][k] = board[i][k - 1];
						
				for (int i = 0; i < width; i++)
					board[i][0] = 0;
				j++;
			}
		}

		return createShape();
	}	
	board::piece board::getShape(int shapeIndex, int rotation)
	{
		switch (shapeIndex)
		{
		case 0: return I_shape[rotation];
		case 1: return L_shape[rotation];
		case 2: return J_shape[rotation];
		case 3: return Z_shape[rotation];
		case 4: return T_shape[rotation];
		case 5: return S_shape[rotation];
		case 6: return O_shape[rotation];
		}

		return { { } };
	}
}