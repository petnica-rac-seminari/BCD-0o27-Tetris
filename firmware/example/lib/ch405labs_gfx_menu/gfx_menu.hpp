/**
 * @file        gfx_menu.hpp
 * @author      Florian Schütz (Grabmoix)
 * @brief       Menus based on htcw gfx
 * @version     0.1
 * @date        2022-08-31
 * 
 * @copyright Copyright (c) 2022, Florian Schütz, released under MIT license
 * 
 * This library allows to easily create menus and submenus based on htcw gfx.
 * 
 * Remarks:
 *  - We use placement new, as embedded development often does not allow for
 *      exception handling. This requires some tricks around dealocation, so
 *      unless you know what you are doying, use destroy() to dealocate objects.
 *
 * @todo Document properly 
 */
#pragma once

#include <stdlib.h>
#include <string.h>
#include <new>
#include "esp_log.h"
#include "st7735_bcd.hpp"
#include "gfx.hpp"
//#include "gfx_cpp14.hpp"
#include "gfx_menu_default_font.h"
#include <vector>

using namespace espidf;
using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////

/** @typedef The error type for any led related error */
typedef int gfxmenu_err_t;

#define GFXMENU_FAIL                       -1   /**< Generic failure */
#define GFXMENU_OK                      0x000   /**< Success */
#define GFXMENU_NOT_INITIALISED         0x010   /**< Object is not properly initialised */
#define GFXMENU_PARTIALLY_FUNCTIONAL    0x011   /**< Only paritally functional */
#define GFXMENU_INVALID_ARG             0x012   /**< Parameter error */
#define GFXMENU_INVALID_STATE           0x013   /**< Invalid state reached */
#define GFXMENU_INVALID_TARGET          0x014   /**< Target of operation not valid */
#define GFXMENU_NO_MEM                  0x015   /**< Memory allocation failure */
#define GFXMENU_NO_SUBMENU              0x020   /**< Item is not a submenu */
#define GFXMENU_NO_PARRENT              0x021   /**< Item has no parrent */
#define GFXMENU_NO_MORE_ENTRIES         0x022   /**< No more entries after active entry */
#define GFXMENU_CURSOR_NOT_SET          0x023   /**< Cursor is not set */


////////////////////////////////////////////////////////////////////////////////
// Defaults
////////////////////////////////////////////////////////////////////////////////
#define DEFAULT_SPACING_BEFORE_ITEM         1
#define DEFAULT_SPACING_AFTER_ITEM          2
#define DEFAULT_MENU_INDICATOR              ">"


////////////////////////////////////////////////////////////////////////////////
// Debugging
////////////////////////////////////////////////////////////////////////////////
static const char TAG_GFXMENU[] = "GFXMENU";    /**< TAG for ESP_LOGX macro. */


////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////

/** @typedef The function that executes the action associated to the menuitem 
 * 
 * A menuitem triggers an action when selected. This action function points to
 * the executor for the action and must be provided by the user. 
 * 
 * @param param A pointer to the argument (or argument list) provided to the 
 *           function. 
 */
typedef int (*hMenuItemAction)(void *param);



namespace gfxmenu {

    typedef uint8_t itemcount_t;

    template<typename Destination, typename PixelType>
    class Entry {
        private:
            inline void init() {
                this->name = NULL;
                this->highlight = false;
                this->highlight_color = color<rgb_pixel<16>>::violet; 
            }

        protected:
            char *name;
            const gfx::font &font = gfx_menu_default_font_FON;
            bool highlight;
            ::gfx::rgb_pixel<16> highlight_color;

        public:
            Entry() {
                init();
            }
            Entry(const char *name) {
                init();
                setName(name);
            }
            ~Entry() {
                // Free the name 
                if(name != NULL) {
                    free(name);
                }
            }

            virtual void destroy() {
                ESP_LOGV(TAG_GFXMENU, "Destroying entry %s.\n", this->name);
                this->~Entry();
            }

            void setName(const char *name) {
                if(this->name != NULL) {
                    free(this->name);
                }
                if(name == NULL) {
                    this->name = NULL;
                } else {
                    this->name = (char *)malloc(strlen(name) + 1);
                    if(this->name != NULL) {
                        strcpy(this->name, name);
                    } else {
                        ESP_LOGE(TAG_GFXMENU, "Could not allocate memory to name menu entry %s\n", name);
                    }
                }
            }

            const char *getName() {
                return name;
            }

            void setHighlight(bool state) {
                this->highlight = state;
            }

            bool getHighlight() {
                return this->highlight;
            }

            void setHighlightColor(PixelType color) {
                this->highlight_color = color;
            }

            ssize16 getSize(const srect16 &area) {
                return font.measure_text((ssize16)area.dimensions(), name);
            }

