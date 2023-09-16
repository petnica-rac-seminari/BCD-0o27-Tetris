Version: 0x00
Applies to: HW Rev. 0x06, FW Rev. 0x01


> [!warning] Safety considerations
> NEVER lay your badge onto a metallic surface. The solder joints on the back of your cyberdeck may conduct and create a short circuit that damages its components. To secure your cyberdeck against this you might want to apply some hot glue.
> 
> Do not put the battery in reverse. The cyberdeck features some protection against this, but better safe than sorry. The cyberdeck uses LiFePo4 batteries, which are safer to use than LiPo batteries. Never use a LiPo battery in the badge. It will likely damage as the badge runs it down to 2.5V. Also read the chapter [[#Charging]].
> 
> Although it always rains in our cyberpunk future, your cyberdeck is not waterproof. Please do not use in wet environments. If you want to secure it against accidental spills, you could apply Plastik 70 spray. Make sure to cover contacts and the display before spraying.

> [!info] Important information
> If your cyberdeck does not power on, it is very likely that you need to kickstart it by giving it a charging impulse over USB-C. The protection chip may need this to wake up. This usually only happens when the battery was fully discharged or it was removed.
> 
> Currently there is a bug in the console driver. The serial console, which can be accessed using the USB-C connector does not properly initialise if you do not have a serial connection open during startup. Serial messages will be printed just fine, but input will not be processed. To fix this, plug in the running cyberdeck and open the serial terminal of your choice. Connect to the cyberdeck and then push the reset button on your cyberdeck (left button between the display and the battery) to restart.
# Getting started
If you did not build your cyberdeck yourself, jump right ahead and just use it. However, if for any reason you do not yet have a firmware installed, get your favourite firmware and consult the  [[Development Guide]] how to flash it. 
Also notice that cyberdecks are personal. The default firmware will provide you with some fun, but the main intention of the cyberdeck is to enable you to make it your own by customising the firmware with your own ideas. 
# Charging
To charge the cyberdeck, use a usb-c cable and plug it either in a charging socket or computer. The default charging will be safe with any usb port that can deliver 500mA of current. 
The cyberdeck also support charging with 1.5A of current. To enable this, unplug your cyberdeck and set jumper JP1 (close to the usb-c connector). However, be careful. If you hook your badge up to a port that does not support delivering this amount of current you might overload the upstream system and - worst case - damage it (never seen that happening, usually it just fails to deliver the current, but you have been warned). 
If you fail to unplug the cyberdeck before closing jumper JP1, the upstream device likely will not notice the increased current requirement and continue to deliver the 500mA. In that case, charging behaviour is undefined (but most likely it will just continue charging with 500mA).
# Booting
When you switch on your cyberdeck, it will start to initialise the drivers for the hardware. You can see the status of driver initialisation by the leds and - if present - the lcd screen. If any of the drivers fails to initialise this is shown by the leds and - if your badge supports a display and it is running - on the display. There are two kinds of failures: "critical failures" and "failures". 
A critical failure means that your cyberdeck cannot function and will not continue to boot. This is indicated by all leds turning red and a red "FAIL" status on the display. 
A failure means that your cyberdeck will start up just fine but that the functionality may take an impact. For example, if the display fails, you will still be able to use the console but any module that uses the display will not function properly. Normal failures are indicated by the led associated to the affected hardware capability turns yellow and - if a display is supported and is running - by a yellow fail message on the display.
The driver initialisation sequence is as follows:
- Led driver
- Display driver
- Controller driver
- Filesystem driver
- UART / console driver
- WiFi driver
If your bootup is all green, the move on to the next chapter.
## Troubleshooting
If any of the drivers fails during bootup, then you may try the following to resolve the situation:
- **Led driver**: 
	- The led driver cannot detect the presence or absence of the hardware. This means that if the driver initialises correctly but your leds malfunction, there is likely a hardware fault such as a cold soldering joint, a damaged WS2812B led or a damaged trace. Note that WS2812B leds forward data to the next led in the chain, so if multiple leds are not functioning it may be that only the first in the chain is the culprit.
	- Another reason why the led driver may fail is that the initialisation of the driver failed due to the general input / output (GPIO) of the esp32 not functioning properly or because the remote control transceiver (rmt) of th esp32 does not work properly. Last but not least, if concurrent use of leds is enable (which it is by default) the allocation of the memory necessary to run the threads may have failed. In all of these cases, the developer of the firmware might be able to help.
- **Display driver**:
	- The driver can neither detect the presence or absence of a display, nor the proper functioning of it. The driver will however check the proper functioning of the spi bus and abort with a critical failure if the spi bus is not working as intended.
-  **Filesystem**:
	- The filesystem is initialised in two steps. First non volatile storage (NVS) is initialised. NVS is needed by the WiFi and potentially other features to store small configuration parameters. If NVS initialisation fails because there are no more free pages, NVS is erased and initialisation retried. If NVS still fails, the bootup sequence aborts with a critical failure. This could be due to a hardware failure or error in the firmware.
	- In the second step, the SPIFFS filesystem is initialised. SPIFFS initialisation may fail due to various reasons. However, most likely the filesystem partition was not properly created and data not uploaded. This can easily be fixed by re-uploading the spiffs partition. Check the [[Development Guide]]. SPIFFS failures are not critical and will continue bootup. However, your cyberdeck may have severe functionality reductions.
- **Controller driver**:
	- If controller driver initialisations fails, this is a non critical failure. Functionality will be impacted, but your cyberdeck will start up and you will still be able to use the console.
	- The controller driver cannot detect hardware failures of the buttons or the shift register. If driver initialisation fails this is likely due to gpio configuration failing.
- **UART / console driver**:
	- NOTE: The console currently has a bug. It will only spawn if your are connected with your terminal software during startup. 
	- UART / console initialisation may fail due to various reasons. None of these failures is critical. If an error occurs, the cyberdeck will continue startup and run without serial console support. 
- **WiFi driver**:
	- The WiFi driver may fail, but its not a critical failure. Bootup will continue without wifi support.

# Controller
The controller layout features four buttons on the left hand side and four buttons on the right hand side.
```
        up                       Y
        O                   X    O
                            O
left O     O right
                                 O
        O                   O    B
       down                 A     
```
While custom firmware / modules may redefine how buttons are used, normally the left hand side is used for navigation and the right hand side for selection. This is why buttons are named as shown above.
# Navigation
Likely, the firmware on your cyberdeck will display a menu after startup. You can navigate the menu up and down using the top and bottom button on the left hand side of the controller.  
Menu entries are either executables - so called action items - or submenus. To execute a menu item or enter a submenu press one of the bottom row buttons of the right hand side of the controller  (button A or button B). Usually buttons X and Y (top row of right hand side of the controller) are used to leave a submenu or terminate execution of an action item. Note that action items do not need to follow this convention, so make sure to read the instructions.
# Modules
A module of the cyberdeck is a unit of functionality. Usually a module is activated by starting it in the menu on the display, using your pushbuttons A or B of the controller. It follows a description of the preinstalled modules of your firmware (make sure the revision of your firmware matches the revision of this guide).
> [!info] Rumor has it...
> ...that there is some hidden functionality that needs unlocking first.
## Sneaky & Mousy
TODO
## Logo Slideshow
TODO
## Party
TODO
## SAO Test
TODO
## Demo Mode
TODO
## Connect WiFi
TODO
# Console
TODO