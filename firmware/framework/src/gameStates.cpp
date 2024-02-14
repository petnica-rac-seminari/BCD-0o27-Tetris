#include "main.hpp"

Main::GameState Main::runStartScreen()
{
	GameState exitState = GameState::Running;	

	draw::filled_ellipse(lcd, rect16(point16(10, 10), lcd.dimensions().inflate(-20, -20)), color<pixel_type>::red);	

	drawJPEG("/a.jpeg", point16(0, 0));	

	const char* start_text = "Start\r\n";
    srect16 start_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), start_text).bounds().center((srect16)lcd.bounds());
	const char* exit_text = "Exit\r\n";
    srect16 exit_text_rect = textFont.measure_text((ssize16)lcd.dimensions(), exit_text).bounds().center(start_text_rect).inflate(0, -start_text_rect.height());

    draw::text(lcd, start_text_rect, start_text, textFont, color<pixel_type>::white);
    draw::text(lcd, exit_text_rect, exit_text, textFont, color<pixel_type>::white);

	ledPattern lp1, lp2;

	{
		ledStates ledStateR, ledStateG, ledStateBlack;

		for(int i = 0; i < LED_IF_NUM_LED; i++) 
		{
			ledStateBlack.led[i] = { .red = 0x00, .green = 0x00, .blue = 0x00 };
			ledStateR.led[i] = { .red = 0x11, .green = 0x00, .blue = 0x00 };
			ledStateG.led[i] = { .red = 0x00, .green = 0x11, .blue = 0x00 };
		}

		LEDPatternGenerator generator;

		generator.setInterruptable(false);
		generator.setRepetitions(1);
		generator.addState(ledStateR, pdMS_TO_TICKS(100));
		generator.addState(ledStateBlack, pdMS_TO_TICKS(100));
		generator.addState(ledStateR, pdMS_TO_TICKS(100));
		generator.addState(ledStateBlack, pdMS_TO_TICKS(100));
		generator.generate(&lp1);

		generator.setInterruptable(false);
		generator.setRepetitions(1);
		generator.addState(ledStateG, pdMS_TO_TICKS(100));
		generator.addState(ledStateBlack, pdMS_TO_TICKS(100));
		generator.addState(ledStateG, pdMS_TO_TICKS(100));
		generator.addState(ledStateBlack, pdMS_TO_TICKS(100));
		generator.generate(&lp2);
	}

	led.reset();
	
	while (true)
	{
		controller.capture();

		if (controller.getButtonState(BUTTON_A))
		{
			led.patternSchedule(lp1);
			led.patternStart();
			vTaskDelay(pdMS_TO_TICKS(400));

			exitState = Main::GameState::Running;
			break;
		}
		if (controller.getButtonState(BUTTON_X))
		{
			led.patternSchedule(lp2);
			led.patternStart();	
			vTaskDelay(pdMS_TO_TICKS(400));
			
			exitState = Main::GameState::Running;
			break;
		}
	}

	lcd.clear(lcd.bounds());

	return exitState;
}

Main::GameState Main::runGameScreen()
{
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