            //template<typename Destination>
            virtual gfx_result draw(
                Destination &destination, 
                const srect16 &area,
                PixelType color, 
                PixelType backcolor=convert<::gfx::rgb_pixel<3>,PixelType>(::gfx::rgb_pixel<3>(0,0,0))) {
                    if(this->getHighlight()) {
                        return draw::text(destination, area, name, font, this->highlight_color);
                    } else {
                        return draw::text(destination, area, name, font, color);
                    }
            }
    };

    template<typename Destination, typename PixelType>
    class ActionItem : public Entry<Destination, PixelType> {
        private:
            hMenuItemAction action;
            void *action_param;
            bool dealloc = false;

            inline void init() {
                action = NULL;
                action_param = NULL;
                dealloc = false;
            }

        public:
            ActionItem() : Entry<Destination, PixelType>() {
                init();
            }
            ActionItem(const char *name) : Entry<Destination, PixelType>(name) {
                init();
            }
            ActionItem(const char *name, hMenuItemAction action, void *param) : Entry<Destination, PixelType>(name) {
                init();
                this->setAction(action, param);
            }
            ~ActionItem() {
                // TODO think about action and param. Those should either not be dynamically
                //      allocated or we leave it to the caller to dealocate or we could copy
                if(dealloc) {
                    free(this->action_param);
                }
            }

            virtual void destroy() {
                ESP_LOGV(TAG_GFXMENU, "Destroying ActionItem %s.\n", this->name);
                this->~ActionItem();
            }

            void setAction(hMenuItemAction action, void *param) {
                this->action = action;
                this->action_param = param;
            }

            void setAction(hMenuItemAction action, void *param, size_t param_size) {
                this->action = action;
                this->action_param = malloc(param_size);
                if(this->action_param != NULL) {
                    memcpy(this->action_param, param, param_size);
                    this->dealloc = true;
                }
                // TODO fail gracefully
            }

            int execute() {
                if(action == NULL) {
                    return -1; // No action registered
                }

                return action(action_param);
            }
    };

    
    /**
     * @brief Submenues that consist of different entries of type Menu::Entry
     * 
     * Limitiations:
     *  - Currently only single selection is supported.
     * 
     */
    template<typename Destination, typename PixelType>
    class Submenu : public Entry<Destination, PixelType> {
        private:
            gfxmenu_err_t status, lasterror;

            /*
                A tree like structure that represents a menu.
            */
            struct menuEntry {
                Entry<Destination, PixelType> *item;
                itemcount_t rank;
                menuEntry *parent;
                menuEntry *children;
                menuEntry *next;
                menuEntry *previous;
            };
            
            menuEntry *root, *selected, *prev_selected, *anchor;

            uint8_t spacing_before_item;
            uint8_t spacing_after_item;

            char *menu_indicator_before;
            char *menu_indicator_after;

            inline void init() {
                status = GFXMENU_OK;
                root = NULL;
                selected = NULL;
                prev_selected = NULL; 
                anchor = NULL;

                // Allocate space for cursor
                cursor = (Cursor *)malloc(sizeof(Cursor));
                if(cursor != NULL) {
                    cursor = new (cursor) Cursor(*this);
                } else {
                    ESP_LOGE(TAG_GFXMENU, "Could not allocate memory for cursor.");
                    status = GFXMENU_NO_MEM;
                    status = GFXMENU_NOT_INITIALISED;
                }

                spacing_before_item = DEFAULT_SPACING_BEFORE_ITEM;
                spacing_after_item = DEFAULT_SPACING_AFTER_ITEM;

                menu_indicator_before = NULL;
                menu_indicator_after = (char *)malloc(strlen(DEFAULT_MENU_INDICATOR) + 1);
                if(menu_indicator_after != NULL) {
                    strcpy(menu_indicator_after, DEFAULT_MENU_INDICATOR);
                } else {
                    // TODO set error states somehow
                    ESP_LOGE(TAG_GFXMENU, "Could not allocate memory for menu indicator.");
                    lasterror = GFXMENU_NO_MEM;
                    if(status == GFXMENU_OK) {
                        // Only update status if there was no previous error condition
                        status = GFXMENU_PARTIALLY_FUNCTIONAL;
                    }
                }
            }

            gfx_result drawFromAnchor(
                Destination &destination, 
                srect16 &area,
                PixelType color, 
                PixelType selected_color,
                PixelType backcolor=convert<::gfx::rgb_pixel<3>,PixelType>(::gfx::rgb_pixel<3>(0,0,0)),
                PixelType selected_backcolor=convert<::gfx::rgb_pixel<3>,PixelType>(::gfx::rgb_pixel<3>(0,0,0))) {
                
                gfx_result res = gfx_result::success;

                res = destination.clear(rect16(area));

                draw::suspend(destination); // TODO implement in driver and test. Move before clear
                menuEntry *item_to_draw = anchor;
                while(res == gfx_result::success && item_to_draw != NULL && area.y2 > area.y1 + spacing_before_item) {
                    area.y1 = area.y1 + spacing_before_item;
                    
                    //TODO remove color
                    res = item_to_draw->item->draw(destination, area, color);
                    
                    if(res != gfx_result::success) {
                        // Something went wrong, abort
                        draw::resume(destination);
                        return res;
                    }
                    area.y1 = area.y1 + item_to_draw->item->getSize(area).height + spacing_after_item; 
                    item_to_draw = item_to_draw->next;
                }
                draw::resume(destination);
                return res;
            }

