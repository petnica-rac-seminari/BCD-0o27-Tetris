#include "main.hpp"

void Main::updateInput()
{
	controller.clear();
	controller.capture();

	bool backButtonPressed_prev_old = backButtonPressed_prev;
	bool leftButtonPressed_prev_old = leftButtonPressed_prev;
	bool rightButtonPressed_prev_old = rightButtonPressed_prev;
	bool downButtonPressed_prev_old = downButtonPressed_prev;
	bool upButtonPressed_prev_old = upButtonPressed_prev;
	bool selectButtonPressed_prev_old = selectButtonPressed_prev;
	bool pauseButtonPressed_prev_old = pauseButtonPressed_prev;

	backButtonPressed_prev = controller.getButtonState(BUTTON_X);
	leftButtonPressed_prev = controller.getButtonState(BUTTON_LEFT);
	rightButtonPressed_prev = controller.getButtonState(BUTTON_RIGHT);
	downButtonPressed_prev = controller.getButtonState(BUTTON_DOWN);
	upButtonPressed_prev = controller.getButtonState(BUTTON_UP);
	selectButtonPressed_prev = controller.getButtonState(BUTTON_B);
	pauseButtonPressed_prev = controller.getButtonState(BUTTON_Y);

	backButtonPressed = backButtonPressed_prev && !backButtonPressed_prev_old;
	leftButtonPressed = leftButtonPressed_prev && !leftButtonPressed_prev_old;
	rightButtonPressed = rightButtonPressed_prev && !rightButtonPressed_prev_old;
	downButtonPressed = downButtonPressed_prev && !downButtonPressed_prev_old;
	upButtonPressed = upButtonPressed_prev && !upButtonPressed_prev_old;
	selectButtonPressed = selectButtonPressed_prev && !selectButtonPressed_prev_old;
	pauseButtonPressed = pauseButtonPressed_prev && !pauseButtonPressed_prev_old;
}

Main::GameState Main::runStartScreen()
{
	GameState exitState = GameState::Running;

	const char *start_text = "Start";
	srect16 start_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), start_text).bounds().center((srect16)lcd.bounds()).offset(0, -6);
	const char *exit_text = "Exit";
	srect16 exit_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), exit_text).bounds().center(start_text_rect).offset(0, start_text_rect.height() + 2);

	lcd.clear(lcd.bounds());

	draw::filled_rectangle(lcd, rect16(point16(45, 46), size16(70, 36)), color<pixel_type>::black);
	draw::rectangle(lcd, rect16(point16(45, 46), size16(70, 36)), color<pixel_type>::white);
	draw::text(lcd, start_text_rect, start_text, textFont, color<pixel_type>::white);
	draw::text(lcd, exit_text_rect, exit_text, textFont, color<pixel_type>::white);

	int selectedButton = 0;
	auto renderScene = [&]()
	{
		rect16 startDot = rect16(point16(start_text_rect.left(), (start_text_rect.y1 + start_text_rect.y2) / 2).offset(-10, -3), size16(6, 6));
		rect16 exitDot = rect16(point16(exit_text_rect.left(), (exit_text_rect.y1 + exit_text_rect.y2) / 2).offset(-10, -3), size16(6, 6));
		switch (selectedButton)
		{
		case 0:
		{
			lcd.clear(exitDot);
			draw::filled_ellipse(lcd, startDot, color<pixel_type>::white);
			break;
		}
		case 1:
			lcd.clear(startDot);
			draw::filled_ellipse(lcd, exitDot, color<pixel_type>::white);
			break;
		}
	};

	renderScene();

	while (true)
	{
		updateInput();

		if (upButtonPressed)
		{
			if (selectedButton > 0)
			{
				--selectedButton;
				renderScene();
			}
		}
		if (downButtonPressed)
		{
			if (selectedButton < 1)
			{
				++selectedButton;
				renderScene();
			}
		}

		if (selectButtonPressed)
		{
			switch (selectedButton)
			{
			case 0:
				exitState = GameState::Running;
				break;
			case 1:
				exitState = GameState::Exit;
				break;
			}

			break;
		}
	}

	lcd.clear(lcd.bounds());

	return exitState;
}

