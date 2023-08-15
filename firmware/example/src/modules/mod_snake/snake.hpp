#pragma once
#ifndef BCD_MODULE_SNAKE_HPP
#define BCD_MODULE_SNAKE_HPP

#include <stdlib.h>
#include <random>
#include <esp_random.h>
#include <string.h>
#include "esp_log.h"
#include "st7735_bcd.hpp"
#include "gfx.hpp"
//#include "gfx_cpp14.hpp"
#include "fonts/Bm437_Acer_VGA_8x8.h"

#include "ch405labs_led.hpp"
#include "controller.hpp"

////////////////////////////////////////////////////////////////////////////////
// Menuconfig options
////////////////////////////////////////////////////////////////////////////////
#define TAG_MOD_SNAKE                   CONFIG_TAG_MOD_SNAKE

#if CONFIG_MOD_SNAKE_LOG_LEVEL == 0
#define MOD_SNAKE_LOG_LEVEL esp_log_level_t::ESP_LOG_NONE
#elif CONFIG_MOD_SNAKE_LOG_LEVEL == 1
#define MOD_SNAKE_LOG_LEVEL esp_log_level_t::ESP_LOG_ERROR
#elif CONFIG_MOD_SNAKE_LOG_LEVEL == 2
#define MOD_SNAKE_LOG_LEVEL esp_log_level_t::ESP_LOG_WARN
#elif CONFIG_MOD_SNAKE_LOG_LEVEL == 3
#define MOD_SNAKE_LOG_LEVEL esp_log_level_t::ESP_LOG_INFO
#elif CONFIG_MOD_SNAKE_LOG_LEVEL == 4
#define MOD_SNAKE_LOG_LEVEL esp_log_level_t::ESP_LOG_DEBUG
#elif CONFIG_MOD_SNAKE_LOG_LEVEL == 5
#define MOD_SNAKE_LOG_LEVEL esp_log_level_t::ESP_LOG_VERBOSE
#endif //CONFIG_MOD_CYBERSPACE_LOG_LEVEL

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////
typedef BaseType_t mod_snake_err_t;

#define MOD_SNAKE_FAIL                                              -1          /**< Generic failure */
#define MOD_SNAKE_OK                                                0x000       /**< All good */
#define MOD_SNAKE_CRITICAL_INITALISATION_FAIL                       0x001       /**< Critical failure in initialisation */
#define MOD_SNAKE_PENDING                                           0x003       /**< Action pending */
#define MOD_SNAKE_OUT_OF_MEMORY                                     0x010       /**< No more free memoty */
#define MOD_SNAKE_INDEX_OUT_OF_BOUNDS                               0x011       /**< Index out of bounds */
#define MOD_SNAKE_BOARD_FIELD_OCCUPIED                              0x020       /**< Board field not empty */
#define MOD_SNAKE_BOARD_NO_MORE_FREE_FIELDS                         0x021       /**< No more free board fields */

#define MOD_SNAKE_REASON_NO_CONTROLLER              "Controller not supported."
#define MOD_SNAKE_REASON_CONTROLLER_FAIL            "Controller initialisation failed."
#define MOD_SNAKE_REASON_NO_DISPLAY                 "Display not supported."


////////////////////////////////////////////////////////////////////////////////
// Game over codes
////////////////////////////////////////////////////////////////////////////////
#define MOD_SNAKE_GAME_OVER_UNDEFINED                               -2          /**< Undefined game over reason */
#define MOD_SNAKE_GAME_OVER_ERROR                                   -1          /**< The game was ended due to error */
#define MOD_SNAKE_GAME_OVER_WIN                                      0          /**< The user won the game */                                   
#define MOD_SNAKE_GAME_OVER_COLLISON                                 1          /**< Player hit obstacle, lost */
#define MOD_SNAKE_GAME_OVER_ABORT                                    2          /**< Player abort game */

////////////////////////////////////////////////////////////////////////////////
// Sprites
////////////////////////////////////////////////////////////////////////////////
#define SNAKE_32x32_IMPLEMENTATION
#include "sprites/snake_32x32.hpp"
#define SNAKE_16x16_IMPLEMENTATION
#include "sprites/snake_16x16.hpp"
#define SNAKE_8x8_IMPLEMENTATION
#include "sprites/snake_8x8.hpp"
#define MOUSE_8x8_IMPLEMENTATION
#include "sprites/mouse_8x8.hpp"

////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define EMPTY                       0
#define SNAKE                       1
#define MOUSE                       2
#define WALL                        3
#define SCORE_MULTIPLIER_MAX        100
#define SCORE_DECREASE_PERCENT      10

////////////////////////////////////////////////////////////////////////////////
// Namespaces
////////////////////////////////////////////////////////////////////////////////
using namespace espidf;
using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////
using pixel_type = rgb_pixel<16>;
using bmp_type = bitmap<pixel_type>;
using const_bmp_type = const_bitmap<rgb_pixel<16>>;
using mask_type = const_bitmap<gsc_pixel<1>>;
using sprite_type = sprite<rgb_pixel<16>>;

namespace bcd_mod_snake {

    ////////////////////////////////////////////////////////////////////////////
    // General stuff
    ////////////////////////////////////////////////////////////////////////////
    class gridPosition {
        public:
            uint8_t x;
            uint8_t y;

            bool operator== (const gridPosition& other);
    };

    typedef enum class movement_direction {
        none,
        up,
        down,
        left,
        right
    } movementDirection;

    typedef struct directional_sprites {
        sprite_type *up;
        sprite_type *down;
        sprite_type *left;
        sprite_type *right;
    } directionalSprites;

    

    class gamingPieces {
        public:
        enum gaming_pieces : uint8_t {
                none,
                mouse,
                snake_segment,
                snake_head,
                wall
            };

        gamingPieces() = default;
        constexpr gamingPieces(gaming_pieces aPiece) : _piece(aPiece) { }


        // Allow switch and comparisons.
        constexpr operator gaming_pieces() const { return _piece; }

        // Prevent usage: if(fruit)
        explicit operator bool() const = delete;        

        const char *getName();

        private:
        gaming_pieces _piece;
    };

    typedef struct board_field {
        gamingPieces piece;
        uint32_t timestamp;
    } boardField;

    ////////////////////////////////////////////////////////////////////////////
    // Game board stuff
    ////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief 
     * 
     * Using _turn saves us from cleaninng the board
     */
    class gameBoard {
        public:
            gameBoard(uint8_t width, uint8_t height);
            gameBoard() = delete;
            ~gameBoard();

            void setGameSpeed(TickType_t speed);
            TickType_t getGameSpeed();
            uint8_t getBoardWidth();
            uint8_t getBoardHeight();

            bool isFieldOccupied(gridPosition field);
            bool isFieldFree(gridPosition field);

            mod_snake_err_t setGamingPiece(gridPosition pos, gamingPieces piece, bool overwrite = false);
            gamingPieces getGamingPiece(gridPosition pos);
            mod_snake_err_t getLastError();

            void nextTurn();
            void refreshField(gridPosition pos);
            mod_snake_err_t findNextFreeField(gridPosition &pos);
            
        private:
            boardField *_board;
            uint8_t _board_width;
            uint8_t _board_height;
            TickType_t _game_speed = pdMS_TO_TICKS(500);
            uint32_t _turn = 0;

            mod_snake_err_t _lasterror = MOD_SNAKE_OK;

            inline uint16_t _index(gridPosition pos);
            inline uint16_t _index(uint8_t x, uint8_t y);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Snake stuff
    ////////////////////////////////////////////////////////////////////////////

    typedef struct snake_segment {
        gridPosition position;
        movementDirection direction;
        directionalSprites sprites;

        sprite_type *getSprite();

    } snakeSegment;

    class snakeSegmentChainElement {
        public:

            // Data
            snakeSegment segment;