        public:
            Submenu() : Entry<Destination, PixelType>() {
                init();
            }
            Submenu(const char *name) : Entry<Destination, PixelType>(name) {
                init();
            }
            ~Submenu() {
                // TODO free all dynamically allocated items
                if(menu_indicator_after != NULL) {
                    free(menu_indicator_after);
                }
                if(menu_indicator_before != NULL) {
                    free(menu_indicator_before);
                }
                if(cursor != NULL) {
                    cursor->destroy();
                    free(cursor);
                }

                menuEntry *temp = root;
                while(root != NULL) {
                    root = root->next;
                    temp->item->destroy();
                    free(temp);
                    temp = root;
                }

                //Parrent destructor is called automatically

            }

            gfxmenu_err_t getStatus() {
                return status;
            }

            gfxmenu_err_t getLastError() {
                return lasterror;
            }

            /**
             * @brief A cursor can be positioned on entries and manipulate them
             * 
             * A cursor can be positionen on entries in the submenu. It then
             * allows to manipulate entires (inserting new ones, etc...).
             * 
             */
            class Cursor {
                private:
                    menuEntry *position;
                    Submenu *menu;

                    Cursor();
        
                public:
                    Cursor(Submenu<Destination, PixelType> &menu) {
                        this->position = NULL;
                        this->menu = &menu;
                    }

                    ~Cursor() {}

                    void destroy() {
                        this->~Cursor();
                    }

                    /*----------------------------------------------------------
                     * Navigation
                     * -------------------------------------------------------*/

                    /**
                     * @brief Position cursor on first entry
                     * 
                     *
                     * @return GFXMENU_NO_MORE_ENTRIES if there are no entries,
                     *          GFXMENU_OK otherwise. 
                     */
                    gfxmenu_err_t first() {
                        if(menu->root == NULL) {
                            return GFXMENU_NO_MORE_ENTRIES;
                        }
                        position = menu->root;
                        return GFXMENU_OK;
                    }

                    /**
                     * @brief Position cursor on last entry
                     * 
                     *
                     * @return GFXMENU_NO_MORE_ENTRIES if there are no entries,
                     *          GFXMENU_OK otherwise. 
                     */
                    gfxmenu_err_t last() {
                        if(root == NULL) {
                            return GFXMENU_NO_MORE_ENTRIES;
                        }

                        // If we are already somewhere, start there. If not,
                        // start at first entry.
                        if(position == NULL) {
                            position = root;
                        }
                        
                        while(position->next != NULL) {
                            position = position->next;
                        }
                        return true;
                    }

                    /**
                     * @brief Position cursor on next entry
                     * 
                     *
                     * @return GFXMENU_NO_MORE_ENTRIES if the cursor is not yet
                     *          set, there are no entries or there is no entry
                     *          after the current cursor position,
                     *          GFXMENU_OK otherwise. 
                     */
                    gfxmenu_err_t next() {
                        if(position == NULL || position->next == NULL) {
                            return GFXMENU_NO_MORE_ENTRIES;
                        }
                        position = position->next;
                        return GFXMENU_OK;
                    }

                    /**
                     * @brief Position cursor on previous entry
                     * 
                     * @return GFXMENU_NO_MORE_ENTRIES if the cursor is not yet
                     *          set, there are no entries or there is no entry
                     *          before the current cursor position,
                     *          GFXMENU_OK otherwise. 
                     */
                    gfxmenu_err_t previous() {
                        if(position == NULL || position->previous == NULL) {
                            return GFXMENU_NO_MORE_ENTRIES;
                        }
                        position = position->previous;
                        return GFXMENU_OK;
                    }

                    /**
                     * @brief Enter a submenu 
                     * 
                     * Enters the submenu the cursor is pointing at if it is a 
                     * valid submenu structure (== it has one or more child 
                     * elements.)
                     *
                     * @return GFXMENU_NO_SUBMENU if the element the cursor is
                     *          at is not at an element that is a submenu.
                     *          GFXMENU_OK otherwise.
                     */
                    gfxmenu_err_t enter() {
                        if(position == NULL || position->children == NULL) {
                            return GFXMENU_NO_SUBMENU;
                        } else {
                            position = position->children;
                            return GFXMENU_OK;
                        }
                    }