pixel_type getColor(int value)
{
	switch (value)
	{
	case 0:
		return color<pixel_type>::black;
	case 1:
		return color<pixel_type>::red;
	case 2:
		return color<pixel_type>::orange;
	case 3:
		return color<pixel_type>::yellow;
	case 4:
		return color<pixel_type>::green;
	case 5:
		return color<pixel_type>::blue;
	case 6:
		return color<pixel_type>::violet;
	default:
		ESP_LOGE(TAG_FS, "Invalid board value");
		return color<pixel_type>::brown;
	}
}

Main::GameState Main::runGameScreen()
{
	const char *TETRIS_text = "TETRIS";
	srect16 TETRIS_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), TETRIS_text).bounds().center_horizontal((srect16)lcd.bounds());
	const char *score_text = "Score";
	srect16 score_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), score_text).bounds().offset(115, 10);
	const char *Next_text = "Next";
	srect16 Next_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), Next_text).bounds().center(score_text_rect).offset(0, 20);
	const char* topScore_text = "Top";
	srect16 topScore_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), topScore_text).bounds().offset(15, 10);

	srect16 GameRectangle_rect = srect16(spoint16(0, 0), ssize16(52, 112)).center_horizontal((srect16)lcd.bounds()).offset(0, 9);
	srect16 NextRectangle_rect = srect16(spoint16(0, 0), ssize16(32, 32)).center_horizontal(Next_text_rect).offset(0, 9);

	draw::text(lcd, TETRIS_text_rect, TETRIS_text, textFont, color<pixel_type>::white);
	draw::text(lcd, score_text_rect, score_text, textFont, color<pixel_type>::white);
	draw::text(lcd, Next_text_rect, Next_text, textFont, color<pixel_type>::white);
	draw::text(lcd, topScore_text_rect, topScore_text, textFont, color<pixel_type>::white);

	for (int i = 0; i < previousScoreCount; ++i)
	{
		char text[128];
		sprintf(text, "%d", previousScores[i]);
		srect16 rect = textFont.measure_text((ssize16)lcd.dimensions(), text).bounds().center(topScore_text_rect).offset(0, 10 + 10 * i);

		draw::text(lcd, rect, text, textFont, color<pixel_type>::gray);
	}
	
	draw::rectangle(lcd, GameRectangle_rect, color<pixel_type>::white);
	draw::rectangle(lcd, NextRectangle_rect, color<pixel_type>::white);
	
	if (!paused)
		board.start();
	paused = false;

	uint8_t* gameBmpBuffer = (uint8_t *)malloc(bmp_type::sizeof_buffer(size16(board.width * 10, board.height * 22))*sizeof(uint8_t));
	bmp_type gameBmp { size16(board.width * 10, board.height * 22), gameBmpBuffer };

	int displayerScore = -1;
	while (true)
	{
		updateInput();		
		TickType_t tick = xTaskGetTickCount();

		if (backButtonPressed)
		{
			break;
		}
		if (controller.getButtonState(BUTTON_LEFT))
		{
			board.moveLeft();
		}
		if (controller.getButtonState(BUTTON_RIGHT))
		{
			board.moveRight();
		}
		if (controller.getButtonState(BUTTON_DOWN))
		{
			board.moveDown();
		}
		if (selectButtonPressed)
		{
			board.rotate();
		}
		if (pauseButtonPressed)
		{
			paused = true;
			return GameState::Paused;
		}
		if (upButtonPressed)
		{
			board.drop(tick);
		}




		if (!board.frame(tick))
		{
			return GameState::Lost;
		}

		if (displayerScore != board.score)
		{
		char score_number[128];
		sprintf(score_number, "%d", board.score);
		srect16 score_number_rect = textFont.measure_text((ssize16)lcd.dimensions(), score_number).bounds().center((srect16)score_text_rect).offset(0, 10);
		draw::filled_rectangle(lcd, score_number_rect, color<pixel_type>::black);
		draw::text(lcd, score_number_rect, score_number, textFont, color<pixel_type>::gray);

		displayerScore = board.score;
		}

		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
			{
				pixel_type rectColor = getColor(abs(board.nextShape[i][j] * board.nextShapeColor));
				rect16 rectangle(point16(NextRectangle_rect.x1 + 6 + i * 5, NextRectangle_rect.y1 + 6 + j * 5), size16(5, 5));
				draw::filled_rectangle(lcd, rectangle, rectColor);
			}
		
		for (int i = 0; i < board.width; ++i)
		{
			for (int j = 0; j < board.height; ++j)
			{				
				pixel_type rectColor = getColor(abs(board.board[i][j]));
				rect16 rectangle(point16(i * 5, j * 5), size16(5, 5));
				draw::filled_rectangle(gameBmp, rectangle, rectColor);
			}
		}

		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (board.currentShape[i][j] < 0)
				{
					rect16 rectangle(point16(board.currentShapeX * 5 + i * 5, board.getDropCoordinate() * 5 + j * 5), size16(5, 5));
					draw::rectangle(gameBmp, rectangle, color<pixel_type>::white);
				}

		draw::bitmap(lcd, rect16(point16(55, 10), size16(50, 110)), gameBmp, rect16(point16(0, 0), size16(50, 110)));
	}

	return GameState::Start;
}

