/**
 * @file        st7735_bcd.hpp
 * @author      Florian Schuetz (fschuetz@ieee.org)
 * @brief       A driver for the st7735 display for the htcw gfx library.
 * @version     0.1
 * @date        2022-09-25
 * 
 * @copyright Copyright (c) 2022, Florian Schuetz, released under MIT license
 * 
 * This driver is inspired by the sample driver in the htcw gfx project 
 * (https://github.com/codewitch-honey-crisis/gfx) and the driver for the ST7735 
 * display at TFT_eSPI porject by Bodmer (https://github.com/Bodmer/TFT_eSPI).
 * 
 * Datasheet: 
 *      https://www.displayfuture.com/Display/datasheet/controller/ST7735.pdf
 * 
 * This display driver uses the 4 line serial interface (SPI). In this mode four
 * data streams exist:
 *  - CSX:  Chip enable.
 *  - D/CX: Data / command flag.
 *  - SCL:  The SPI clock.
 *  - SDA:  Serial data input / output.
 * 
 * Serial Clock: Used to communicate with the microcontroller. Can be stopped if
 *          no communication is needed.
 * 
 * Command write mode: When the microcontroller writes commands / data to the 
 *          display controller.
 * 
 * Open Issues
 *  - Async functions for SPI transfers larger than spi max transfer size not
 *      working - need to implement fragmentation code.
 */
#pragma once
#define HTCW_ST7735_OVERCLOCK
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "./common/tft_spi_driver.hpp"
#include "gfx_core.hpp"
#include "gfx_positioning.hpp"
#include "gfx_pixel.hpp"
#include "gfx_palette.hpp"
#include "sdkconfig.h"

#define PIXEL_TYPE          gfx::rgb_pixel<16>
#define PIXEL_DEPTH         gfx::rgb_pixel<16>::bit_depth / 8

#ifdef CONFIG_MAD_RGB
    #define TFT_MAD_COLOR_ORDER 0x00
#elif CONFIG_MAD_BGR
    #define TFT_MAD_COLOR_ORDER 0x08
#endif

#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

namespace espidf {

    /*******************************************************************************************************
     * Initialisation commands for the ST7735 display 
     * 
     * The bytes that need to be sent to the hardware in order to configure the display. Mostly copied from
     * the Bodmer TFT library for arduino
     * (https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_Drivers/ST7735_Init.h). Datasheet for the Display
     * can be found at: https://www.displayfuture.com/Display/datasheet/controller/ST7735.pdf 
     *******************************************************************************************************/
    namespace st7735_helpers {
        DRAM_ATTR static const uint8_t generic_st7735[] =  { 
                                            // 7735R init, part 1
            15,                             // 15 commands in list:
            ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
            150,                          //     150 ms delay
            ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
            255,                          //     500 ms delay
            ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
            0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
            ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
            0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
            ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
            0x01, 0x2C, 0x2D,             //     Dot inversion mode
            0x01, 0x2C, 0x2D,             //     Line inversion mode
            ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
            0x07,                         //     No inversion
            ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
            0xA2,
            0x02,                         //     -4.6V
            0x84,                         //     AUTO mode
            ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
            0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
            ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
            0x0A,                         //     Opamp current small
            0x00,                         //     Boost frequency
            ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
            0x8A,                         //     BCLK/2,
            0x2A,                         //     opamp current small & medium low
            ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
            0x8A, 0xEE,
            ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
            0x0E,
            ST77XX_INVOFF,  0,              // 13: Don't invert display, no args
            ST77XX_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
            0xC0 | TFT_MAD_COLOR_ORDER,   //     row/col addr, bottom-top refresh
            ST77XX_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
            0x05 
        };                       //     16-bit color

        static const uint8_t generic_st7735_3[] = {                       // 7735R init, part 3 
            4,                              //  4 commands in list:
            ST7735_GMCTRP1, 16      ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
            0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
            0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
            0x29, 0x25, 0x2B, 0x39,
            0x00, 0x01, 0x03, 0x10,
            ST7735_GMCTRN1, 16      ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
            0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
            0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
            0x2E, 0x2E, 0x37, 0x3F,
            0x00, 0x00, 0x02, 0x10,
            ST77XX_NORON,     ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
            10,                           //     10 ms delay
            ST77XX_DISPON,    ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
            100 
        };                        //     100 ms delay

    }

