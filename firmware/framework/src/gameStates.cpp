#include "main.hpp"

Main::GameState Main::runStartScreen()
{
	GameState exitState = GameState::Running;		

	const char* start_text = "Start";
    srect16 start_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), start_text).bounds().center((srect16)lcd.bounds()).offset(0, -3);
	const char* exit_text = "Exit";
    srect16 exit_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), exit_text).bounds().center(start_text_rect).offset(0, 2);
	
	//draw::filled_ellipse(lcd, rect16(point16(10, 10), lcd.dimensions().inflate(-20, -20)), color<pixel_type>::red);	
	//drawJPEG("/a.jpeg", point16(0, 0));

	lcd.clear(lcd.bounds());

	draw::filled_rectangle(lcd, rect16(point16(45, 46),size16(70, 36)), color<pixel_type>::black);
	draw::rectangle(lcd, rect16(point16(45, 46), size16(70, 36)), color<pixel_type>::white);
	draw::text(lcd, start_text_rect, start_text, textFont, color<pixel_type>::white);
    draw::text(lcd, exit_text_rect, exit_text, textFont, color<pixel_type>::white);

	int selectedButton = 0;
	auto renderScene = [&]() 
	{
		rect16 startDot = rect16(point16(start_text_rect.left(), (start_text_rect.y1 + start_text_rect.y2) / 2).offset(-10, -6), size16(6, 6));
		rect16 exitDot = rect16(point16(exit_text_rect.left(), (exit_text_rect.y1 + exit_text_rect.y2) / 2).offset(-10, -6), size16(6, 6));
		switch (selectedButton)
		{
		case 0: {
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

Main::GameState Main::runGameScreen()
{
	const char* TETRIS_text = "TETRIS";
    srect16 TETRIS_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), TETRIS_text).bounds().center_horizontal((srect16)lcd.bounds());
	const char* exitInfo_text = "Click <BACK> to exit game";
    srect16 exitInfo_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), exitInfo_text).bounds();
	exitInfo_text_rect.y1 = 120;
	exitInfo_text_rect.y2 = 128;

	draw::text(lcd, TETRIS_text_rect, TETRIS_text, textFont, color<pixel_type>::white);
	draw::text(lcd, exitInfo_text_rect, exitInfo_text, textFont, color<pixel_type>::white);

	while (true)
	{
		TickType_t tick = xTaskGetTickCount();
		board.frame(tick);
	}

	return GameState::Start;
}

Main::GameState Main::runEndScreen()
{
	GameState exitState = GameState::Running;		

	const char* play_again_text = "Play again\r\n";
    srect16 play_again_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), play_again_text).bounds().center((srect16)lcd.bounds()).offset(0, -3);
	const char* exit_text = "Exit\r\n";
    srect16 exit_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), exit_text).bounds().center(play_again_text_rect).offset(0, 12);
	
	draw::filled_ellipse(lcd, rect16(point16(10, 10), lcd.dimensions().inflate(-20, -20)), color<pixel_type>::red);	
	//drawJPEG("/a.jpeg", point16(0, 0));

	draw::filled_rectangle(lcd, rect16(point16(45, 46),size16(70, 36)), color<pixel_type>::black);
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
		case 0: {
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

void Main::drawJPEG(const char* path, point16 destination)
{
	_drawJPEG_destination = destination;
	file_stream fs(path);
	if (!fs.caps().read)
	{
		ESP_LOGE(TAG_FS, "Failed to open a.jpeg");
	}
	else
	{			
		jpeg_image::load(&fs, 
			[](size16 dimensions, jpeg_image::region_type& region, point16 location, void* state) 
			{

				_drawJPEG_destination.x += location.x;
				_drawJPEG_destination.y += location.y;
				return draw::bitmap(bcd_sys.getDisplay(), rect16(_drawJPEG_destination, region.dimensions()), region, region.bounds());
			}, 
			nullptr);
	}
}