The BalcCon Cyberdeck 0o27 (BCD-0o27) comes with a development framework that makes it easier to add functionality to the cyberdeck. This development guide introduces the framework, advises coding best practices and provides examples for adding functionality to the BCD.
Of course, you can ignore all of that and program the BCD either with esp-idf, rust, or any other framework that allows you to program ESP32 chips directly. However, this is not covered in this guideline.

# Introduction to the BCD-0o27 Framework
Before reading this chapter it is advisable to get  a local copy of the framework (see [[#Getting the Framework]]) and set up the development environment (see [[#Setting up the Development Toolchain]]). 
The framework consists of the following components:
- **Drivers**: The drivers for the hardware, namely the ST7735 display, the WS2812 LEDs  and the input buttons (aka controller) as well as the serial console. (Sometimes, drivers and libraries are not distinguished)
- **Libraries / Components**: Libraries and components are provided for common tasks like opening a console on the serial port, establishing ssl connections or displaying graphical elements.
- **Modules**: Modules are programs that can be launched by the user. They can be simplest demo applications or complex programs.
- **Console Commands**: Console commands are programs that can be launched from the console. As modules, console commands can have any level of complexity. While usually they mainly interact with the serial console, they may - just as modules - access other hardware as well.

We distinguish two kinds of development work: Firmware development and framework development. Firmware development is using the framework to develop your own firmware to run on the hardware. Framework development modifying the framework itself to improve its performance or utility. Of course, the separation is not strict, as developing custom firmware might require modifications at the framework. For the sake of this guide we will differ between the two as clean cut as possible.

This document is currently being developed. For the time being, it is recommended to check the appendix to clone the repository. Then use the example firmware provided in the directory.

> [!info]
> You can connect to the badge using a serial console and a usb-c data cable. Make sure you use a terminal that understands ansi sequences. You can use the integrated serial monitor of platformio or espressif, but its highly recommended to use syncterm or minicom.


> [!warning]
> There currently is a bug in the serial console that will not give you a prompt when you connect using a serial terminal. In that case, leave the connection open and simply reset the badge by pushing the reset button - left button under the display.

# Firmware Development
This chapter will explain the code structure for developing firmware. Driver and library interfaces / usage is described in chapter . To develop firmware using the provided framework, you will likely add custom modules and / or console commands. You can either start out with a clean slate by cloning the pure framework or you can start modifying the example firmware. Both can be found in the [gitlab repository](https://gitlab.com/fschuetz/bcd-0o27) Details on how to set up the repository and development environment can be found in the [[#Appendix]].
If you are eager to get your hands dirty, then feel free to jump right into the example subsection of the according section about modules or commands. The example section will be an easy to follow mini tutorial. Just make sure to revisit the whole section, as the example will not cover everything and not explain why things are done.
## Modules
Modules are "self contained" pieces of code that are usually started through the menu (or execute on their own without user interaction). Modules should be structured as follows:
```
mod_<name of module>/
       |
       |--> include/
       |        |
       |        |--> mod_<name of module>.hpp
       |        |--> ...
       |
       |--> src/
       |     |
       |     |--> mod_<name of module>.cpp
       |     |--> ...
       |
       |--> CMakeLists.txt
       |--> KConfig
```
Modules must be stored in the `/modules` directory of the framework. They usually reside in their own git repository and are added to the `/modules` directory as a submodule. 
You could clone the framework from the original repository and then just add a directory for your module (and would then not need the CMakeLists.txt and the KConfig). However, this makes it hard to share your module with others. 
### Include Directory
The include directory must at least contain the header file for your module. It should be named the same as you top level directory with the file extension .hpp (as you are likely writing a c++ module. If not, you can do it, but you there are certain things to respect on how to use a c module within.). If your module needs other header files, this is also the right place to put them if they are needed outside your module. If not you can put them wherever you want.

### CMakeLists.txt
The `CMakeLists.txt` file is important for compilation. It will enable your module to be found. The file template looks as follows:
```
idf_component_register( SRC_DIRS     "./src"
						INCLUDE_DIRS "./include")
```
This will tell the build system to register your component (under the name of the top level directory) and to find the include files in the `include` directory and the source files in the `src` directory. 
If your module depends on other modules, then you need to make this explicit in the `CMakeLists.txt` file. For example, if your module uses GFX (which requires the module gfx for the library and ch405labs_gfx_drivers for the graphics driver) to draw graphics on the screen, you need to modify the `CMakeLists.txt` as follows:
```
idf_component_register( SRC_DIRS "./src"
						INCLUDE_DIRS "./include"
						REQUIRES ch405labs_gfx_drivers gfx)
```
### Using Framework Components (Drivers / Libraries)
The framework already provides you some special modules (called components) that make it easier for you to develop your own things. See chapter [[#Drivers and Libraries]] on how to include them into your project.
### Example
In this chapter we will write a little example module from scratch to demonstrate the workflow. All paths used in this example are relative to the home directory. Adapt them to whatever structures you choose. The example as it is presented can be directly applied on a linux or mac computer using a shell of your choice. On windows, some adjustments of eg. path names may be needed.
Further, this chapter assumes that you have set up the development toolchain properly. If you haven't done so, consult If you have not yet set up Visual Studio Code see [[#Setting up the Development Toolchain]] to set up your development environment first!

#### Preparation
First, we clone the framework (if you haven't done so already). Make sure to also clone the submodules:
```bash
git clone --recurse-submodules https://gitlab.com/fschuetz/bcd-0o27.git
```
Now it is time to set up an empty git repository of your choice ([github](github.com), [gitlab](gitlab.com) or any other you like). Choose `mod_example` as the name for your module. You can initialise the module with a README.md if you want.
When done, clone it as a submodule into the `/modules` directory of the framework. Make sure to use the path to your git repository and make sure to clone it such that you can push your changes back to the repository.
```bash
cd bcd-0o27/firmware/framework/modules/
git submodule add -b main git@gitlab.com:BCD_0o27/modules/mod_example.git
```
To write a new module, it is easiest to copy the template module from the repository. The framework has the template module already present in the modules directory. Lets copy the relevant files into our own module and rename the `.cpp` and the `.hpp` file.
```bash
cp mod_template/CMakeLists.txt mod_example
cp mod_template/Kconfig mod_example
cp mod_template/README.md mod_example
cp mod_template/LICENSE mod_example
cp -r mod_template/include mod_example
cp -r mod_template/src mod_example
mv mod_example/include/mod_template.hpp mod_example/include/mod_example.hpp
mv mod_example/src/mod_template.cpp mod_example/src/mod_example.cpp
```
We now need to adapt the template to fit our module name. For this, open a new window (`File -> New Winow`)in Visual Studio Code. Then open the directory `bcd-0o27/firmware/framework/`. Allow some time for platformio to properly initialise. 
Check that you can compile the framework by either to the platformio menu (the alien head in your left menu bar) and open the `BCD-0o27-5KY` folder (it may take a while to load). There choose `Build`from the `General` subfolder. If all went well, do the same for `BCD-0o27-5KY-release`.
Next we need to adapt some defines, variables and other elements to reflect the name of our module. We start with the `Kconfig` file. In Visual Studio Code open the `Kconfig` file in the folder `modules/Kconfig`. Then find and replace all `MOD_TEMPLATE` with `MOD_EXAMPLE`.  Also replace the name `Template`in the statement `menu "Console Command: Template"` with `Example`. Alternatively you can issue the following command from the command line in the `modules/mod_example` directory:
```bash
sed -i '' -e 's/MOD_TEMPLATE/MOD_EXAMPLE/g' Kconfig
sed -i '' -e 's/Template/Example/g' Kconfig
```
Feel free to adapt or remove the comments in the file (marked by `#` as the first character or the line).
Your Kconfig should now look something like this:
```Kconfig
menu "Module: Example"
	config TAG_MOD_EXAMPLE
	string "Tag for logging"
	default "MOD_EXAMPLE"
	help
		The tag to use for log messages.
	
	choice MOD_EXAMPLE_LOG_LEVEL
            bool "Log level for Example module"
            default MOD_EXAMPLE_LOG_EQUALS_DEFAULT

            config MOD_EXAMPLE_LOG_EQUALS_DEFAULT
                bool "Same as default"
            config MOD_EXAMPLE_LOG_LEVEL_NONE
                bool "No output"
            config MOD_EXAMPLE_LOG_LEVEL_ERROR
                bool "Error"
                depends on LOG_MAXIMUM_LEVEL >= 1
            config MOD_EXAMPLE_LOG_LEVEL_WARN
                bool "Warning"
                depends on LOG_MAXIMUM_LEVEL >= 2
            config MOD_EXAMPLE_LOG_LEVEL_INFO
                bool "Info"
                depends on LOG_MAXIMUM_LEVEL >= 3
		config MOD_EXAMPLE_LOG_LEVEL_DEBUG
                bool "Debug"
                depends on LOG_MAXIMUM_LEVEL >= 4
            config MOD_EXAMPLE_LOG_LEVEL_VERBOSE
                bool "Verbose"
                depends on LOG_MAXIMUM_LEVEL = 5
        endchoice

        config MOD_EXAMPLE_LOG_LEVEL
            int
            default LOG_DEFAULT_LEVEL if MOD_EXAMPLE_LOG_EQUALS_DEFAULT
            default 0 if MOD_EXAMPLE_LOG_LEVEL_NONE
            default 1 if MOD_EXAMPLE_LOG_LEVEL_ERROR
            default 2 if MOD_EXAMPLE_LOG_LEVEL_WARN
            default 3 if MOD_EXAMPLE_LOG_LEVEL_INFO
            default 4 if MOD_EXAMPLE_LOG_LEVEL_DEBUG
            default 5 if MOD_EXAMPLE_LOG_LEVEL_VERBOSE
    endmenu
```
Now replace all the `MOD_TEMPLATE` and `mod_template` in `include/mod_example.hpp`. You can do this in the editor or from the command line. Make sure to keep the case, it matters:
```bash
sed -i '' -e 's/mod_template/mod_example/g' include/mod_example.hpp
sed -i '' -e 's/MOD_TEMPLATE/MOD_EXAMPLE/g' include/mod_example.hpp
```
Now we are going to edit the mod_example include file to prepare it for writing our program. Open up the file in Visual Studio Code and do the following:
1. Adjust the information about your module and you the author in the first comment block
   - @author: Your name an email where ppl can reach you for questions.
   - @brief: A brief description of your module, eg. `Example file for BalcCon Cyberdeck modules.` 
   - @version: 0.1
   - @date: The current date
   - @copyright: `Copyright (c) <year>, <your name>, released under MIT license` (or choose another license that suits your purpose better).
   - Replace the longer paragraph describing the module with a suitable description, eg: `This module display a hello world text on the display and blinks the led's. The module will terminate, whenever the user presses a button.` 
2. You can either choose to keep both, `#pragma once`and the include guard `#ifndef BCD_MOD_EXAMPLE_HPP ...` or you can choose one of them. If you are unsure, just leave them untouched and use google to see why this is bad or why it is good and why one is better than the other - or not. Depending on the community.
3. We do currently not need to change our includes, as we intend to use all of those. Of course, if for example you future module does not need the controller, you can remove the controller include (and adapt the CMakeLists.txt to remove this dependency).
4. For the sake of trying to keep coding style consistent, make sure the comments after `#define MOD_EXAMPLE_FAIL ...` and `#define MOD_EXAMPLE_OK ...` start at character 81 (you need to move them one character back by adding a space in front).
5. In the comment of the Main function, adapt the description. Eg. to `This is the entry point into our module. It will run a loop that blinks the leds and aborts if a button is pressed long enough`.
This is all the adaption we need to do at the current stage. Your `mod_example.hpp` should now look similar to this:
```cpp
/**
 * @file mod_example.hpp
 * @author Florian Schütz (fschuetz@ieee.org)
 * @brief Example file for BalcCon Cyberdeck modules.
 * @version 1.0
 * @date 12.09.2023
 * @copyright Copyright (c) 2022, Florian Schuetz, released under MIT license
 *
 * `This module display a hello world text on the display and blinks the led's.
 * The module will terminate, whenever the user presses a button.
 */ 

#pragma once
#ifndef BCD_MOD_EXAMPLE_HPP
#define BCD_MOD_EXAMPLE_HPP

#include <FreeRTOS/portmacro.h>
#include <stdlib.h>
#include "esp_log.h"
#include "st7735_bcd.hpp"
#include "gfx.hpp"
#include "ch405labs_esp_led.hpp"
#include "ch405labs_esp_debug.h"
#include "ch405labs_esp_wifi.hpp"
#include "ch405labs_esp_controller.hpp"

////////////////////////////////////////////////////////////////////////////////
// Menuconfig options
////////////////////////////////////////////////////////////////////////////////
#define TAG_MOD_EXAMPLE CONFIG_TAG_MOD_EXAMPLE

#if CONFIG_MOD_EXAMPLE_LOG_LEVEL == 0
#define MOD_EXAMPLE_LOG_LEVEL esp_log_level_t::ESP_LOG_NONE
#elif CONFIG_MOD_EXAMPLE_LOG_LEVEL == 1
#define MOD_EXAMPLE_LOG_LEVEL esp_log_level_t::ESP_LOG_ERROR
#elif CONFIG_MOD_EXAMPLE_LOG_LEVEL == 2
#define MOD_EXAMPLE_LOG_LEVEL esp_log_level_t::ESP_LOG_WARN
#elif CONFIG_MOD_EXAMPLE_LOG_LEVEL == 3
#define MOD_EXAMPLE_LOG_LEVEL esp_log_level_t::ESP_LOG_INFO
#elif CONFIG_MOD_EXAMPLE_LOG_LEVEL == 4
#define MOD_EXAMPLE_LOG_LEVEL esp_log_level_t::ESP_LOG_DEBUG
#elif CONFIG_MOD_EXAMPLE_LOG_LEVEL == 5
#define MOD_EXAMPLE_LOG_LEVEL esp_log_level_t::ESP_LOG_VERBOSE
#endif //CONFIG_MOD_EXAMPLE_LOG_LEVEL

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////
typedef BaseType_t mod_example_err_t; 

#define MOD_EXAMPLE_FAIL -1                              /**< Generic failure */
#define MOD_EXAMPLE_OK 0x000                             /**< All good */

////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////
// Put any macro here.

////////////////////////////////////////////////////////////////////////////////
// Namespaces
////////////////////////////////////////////////////////////////////////////////
// If you "use" namespaces, put them here.
using namespace espidf;
using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// Various Types
////////////////////////////////////////////////////////////////////////////////
// Put type definitions that don't fit above here.

// Any module should reside in its own namespace using the scheme
// bcd_mod_<name of your module>. Please adapt to the name of your module.
namespace bcd_mod_example {

////////////////////////////////////////////////////////////////////////////
// Main function
////////////////////////////////////////////////////////////////////////////
// This is the entry point into our module. It will run a loop that blinks
// the leds and aborts if a button is pressed long enough`.

template<typename Destination>

int module_main(void *param) {

#ifdef CONFIG_DEBUG_STACK
	UBaseType_t uxHighWaterMark;
	/* Inspect our own high water mark on entering the task. */
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		ESP_LOGD(CONFIG_TAG_STACK, "mod_example::module_main(): High watermark "
		"for stack at start is: %d",
		uxHighWaterMark);
#endif

#ifdef CONFIG_DEBUG_HEAP
	multi_heap_info_t heap_info;
	heap_caps_get_info(&heap_info, MALLOC_CAP_DEFAULT);
	ESP_LOGD(CONFIG_TAG_HEAP, "mod_example::module_main(): Heap state at "
								"start: \n"
								" Free blocks: %d\n"
								" Allocated blocks: %d\n"
								" Total blocks: %d\n"
								" Largest free block: %d\n"
								" Total free bystes: %d\n"
								" Total allocated bytes: %d\n"
								" Minimum free bytes: %d\n"
								, heap_info.free_blocks
								, heap_info.allocated_blocks
								, heap_info.total_blocks
								, heap_info.largest_free_block
								, heap_info.total_free_bytes
								, heap_info.total_allocated_bytes
								, heap_info.minimum_free_bytes);
#endif // CONFIG_DEBUG_HEAP


// <--- PLACE CODE BELOW HERE -->


// <--- PLACE CODE ABOVE HERE -->

#ifdef CONFIG_DEBUG_STACK
	/* Inspect our own high water mark before exiting the task. */
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	ESP_LOGD(CONFIG_TAG_STACK, "mod_example::module_main(): High watermark"
		" for stack at start "
		"is: %d", uxHighWaterMark);
#endif

return 0;

}
} // namespace bcd_mod_example
#endif // BCD_MOD_EXAMPLE_HPP
```
Next we prepare the `src/mod_example.cpp` file. Replace all the `mod_template`stings by `mod_example`. As done before, you can either do this in Visual Studio Code or on the command line:
```bash
sed -i '' -e 's/mod_template/mod_example/g' src/mod_example.cpp
```
Then copy the first comment block from you `include/mod_example.hpp` and replace the first comment block in `src/mod_example.cpp`. Your file should now look something like this:
```cpp
/**
 * @file mod_example.hpp
 * @author Florian Schütz (fschuetz@ieee.org)
 * @brief Example file for BalcCon Cyberdeck modules.
 * @version 1.0
 * @date 12.09.2023
 * @copyright Copyright (c) 2022, Florian Schuetz, released under MIT license
 *
 * This module display a hello world text on the display and blinks the led's.
 * The module will terminate, whenever the user presses a button.
 */

#include "../include/mod_example.hpp"
```
#### Adding the Module to the Framework
While our module does not yet do anything, this is a good time to add it to the framework and check if everything compiles as intended. For our case, our module will be added such that it shows up in the main menu and can be executed by selecting it. 
First, we need to add our module header file in the includes section of the framework. For this, open the file `src/main.hpp` from the root directory of your framework. Find the line `// <----------------------------- Modules ------------------------------------->`and add the following after the last `#include` (but before the includes of the commands).
```cpp
#include "mod_example.hpp"
```
While you are at it, you can also remove the `#include "mod_template.hpp"` as we will not need it, now that we are developing a proper module.
>[!WARNING]
>Do not add your namespace to the namespaces or you module_main() will clash and lead to compilation errors.

Next open the `src/main.cpp`file. Search for the line that says `// <--- Set up objects etc... used throughout firmware below -->`. Here we will initialise our menu object. To do this add the following code:
```cpp
gfxmenu::MenuController<lcd_type, lcd_type::pixel_type> mc;
```

Now look for the line `// <--- Register modules below -->`. Below this line add the following code:
```cpp
mc.cursor->addEntry(mc.createActionItem("Hello BalcCon", bcd_mod_example::module_main<lcd_type>, &lcd));
```
This registers and action item as the first menu entry. An action item is an item that is executable. In our case, the execution starts in the `module_main(...)` function of our module. For more information about how to build menus consult the chapter [[#GFXMenu]].
Las but not least, we need to display our menu and then run a main loop. For this to happen we first add the code to display the menu in the setup section. Look for the line `// <--- Put setup code and one time acitons below -->` and add the following code below:
```cpp
// Reset cursor and select first entry
mc.cursor->setToRoot();
mc.cursor->first();
mc.cursor->selectEntry();
lcd.clear(lcd.bounds());
mc.cursor->drawMenu(lcd, srect16(lcd.bounds()), lcd_color::alice_blue,
	lcd_color::dark_goldenrod);
```
As menus are always drawn from the element the cursor points at, we must set the cursor to the first element of the root menu (aka main menu). This is done with the first two statement. Then, we want the first element to be pre-selected (third statement). Next we clear the screen and then draw the menu.
Now its time to implement the main loop. In our case, the loop simply checks if button A or button B are pressed. If they are, the currently selected menu item is executed. Pressing the buttons up or down will select the next of previous menu item. In our case, as there is only one menu item, this will not do anything but it shows you how to do it when you add more modules. Look for the line that states `// <--- Implement your main loop below -->` and add the following code:
```cpp
for(;;) {
	// Query button and execute menuitem if button A or B is pressed.
	controller.capture();
	
	if(controller.getButtonState(BUTTON_DOWN)) {
		mc.cursor->deselectEntry();
		mc.cursor->next();
		mc.cursor->selectEntry();
		mc.cursor->drawMenu(lcd, srect16(lcd.bounds()),
		lcd_color::alice_blue, lcd_color::dark_goldenrod);
	} else if(controller.getButtonState(BUTTON_UP)) {
		mc.cursor->deselectEntry();
		mc.cursor->previous();
		mc.cursor->selectEntry();
		mc.cursor->drawMenu(lcd, srect16(lcd.bounds()),
		lcd_color::alice_blue, lcd_color::dark_goldenrod);
	}
	 
	if(controller.getButtonState(BUTTON_A)
		|| controller.getButtonState(BUTTON_B)) {
		
		const gfxmenu::Entry<lcd_type, lcd_type::pixel_type> *e =
		mc.cursor->getEntry();
		if(e != NULL && typeid(*e) == typeid(gfxmenu::ActionItem<lcd_type, lcd_type::pixel_type>)) {
			int return_code = ((gfxmenu::ActionItem<lcd_type, lcd_type::pixel_type> *)e)->execute();
			if(return_code < 0) {
				ESP_LOGE(TAG_STATE, "Could not execute menut item. Code %d", return_code);
			} else if (return_code > 0) {
				ESP_LOGW(TAG_STATE,"Menu execution returned with code %d.", return_code);
			};
			// Need to draw menu again, as execution could have used screen
			mc.cursor->drawMenu(lcd, srect16(lcd.bounds()),
				lcd_color::alice_blue, lcd_color::dark_goldenrod);
		}
	}
	// Delay to allow user to release buttons
	vTaskDelay(pdMS_TO_TICKS(250));
}
```
In our infinite loop, we first capture the pressed keys. Capturing keys records the state of the keys at the very moment this function is called. This is the reason why the last command in the loop is the `vTaskDelay()` command which puts the main task to sleep for 250 milliseconds. The sleep allows the user to release buttons before reading them again. If this command would not be here, the loop would record a pressed button many times and for example scroll through commands very fast. Make sure to read the chapter [[#Controller Driver]] for more information.
After reading the button presses from the controller, we check if the down button was pressed. If the down button was pressed, we deselect the current menu item, move the cursor one menu item down and select it. We then need to redraw the menu to reflect the change on the display. Note that this does not have any effect in our current state, as we just have one menu item.
If the down button was not pressed, we check if the up button was pressed. If this is the case, we deselect the current entry, move the cursor one item up an in the menu and select it and then redraw the menu. If neither up nor down were pressed, we finally check if the A or B button was pressed. If this is the case, we check if the active menu item is an action item. This is done by checking the object type of the menu item. If the item is an action item, we execute it and check the return code. If the return code is 0, all is good. If it is smaller than 0, execution failed and we display an error to the console. If the return code is larger than 0 we display a warning that the module returned with a non zero code. After executing, we also need to redraw the menu, as the module may have changed what is displayed on the screen. Make sure to read the chapter [[#GFXMenu]] for more information on how to build an navigate menus.
The code above successfully registers our module in a menu that is displayed on the cyberdeck and can be executed. Your can test this by compiling the module and flashing it to your cyberdeck. To do this, select Platformio in the left menu bar (the alien head). Then in the `Project Tasks` open the folder `BCD-0o27-5KY`. Wait for the project to self configure and then choose `General->Build`. If everything compiles fine, attach you cyberdeck to your computer using a USB-C cable and choose `General->Upload`. This flashes the firmware to your cyberdeck. Before doing this, make sure you do not have any serial console open that is connected to your cyberdeck. Otherwise flashing will fail.  
When you move up and down in the menu or execute the module you will see a short flicker, as neither the module does anything yet nor are there other menu items to scroll through.
If all of this is working, move on to the next chapter, where we finally implement the functionality of our module.
#### Implementing the Module
TODO
#### Implementing the Code
>[!INFO] Relative line numbers
>For the rest of this tutorial we are going to reference to line numbers relative to anchor points, because if you used longer descriptions in the preparation chapter, your line numbers culd differ. To enable relative line numbers in Visual Studio Code you can go to `Settings->Settings`. Search for "line numbers" and choose `relative`.
>Alternatively you can just calculate them in your head.



> [!NOTE] Using submodules in your module
> A note about using submodules in you module: In case your submodule has other submodules, you also need to intitialise those using the `git submodule update --init` command in the directory of your module.

## Console Commands


## Core Configuration
### Main Stack Size
TODO - explain stack of main task
The main task stack size depends heavily on how many modules and commands you use in your custom firmware and how much memory in the main task they require (some modules and commands may spawn own threads with their own stack). If you prepare a firmware, make sure to set the `CONFIG_ESP_MAIN_TASK_STACK_SIZE` in `menuconfig` accordingly. Also, it is recommended to add the appropriate stack size to the `sdkconfig.defaults`. 

> [!warning]
> Setting the value in `sdkconfig.defaults` does not overwrite existing configurations. It will rather take effect, if there is no existing configuration. So if you make that change during development, you need to set your `menuconfig` option anyways.
### Partition Table
Set the partition table in menuconfig. Set `custom_partition_table` to `partiontions.csv`. Set `Offset of partition table` to `0x8000`. 

> [!info]
> When using verbose logging or higher for the bootloader, it might be necessary to move the partition table to 0x10000 to make enough space for the bootloader.

## Preparing Firmware for Flashing / Delivery
Once firmware is final, potentially many devices need to be flashed. It is cumbersome to do this through the development environment. It is recommended to prepare the firmware as a binary package ready for flashing. To do this with platformio, first create a release profile if you haven't done so yet:
```

```
Next build the firmware in the development environment, just to make sure everything is fine. After all is set up correctly in the platformio core cli the following command will allow us to identify the needed parameters to build the project:
```bash
pio run -v -t upload
``` 
Depending on your setup, you might need to get the idf framework first by issuing `get_idf`. Also make sure you have a cyberdeck attached as the flash command should be able to execute.

The project is now being built giving verbose output. Towards the end, there will be the flash command with all parameters. The output may look something like this:
```bash
"/Users/fschuetz/.platformio/penv/bin/python" "/Users/fschuetz/.platformio/packages/tool-esptoolpy/esptool.py" --chip esp32s3 --port "/dev/cu.usbserial-1430" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 /Users/fschuetz/Development/balccon_badge_workbench/checked_out_branches/firmware-design/balccon-badge/firmware/BCD-0o26_framework/.pio/build/BCD-0o26-5KY/bootloader.bin 0x8000 /Users/fschuetz/Development/balccon_badge_workbench/checked_out_branches/firmware-design/balccon-badge/firmware/BCD-0o26_framework/.pio/build/BCD-0o26-5KY/partitions.bin 0x10000 .pio/build/BCD-0o26-5KY/firmware.bin
```

### Some things to consider
TODO - Full clean wipes menuconfig. Add rtti and adapt main task stack size.
# Drivers and Libraries
# Framework Development
## Drivers
TODO
### Controller Driver
TODO
## Libraries 
TODO
### GFXMenu
TODO
## Components
TODO

# Graphics
## Displaying images
### Converting images into header files
```
https://github.com/jimmywong2003/PNG-to-RGB565
```



# Appendix
## Getting the Framework
The framework is hosted as a git repository at https://gitlab.com/fschuetz/bcd-0o27.git. The repository makes heavy use of submodules, as many components for the BCD-0o27 are developed in parallel.
To create a local repository using git follow these steps:
```bash
git clone --recurse-submodules https://gitlab.com/fschuetz/bcd-0o27
```

## Setting up the Development Toolchain
In this chapter we will show how to set up the development environment consisting of git, espidf, Microsoft Visual Studio Code and Plattformio. If you are an experienced user, feel free to use the IDE of your choice.
### Installing GIT and cloning the repository
We will use git from the command line, as we have to deal with submodules, which are not well supported in the Visual Studio Code IDE.
If you have git already installed, you can skip this step.
1. Install git for your platform. Follow the guide at [Gitlab](https://docs.gitlab.com/ee/topics/git/how_to_install_git/)
2. Clone the repository: 
    ```bash
    git clone --recurse-submodules https://gitlab.com/fschuetz/bcd-0o27
    ```
Some tips on working with submodules, in case you run into a head detached situation: https://stackoverflow.com/questions/18770545/why-is-my-git-submodule-head-detached-from-master

### Install Microsoft Visual Studio Code (VSCode)
1. Install [VSCode](https://code.visualstudio.com/) for your operating system.
2. Open VSCode.

### Install C/C++ and PlatformIO Extensions
1. In VSCode: Open the "Extensions" side bar on the left side.
2. Search for the "C/C++" extension by Mircrosoft and install it.
3. Search for the PlatformIO IDE by PlatformIO and install it. See [PlatformIO IDE for VSCode](https://docs.platformio.org/en/latest/integration/ide/vscode.html#ide-vscode)
4. Once installation has finished, restart VSCode

### Build and Upload the Firmware 
In this chapter we will build the filesystem image and the firmware and upload both to the badge. If this works, then you can start developing your own firmware using the framework. For testing we highly recommend, taking the example in the directory `firmware/example` rather than the framework, as the example will give you running firmware that allows you to see something actually happening.
1. Start Microsoft Visual Studio and make sure you are in a new window, with no project loaded.
2. Select the alien head in the left bar (Platformio menu)
3. A left band appears. Select the blue button "Pick a folder"
4. From the cloned repository choose the `firmware/example` or the `firmware/framework` folder (as written above, its recommended to start with the example rather than the plain framework).
5. Click open.
6. Wait for platformio to configure itself. This is IMPORTANT: Between now and building the file there might raise popups in the bottom left corner asking you to install extensions or configurations. Usually you are asked to intall C++ specific extensions and CMake specific extensions. Confirm all of them. For CMake, if asked, you can "scan for kits". Scan for kits may also go away on itself, then everything is fine as well. 
7. If all went well so far you will recognise a blue ribbon / button on the bottom and some symbols like a home, a checkmark, roght pointing arrow etc.. If there is the message "Platformio: Loading tasks..." wait until it disapears. (this may take a long while). PlatformIO will automatically configure the project.
8. Open the file main.cpp in the "src" folder.
9. In the bottom task list (with the blue ribbon / button and the checkmark etc...), first choose the build environment by clicking on the symbol that looks like a folder and says "Switch Platformio Project Environment" if you hover over it. Usually it reads default right to the symbol. Choose `env:BCD-0o27-5KY` (or the `env:BCD-0o27-5KY-release` if you want to build for release rather than debug - if unsure make a debug build).
10. Now select the checkmark to build the project. This builds the project. Dependencies are pulled in automatically. In case Platformio does not pull in the Espressif 32 framework, follow the steps in the note below to install it manually.
11. If the project builds successfully, connect your badge to the computer using a usb-c cable (make sure it is actually a data cable and not just a charging cable)
12. Make sure the device is recognised. If the device is not recognised and a com port assigned, you might have to install a driver for the CH340 serial bridge. Follow the instructions from [Sparkfun](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all#drivers-if-you-need-them)
13. Once the device is recognised, in VSCode we will build the file system image, upload it and then upload the code.
14. From the side bar choose the PlatformIO icon (alien head)
15. Select BCD-0o27 -> Plattform -> Build Filesystem Image
16. When finished, select BCD-0o27 -> Platform -> Upload Filesystem Image (BCD-0o27)
17. When finished, select the right pointing arrow ("Platformio: Upload") in the blue ribbon at the bottom. The project will be built and uploaded to the badge.
18. Either launch the serial console from the blue ribbon by selecting the plug icon ("Platformio: Serial Monitor") or use minicom (preferred). Note, that the Platformio Serial Monitor may display some strange characters or double print the prompt as it does not fully support the output mode. IMPORTANT: Due to a bug, the serial console only works for input if the connection was made at badge startup. If you do not see a prompt, leave the console on and restart the badge by pushing the power button (left between display and battery holder).
19. CRITICAL: Make sure, that you DO NOT have a serial monitor running when trying to flash the badge. This will lead to flash failure (for both, filesystem and firmware).

Note: The Espressif 32 framework should install automatically when building the project. If this is not the case, you can install it manually:
1. Open the Platformio sidebar (left side, alien head icon).
2. Select PIO Home -> Platforms
3. Click on the "Embedded" tab. Search for "Espressif 32" and click it.
4. Install the latest version. (At the time of writing this is version 5.1.0. While we plan to adapt the framework to newer versions, you might want to select this version if the newest version is not working due to breaking changes.)
5. Once the installation has completed confirm the success dialog.
### Helpful Resources
[Espressif 32 for Platformio](https://docs.platformio.org/en/latest/platforms/espressif32.html)

## Misc notes - ignore please
### Upload firmware directly
```bash
esptool.py --chip esp32s3 --port "/dev/cu.usbserial-1430" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 /Users/fschuetz/Development/balccon_badge_workbench/checked_out_branches/firmware-design/balccon-badge/firmware/BCD-0o26_framework/.pio/build/BCD-0o26-5KY/bootloader.bin 0x8000 /Users/fschuetz/Development/balccon_badge_workbench/checked_out_branches/firmware-design/balccon-badge/firmware/BCD-0o26_framework/.pio/build/BCD-0o26-5KY/partitions.bin 0x10000 .pio/build/BCD-0o26-5KY/firmware.bin
```


### VSCode - Git limits
Might want to change the default 10 to a higher number
```
Git: Detect Submodules Limit

Controls the limit of git submodules detected.
```


### Updating mirrors 
Go to lib directory of mirror
```
git fetch upstream 
git merge upstream/master
git push
```

### Updating submodules
```
git fetch
git pull
```

# Coding Style
```c++
ESP_LOGE(TAG_MODULE_NAME, "This is a rather long error message."
		"It should be spred on two lines");
```
If functions too long, spread on multiple lines. Two lines, indent second line by one tab and finish the line with closing bracket and ;. If more than two lines use one argument per line and close on a separate line as follows:
```c++
myfunction_with_many_arguments(
	argument 1,
	argument 2,
	argument 3

```

TO BE CONTINUED