                    /**
                     * @brief Leaves the current submenu
                     * 
                     * Leaves the current submenu and sets the cursor to the
                     * parrent element.
                     *
                     * @return GFXMENU_NO_PARRENT if we are at the root element 
                     *          or empty menu,
                     *          GFXMENU_OK otherwise.
                     */
                    gfxmenu_err_t leave() {
                        if(position == NULL || position->parent == NULL) {
                            return GFXMENU_NO_PARRENT;
                        } else {
                            position = position->parent;
                            return GFXMENU_OK;
                        }
                    }

                    /**
                     * @brief Return a pointer to the entry the cursor is at
                     * 
                     * @return A pointer to the entry or NULL if the cursor is
                     *          not set on a valid entry
                     */
                    const Entry<Destination, PixelType> *getEntry() {
                        Entry<Destination, PixelType> *e = NULL;
                        if(position != NULL) {
                            e = position->item;
                        }
                        return e;
                    }


                    /*----------------------------------------------------------
                     * Manipulation
                     *--------------------------------------------------------*/

                    /**
                     * @brief Inserts an entry at the cursor
                     * 
                     * The entry is inserted after the element the cursor
                     * currently points at. The cursor is set to the new element.
                     *
                     * @return 
                     *      GFXMENU_CURSOR_NOT_SET if the cursor is not set to 
                     *          an entry (or NULL if none present).
                     * 
                     *      GFXMENU_NO_MEM if space for the entry could not be
                     *          allocated.
                     * 
                     *      GFXMENU_OK on success
                     */
                    gfxmenu_err_t addEntry(Entry<Destination, PixelType> *new_entry) {
                        gfxmenu_err_t res = GFXMENU_OK;

                        if(menu->root == NULL && position != NULL) {
                            // Cursor is not set. Fail
                            ESP_LOGE(TAG_GFXMENU, "Cursor is in inconsistent state. Cannot add entry to submenu. Reseting cursor.");
                            position = NULL;
                            return GFXMENU_CURSOR_NOT_SET; // TODO better error
                        }

                        if(menu->root != NULL && position == NULL) {
                            // Cursor is not set. Fail
                            ESP_LOGD(TAG_GFXMENU, "Cursor is not set. Cannot add entry to submenu.");
                            return GFXMENU_CURSOR_NOT_SET; 
                        }
               
                        menuEntry *new_menu_entry = (menuEntry *)malloc(sizeof(menuEntry));
                        if(new_menu_entry == NULL) {
                            ESP_LOGE(TAG_GFXMENU, "Not enough memory to add entry to submenu.");
                            return GFXMENU_NO_MEM;
                        }
                        new_menu_entry->item = new_entry;

                        if(position == NULL && menu->root == NULL) {
                            // No entries there yet, so cursor canot be set.
                            // Make first entry and set cursor
                            ESP_LOGD(TAG_GFXMENU, "Adding first entry to submenu.");
                            new_menu_entry->next = NULL;
                            new_menu_entry->previous = NULL;
                            new_menu_entry->rank = 1;
                            position = new_menu_entry;
                            menu->root = position;
                        } else {
                            ESP_LOGD(TAG_GFXMENU, "Adding entry to submenu after cursor.");
                            menuEntry *temp = position->next;
                            position->next = new_menu_entry;
                            if(temp != NULL) {
                                temp->previous = new_menu_entry;
                            };
                            new_menu_entry->previous = position;
                            new_menu_entry->next = temp;
                            new_menu_entry->rank = position->rank + 1;
                            while(temp != NULL) {
                                temp->rank++;
                                temp = temp->next;
                            }
                            position = position->next;
                        }
                        return res;
                    }


                    /*----------------------------------------------------------
                     * Selection
                     *--------------------------------------------------------*/

                    /**
                     * @brief Select the entry at cursor position
                     * 
                     * Results in highlighting the selected entry when
                     * displaying it.
                     * 
                     * @return 
                     *      GFXMENU_INVALID_TARGET if cursor is not set to a
                     *          selectable entry.
                     * 
                     *      GFXMENU_OK otherwise.
                     */
                    gfxmenu_err_t selectEntry() {
                        if(position == NULL) {
                            return GFXMENU_INVALID_TARGET;
                        }
                        position->item->setHighlight(true);
                        return GFXMENU_OK;   
                    }

                    /**
                     * @brief Deselect the entry at cursor position
                     * 
                     * Results in not highlighting the selected entry when
                     * displaying it.
                     * 
                     * @return 
                     *      GFXMENU_INVALID_TARGET if cursor is not set to a
                     *          selectable entry.
                     * 
                     *      GFXMENU_OK otherwise.
                     */
                    gfxmenu_err_t deselectEntry() {
                        if(position == NULL) {
                            return GFXMENU_INVALID_TARGET;
                        }
                        position->item->setHighlight(false);
                        return GFXMENU_OK;  
                    }

