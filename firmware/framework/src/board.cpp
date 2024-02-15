
#include <freertos/mpu_wrappers.h>
#include <cstdlib>
#include "board.h"
#include <freertos/projdefs.h>

namespace tetrics_module
{
	void board::start()
	{
        downDifMS = 500;
		checkDifMS = 250;
		score = 0;
		currentRotation = 0;
		inc = 0;
		clear();
		createShape();
		nextShape[0][0] = 69;
	}
	bool board::frame(TickType_t currtick)
	{
		TickType_t downDif = pdMS_TO_TICKS(downDifMS);
		TickType_t checkDif = pdMS_TO_TICKS(checkDifMS);
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

		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
				if(board[i][j] > 0 && rotatedShape[i - currentShapeX][j - currentShapeY] < 0)
					return;

		currentShape = rotatedShape;

		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
			{
				if(board[i][j] < 0) 
					board[i][j] = 0;

				if(currentShape[i - currentShapeX][j - currentShapeY] < 0)
					board[i][j] = currentShapeColor;
			}
	}	
	bool board::createShape()
	{
		currentShapeX = 3;
		currentShapeY = 0;
		currentRotation = 0;
		if(nextShape[0][0] == 69) {
			shapeIndex = rand() % 7;
			currentShapeColor = rand() % 6 + 1;
			currentShape = getShape(shapeIndex, currentRotation);
			nextShapeColor = rand() % 6 + 1;
			nextShapeIndex = rand() % 7;
			nextShape = getShape(nextShapeIndex, currentRotation);	
		}else {
			currentShape = nextShape;
			currentShapeColor = nextShapeColor;
			shapeIndex = nextShapeIndex;
			nextShapeColor = rand() % 6 + 1;
			nextShapeIndex = rand() % 7;
			nextShape = getShape(nextShapeIndex, currentRotation);
		}

		for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)		
			for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
			{
				if (board[i][j] != 0 && currentShape[i - currentShapeX][j] < 0)
					return false;

				board[i][j] = currentShape[i - currentShapeX][j] * currentShapeColor;
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
				if (board[i][j] < 0 && (j + 1 == height || board[i][j + 1] > 0)){
					canMove = false;
					updateScore(4);
				}

		if (!canMove)
		{
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
					updateScore(10);
					for (int k = j; k > 0; k--)
						for (int i = 0; i < width; i++)
							board[i][k] = board[i][k - 1];

					for (int i = 0; i < width; i++)
						board[i][0] = 0;

					j++;
				}
			}
			
			for (int i = std::max(currentShapeX, 0); i < std::min(currentShapeX + 4, width); i++)
				for (int j = std::max(currentShapeY, 0); j < std::min(currentShapeY + 4, height); j++)
					if (board[i][j] < 0)
						board[i][j] = -board[i][j];

			return createShape();
		}
		return true;
	}	
	void board::updateScore(int increase) {
		score += increase;
		inc += increase;
		if(inc > 10 && downDifMS >= 100) {
			downDifMS -= speedUp*2;
			checkDifMS -= speedUp;
			inc = 0;
		}
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