Main::GameState Main::runPauseScreen()
{
	const char* text1 = "PAUSED";
	const char* text2 = "Press to continue";		
	srect16 text1_rect = textFont.measure_text((ssize16)lcd.dimensions(), text1).bounds().center((srect16)lcd.bounds().offset(0, -5));
	srect16 text2_rect = textFont.measure_text((ssize16)lcd.dimensions(), text2).bounds().center((srect16)lcd.bounds().offset(0, +5));
	srect16 textRectangle_rect = srect16(spoint16(0, 0), ssize16(text2_rect.width() + 2, 34)).center((srect16)lcd.bounds());

	draw::filled_rectangle(lcd, textRectangle_rect, color<pixel_type>::black);
	draw::rectangle(lcd, textRectangle_rect, color<pixel_type>::white);
	draw::text(lcd, text1_rect, text1, textFont, color<pixel_type>::white);
	draw::text(lcd, text2_rect, text2, textFont, color<pixel_type>::white);	

	while (true)
	{
		updateInput();

		if (upButtonPressed ||
			downButtonPressed ||
			leftButtonPressed ||
			rightButtonPressed ||
			selectButtonPressed ||
			backButtonPressed ||
			pauseButtonPressed)
			break;
	}

	lcd.clear(lcd.bounds());

	return GameState::Running;
}

Main::GameState Main::runLostScreen()
{
	const char* text1 = "YOU LOST";
	char text2[128];
	sprintf(text2, "Score: %d", board.score);
	srect16 text1_rect = textFont.measure_text((ssize16)lcd.dimensions(), text1).bounds().center((srect16)lcd.bounds().offset(0, -5));
	srect16 text2_rect = textFont.measure_text((ssize16)lcd.dimensions(), text2).bounds().center((srect16)lcd.bounds().offset(0, +5));
	srect16 textRectangle_rect = srect16(spoint16(0, 0), ssize16(text2_rect.width() + 2, 34)).center((srect16)lcd.bounds());

	draw::filled_rectangle(lcd, textRectangle_rect, color<pixel_type>::black);
	draw::rectangle(lcd, textRectangle_rect, color<pixel_type>::white);
	draw::text(lcd, text1_rect, text1, textFont, color<pixel_type>::white);
	draw::text(lcd, text2_rect, text2, textFont, color<pixel_type>::white);

	for (int i = 0; i < 9; ++i)
		previousScores[i + 1] = previousScores[i];

	if (previousScoreCount < 10)
		++previousScoreCount;

	previousScores[0] = board.score;

	while (true)
	{
		updateInput();

		if (upButtonPressed ||
			downButtonPressed ||
			leftButtonPressed ||
			rightButtonPressed ||
			selectButtonPressed ||
			backButtonPressed ||
			pauseButtonPressed)
			break;
	}

	lcd.clear(lcd.bounds());

	return GameState::Start;
}