                    /**
                     * @brief Toggle seleciton of the entry at cursor position
                     * 
                     * Results in changing the highlight to off/on if it was
                     * on/off the selected entry when displaying it.
                     * 
                     * @return 
                     *      GFXMENU_INVALID_TARGET if cursor is not set to a
                     *          selectable entry.
                     * 
                     *      GFXMENU_OK otherwise.
                     */
                    gfxmenu_err_t toggleEntrySelection() {
                        if(position == NULL) {
                            return GFXMENU_INVALID_TARGET;
                        }
                        position->item->setHighlight(!position->item->getHighlight());
                        return GFXMENU_OK;
                    }
            };
            friend class Cursor;
            Cursor *cursor;

            virtual void destroy() {
                ESP_LOGV(TAG_GFXMENU, "Destroying Submenu %s.\n", this->name);
                this->~Submenu();
            }

            // Formating
            void setSpacing(uint16_t before, uint16_t after) {
                this->spacing_before_item = before;
                this->spacing_after_item = after;
            }
            void setSpacingBefore(uint16_t spacing) {
                this->spacing_before_item = spacing;
            }
            void setSpacingAfter(uint16_t spacing) {
                this->spacing_after_item = spacing;
            }
            uint16_t getSpacingBefore() {
                return this->spacing_before_item;
            }
            uint16_t getSpacingAfter() {
                return this->spacing_after_item;
            }
            gfxmenu_err_t setMenuIndicator(const char *before, const char* after) {
                gfxmenu_err_t res = GFXMENU_OK;
                if(this->menu_indicator_before != NULL) {
                    free(this->menu_indicator_before);
                }
                if(before != NULL) {
                    this->menu_indicator_before = (char *)malloc(strlen(before)+1);
                    if(this->menu_indicator_before != NULL) {
                        strcpy(this->menu_indicator_before, before);
                    } else {
                        res = GFXMENU_NO_MEM;
                    }
                } else {
                    this->menu_indicator_before = NULL;
                }
            
                if(this->menu_indicator_after != NULL) {
                    free(this->menu_indicator_after);
                }
                if(after != NULL) {
                    this->menu_indicator_after = (char *)malloc(strlen(after)+1);
                    if(this->menu_indicator_after != NULL) {
                        strcpy(this->menu_indicator_after, after);
                    } else {
                        res = GFXMENU_NO_MEM;
                    }
                } else {
                    this->menu_indicator_after = NULL;
                }

                return res;
            }

            gfx_result draw(
                Destination &destination, 
                const srect16 &area,
                PixelType color, 
                PixelType backcolor=convert<::gfx::rgb_pixel<3>,PixelType>(::gfx::rgb_pixel<3>(0,0,0))) {
                    gfx_result res = gfx_result::success;
                    size_t name_rep_len = 0;
                    if(this->name != NULL) {
                        name_rep_len += strlen(this->name);
                    }
                    if(this->menu_indicator_before != NULL) {
                        name_rep_len += strlen(this->menu_indicator_before);
                    }
                    if(this->menu_indicator_after != NULL) {
                        name_rep_len += strlen(this->menu_indicator_after);
                    }
                    char *name_rep = (char *)malloc(name_rep_len + 1);
                    if(name_rep != NULL) {
                        name_rep[0] = '\0';
                        if(this->menu_indicator_before != NULL) {
                            strcat(name_rep, this->menu_indicator_before);
                        }
                        if(this->name != NULL) {
                            strcat(name_rep, this->name);
                        }
                        if(this->menu_indicator_after != NULL) {
                            strcat(name_rep, this->menu_indicator_after);
                        }
                        if(this->getHighlight()) {
                            return draw::text(destination, area, name_rep, this->font, this->highlight_color);
                        } else {
                            return draw::text(destination, area, name_rep, this->font, color);
                        }
                        free(name_rep);
                    } else {
                        // TODO - we could draw one after the other, but we are lazy and just fail.
                        res = gfx_result::out_of_memory;
                    }   
                    return res;
            }