            struct snakeSegmentChainElement *previous;
            struct snakeSegmentChainElement *next;

            // Prefix increment
            snakeSegmentChainElement* operator++() { return this->next; }  

            // Postfix increment
            snakeSegmentChainElement* operator++(int) { snakeSegmentChainElement *tmp = this; ++(*this); return tmp; }

            sprite_type *getSprite();

    };

    class snake {
        private:
            template <typename PointerType> class _iterator {
                // TODO make the iterator constant
                public: 
                    using iterator_category = std::forward_iterator_tag;
                    using difference_type   = std::ptrdiff_t;
                    using value_type        = PointerType;
                    using pointer           = PointerType*;  
                    using reference         = PointerType&;  

                    _iterator(pointer ptr) : m_ptr(ptr) {}

                    reference operator*() const { return *m_ptr; }
                    pointer operator->() { return m_ptr; }

                    // Prefix increment
                    _iterator& operator++() { m_ptr = m_ptr->next; return *this; }  

                    // Postfix increment
                    _iterator operator++(int) { _iterator tmp = *this; m_ptr = m_ptr->next; return tmp; }

                    friend bool operator== (const _iterator& a, const _iterator& b) { return a.m_ptr == b.m_ptr; };
                    friend bool operator!= (const _iterator& a, const _iterator& b) { return a.m_ptr != b.m_ptr; };   

                private:
                    PointerType *m_ptr;
            };
        public:
            snake(
                gridPosition pos, 
                movementDirection dir,
                directionalSprites head_sprites,
                directionalSprites segments_sprites
            );
            ~snake();

            void move(movementDirection dir, int distance=1);

            typedef _iterator<snakeSegmentChainElement> Iterator;
            typedef _iterator<const snakeSegmentChainElement> ConstantIterator;

            Iterator begin() { return Iterator(&_head); }
            Iterator end()   { return Iterator(&_tail); } 

            ConstantIterator cbegin() const { return ConstantIterator(&_head); }
            ConstantIterator cend()   const { return ConstantIterator(&_tail); }

            gridPosition getHeadPosition();

            /**
             * @brief 
             *
             * Segments are only added, when the snake moves! 
             *
             * @return 
             */
            mod_snake_err_t grow();


        private:
            snakeSegmentChainElement _head;
            // Dummy tail to provide a stop for the iterator
            snakeSegmentChainElement _tail = {
                .segment = {
                    .position = {0xFF,0xFF},
                    .direction = movementDirection::none,
                    .sprites {
                        NULL,
                        NULL,
                        NULL,
                        NULL
                    }
                },
                .previous = &_head,
                .next = NULL,         
            };
            directionalSprites _segment_sprites;
            int _num_segments = 0;
            bool _add_segment = false;
    };

    ////////////////////////////////////////////////////////////////////////////
    // Mouse stuff
    ////////////////////////////////////////////////////////////////////////////
    class mouse {
        public:
            /**
             * @brief 
             * 
             *
             * @param pos 
             * @param dir Not supported, set any value. 
             * @param sprites 
             */
            mouse(
                gridPosition pos,
                movementDirection dir, 
                directionalSprites sprites
            );
            mouse() = delete;

            void setPosition(gridPosition pos);
            gridPosition getPosition();

            sprite_type *getSprite();

        private:
            gridPosition _pos;
            directionalSprites _sprites;
            movementDirection _dir;
    };


    ////////////////////////////////////////////////////////////////////////////
    // The game
    ////////////////////////////////////////////////////////////////////////////
    
    
    /**
     * @brief The full game
     * 
     * Scoring
     * -------
     * Prepare the variables needed for scoring. The scoring consists of 
     * two components. First, every time we eat a mouse we get a fixed
     * score equal to the number of segments in length we will have after
     * eating it. This means, that the first mouse gives a base score of 1,
     * the second a base score of 2 etc... The base score is multiplied by
     * a multiplier. The multiplier starts at SCORE_MULTIPLIER_MAX and 
     * decrease with each move of sneaky by SCORE_DECREASE_PERCENT percent 
     * down to 1. Thus, the faster you eat the mouse, the higher the score.      
     * 
     * Display buffering
     * -----------------
     * The game tries to create a bmp to buffer the display. This is due to the 
     * reason, that the ST7735 driver does not support bufferin and performance
     * sucks if we write directly to the display.
     */
    template<typename Destination>
    class snakeGame {
        public:
            snakeGame(Destination &display) : _display(display), _screen_bounds(display.bounds()) {
                // Set log level
                esp_log_level_set(TAG_MOD_SNAKE, MOD_SNAKE_LOG_LEVEL);

                // Stage 0 - Initialise Hardware
                //
                // The snake game requires a controller and a display to
                // function properly.
                //
                // TODO:
                //  - Might want to add a demo mode to run if controller is 
                //      missing.
                //  - Might want to add optional led support

                // Check if preconditions are met - namely keyboard and display 
                // supported and working.
#ifndef CONFIG_DISPLAY_SUPPORT
                ESP_LOGE(TAG_MOD_SNAKE, "Snake needs display support.");
                _lasterror = MOD_SNAKE_CRITICAL_INITALISATION_FAIL;
                _lasterror_reason = MOD_SNAKE_REASON_NO_DISPLAY;
                return;
#endif

#ifndef CONFIG_INPUT_SUPPORT
                ESP_LOGE(TAG_MOD_SNAKE, "Controller not supported. Aborting...");
                _lasterror = MOD_SNAKE_CRITICAL_INITALISATION_FAIL;
                _lasterror_reason = MOD_SNAKE_REASON_NO_CONTROLLER;
                return;
                // TODO maybe implement demo mode
#endif //CONFIG_INPUT_SUPPORT

               

                // Stage 0.1 - Display preparation
                //
                // In this subsection we prepare the buffer for the display to 
                // speed up rendering (if we have the memory to do it) and some 
                // parameters we use throughout the game to display stuff on 
                // screen.

                // Initialise display variables
                //_display = display;
                //_screen_bounds = display.bounds();              
            
                
                // Prepare a display buffer (if possible)
                _screenbuffer_buf = (uint8_t *)malloc(
                    bmp_type::sizeof_buffer(_screen_bounds.dimensions()) 
                        * sizeof(uint8_t));
                if(_screenbuffer_buf == nullptr) {
                    ESP_LOGW(TAG_MOD_SNAKE, 
                        "Not buffering display: Not enough free memory.");
                } else {
                    _screenbuffer = (bmp_type *)malloc(sizeof(bmp_type));
                    if(_screenbuffer != nullptr) {
                        _screenbuffer = 
                            new (_screenbuffer) bmp_type(
                                _screen_bounds.dimensions(), _screenbuffer_buf);
                    } else {
                        ESP_LOGW(TAG_MOD_SNAKE, 
                            "Not buffering display: Not enough free memory.");
                        free(_screenbuffer_buf);
                        _screenbuffer_buf = nullptr;
                    }
                }

                // Stage 0.2 - Controller initialisation
                // 
                // We need a controller to play the game. So we make sure that it is
                // properly initialised and functiong. If it is not, we abort.
                controller_err_t controller_err = _controller.config();

                if(controller_err != CONTROLLER_OK && controller_err != CONTROLLER_ALREADY_CONFIGURED) {
                    ESP_LOGE(TAG_CONTROLLER, 
                        "Controller not functioning. (%d)", controller_err);
                    _lasterror = MOD_SNAKE_CRITICAL_INITALISATION_FAIL;
                    _lasterror_reason = MOD_SNAKE_REASON_CONTROLLER_FAIL;
                    return;
                }

                // Stage 2 - Initialise the sprites
                // Define the sizes of the sprites
                constexpr static const size16 sprite_32x32_size(32, 32); 
                constexpr static const size16 sprite_8x8_size(8, 8);

                // Instantiate the sprites

                // The 32x32 snake for the titlescreen
                // Masks
                mask_type snakehead_32x32_mask(sprite_32x32_size, 
                    snake_head_32x32_mask);
                mask_type snakebody_32x32_mask = snakehead_32x32_mask;                  // Head and body use same mask
                // Bitmaps
                const_bmp_type snakehead_32x32_bmp(sprite_32x32_size, 
                    snake_head_32x32_data);
                const_bmp_type snakebody_32x32_bmp(sprite_32x32_size, 
                    snake_body_32x32_data);
                // Declare the sprites
                sprite_type snakehead_32x32_sprite(
                    sprite_32x32_size, 
                    (void *)snakehead_32x32_bmp.begin(), 
                    (void *)snakehead_32x32_mask.begin()
                );
                sprite_type snakebody_32x32_sprite(
                    sprite_32x32_size, 
                    (void *)snakebody_32x32_bmp.begin(), 
                    (void *)snakebody_32x32_mask.begin()
                );

                // Instantiate Sneaky (the 8x8 snake)
                // Masks
                mask_type snakehead_8x8_mask(sprite_8x8_size, snake_head_8x8_mask);
                mask_type snakebody_8x8_mask = snakehead_8x8_mask;
                mask_type snaketongue_8x8_mask(sprite_8x8_size, snake_tongue_8x8_mask);
                // Bitmaps
                const_bmp_type snakehead_8x8_right_bmp(sprite_8x8_size, 
                    snake_head_8x8_right_data);
                const_bmp_type snakehead_8x8_left_bmp(sprite_8x8_size, 
                    snake_head_8x8_left_data);
                const_bmp_type snakehead_8x8_up_bmp(sprite_8x8_size, 
                    snake_head_8x8_up_data);
                const_bmp_type snakehead_8x8_down_bmp(sprite_8x8_size, 
                    snake_head_8x8_down_data);
                const_bmp_type snakebody_8x8_bmp(sprite_8x8_size, snake_body_8x8_data);
                const_bmp_type snaketongue_8x8_bmp(sprite_8x8_size, 
                    snake_tongue_8x8_data);
                // Declare the sprites
                sprite_type snakehead_8x8_right_sprite(
                    sprite_8x8_size, 
                    (void *)snakehead_8x8_right_bmp.begin(), 
                    (void *)snakehead_8x8_mask.begin()
                );
                sprite_type snakehead_8x8_left_sprite(
                    sprite_8x8_size, 
                    (void *)snakehead_8x8_left_bmp.begin(), 
                    (void *)snakehead_8x8_mask.begin()
                );
                sprite_type snakehead_8x8_up_sprite(
                    sprite_8x8_size, 
                    (void *)snakehead_8x8_up_bmp.begin(), 
                    (void *)snakehead_8x8_mask.begin()
                );
                sprite_type snakehead_8x8_down_sprite(
                    sprite_8x8_size, 
                    (void *)snakehead_8x8_down_bmp.begin(), 
                    (void *)snakehead_8x8_mask.begin()
                );
                sprite_type snakebody_8x8_sprite(
                    sprite_8x8_size, 
                    (void *)snakebody_8x8_bmp.begin(), 
                    (void *)snakebody_8x8_mask.begin()
                );
                sprite_type snaketongue_8x8_sprite(
                    sprite_8x8_size, 
                    (void *)snaketongue_8x8_bmp.begin(), 
                    (void *)snaketongue_8x8_mask.begin()
                );
                
                // Instatiate Mousy McMouseface (the 8x8 mouse)
                // Mask
                mask_type mousefull_8x8_mask(sprite_8x8_size, mouse_8x8_mask);
                // Bitmap
                const_bmp_type mousefull_8x8_bmp(sprite_8x8_size, mouse_8x8_data);
                // Declare the sprite
                sprite_type mouse_8x8_sprite(
                    sprite_8x8_size, 
                    (void *)mousefull_8x8_bmp.begin(), 
                    (void *)mousefull_8x8_mask.begin()
                );


                // Game is properly initialised
                _init_complete = true;
            };