    /*******************************************************************************************************
     * Driver for the ST7735 display 
     * 
     * The driver for the ST7735 display. Based on previous HTCW ST7735 driver and Bodmer TFT driver.
     *******************************************************************************************************/
    template<uint16_t Width,
            uint16_t Height,
            spi_host_device_t HostId,
            gpio_num_t PinCS,
            gpio_num_t PinDC,
            gpio_num_t PinRst,
            gpio_num_t PinBacklight,
            uint8_t Rotation = 0,
            size_t MaxTransferSize=0, // NEW - TODO VERIFY - CHANGE DEFAULT TO SOMETHING THAT MAKES SENSE
            size_t MaxTransactions=7,
            bool UsePolling = true,
            long DmaSize = -1,          // TODO verify - changed from size_t to long - sign error
            TickType_t Timeout=5000/portTICK_PERIOD_MS,
            uint32_t BatchBufferSize=64
            >

    struct st7735 : 
        public tft_spi_driver<(Rotation & 1 ? Height : Width),            // Set depending on orientation
                                (Rotation & 1 ? Width : Height),          // Set depending on orientation
                                PIXEL_DEPTH,
                                HostId,
                                PinCS,
                                PinDC,
#ifdef HTCW_ST7735_OVERCLOCK
                                26*1000*1000,
#else
                                10*1000*1000,
#endif
                                MaxTransferSize,
                                MaxTransactions,
                                UsePolling,
                                DmaSize,
                                Timeout,
                                BatchBufferSize> {
                                    
        using base_type = tft_spi_driver<Width,
                            Height,
                            PIXEL_DEPTH,
                            HostId,
                            PinCS,
                            PinDC,
#ifdef HTCW_ST7735_OVERCLOCK
                            26*1000*1000,
#else
                            10*1000*1000,
#endif
                            MaxTransactions,
                            UsePolling,
                            DmaSize,
                            Timeout,
                            BatchBufferSize>;
         
        // the RST pin
        constexpr static const gpio_num_t pin_rst = PinRst;
        // the BL pin
        constexpr static const gpio_num_t pin_backlight = PinBacklight;
        // the display rotation NEW
        constexpr static const uint8_t rotation = Rotation & 3;
        // the maximum number of "in the air" transactions that can be queued
        constexpr static const size_t max_transactions = (0==MaxTransactions)?1:MaxTransactions;
    private:
       
        tft_spi_driver_result write_window_impl(const tft_spi_driver_rect& w,bool queued,tft_spi_driver_set_window_flags set_flags) {
            tft_spi_driver_rect win;
            const uint16_t offsx = rotation & 1 ? 1 : 2;       // Offset -> TODO - make parameter
            const uint16_t offsy = rotation & 1 ? 2 : 1;       // Offset -> TODO - make parameter
            win.x1=w.x1+offsx;
            win.x2=w.x2+offsx;
            win.y1=w.y1+offsy;
            win.y2=w.y2+offsy;
            //printf("write_window_impl: win: (%d, %d)-(%d, %d)\r\n",win.x1,win.y1,win.x2,win.y2);
            tft_spi_driver_result r;
            uint8_t tx_data[4];
            //Column Address Set
            r=this->send_next_command(0x2A,queued);
            if(tft_spi_driver_result::success!=r)
                return r;
            if(set_flags.x1 || set_flags.x2) {
                tx_data[0]=win.x1>>8;             //Start Col High
                tx_data[1]=win.x1&0xFF;           //Start Col Low
                tx_data[2]=win.x2>>8;             //End Col High
                tx_data[3]=win.x2&0xff;           //End Col Low
                r=this->send_next_data(tx_data,4,queued,true);
                if(tft_spi_driver_result::success!=r)
                    return r;
            }
            if(set_flags.y1 || set_flags.y2 || !(set_flags.x1 || set_flags.x2)) {
                //Page address set
                r=this->send_next_command(0x2B,queued,true);
                if(tft_spi_driver_result::success!=r)
                    return r;
                tx_data[0]=win.y1>>8;        //Start page high
                tx_data[1]=win.y1&0xff;      //start page low
                tx_data[2]=win.y2>>8;        //end page high
                tx_data[3]=win.y2&0xff;      //end page low
                r=this->send_next_data(tx_data,4,queued,true);
                if(tft_spi_driver_result::success!=r)
                    return r;
            }
            // Memory write
            return this->send_next_command(0x2C,queued,true);
        }
        tft_spi_driver_result send_init_commands(const uint8_t* addr) {
            uint8_t numCommands, cmd, numArgs;
            uint16_t ms;
            tft_spi_driver_result r;
            numCommands = *(addr++);                            // Number of commands to follow
            while (numCommands--) {                             // For each command...
                cmd = *(addr++);                                // Read command
                numArgs = *(addr++);                            // Number of args to follow
                ms = numArgs & ST_CMD_DELAY;                    // If hibit set, delay follows args
                numArgs &= ~ST_CMD_DELAY;                       // Mask out delay bit
                r= this->send_init_command(cmd);
                if(tft_spi_driver_result::success!=r) {
                    return r;
                }
                r= this->send_init_data(addr, numArgs);
                if(tft_spi_driver_result::success!=r) {
                    return r;
                }
                addr += numArgs;
                if (ms) {
                    ms = *(addr++); // Read post-command delay time (ms)
                    if (ms == 255)
                        ms = 500; // If 255, delay for 500 ms
                    vTaskDelay(ms/portTICK_PERIOD_MS);
                }
            }
            return tft_spi_driver_result::success;
        }
    public:
        // constructs a new instance of the driver
        st7735() {
            
        }
        virtual ~st7735() {}

        // forces initialization of the driver
        tft_spi_driver_result initialize()
        {
            if(!this->initialized()) {
                static const TickType_t ts = 100/portTICK_PERIOD_MS;                
                //Initialize non-SPI GPIOs
                gpio_set_direction(base_type::pin_dc, GPIO_MODE_OUTPUT);
                gpio_set_direction(pin_rst, GPIO_MODE_OUTPUT);
                gpio_config_t pc;
                pc.pin_bit_mask = 1<<(int)pin_backlight;
                pc.mode=GPIO_MODE_OUTPUT;
                pc.intr_type = GPIO_INTR_DISABLE;
                pc.pull_down_en = GPIO_PULLDOWN_DISABLE;
                pc.pull_up_en = GPIO_PULLUP_DISABLE;
                gpio_config(&pc);
                //Reset the display
                gpio_set_level(pin_rst, 0);
                vTaskDelay(ts);
                gpio_set_level(pin_rst, 1);
                vTaskDelay(ts);
                tft_spi_driver_result r;
                r=send_init_commands(st7735_helpers::generic_st7735);
                if(tft_spi_driver_result::success!= r) {
                    return r;
                }
                r=this->send_init_command(ST77XX_CASET);
                if(tft_spi_driver_result::success!= r) {
                    return r;
                }
                uint8_t c_init_data[] = {0,0,0,Width-1};
                r=this->send_init_data(c_init_data,4);
                if(tft_spi_driver_result::success!= r) {
                    return r;
                }
                r= this->send_init_command(ST77XX_RASET);  //  6: Row addr set, 4 args, no delay:
                if(tft_spi_driver_result::success!= r) {
                    return r;
                }
                uint8_t r_init_data[] ={0,0,0,Height-1};
                r=this->send_init_data(r_init_data,4);
                if(tft_spi_driver_result::success!= r) {
                    return r;
                }
                r=send_init_commands(st7735_helpers::generic_st7735_3);
                if(tft_spi_driver_result::success!= r) {
                    return r;
                }

                // Set orientation of display
                uint8_t madctl;
                switch (rotation) {
                    case 0:
                        // Portrait
                        madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | TFT_MAD_COLOR_ORDER;
                        break;
                    case 1:
                        // Lanscape
                        madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | TFT_MAD_COLOR_ORDER;
                        break;
                    case 2:
                        // Portrait
                        madctl = TFT_MAD_COLOR_ORDER;
                        break;

                    case 3:
                        // Landscape
                        madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | TFT_MAD_COLOR_ORDER;
                        break;
                }

                r=this->send_init_command(ST77XX_MADCTL);
                if(tft_spi_driver_result::success!= r) {
                    return r;
                }
                uint8_t rot_init_data[] = {madctl};
                r=this->send_init_data(rot_init_data,1);
                if(tft_spi_driver_result::success!= r) {
                    return r;
                }
                
                //Enable backlight
                gpio_set_level(pin_backlight, 1);
                
            }
            return tft_spi_driver_result::success;
        }
protected:
        virtual tft_spi_driver_result write_window(const tft_spi_driver_rect& bounds,tft_spi_driver_set_window_flags set_flags) {
            return write_window_impl(bounds,false,set_flags);
        }
        virtual tft_spi_driver_result queued_write_window(const tft_spi_driver_rect& bounds,tft_spi_driver_set_window_flags set_flags) {
            return write_window_impl(bounds,true,set_flags);
        }

        // GFX bindings
 public:
        // indicates the type, itself
        using type = st7735<Width,Height,HostId,PinCS,PinDC,PinRst,PinBacklight,MaxTransactions,UsePolling,DmaSize,Timeout,BatchBufferSize>;
        // indicates the pixel type
        using pixel_type = PIXEL_TYPE;
        // Set the capabilities of the driver:
        //      Blt      - not supported, set to false
        //      Async    - asynchronous support, set to true
        //      Batch    - batch processing, set to true
        //      CopyFrom - Set to true if SPI buffer is at least half the size 
        //                 of the display size * bytes per pixel + 8 
        //                 (eg. 128*160*2). Otherwise will be set to false  as
        //                 this might be more efficient - you might want to do 
        //                 your own benchmarking and adjust.
        //      Suspend  - not supported, set to false
        //      Read     - not supported, set to false
        //      CopyTo   - not supported, set to false
        //
        // A note on the CopyFrom capability. On some chips, like the ESP32-S3,
        // the maximal amount of data that can be transfered over the SPI bus
        // using DMA transfers may be limited to less than the what would be 
        // needed to update the full display. In this case, tft_spi_driver will
        // break a frame down in segments and transfer those. Depending on the
        // max transfer size, there may be more or less segments and it comes a
        // point where this gets inefficient. 
        // On limitations of the ESP32-S3 SPI bus max transfer size see also 
        // https://github.com/espressif/esp-idf/issues/7804
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
        // TODO fix async spi transfer and then reenable capability
        // TODO implement suspend as a software buffer (bmp)
        using caps = gfx::gfx_caps<false,false,true,true,false,false,false>;
#else // CONFIG_IDF_TARGET_ESP32
        using caps = gfx::gfx_caps<false,true,true,true,false,false,false>;
#endif //CONFIG_IDF_TARGET
        
 
 private:
        gfx::gfx_result xlt_err(tft_spi_driver_result r) {
            switch(r) {
                case tft_spi_driver_result::io_error:
                    return gfx::gfx_result::device_error;
                case tft_spi_driver_result::out_of_memory:
                    return gfx::gfx_result::out_of_memory;
                case tft_spi_driver_result::success:
                    return gfx::gfx_result::success;
                default:
                    return gfx::gfx_result::invalid_argument;
            }
        }
        template<typename Source>
        gfx::gfx_result copy_from_impl(const gfx::rect16& src_rect,const Source& src,gfx::point16 location,bool async) {
            tft_spi_driver_result r;
            gfx::rect16 srcr = src_rect.normalize().crop(src.bounds());
            gfx::rect16 dstr(location,src_rect.dimensions());
            dstr=dstr.crop(bounds());
            if(srcr.width()>dstr.width()) {
                srcr.x2=srcr.x1+dstr.width()-1;
            }
            if(srcr.height()>dstr.height()) {
                srcr.y2=srcr.y1+dstr.height()-1;
            }
            // printf("copy_from_impl, srcr: (%d,%d)-(%d,%d)\n",srcr.x1,srcr.y1,srcr.x2,srcr.y2);
            // printf("copy_from_impl, src.bounds(): width: %d, height: %d\n", src.bounds().width(), src.bounds().height());
            // printf("copy_from_impl, dstr: (%d,%d)-(%d,%d)\n",dstr.x1,dstr.y1,dstr.x2,dstr.y2);
            
            if(gfx::helpers::is_same<pixel_type,typename Source::pixel_type>::value && Source::caps::blt) {
                // direct blt
                if(src.bounds().width()==srcr.width() && srcr.x1==0) {
                    tft_spi_driver_rect dr = {dstr.x1,dstr.y1,dstr.x2,dstr.y2};
                    if(!async) {
                        // printf("copy_from_impl, dr: (%d,%d)-(%d,%d)\n",dr.x1,dr.y1,dr.x2,dr.y2);
                        r=this->frame_write(dr,src.begin()+(srcr.y1*src.dimensions().width*2));
                    }
                    else {
                        // DEBUG printf("Using async"); // TODO diag
                        r=this->queued_frame_write(dr,src.begin()+(srcr.y1*src.dimensions().width*2));
                    }
                    if(tft_spi_driver_result::success!=r) {
                        return xlt_err(r);
                    }
                    return gfx::gfx_result::success;
                }
                // line by line blt
                // TODO - suerly this can be made more efficient
                uint16_t yy=0;
                uint16_t hh=srcr.height();
                uint16_t ww = src.dimensions().width;
                while(yy<hh) {
                    tft_spi_driver_rect dr = {dstr.x1,uint16_t(dstr.y1+yy),dstr.x2,uint16_t(dstr.y1+yy)}; // DONE - Fixed Bug: Final coordinate must be y1+yy and not ,uint16_t(dstr.x2+yy)}; 
                    // printf("copy_from_impl, dr: (%d,%d)-(%d,%d)\n",dr.x1,dr.y1,dr.x2,dr.y2);
                    // printf("Offset calculation gives: %d", (ww*(srcr.y1+yy)+srcr.x1));
                    if(!async)
                        r = this->frame_write(dr,src.begin()+this->pixel_depth*((ww*(srcr.y1+yy)+srcr.x1))); // DONE - Fixed Bug: Needs to be multiplied by pixel depth in bytes (2)
                    else
                        r = this->queued_frame_write(dr,src.begin()+(ww*(srcr.y1+yy)+srcr.x1));
                    if(tft_spi_driver_result::success!=r) {
                        return xlt_err(r);
                    }
                    ++yy;
                }
                return gfx::gfx_result::success;
            }
            uint16_t w = dstr.dimensions().width;
            uint16_t h = dstr.dimensions().height;
            tft_spi_driver_rect drr = {dstr.x1,dstr.y1,dstr.x2,dstr.y2};
            if(!async)
                r=this->batch_write_begin(drr);
            else
                r=this->queued_batch_write_begin(drr);
            if(tft_spi_driver_result::success!=r) {
                return xlt_err(r);
            }
            for(uint16_t y=0;y<h;++y) {
                for(uint16_t x=0;x<w;++x) {
                    typename Source::pixel_type pp;
                    gfx::gfx_result rr=src.point(gfx::point16(x+srcr.x1,y+srcr.y1), &pp);
                    if(rr!=gfx::gfx_result::success)
                        return rr;
                    pixel_type p;
                    rr=gfx::convert_palette_to(src,pp,&p);
                    if(gfx::gfx_result::success!=rr) {
                        return rr;
                    }
                    uint16_t pv = p.value();
                    if(!async)
                        r = this->batch_write(&pv,1);
                    else
                        r = this->queued_batch_write(&pv,1);
                    if(tft_spi_driver_result::success!=r) {
                        return xlt_err(r);
                    }
                }
            }
            if(!async)
                r=this->batch_write_commit();
            else
                r=this->queued_batch_write_commit();
            if(tft_spi_driver_result::success!=r) {
                return xlt_err(r);
            }
            return gfx::gfx_result::success;
        }
 public:
        // retrieves the dimensions of the screen
        constexpr inline gfx::size16 dimensions() const {
            return rotation & 1 ? gfx::size16(base_type::height,base_type::width)
                                : gfx::size16(base_type::width,base_type::height);
            
        }
        // retrieves the bounds of the screen
        constexpr inline gfx::rect16 bounds() const {
            return gfx::rect16(gfx::point16(0,0),dimensions());
        }
        // sets a point to the specified pixel
        gfx::gfx_result point(gfx::point16 location,pixel_type pixel) {
            tft_spi_driver_result r = this->pixel_write(location.x,location.y,pixel.value());
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // asynchronously sets a point to the specified pixel
        gfx::gfx_result point_async(gfx::point16 location,pixel_type pixel) {
            tft_spi_driver_result r = this->queued_pixel_write(location.x,location.y,pixel.value());
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // fills the specified rectangle with the specified pixel
        gfx::gfx_result fill(const gfx::rect16& bounds,pixel_type color) {
            tft_spi_driver_rect b = {bounds.x1,bounds.y1,bounds.x2,bounds.y2};
            tft_spi_driver_result r=this->frame_fill(b,color.value());
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // asynchronously fills the specified rectangle with the specified pixel
        gfx::gfx_result fill_async(const gfx::rect16& bounds,pixel_type color) {
            tft_spi_driver_rect b = {bounds.x1,bounds.y1,bounds.x2,bounds.y2};
            tft_spi_driver_result r=this->queued_frame_fill(b,color.value());
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // clears the specified rectangle
        inline gfx::gfx_result clear(const gfx::rect16& bounds) {
            pixel_type p;
            return fill(bounds,p);
        }
        // asynchronously clears the specified rectangle
        inline gfx::gfx_result clear_async(const gfx::rect16& bounds) {
            pixel_type p;
            return fill_async(bounds,p);
        }
        // begins a batch operation for the specified rectangle
        gfx::gfx_result begin_batch(const gfx::rect16& bounds) {
            tft_spi_driver_rect b = {bounds.x1,bounds.y1,bounds.x2,bounds.y2};
            if(tft_spi_driver_result::success!= this->batch_write_begin(b))
                return gfx::gfx_result::device_error;
            return gfx::gfx_result::success;
        }
        // asynchronously begins a batch operation for the specified rectangle
        gfx::gfx_result begin_batch_async(const gfx::rect16& bounds) {
            tft_spi_driver_rect b = {bounds.x1,bounds.y1,bounds.x2,bounds.y2};
            tft_spi_driver_result r = this->queued_batch_write_begin(b);
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // writes a pixel to a pending batch
        gfx::gfx_result write_batch(pixel_type color) {
            uint16_t p = color.value();
            tft_spi_driver_result r = this->batch_write(&p,1);
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // asynchronously writes a pixel to a pending batch
        gfx::gfx_result write_batch_async(pixel_type color) {
            uint16_t p = color.value();
            tft_spi_driver_result r = this->queued_batch_write(&p,1);
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // commits a pending batch
        gfx::gfx_result commit_batch() {
            tft_spi_driver_result r=this->batch_write_commit();
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
                
            return gfx::gfx_result::success;
        }
        // asynchronously commits a pending batch
        gfx::gfx_result commit_batch_async() {
            tft_spi_driver_result r=this->queued_batch_write_commit();
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }
        // copies source data to a frame
        template<typename Source> inline gfx::gfx_result copy_from(const gfx::rect16& src_rect,const Source& src,gfx::point16 location) {
            return copy_from_impl(src_rect,src,location,false);
        }
        // asynchronously writes source data to a frame
        template<typename Source>
        inline gfx::gfx_result copy_from_async(const gfx::rect16& src_rect,const Source& src,gfx::point16 location) {
            return copy_from_impl(src_rect,src,location,true);
        }
        // waits for all pending asynchronous operations to complete
        gfx::gfx_result wait_all_async() {
            tft_spi_driver_result r=this->queued_wait_all();
            if(tft_spi_driver_result::success!=r)
                return xlt_err(r);
            return gfx::gfx_result::success;
        }

        // suspend - TODO make real
        gfx::gfx_result suspend() {
            // TODO make right verify - test only
            return gfx::gfx_result::success;
        }

        // resume - TODO make real
        gfx::gfx_result resume(bool force=false) {
            return gfx::gfx_result::success;
        }
        
    };

}