            /**
             * @brief 
             * 
             * Caution: The function will overwrite the area. Do not pass a dynamically
             *          allocated reference until you store it somewhere to free it later.
             * 
             * @tparam Destination 
             * @tparam PixelType 
             * @param destination 
             * @param area 
             * @param color 
             * @param backcolor 
             * @return gfx_result 
             */
            gfx_result drawMenu(
                Destination &destination, 
                srect16 &area,
                PixelType color, 
                PixelType selected_color,
                PixelType backcolor=convert<::gfx::rgb_pixel<3>,PixelType>(::gfx::rgb_pixel<3>(0,0,0)),
                PixelType selected_backcolor=convert<::gfx::rgb_pixel<3>,PixelType>(::gfx::rgb_pixel<3>(0,0,0))) {

                const gfx::rect16 dst_bounds = destination.bounds();

                // First lets check if we have to draw anything
                if(root == NULL) {
                    // Clear the destination as menu is empty
                    return destination.clear(dst_bounds);
                }

                // TODO - maybe we should normalize area to ensure x2 >= x1, y2 >= y1

                // Check if we are moving up or down or not at all. For this we need to first
                // check if there is a menu item selected and if that is the case then we
                // need to check if there was an item selected before that to determine if
                // we are moving up or down (for the scrolling).
                // Once we determined the direction we determin where to start drawing and
                // how many items to draw.
                int16_t area_height = area.height();
                if(selected != NULL && prev_selected != NULL && selected != prev_selected) {
                    // We are moving, as there is a selected entry and previously there
                    // was another selected entry
                    if(selected->rank > prev_selected->rank) {
                        // Moving down
                        //
                        // Either we have a valid anchor (rank lower or equal to
                        // slected) and it still fits the screen, or we need to
                        // set a new anchor. In any case, we search back from the
                        // sleected item until we arrive either at the anchor or
                        // run out of space for drawing and set this element as
                        // new anchor.       
                        menuEntry *item_to_draw = selected;

                        int16_t total_height = -spacing_after_item; // Correct for last spacing TODO - verify
                        while(item_to_draw != anchor && total_height < area_height) {
                            total_height += 
                                spacing_before_item
                                + item_to_draw->item->getSize(area).height
                                + spacing_after_item;
                            item_to_draw = item_to_draw->previous;
                        }
                        // We either arrived at the anchor or our anchor now
                        // is out of reach and we need to set a new anchor
                        anchor = item_to_draw;
                    } else {
                        // Moving up
                        // 
                        if(anchor != NULL && anchor->rank <= selected->rank) {
                            // The anchor points to the entry where we started drawing last
                            // time. However, the selected could push the anchor
                            // off screen, so we need to calculate back to either
                            // verify the existing anchor or set a new one.
                            menuEntry *item_to_draw = selected;

                            int16_t total_height = -spacing_after_item; // Correct for last spacing TODO - verify
                            while(item_to_draw != anchor && total_height < area_height) {
                                total_height += 
                                    spacing_before_item
                                    + item_to_draw->item->getSize(area).height
                                    + spacing_after_item;
                                item_to_draw = item_to_draw->previous;
                            }
                            // We either arrived at the anchor or our anchor now
                            // is out of reach and we need to set a new anchor
                            anchor = item_to_draw;

                        } else if(anchor != NULL && anchor->rank > selected->rank) {
                            // We passed the anchor while moving up. The selected becomes
                            // the new anchor
                            anchor = selected;

                        }

                    }

                } else if(selected != NULL) {
                    // We are not moving, but we have something selected. The 
                    // anchor is either the existing anchor (if selected is on
                    // the screen or we calculate a new anchor.)
                    if(anchor != NULL && anchor->rank <= selected->rank) {
                        menuEntry *item_to_draw = selected;
                        int16_t total_height = -spacing_after_item; // Correct for last spacing TODO - verify
                        while(item_to_draw != anchor && total_height < area_height) {
                            total_height += 
                                spacing_before_item
                                + item_to_draw->item->getSize(area).height
                                + spacing_after_item;
                            item_to_draw = item_to_draw->previous;
                        }
                        // We either arrived at the anchor or an item below the
                        // anchor that still fits the screen. So its safe to 
                        // set the anchor to the current item to draw and then 
                        // draw the number of items calculated. If there remains
                        // height for drawing, we draw more items after the 
                        // selected until the screen is full.
                        anchor = item_to_draw;
                        
                    } else {
                        // No valid anchor, compute a new one such that the
                        // selected item is the last on the screen
                        anchor = selected;
                    }

                } else {
                    // We are not moving, nothing is selected. Set the anchor for drawing to 
                    // the first entry and draw as many elements as possible.
                    anchor = root;
                    
                }

                // Now we draw from the anchor
                return drawFromAnchor(destination, area, color, selected_color);
            }
    };

    /**
     * @brief Manages menus
     * 
     * TODO
     *  - Implement "rule of three" or even "rule of five"
     * 
     */
    template<typename Destination, typename PixelType>
    class MenuController {
        private:
            Entry<Destination, PixelType> *active_menu;
            Submenu<Destination, PixelType> main_menu;

            gfxmenu_err_t last_error;

        public:
            class Cursor {
                private:
                    Submenu<Destination, PixelType> *menu;      // The submenu which the element is a member of
                    std::vector<Submenu<Destination, PixelType>*> menu_stack;

                    bool softbuffering;

                    Cursor();
        
                public:
                    Cursor(Submenu<Destination, PixelType> &rmenu) {
                        softbuffering = false;
                        this->menu = &rmenu;
                    }