            snakeGame() = delete;

            /**
             * @brief Destructor to free all allocated resources
             * 
             * The destructor makes sure that all dynamically allocated memory 
             * is freed. Namely this is the display buffer.
             */
            ~snakeGame() {
                // Free the display buffer
                if(_screenbuffer != nullptr) {
                    _screenbuffer->~bmp_type();
                    free(_screenbuffer);
                }

                if(_screenbuffer_buf != nullptr) {
                    free(_screenbuffer_buf);
                }
            };

            mod_snake_err_t run() {
                mod_snake_err_t return_code = MOD_SNAKE_OK;

                if(!_init_complete) {
                    return_code = MOD_SNAKE_FAIL;
                }

                return return_code;
            };

            bool isBuffering() {
                return (_screenbuffer != nullptr);
            };
        private:
            // Error handling
            mod_snake_err_t _lasterror = MOD_SNAKE_OK;                          /**< Conatins last occured error code */
            const char *_lasterror_reason = NULL;                               /**< Contains reason for last error code */
            
            // Variables to track initialisation status
            bool _init_complete = false;

            // Game state tracking variables
            int8_t _game_over_code = MOD_SNAKE_GAME_OVER_UNDEFINED;
            uint32_t _score_base = 1;
            uint32_t _score_multiplier = SCORE_MULTIPLIER_MAX;
            uint32_t _score = 0;

            // Display (Driver et al.) 
            Destination &_display;
            const rect16 _screen_bounds;
            bmp_type *_screenbuffer = nullptr;
            uint8_t *_screenbuffer_buf = nullptr;
            const font &_font = Bm437_Acer_VGA_8x8_FON;

            // Controller (Driver et al.)
            controllerDriver _controller;
    };