Main::GameState Main::runEndScreen()
{
	GameState exitState = GameState::Running;

	const char *play_again_text = "Play again\r\n";
	srect16 play_again_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), play_again_text).bounds().center((srect16)lcd.bounds()).offset(0, -3);
	const char *exit_text = "Exit\r\n";
	srect16 exit_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), exit_text).bounds().center(play_again_text_rect).offset(0, 12);

	draw::filled_ellipse(lcd, rect16(point16(10, 10), lcd.dimensions().inflate(-20, -20)), color<pixel_type>::red);
	// drawJPEG("/a.jpeg", point16(0, 0));

	draw::filled_rectangle(lcd, rect16(point16(45, 46), size16(70, 36)), color<pixel_type>::black);
	draw::rectangle(lcd, rect16(point16(45, 46), size16(70, 36)), color<pixel_type>::white);
	draw::text(lcd, play_again_text_rect, play_again_text, textFont, color<pixel_type>::white);
	draw::text(lcd, exit_text_rect, exit_text, textFont, color<pixel_type>::white);

	int selectedButton = 0;
	auto renderScene = [&]()
	{
		rect16 startDot = rect16(point16(play_again_text_rect.left(), (play_again_text_rect.y1 + play_again_text_rect.y2) / 2).offset(-10, -6), size16(6, 6));
		rect16 exitDot = rect16(point16(exit_text_rect.left(), (exit_text_rect.y1 + exit_text_rect.y2) / 2).offset(-10, -6), size16(6, 6));
		switch (selectedButton)
		{
		case 0:
		{
			lcd.clear(exitDot);
			draw::filled_ellipse(lcd, startDot, color<pixel_type>::white);
			break;
		}
		case 1:
			lcd.clear(startDot);
			draw::filled_ellipse(lcd, exitDot, color<pixel_type>::white);
			break;
		}
	};

	renderScene();

	while (true)
	{
		controller.capture();

		if (controller.getButtonState(BUTTON_UP))
		{
			if (selectedButton > 0)
			{
				--selectedButton;
				renderScene();
			}
		}
		if (controller.getButtonState(BUTTON_DOWN))
		{
			if (selectedButton < 1)
			{
				++selectedButton;
				renderScene();
			}
		}

		if (controller.getButtonState(BUTTON_A))
		{
			switch (selectedButton)
			{
			case 0:
				exitState = GameState::Running;
				break;
			case 1:
				exitState = GameState::Exit;
			}

			break;
		}
	}

	lcd.clear(lcd.bounds());

	return exitState;
}

point16 _drawJPEG_destination;

void Main::drawJPEG(const char *path, point16 destination)
{
	_drawJPEG_destination = destination;
	file_stream fs(path);
	if (!fs.caps().read)
	{
		ESP_LOGE(TAG_FS, "Failed to open a.jpeg");
	}
	else
	{
		jpeg_image::load(
			&fs,
			[](size16 dimensions, jpeg_image::region_type &region, point16 location, void *state)
			{
				_drawJPEG_destination.x += location.x;
				_drawJPEG_destination.y += location.y;
				return draw::bitmap(bcd_sys.getDisplay(), rect16(_drawJPEG_destination, region.dimensions()), region, region.bounds());
			},
			nullptr);
	}
}