                    ~Cursor() {}

                    void destroy() {
                        this->~Cursor();
                    }

                    gfxmenu_err_t setToRoot() {
                        if(menu_stack.size() > 0) {
                            this->menu = menu_stack.front();
                            menu_stack.clear();
                        }
                        return GFXMENU_OK;
                    }

                    gfxmenu_err_t enter() {
                        gfxmenu_err_t err = GFXMENU_OK;
                        const Entry<Destination, PixelType> *e = menu->cursor->getEntry();
                        if (e != NULL && typeid(*e) == typeid(Submenu<Destination, PixelType>)) {
                            this->menu_stack.push_back(this->menu);
                            this->menu = (Submenu<Destination, PixelType> *)const_cast<gfxmenu::Entry<Destination, PixelType>*>(e);
                        } else {
                            err = GFXMENU_NO_SUBMENU;
                        }
                        return err;
                    }

                    gfxmenu_err_t leave() {
                        gfxmenu_err_t err = GFXMENU_OK;
                        if(menu_stack.size() > 0) {
                            this->menu = menu_stack.back();
                            this->menu_stack.pop_back();
                        } else {
                            err = GFXMENU_NO_PARRENT;
                        }
                        return err;
                    }

                    gfxmenu_err_t first() {
                        return menu->cursor->first();
                    }

                    gfxmenu_err_t last() {
                        return menu->cursor->last();
                    }

                    gfxmenu_err_t next() {
                        return menu->cursor->next();
                    }

                    gfxmenu_err_t previous() {
                        return menu->cursor->previous();
                    }

                    /**
                     * @brief Return a pointer to the entry the cursor is at
                     * 
                     * @return A pointer to the entry or NULL if the cursor is
                     *          not set on a valid entry
                     */
                    const Entry<Destination, PixelType> *getEntry() {
                        return menu->cursor->getEntry();
                    }


                    /*----------------------------------------------------------
                     * Manipulation
                     *--------------------------------------------------------*/

                    /**
                     * @brief Inserts an entry at the cursor
                     * 
                     * The entry is inserted after the element the cursor
                     * currently points at. The cursor is set to the new element.
                     *
                     * @return 
                     *      GFXMENU_CURSOR_NOT_SET if the cursor is not set to 
                     *          an entry (or NULL if none present).
                     * 
                     *      GFXMENU_NO_MEM if space for the entry could not be
                     *          allocated.
                     * 
                     *      GFXMENU_OK on success
                     */
                    gfxmenu_err_t addEntry(Entry<Destination, PixelType> *new_entry) {
                        return menu->cursor->addEntry(new_entry);
                    }


                    /*----------------------------------------------------------
                     * Selection
                     *--------------------------------------------------------*/

                    /**
                     * @brief Select the entry at cursor position
                     * 
                     * Results in highlighting the selected entry when
                     * displaying it.
                     * 
                     * @return 
                     *      GFXMENU_INVALID_TARGET if cursor is not set to a
                     *          selectable entry.
                     * 
                     *      GFXMENU_OK otherwise.
                     */
                    gfxmenu_err_t selectEntry() {
                        return menu->cursor->selectEntry(); 
                    }

                    /**
                     * @brief Deselect the entry at cursor position
                     * 
                     * Results in not highlighting the selected entry when
                     * displaying it.
                     * 
                     * @return 
                     *      GFXMENU_INVALID_TARGET if cursor is not set to a
                     *          selectable entry.
                     * 
                     *      GFXMENU_OK otherwise.
                     */
                    gfxmenu_err_t deselectEntry() {
                        return menu->cursor->deselectEntry(); 
                    }

                    /**
                     * @brief Toggle seleciton of the entry at cursor position
                     * 
                     * Results in changing the highlight to off/on if it was
                     * on/off the selected entry when displaying it.
                     * 
                     * @return 
                     *      GFXMENU_INVALID_TARGET if cursor is not set to a
                     *          selectable entry.
                     * 
                     *      GFXMENU_OK otherwise.
                     */
                    gfxmenu_err_t toggleEntrySelection() {
                        return menu->cursor->toggleEntrySelection();
                    }

                    /*----------------------------------------------------------
                     * Drawing
                     *--------------------------------------------------------*/

                    /**
                     * @brief Draws the enclosing menu of the cursor.
                     * 
                     * Caution: The function will overwrite the area. Do not pass a dynamically
                     *          allocated reference until you store it somewhere to free it later.
                     * 
                     * @tparam Destination 
                     * @tparam PixelType 
                     * @param destination 
                     * @param area 
                     * @param color 
                     * @param backcolor 
                     * @return gfx_result 
                     */
                    gfx_result drawMenu(
                        Destination &destination, 
                        srect16 &area,
                        PixelType color, 
                        PixelType selected_color,
                        PixelType backcolor=convert<::gfx::rgb_pixel<3>,PixelType>(::gfx::rgb_pixel<3>(0,0,0)),
                        PixelType selected_backcolor=convert<::gfx::rgb_pixel<3>,PixelType>(::gfx::rgb_pixel<3>(0,0,0))) {

                        return menu->drawMenu(destination, area, color, selected_color, backcolor, selected_backcolor);

                    }