    template<typename Destination>
    int game_over_and_cleanup(int8_t reason, uint32_t score, Destination *lcd, controllerDriver *controller, bmp_type *screenbuffer, uint8_t *screenbuffer_buf) {
        
        int return_code = 0;
        const font &font = Bm437_Acer_VGA_8x8_FON;
        
        // Display Game Over screen.
        switch(reason) {
            case MOD_SNAKE_GAME_OVER_WIN:
            {
                const char *win_msg = "You won!";
                ssize16 win_msg_size = font.measure_text((ssize16)lcd->dimensions(), win_msg);
                uint32_t win_msg_left_indent = (lcd->dimensions().width - win_msg_size.width) / 2;
                srect16 win_msg_rect(win_msg_left_indent, 40, win_msg_left_indent + win_msg_size.width, 40 + win_msg_size.height);
                const char *score_msg = "Score: ";
                ssize16 score_msg_size = font.measure_text((ssize16)lcd->dimensions(), score_msg);
                char score_string[11];                                                 // Biggest uint32_t is 10 char in decimal
                int score_len = snprintf(score_string, 11, "%lu", score);
                ssize16 score_string_size = font.measure_text((ssize16)lcd->dimensions(), score_string);
                uint32_t score_message_left_indent = (lcd->dimensions().width - (score_msg_size.width + score_string_size.width))/2;
                uint32_t score_string_left_indent = score_message_left_indent + score_msg_size.width;
                srect16 score_msg_rect(score_message_left_indent, 80, win_msg_left_indent + win_msg_size.width, 80 + win_msg_size.height);
                srect16 score_string_rect(score_string_left_indent, 80, score_string_left_indent + score_string_size.width, 80 + score_string_size.height);
                draw::text(*lcd, win_msg_rect, win_msg, font, color<pixel_type>::alice_blue);
                draw::text(*lcd, score_msg_rect, score_msg, font, color<pixel_type>::alice_blue);
                draw::text(*lcd, score_string_rect, score_string, font, color<pixel_type>::alice_blue);
                break;
            }

            case MOD_SNAKE_GAME_OVER_COLLISON:
            {
                const char *game_over_msg = "Game Over";
                ssize16 game_over_msg_size = font.measure_text((ssize16)lcd->dimensions(), game_over_msg);
                uint32_t game_over_msg_left_indent = (lcd->dimensions().width - game_over_msg_size.width) / 2;
                srect16 game_over_msg_rect(game_over_msg_left_indent, 40, game_over_msg_left_indent + game_over_msg_size.width, 40 + game_over_msg_size.height);
                const char *score_msg = "Score: ";
                ssize16 score_msg_size = font.measure_text((ssize16)lcd->dimensions(), score_msg);
                char score_string[11];                                                 // Biggest uint32_t is 10 char in decimal
                int score_len = snprintf(score_string, 11, "%lu", score);
                ssize16 score_string_size = font.measure_text((ssize16)lcd->dimensions(), score_string);
                uint32_t score_message_left_indent = (lcd->dimensions().width - (score_msg_size.width + score_string_size.width))/2;
                uint32_t score_string_left_indent = score_message_left_indent + score_msg_size.width;
                srect16 score_msg_rect(score_message_left_indent, 80, game_over_msg_left_indent + game_over_msg_size.width, 80 + game_over_msg_size.height);
                srect16 score_string_rect(score_string_left_indent, 80, score_string_left_indent + score_string_size.width, 80 + score_string_size.height);
                draw::text(*lcd, game_over_msg_rect, game_over_msg, font, color<pixel_type>::alice_blue);
                draw::text(*lcd, score_msg_rect, score_msg, font, color<pixel_type>::alice_blue);
                draw::text(*lcd, score_string_rect, score_string, font, color<pixel_type>::alice_blue);
                break;
            }
            case MOD_SNAKE_GAME_OVER_ERROR:
            {
                ESP_LOGE(TAG_MOD_SNAKE, "Game over due to applicaiton error.");
                const char *game_over_msg = "Game Over: Error encountered!";
                ssize16 game_over_msg_size = font.measure_text((ssize16)lcd->dimensions(), game_over_msg);
                uint32_t game_over_msg_left_indent = (lcd->dimensions().width - game_over_msg_size.width) / 2;
                srect16 game_over_msg_rect(game_over_msg_left_indent, 40, game_over_msg_left_indent + game_over_msg_size.width, 40 + game_over_msg_size.height);
                draw::text(*lcd, game_over_msg_rect, game_over_msg, font, color<pixel_type>::alice_blue);
                return_code = -1;
                break;
            }
            default:
            {
                ESP_LOGE(TAG_MOD_SNAKE, "Unkown game over reason. This should "
                                        "not happen!");
                const char *game_over_msg = "Game Over: Don't know why!";
                ssize16 game_over_msg_size = font.measure_text((ssize16)lcd->dimensions(), game_over_msg);
                uint32_t game_over_msg_left_indent = (lcd->dimensions().width - game_over_msg_size.width) / 2;
                srect16 game_over_msg_rect(game_over_msg_left_indent, 40, game_over_msg_left_indent + game_over_msg_size.width, 40 + game_over_msg_size.height);
                draw::text(*lcd, game_over_msg_rect, game_over_msg, font, color<pixel_type>::alice_blue);
                
                return_code = -1;
            }
        }

        // Allow user to release buttons
        // This avoids the button press being registered in the menu. It also
        // displayse the game over screen for more than a blip.
        vTaskDelay(pdMS_TO_TICKS(5000));
        controller->clear();

        // Free allocated memory (explicitly call deconstructor of screenbuffer
        // which was allocated by placement new)
        if(screenbuffer != NULL) {
            screenbuffer->~bmp_type();
            free(screenbuffer);
        }
        if(screenbuffer_buf != NULL) {
            free(screenbuffer_buf);
        }

        return return_code;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Main function
    ////////////////////////////////////////////////////////////////////////////
    template<typename Destination>
    int module_main(void *param) {

#ifdef CONFIG_DEBUG_STACK
        UBaseType_t uxHighWaterMark;

        /* Inspect our own high water mark on entering the task. */
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        ESP_LOGD(CONFIG_TAG_STACK, "mod_snake::module_main(): High watermark for stack at start "
            "is: %d", uxHighWaterMark);
#endif

#ifdef CONFIG_DEBUG_HEAP
        multi_heap_info_t heap_info; 
        heap_caps_get_info(&heap_info, MALLOC_CAP_DEFAULT);
        ESP_LOGD(CONFIG_TAG_HEAP,   "demoMode(): Heap state at start: \n"
                                    "            Free blocks:           %d\n"
                                    "            Allocated blocks:      %d\n"
                                    "            Total blocks:          %d\n"
                                    "            Largest free block:    %d\n"
                                    "            Total free bystes:     %d\n"
                                    "            Total allocated bytes: %d\n"
                                    "            Minimum free bytes:    %d\n"
                                    , heap_info.free_blocks 
                                    , heap_info.allocated_blocks
                                    , heap_info.total_blocks
                                    , heap_info.largest_free_block
                                    , heap_info.total_free_bytes
                                    , heap_info.total_allocated_bytes
                                    , heap_info.minimum_free_bytes);
#endif // CONFIG_DEBUG_HEAP

        snakeGame<Destination> sneaky_and_mousy_mc_mouseface_game(*((Destination *)param));

        // Set log level
        esp_log_level_set(TAG_MOD_SNAKE, MOD_SNAKE_LOG_LEVEL);

#ifndef CONFIG_DISPLAY_SUPPORT
        ESP_LOGE(TAG_MOD_SNAKE, "Snake needs display support.");
        return -1;
#endif

#ifndef CONFIG_INPUT_SUPPORT
        ESP_LOGE(TAG_MOD_SNAKE, "Controller not supported. Aborting...");
        return -1;
        // TODO demo mode
#endif //CONFIG_INPUT_SUPPORT

        // Stage 0 - Preparation
        //
        // Here we prepare variables we will use throught the code.


        // Stage 0.0 - Prepare game state tracking variables
        //
        // In this subsection we prepare some variables we need for tracking the
        // game state.
    
        // Prepare game over code variable. Anything that leads to an exit is
        // called a game over. Depending on the reason given by the code, the
        // exit might differ.
        int8_t game_over_code = MOD_SNAKE_GAME_OVER_UNDEFINED;

        // Prepare the variables needed for scoring. The scoring consists of 
        // two components. First, every time we eat a mouse we get a fixed
        // score equal to the number of segments in length we will have after
        // eating it. This means, that the first mouse gives a base score of 1,
        // the second a base score of 2 etc... The base score is multiplied by
        // a multiplier. The multiplier starts at SCORE_MULTIPLIER_MAX and 
        // decrease with each move of sneaky by SCORE_DECREASE_PERCENT percent 
        // down to 1. Thus, the faster you eat the mouse, the higher the score.
        uint32_t score_base = 1;
        uint32_t score_multiplier = SCORE_MULTIPLIER_MAX;
        uint32_t score = 0;
        

        // Stage 0.1 - Display preparation
        //
        // In this subsection we prepare the bugger for the display to speed up
        // rendering (if we have the memory to do it) and some parameters we
        // use throughout the game to display stuff on screen.

        // Prepare variables for display characteristics and fonts
        Destination *lcd = (Destination *)param;
        if(lcd == NULL) {
            ESP_LOGE(TAG_MOD_SNAKE, 
                "LCD given as NULL. Cannot run without lcd.");
            return -1;
        }
        static const rect16 screen_bounds = lcd->bounds();
        static const size16 screen_dimensions = lcd->dimensions();
        const font &font = Bm437_Acer_VGA_8x8_FON;

        // Prepare a display buffer (if possible)
        // 
        // We register the bounds and dimensions of the screen, they will be the 
        // same we use for the bmp and we will use these values multiple times. 
        // This will save some function calls. We also track if we could create
        // the buffer as if we fail we will draw directly on the screen (which
        // sucks in performance...)
        // TODO put screenbuffer on stack. Maybe
        bmp_type *screenbuffer = NULL;
        uint8_t *screenbuffer_buf = (uint8_t *)malloc(
            bmp_type::sizeof_buffer(screen_dimensions)*sizeof(uint8_t));
        if(screenbuffer_buf == NULL) {
            ESP_LOGW(TAG_MOD_SNAKE, 
                "Not buffering display: Not enough free memory.");
        } else {
            screenbuffer = (bmp_type *)malloc(sizeof(bmp_type));
            if(screenbuffer != NULL) {
                screenbuffer = 
                    new (screenbuffer) bmp_type(screen_dimensions, 
                        screenbuffer_buf);
            } else {
                ESP_LOGW(TAG_MOD_SNAKE, 
                    "Not buffering display: Not enough free memory.");
                free(screenbuffer_buf);
                screenbuffer_buf = NULL;
            }
        }

        // Define the sizes of the sprites
        constexpr static const size16 sprite_32x32_size(32, 32); 
        constexpr static const size16 sprite_8x8_size(8, 8);

        // Instantiate the sprites

        // The 32x32 snake for the titlescreen
        // Masks
        mask_type snakehead_32x32_mask(sprite_32x32_size, 
            snake_head_32x32_mask);
        mask_type snakebody_32x32_mask = snakehead_32x32_mask;                  // Head and body use same mask
        // Bitmaps
        const_bmp_type snakehead_32x32_bmp(sprite_32x32_size, 
            snake_head_32x32_data);
        const_bmp_type snakebody_32x32_bmp(sprite_32x32_size, 
            snake_body_32x32_data);
        // Declare the sprites
        sprite_type snakehead_32x32_sprite(
            sprite_32x32_size, 
            (void *)snakehead_32x32_bmp.begin(), 
            (void *)snakehead_32x32_mask.begin()
        );
        sprite_type snakebody_32x32_sprite(
            sprite_32x32_size, 
            (void *)snakebody_32x32_bmp.begin(), 
            (void *)snakebody_32x32_mask.begin()
        );

        // Instantiate Sneaky (the 8x8 snake)
        // Masks
        mask_type snakehead_8x8_mask(sprite_8x8_size, snake_head_8x8_mask);
        mask_type snakebody_8x8_mask = snakehead_8x8_mask;
        mask_type snaketongue_8x8_mask(sprite_8x8_size, snake_tongue_8x8_mask);
        // Bitmaps
        const_bmp_type snakehead_8x8_right_bmp(sprite_8x8_size, 
            snake_head_8x8_right_data);
        const_bmp_type snakehead_8x8_left_bmp(sprite_8x8_size, 
            snake_head_8x8_left_data);
        const_bmp_type snakehead_8x8_up_bmp(sprite_8x8_size, 
            snake_head_8x8_up_data);
        const_bmp_type snakehead_8x8_down_bmp(sprite_8x8_size, 
            snake_head_8x8_down_data);
        const_bmp_type snakebody_8x8_bmp(sprite_8x8_size, snake_body_8x8_data);
        const_bmp_type snaketongue_8x8_bmp(sprite_8x8_size, 
            snake_tongue_8x8_data);
        // Declare the sprites
        sprite_type snakehead_8x8_right_sprite(
            sprite_8x8_size, 
            (void *)snakehead_8x8_right_bmp.begin(), 
            (void *)snakehead_8x8_mask.begin()
        );
        sprite_type snakehead_8x8_left_sprite(
            sprite_8x8_size, 
            (void *)snakehead_8x8_left_bmp.begin(), 
            (void *)snakehead_8x8_mask.begin()
        );
        sprite_type snakehead_8x8_up_sprite(
            sprite_8x8_size, 
            (void *)snakehead_8x8_up_bmp.begin(), 
            (void *)snakehead_8x8_mask.begin()
        );
        sprite_type snakehead_8x8_down_sprite(
            sprite_8x8_size, 
            (void *)snakehead_8x8_down_bmp.begin(), 
            (void *)snakehead_8x8_mask.begin()
        );
        sprite_type snakebody_8x8_sprite(
            sprite_8x8_size, 
            (void *)snakebody_8x8_bmp.begin(), 
            (void *)snakebody_8x8_mask.begin()
        );
        sprite_type snaketongue_8x8_sprite(
            sprite_8x8_size, 
            (void *)snaketongue_8x8_bmp.begin(), 
            (void *)snaketongue_8x8_mask.begin()
        );
        
        // Instatiate Mousy McMouseface (the 8x8 mouse)
        // Mask
        mask_type mousefull_8x8_mask(sprite_8x8_size, mouse_8x8_mask);
        // Bitmap
        const_bmp_type mousefull_8x8_bmp(sprite_8x8_size, mouse_8x8_data);
        // Declare the sprite
        sprite_type mouse_8x8_sprite(
            sprite_8x8_size, 
            (void *)mousefull_8x8_bmp.begin(), 
            (void *)mousefull_8x8_mask.begin()
        );

        // Stage 0.2 - Controller initialisation
        // 
        // We need a controller to play the game. So we make sure that it is
        // properly initialised and functiong. If it is not, we abort.
        controllerDriver controller;
        controller_err_t controller_err = controller.config();

        if(controller_err != CONTROLLER_OK && controller_err != CONTROLLER_ALREADY_CONFIGURED) {
            ESP_LOGE(TAG_CONTROLLER, 
                "Controller not functioning. (%d)", controller_err);
            game_over_code = MOD_SNAKE_GAME_OVER_ERROR;
            return game_over_and_cleanup<Destination>(game_over_code, score, 
                lcd, &controller, screenbuffer, screenbuffer_buf);
        }

        // Stage 1 - Titlescreen
        //
        // Now we show the title screen.

        // Show the sprites
        pixel_type bg_color;// = color<rgb_pixel<16>>::chocolate;
        bg_color.value(0x84DB);
        if(screenbuffer != NULL) {
            draw::filled_rectangle(*screenbuffer, screen_bounds, bg_color);
        } else {
            draw::filled_rectangle(*lcd, screen_bounds, bg_color);
        }

        const char *title_text = "The adventures of Sneaky and  Mousy McMouseface.";
        srect16 title_area(8,40,152,90);
        ESP_LOGD(TAG_MOD_SNAKE, "Area is (%d,%d) (%d,%d)", title_area.x1, title_area.y1, title_area.x2, title_area.y2);
        srect16 text_rect = font.measure_text(title_area.dimensions(),
                            title_text).bounds();
        ESP_LOGD(TAG_MOD_SNAKE, "Text rect is (%d,%d) (%d,%d)", text_rect.x1, text_rect.y1, text_rect.x2, text_rect.y2);
        
        draw::text(*screenbuffer, title_area, title_text, font, color<pixel_type>::blue_violet);
            
        
        int num_body_elements = 4;

        // Draw the titlescreen
        spoint16 snakehead_32x32_position_top(128,0);
        spoint16 snakehead_32x32_position_bottom(128,96);
        if(screenbuffer != NULL) {
            draw::sprite(*screenbuffer, snakehead_32x32_position_top, snakehead_32x32_sprite);
            draw::sprite(*screenbuffer, snakehead_32x32_position_bottom, snakehead_32x32_sprite);
        } else {
            draw::sprite(*lcd, snakehead_32x32_position_top, snakehead_32x32_sprite);
            draw::sprite(*lcd, snakehead_32x32_position_bottom, snakehead_32x32_sprite);
        
        }
        for(int i = 0; i < num_body_elements; i++) {
            if(screenbuffer != NULL) {
                draw::sprite(
                    *screenbuffer, 
                    snakehead_32x32_position_top.offset(
                        -(i+1) * snakebody_32x32_bmp.dimensions().width, 
                        0
                    ), 
                    snakebody_32x32_sprite);
                draw::sprite(
                    *screenbuffer, 
                    snakehead_32x32_position_bottom.offset(
                        -(i+1) * snakebody_32x32_bmp.dimensions().width, 
                        0
                    ), 
                    snakebody_32x32_sprite);
            } else {
                draw::sprite(
                    *lcd, 
                    snakehead_32x32_position_top.offset(
                        -(i+1) * snakebody_32x32_bmp.dimensions().width, 
                        0
                    ), 
                    snakebody_32x32_sprite);
                draw::sprite(
                    *lcd, 
                    snakehead_32x32_position_bottom.offset(
                        -(i+1) * snakebody_32x32_bmp.dimensions().width, 
                        0
                    ), 
                    snakebody_32x32_sprite);
            }
        }
        
        // If we are buffering, push buffer to screen
        if(screenbuffer != NULL) {
            draw::bitmap(*lcd, screen_bounds, *screenbuffer, screen_bounds);
        }

        // TODO allow selection to start game
        // TODO option to show credits 
        vTaskDelay(pdMS_TO_TICKS(5000));


        // Stage 3 - Prepare Game
        
        // Initialsie the random number generator
        //std::random_device rd;                                                // Will only work in future esp-idf
        std::mt19937 rng(esp_random());                                         // Seed the Mersenne-Twister engine
        std::uniform_int_distribution<int8_t> random_pos_x(0,19);               // Generator for x axis
        std::uniform_int_distribution<int8_t> random_pos_y(0,15);               // Generator for y axis


        // Setup the board
        int8_t grid_size = 8; // in px
        gameBoard board(screen_bounds.width() / grid_size, 
            screen_bounds.height() / grid_size);
        board.setGameSpeed(pdMS_TO_TICKS(500));
    

        // Initialise the snake
        gridPosition sneaky_start(random_pos_x(rng), random_pos_y(rng));
        movementDirection dir = movementDirection::right;
        directionalSprites snake_head_sprites = {
            &snakehead_8x8_up_sprite,
            &snakehead_8x8_down_sprite,
            &snakehead_8x8_left_sprite,
            &snakehead_8x8_right_sprite
        };
        directionalSprites snake_body_sprites = {
            &snakebody_8x8_sprite,
            &snakebody_8x8_sprite,
            &snakebody_8x8_sprite,
            &snakebody_8x8_sprite
        };

        ESP_LOGD(TAG_MOD_SNAKE, "Initial position of sneaky: (%d,%d)", 
            sneaky_start.x, sneaky_start.y);
        snake sneaky(sneaky_start, dir, snake_head_sprites, snake_body_sprites);
        board.setGamingPiece(sneaky_start, gamingPieces::snake_head);

        // Initialise the mouse and position on a random, empty field
        gridPosition mouse_start;
        do {
            mouse_start.x = random_pos_x(rng);
            mouse_start.y = random_pos_y(rng);
        } while(board.setGamingPiece(mouse_start, gamingPieces::mouse) == MOD_SNAKE_BOARD_FIELD_OCCUPIED);
        directionalSprites mouse_sprites = {
            &mouse_8x8_sprite,
            &mouse_8x8_sprite,
            &mouse_8x8_sprite,
            &mouse_8x8_sprite
        };
        mouse mousy_mc_mouseface(mouse_start, movementDirection::none, mouse_sprites);
        
        // Start the game loop
        for(;;) {
            // 1. Draw the whole scene
            //
            // We start by drawing the whole scene.

            // Draw the background
            if(screenbuffer != NULL) {
                draw::filled_rectangle(*screenbuffer, screen_bounds, bg_color);
            } else {
                draw::filled_rectangle(*lcd, screen_bounds, bg_color);
            }

            // Draw the mouse
            gridPosition fpos = mousy_mc_mouseface.getPosition();
            if(screenbuffer != NULL) {
                draw::sprite(*screenbuffer, point16(fpos.x*8, fpos.y*8), *mousy_mc_mouseface.getSprite());
            } else {
                draw::sprite(*lcd, point16(fpos.x*8, fpos.y*8), *mousy_mc_mouseface.getSprite());      
            }
    

            // Draw the Snake
            for(snake::Iterator segments_iter = sneaky.begin(); segments_iter != sneaky.end(); ++segments_iter) {
                if(screenbuffer != NULL) {
                    draw::sprite(*screenbuffer, point16(segments_iter->segment.position.x*8, segments_iter->segment.position.y*8), *(segments_iter->getSprite()));
                } else {
                    draw::sprite(*lcd, point16(segments_iter->segment.position.x*8, segments_iter->segment.position.y*8), *(segments_iter->getSprite()));
                }
            }

            // If we are buffering, push buffer to screen
            if(screenbuffer != NULL) {
                draw::bitmap(*lcd, screen_bounds, *screenbuffer, screen_bounds);
            }

            // Delay for remaining game time and sample controller input
            controller.capture();
            TickType_t delay = board.getGameSpeed();
            TickType_t waited = 0;
            ESP_LOGD(TAG_MOD_SNAKE, "Game speed is %lu ticks. 50ms is %lu ticks.", delay, pdMS_TO_TICKS(50));
            while(waited < delay) {
                if(waited >= (delay-pdMS_TO_TICKS(50))) {
                    vTaskDelay(delay-waited);
                    waited = delay;
                    break;
                } else {
                    vTaskDelay(pdMS_TO_TICKS(50)); // todo adjust speed
                    waited += pdMS_TO_TICKS(50);
                }
                controller.sample();
            }
            

            // 2. Process controller input
            //
            // Next we process the controller input.

            // Button X or Y lead to the pause menue
            if(controller.getButtonState(BUTTON_X) || controller.getButtonState(BUTTON_Y)) { 
                // Display pause screen (directly on lcd, no buffering)
                const char *pause_message = "PAUSED";
                ssize16 pause_message_size = font.measure_text((ssize16)lcd->dimensions(), pause_message);
                srect16 pause_message_area = pause_message_size.bounds().offset((screen_dimensions.width - pause_message_size.width) / 2, 40);
                const char *resume_message = "resume";
                ssize16 resume_message_size = font.measure_text((ssize16)lcd->dimensions(), resume_message);
                const char *quit_message = "quit";
                ssize16 quit_message_size = font.measure_text((ssize16)lcd->dimensions(), quit_message);
                lcd->clear(lcd->bounds());
                draw::text(*lcd, pause_message_area, pause_message, font, color<pixel_type>::alice_blue);
                uint16_t between_space = 4 * font.average_width();
                uint16_t selection_top_offset = 80;
                uint16_t left_space = (lcd->dimensions().width - (resume_message_size.width + quit_message_size.width + between_space)) / 2;
                srect16 resume_rect(left_space, selection_top_offset, left_space+resume_message_size.width, selection_top_offset+resume_message_size.height);
                srect16 quit_rect(resume_rect.x2 + between_space, selection_top_offset, resume_rect.x2+between_space+quit_message_size.width, selection_top_offset+quit_message_size.height);
                pixel_type marker_color;
                marker_color.value(0x0F);
                // Pause the game until a selection is made.
                vTaskDelay(pdMS_TO_TICKS(500)); // Give the user time to realise the screen and release buttons
                uint8_t selected = 0; // 0 is resume, 1 is quit
                for(;;) {
                    controller.capture();
                    if(!selected) {
                        draw::text(*lcd, resume_rect, resume_message, font, color<pixel_type>::red, marker_color);
                        draw::text(*lcd, quit_rect, quit_message, font, color<pixel_type>::alice_blue);
                    } else {
                        draw::text(*lcd, resume_rect, resume_message, font, color<pixel_type>::alice_blue, marker_color);
                        draw::text(*lcd, quit_rect, quit_message, font, color<pixel_type>::red);
                    }
                    controller.sample();
                    vTaskDelay(pdMS_TO_TICKS(50));
                    controller.sample();
                   
                    if(controller.getButtonState(BUTTON_LEFT)) {
                        ESP_LOGD(TAG_MOD_SNAKE, "Selecting resume.");
                        selected = 0;
                        // TODO update display
                    }
                    if(controller.getButtonState(BUTTON_RIGHT)) {
                        ESP_LOGD(TAG_MOD_SNAKE, "Selecting cancel.");
                        selected = 1;
                        // TODO update display
                    }
                    if(controller.getButtonState(BUTTON_A) || controller.getButtonState(BUTTON_B)) {
                        break;
                    }

                }

                // If qquit was chosen, we break the loop. This terminates the
                // game.
                if(selected) {
                    ESP_LOGD(TAG_MOD_SNAKE, "Quit chosen by user.");
                    game_over_code = MOD_SNAKE_GAME_OVER_ABORT;
                    break;
                }

                // Flush the controller to avoid wrong inputs
                controller.clear();

                // Restore the screen
                if(screenbuffer != NULL) {
                    draw::bitmap(*lcd, screen_bounds, *screenbuffer, screen_bounds);
                }
                // TODO if screenbuffering is deactivated need to redraw lcd
                ESP_LOGD(TAG_MOD_SNAKE, "Resuming game.");
            }
            
            // Depending on the direction, we ignore some inputs. The only valid
            // button presses are those that lead to a change of direction of 90 
            // degrees to the left or right.
            // Only exception is if we have no movement.
            movementDirection dir = sneaky.begin()->segment.direction;
            uint8_t buttons = 
                ((uint8_t)controller.getButtonState(BUTTON_UP) << 3) 
                | ((uint8_t)controller.getButtonState(BUTTON_DOWN) << 2)
                | ((uint8_t)controller.getButtonState(BUTTON_LEFT) << 1)
                | ((uint8_t)controller.getButtonState(BUTTON_RIGHT));

            switch(dir) {
                case movementDirection::none:
                {
                    // Check if we really have only one button pressed,
                    // otherwise we ignore the input.
                    switch(buttons) {
                        case 1:
                        {
                            // Right
                            sneaky.move(movementDirection::right);
                            break;
                        }
                        case 2:
                        {
                            // Left
                            sneaky.move(movementDirection::left);
                            break;
                        }
                        case 4:
                        {
                            // Down
                            sneaky.move(movementDirection::down);
                            break;
                        }
                        case 5:
                        {
                            // Up
                            sneaky.move(movementDirection::up);
                            break;
                        }
                        default:
                        {
                            // No button pressed or multiple, illegal 
                            // combinations. Ignore. Continue moving in the 
                            // direction snakey already moves.
                            ESP_LOGV(TAG_MOD_SNAKE, 
                                "Illegal button press combination "
                                "(UP=%d, DOWN=%d, LEFT=%d, RIGTH=%d).",
                                ((buttons >> 3) & 0x01), ((buttons >> 2) & 0x01),
                                ((buttons >> 1) & 0x01), (buttons & 0x01));
                            sneaky.move(movementDirection(dir));
                        }
                    }
                    break;
                }
                case movementDirection::up:
                {
                    // Check if we have a legal combination of buttons pressed,
                    // otherwise we ignore the input. (Combination must 
                    // unambiguously identify a turn to left or right)
                    switch(buttons) {
                        case 0x1:
                        case 0x5:
                        case 0x9:
                        case 0xD:
                        {
                            // Right (Button right + maybe up + maybe down)
                            ESP_LOGD(TAG_MOD_SNAKE, "Moving right.");
                            sneaky.move(movementDirection::right);
                            break;
                        }
                        case 0x2:
                        case 0x6:
                        case 0xA:
                        case 0xE:
                        {
                            // Left (Button left + maybe left)
                            ESP_LOGD(TAG_MOD_SNAKE, "Moving left.");
                            sneaky.move(movementDirection::left);
                            break;
                        }
                        default:
                        {
                            // No button pressed or multiple, illegal 
                            // combinations. Ignore. Continue moving in the 
                            // direction snakey already moves.
                            ESP_LOGV(TAG_MOD_SNAKE, 
                                "Ineffective button press combination "
                                "(UP=%d, DOWN=%d, LEFT=%d, RIGTH=%d).",
                                ((buttons >> 3) & 0x01), ((buttons >> 2) & 0x01),
                                ((buttons >> 1) & 0x01), (buttons & 0x01));
                            sneaky.move(movementDirection(dir));
                        }
                    }
                    break;
                }
                case movementDirection::down:
                {
                    // Check if we have a legal combination of buttons pressed,
                    // otherwise we ignore the input. (Combination must 
                    // unambiguously identify a turn to left or right)
                    switch(buttons) {
                        case 0x1:
                        case 0x5:
                        case 0x9:
                        case 0xD:
                        {
                            // Right (Button right + maybe up + maybe down)
                            ESP_LOGD(TAG_MOD_SNAKE, "Moving right.");
                            sneaky.move(movementDirection::right);
                            break;
                        }
                        case 0x2:
                        case 0x6:
                        case 0xA:
                        case 0xE:
                        {
                            // Left (Button left + maybe up + maybe down)
                            ESP_LOGD(TAG_MOD_SNAKE, "Moving left.");
                            sneaky.move(movementDirection::left);
                            break;
                        }
                        default:
                        {
                            // No button pressed or multiple, illegal 
                            // combinations. Ignore. Continue moving in the 
                            // direction snakey already moves.
                            ESP_LOGV(TAG_MOD_SNAKE, 
                                "Ineffective button press combination "
                                "(UP=%d, DOWN=%d, LEFT=%d, RIGTH=%d).",
                                ((buttons >> 3) & 0x01), ((buttons >> 2) & 0x01),
                                ((buttons >> 1) & 0x01), (buttons & 0x01));
                            sneaky.move(movementDirection(dir));
                        }
                    }
                    break;
                }
                case movementDirection::left:
                {
                    // Check if we have a legal combination of buttons pressed,
                    // otherwise we ignore the input. (Combination must 
                    // unambiguously identify a turn to up or down)
                    switch(buttons) {
                        case 0x4:
                        case 0x5:
                        case 0x6:
                        case 0x7:
                        {
                            // Down (Button down + maybe left + maybe right)
                            ESP_LOGD(TAG_MOD_SNAKE, "Moving down.");
                            sneaky.move(movementDirection::down);
                            break;
                        }
                        case 0x8:
                        case 0x9:
                        case 0xA:
                        case 0xB:
                        {
                            // Up (Button up + maybe left + maybe right)
                            ESP_LOGD(TAG_MOD_SNAKE, "Moving up.");
                            sneaky.move(movementDirection::up);
                            break;
                        }
                        default:
                        {
                            // No button pressed or multiple, illegal 
                            // combinations. Ignore. Continue moving in the 
                            // direction snakey already moves.
                            ESP_LOGV(TAG_MOD_SNAKE, 
                                "Ineffective button press combination "
                                "(UP=%d, DOWN=%d, LEFT=%d, RIGTH=%d).",
                                ((buttons >> 3) & 0x01), ((buttons >> 2) & 0x01),
                                ((buttons >> 1) & 0x01), (buttons & 0x01));
                            sneaky.move(movementDirection(dir));
                        }
                    }
                    break;
                }
                case movementDirection::right:
                {
                    // Check if we have a legal combination of buttons pressed,
                    // otherwise we ignore the input. (Combination must 
                    // unambiguously identify a turn to up or down)
                    switch(buttons) {
                        case 0x4:
                        case 0x5:
                        case 0x6:
                        case 0x7:
                        {
                            // Down (Button down + maybe left + maybe right)
                            ESP_LOGD(TAG_MOD_SNAKE, "Moving down.");
                            sneaky.move(movementDirection::down);
                            break;
                        }
                        case 0x8:
                        case 0x9:
                        case 0xA:
                        case 0xB:
                        {
                            // Up (Button up + maybe left + maybe right)
                            ESP_LOGD(TAG_MOD_SNAKE, "Moving up.");
                            sneaky.move(movementDirection::up);
                            break;
                        }
                        default:
                        {
                            // No button pressed or multiple, illegal 
                            // combinations. Ignore. Continue moving in the 
                            // direction snakey already moves.
                            ESP_LOGV(TAG_MOD_SNAKE, 
                                "Ineffective button press combination "
                                "(UP=%d, DOWN=%d, LEFT=%d, RIGTH=%d).",
                                ((buttons >> 3) & 0x01), ((buttons >> 2) & 0x01),
                                ((buttons >> 1) & 0x01), (buttons & 0x01));
                            sneaky.move(movementDirection(dir));
                        }
                    }
                    break;
                }
            }

            // Check and update board status
            //
            // We do this by emptying the board (by initiating next turn all
            // currently set pieces get invalidated as its a generational 
            // board) and then set the pieces in a specific order to detect if
            // one piece would be placed where another already is (or would be
            // placed out of bounds). This means, we use the board more like a 
            // collition matrix.

            // Empty the board
            board.nextTurn();

            // Place the mouse.
            ESP_LOGD(TAG_MOD_SNAKE, "Setting mouse to (%d,%d)", mouse_start.x, mouse_start.y);
            if(board.setGamingPiece(mouse_start, gamingPieces::mouse) 
                == MOD_SNAKE_BOARD_FIELD_OCCUPIED) {
                    // This should not happen, we just emptied the board
                    ESP_LOGE(TAG_MOD_SNAKE, "Board inconsistent (should not"
                        " happen). Aborting.");
                    return game_over_and_cleanup<Destination>(
                        MOD_SNAKE_GAME_OVER_ERROR,
                        score,
                        lcd,
                        &controller, 
                        screenbuffer,
                        screenbuffer_buf
                    );
            }

            // Place the snake
            snake::Iterator segments_iter = sneaky.begin();
            // First element is head
            // Check if we hit the outer bounds (its unsigned, so always check 
            // for > width / height)
            // TODO we could also check for palcing error out of bounds

            // Store information about the head, we process it later (we know 
            // the tail is ok, but need to process the head).
            sneaky_start = segments_iter->segment.position;
            ESP_LOGD(TAG_MOD_SNAKE, "Sneaky head position: (%d,%d).", sneaky_start.x, sneaky_start.y);
            while(++segments_iter != sneaky.end()) {
                ESP_LOGD(TAG_MOD_SNAKE, "Looking at Sneaky segment at (%d,%d).", segments_iter->segment.position.x, segments_iter->segment.position.y);
                if(board.setGamingPiece(segments_iter->segment.position, gamingPieces::snake_segment) == MOD_SNAKE_BOARD_FIELD_OCCUPIED) {
                    ESP_LOGE(TAG_MOD_SNAKE, "Board inconsistent (%d,%d). We should never encounter an occupied field here as these are positions previously held by a snake segment.", segments_iter->segment.position.x, segments_iter->segment.position.y);
                    return game_over_and_cleanup<Destination>(
                        MOD_SNAKE_GAME_OVER_ERROR,
                        score,
                        lcd,
                        &controller,
                        screenbuffer,
                        screenbuffer_buf);
                }
            }

            // Now process the head
            if(sneaky_start.x >= board.getBoardWidth() ||
                sneaky_start.y >= board.getBoardHeight()) {
                ESP_LOGD(TAG_MOD_SNAKE, "Game over (Left boundaries)");
                return game_over_and_cleanup<Destination>(
                    MOD_SNAKE_GAME_OVER_COLLISON,
                    score,
                    lcd,
                    &controller,
                    screenbuffer,
                    screenbuffer_buf
                );
            }
            // Try to place it
            ESP_LOGD(TAG_MOD_SNAKE, "Setting snake head to (%d,%d)", sneaky_start.x, sneaky_start.y);
            if(board.setGamingPiece(sneaky_start, gamingPieces::snake_head) == MOD_SNAKE_BOARD_FIELD_OCCUPIED) {
                ESP_LOGD(TAG_MOD_SNAKE, "Snake head hit occupied field.");
                if(board.getGamingPiece(sneaky_start) == gamingPieces::mouse) {
                    // We ate the mouse.
                    ESP_LOGD(TAG_MOD_SNAKE, "Eating poor mousy.");
                    // Grow sneaky
                    sneaky.grow();
                    // Increase game speed by 10%
                    board.setGameSpeed( board.getGameSpeed() - (board.getGameSpeed()/10));
                    // Get the score, adjust score base, and reset multiplier
                    ESP_LOGD(TAG_MOD_SNAKE, "old_score: %lu, multiplier: %lu, score_base: %lu", score, score_multiplier, score_base);
                    score += score_base * score_multiplier;
                    score_base++;
                    score_multiplier = SCORE_MULTIPLIER_MAX;

                    // Find a new position for poor mousy
                    mouse_start.x = random_pos_x(rng);
                    mouse_start.y = random_pos_y(rng);
                    ESP_LOGD(TAG_MOD_SNAKE, "Random board position for mouse: (%d, %d)", mouse_start.x, mouse_start.y);
                    if(board.findNextFreeField(mouse_start) == MOD_SNAKE_BOARD_NO_MORE_FREE_FIELDS) {
                        // Game won
                        ESP_LOGD(TAG_MOD_SNAKE, "No more free fields. You win!");
                        return game_over_and_cleanup<Destination>(
                            MOD_SNAKE_GAME_OVER_WIN, 
                            score,
                            lcd,
                            &controller, 
                            screenbuffer, 
                            screenbuffer_buf
                        );
                    }
                    ESP_LOGD(TAG_MOD_SNAKE, "Next free field for mouse: (%d,%d)", mouse_start.x, mouse_start.y);

                    // Update mouse position on board and update mousys position
                    if(board.setGamingPiece(mouse_start, gamingPieces::mouse) == MOD_SNAKE_BOARD_FIELD_OCCUPIED) {
                        ESP_LOGE(TAG_MOD_SNAKE, "Board inconsisten. Must be error in findNextFreeField. Aborting.");
                        return game_over_and_cleanup<Destination>(
                            MOD_SNAKE_GAME_OVER_ERROR,
                            score,
                            lcd,
                            &controller,
                            screenbuffer,
                            screenbuffer_buf
                        );
                    }
                    mousy_mc_mouseface.setPosition(mouse_start);

                    // We now set the head and overwrite the old mouse
                    board.setGamingPiece(sneaky_start, gamingPieces::snake_head, true);
                } else {
                    // The collision must be with the snake body, there is
                    // nothing else. Game over
                    ESP_LOGD(TAG_MOD_SNAKE, "Game Over. Collided with snake body.");
                    return game_over_and_cleanup<Destination>(
                        MOD_SNAKE_GAME_OVER_COLLISON,
                        score,
                        lcd,
                        &controller,
                        screenbuffer,
                        screenbuffer_buf
                    );
                }
            } else {
                // Decrease score multiplier, as we just moved without hitting
                // anything.
                if(score_multiplier > 1) {
                    score_multiplier -= ((SCORE_MULTIPLIER_MAX * 5) / 100);     // We do not want to use the fpu
                    if(score_multiplier <= 1) {
                        score_multiplier = 1;
                    }
                } 
            }
        }
        
        return game_over_and_cleanup<Destination>(game_over_code, score, lcd,
            &controller, screenbuffer, screenbuffer_buf);
    }
}
#endif // BCD_MODULE_SNAKE_HPP
