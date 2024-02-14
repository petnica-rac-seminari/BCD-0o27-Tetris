#include "main.hpp"

Main::GameState Main::runStartScreen()
{
	GameState exitState = GameState::Running;		

	const char* start_text = "Start\r\n";
    srect16 start_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), start_text).bounds().center((srect16)lcd.bounds());
	const char* exit_text = "Exit\r\n";
    srect16 exit_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), exit_text).bounds().center(start_text_rect).offset(0, start_text_rect.height());
	
	int selectedButton = 0;
	auto renderScene = [&]() {
		draw::filled_ellipse(lcd, rect16(point16(10, 10), lcd.dimensions().inflate(-20, -20)), color<pixel_type>::red);	

		drawJPEG("/a.jpeg", point16(0, 0));

    	draw::text(lcd, start_text_rect, start_text, textFont, color<pixel_type>::white);
    	draw::text(lcd, exit_text_rect, exit_text, textFont, color<pixel_type>::white);

		switch (selectedButton)
		{
		case 0: {
			point16 pos = point16(start_text_rect.x1, (start_text_rect.y1 + start_text_rect.y2) / 2).offset(-20, -5);
			draw::filled_ellipse(lcd, rect16(pos, size16(6, 6)), color<pixel_type>::white);
			break;
		}
		case 1:
			point16 pos = point16(exit_text_rect.x1, (exit_text_rect.y1 + exit_text_rect.y2) / 2).offset(-20, -5);
			draw::filled_ellipse(lcd, rect16(pos, size16(6, 6)), color<pixel_type>::white);
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
				--selectedButton;

			renderScene();
		}
		if (controller.getButtonState(BUTTON_DOWN))
		{
			if (selectedButton < 1)
				++selectedButton;

			renderScene();
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
	while (true)
	{
		board.frame();
	}

	return GameState::Start;
}

Main::GameState Main::runEndScreen()
{
	return GameState::Start;
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