                    /**
                     * @brief Draws the element at the cursor.
                     * 
                     * Caution: The function will overwrite the area. Do not pass a dynamically
                     *          allocated reference until you store it somewhere to free it later.
                     * 
                     * @tparam Destination 
                     * @tparam PixelType 
                     * @param destination 
                     * @param area 
                     * @param color 
                     * @param backcolor 
                     * @return gfx_result 
                     */
                    gfx_result draw(
                        Destination &destination, 
                        srect16 &area,
                        PixelType color, 
                        PixelType backcolor=convert<::gfx::rgb_pixel<3>,PixelType>(::gfx::rgb_pixel<3>(0,0,0))) {

                        return menu->cursor->getEntry()->draw(destination, area, color, backcolor);
                    }

                    
            };
            friend class Cursor;
            Cursor *cursor;

            MenuController() {
                active_menu = NULL;
                last_error = GFXMENU_OK;
                main_menu.setSpacingBefore(1U);
                main_menu.setSpacingAfter(2U);
                main_menu.setName("Main Menu");
                main_menu.setHighlightColor(convert<::gfx::rgb_pixel<16>,PixelType>(color<rgb_pixel<16>>::violet));

                cursor = (gfxmenu::MenuController<Destination, PixelType>::Cursor *)malloc(sizeof(gfxmenu::MenuController<Destination, PixelType>::Cursor));
                if(cursor != NULL) {
                    cursor = new (cursor) Cursor(main_menu);
                } else {
                    ESP_LOGE(TAG_GFXMENU, "Could not allocate memory for cursor.");
                    last_error = GFXMENU_NO_MEM;
                    last_error = GFXMENU_NOT_INITIALISED;
                }

            }
            ~MenuController() {
                if(cursor != NULL) {
                    cursor->~Cursor();
                    free(cursor);
                }
            }

            gfxmenu_err_t getLastError() {
                return last_error;
            }

            ActionItem<Destination, PixelType> *createActionItem(const char *name, hMenuItemAction action, void *default_param) {
                last_error = GFXMENU_OK;
                // Allocate new menuitem
                void *new_entry_buf = (void *)malloc(sizeof(ActionItem<Destination, PixelType>));
                if(new_entry_buf == NULL) {
                    ESP_LOGE(TAG_GFXMENU, "Failed to allocate memory for menu item name.");
                    last_error = GFXMENU_NO_MEM;
                    return NULL;
                }
                ActionItem<Destination, PixelType> *new_entry = new (new_entry_buf) ActionItem<Destination, PixelType>(name, action, default_param);
                
                return new_entry;
            }

            ActionItem<Destination, PixelType> *createActionItem(const char *name, hMenuItemAction action, void *default_param, size_t param_size) {
                last_error = GFXMENU_OK;
                // Allocate new menuitem
                void *new_entry_buf = (void *)malloc(sizeof(ActionItem<Destination, PixelType>));
                if(new_entry_buf == NULL) {
                    ESP_LOGE(TAG_GFXMENU, "Failed to allocate memory for menu item name.");
                    last_error = GFXMENU_NO_MEM;
                    return NULL;
                }
                ActionItem<Destination, PixelType> *new_entry = new (new_entry_buf) ActionItem<Destination, PixelType>(name);
                new_entry->setAction(action, default_param, param_size);
                
                return new_entry;
            }

            Submenu<Destination, PixelType> *createSubmenu(const char *name) {
                last_error = GFXMENU_OK;
                // Allocate new menuitem
                void *new_entry_buf = (void *)malloc(sizeof(Submenu<Destination, PixelType>));
                if(new_entry_buf == NULL) {
                    ESP_LOGE(TAG_GFXMENU, "Failed to allocate memory for menu item name.");
                    last_error = GFXMENU_NO_MEM;
                    return NULL;
                }
                Submenu<Destination, PixelType> *new_entry = new (new_entry_buf) Submenu<Destination, PixelType>(name);
                
                return new_entry;
            }      

            gfxmenu_err_t enableSoftwareBuffering() {
                if(cursor != NULL) {
                    cursor->softbuffering = true;
                } else {
                    return GFXMENU_NOT_INITIALISED;
                }
                return GFXMENU_OK;
            }   

            gfxmenu_err_t disableSoftwareBuffering() {
                if(cursor != NULL) {
                    cursor->softbuffering = false;
                } else {
                    return GFXMENU_NOT_INITIALISED;
                }
                return GFXMENU_OK;
            }

               
    };
}
