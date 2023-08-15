#include "snake.hpp"

bool bcd_mod_snake::gridPosition::operator== (const gridPosition& other) { 
    return (x == other.x) && (y == other.y); 
}

const char *bcd_mod_snake::gamingPieces::getName() {
    switch(_piece) {
        case gamingPieces::none:
            return "none";
            break;
        case gamingPieces::mouse:
            return "mouse";
            break;
        case gamingPieces::snake_segment:
            return "snake_segment";
            break;
        case gamingPieces::snake_head:
            return "snake_head";
            break;
        case gamingPieces::wall:
            return "wall";
            break;
    }

    // Will never be reached
    return "";    
}

////////////////////////////////////////////////////////////////////////////////
// Snake stuff implementation
////////////////////////////////////////////////////////////////////////////////
bcd_mod_snake::gameBoard::gameBoard(uint8_t width, uint8_t height) {
    _board = (boardField *)malloc((width * height)*sizeof(boardField));
    if(_board == NULL) {
        _lasterror = MOD_SNAKE_OUT_OF_MEMORY;
        _board_width = 0;
        _board_height = 0;
    } else {
        _board_width = width;
        _board_height = height;

        // Initialise the board to empty
        for(int x = 0; x < width; x++ ) {
            for(int y = 0; y < height; y++) {
                _board[_index(x,y)] = {
                    .piece = gamingPieces::none,
                    .timestamp = _turn
                };
            }
        }
    }
}

/**
 * @brief Clean up the gameBoard object
 * 
 * Frees the allocated memory of the game board.
 */
bcd_mod_snake::gameBoard::~gameBoard() {
    if(_board != NULL) {
        free(_board);
    }
}

void bcd_mod_snake::gameBoard::setGameSpeed(TickType_t speed) {
    _game_speed = speed;
}

uint8_t bcd_mod_snake::gameBoard::getBoardWidth() {
    return _board_width;
}

uint8_t bcd_mod_snake::gameBoard::getBoardHeight() {
    return _board_height;
}

TickType_t bcd_mod_snake::gameBoard::getGameSpeed() {
    return _game_speed;
}

bool bcd_mod_snake::gameBoard::isFieldOccupied(gridPosition pos) {
    if(pos.x >= _board_width || pos.y >= _board_height) {
        _lasterror = MOD_SNAKE_INDEX_OUT_OF_BOUNDS;
        return true;                                                            // Lets just claim its occupied as its not valid
    }
    bool occupied = !isFieldFree(pos);
    return occupied;
}

bool bcd_mod_snake::gameBoard::isFieldFree(gridPosition pos) {
    bool res = true;

    if(pos.x >= _board_width || pos.y >= _board_height) {
        _lasterror = MOD_SNAKE_INDEX_OUT_OF_BOUNDS;
        return false;                                                           // Lets just claim its occupied as its not valid
    }

    if(_board[_index(pos)].timestamp == _turn
        && _board[_index(pos)].piece != gamingPieces::none) {
            res = false;
    } 
    return res;
}

mod_snake_err_t bcd_mod_snake::gameBoard::setGamingPiece(gridPosition pos, gamingPieces piece, bool overwrite) {
    if(pos.x >= _board_width || pos.y >= _board_height) {
        _lasterror = MOD_SNAKE_INDEX_OUT_OF_BOUNDS;
        return _lasterror;
    }

    ESP_LOGV(TAG_MOD_SNAKE, "setGamingPiece: Looking at field (%d,%d / field index: %d). Filed parameters are: turn: %lu / timestamp: %lu / piece: %s).", pos.x, pos.y, _index(pos), _turn, _board[_index(pos)].timestamp, _board[_index(pos)].piece.getName());
    if(!overwrite && isFieldOccupied(pos)) {
        ESP_LOGV(TAG_MOD_SNAKE, "Field is occupied, not overwriting.");
        _lasterror = MOD_SNAKE_BOARD_FIELD_OCCUPIED;
        return _lasterror;
    } else {
        ESP_LOGV(TAG_MOD_SNAKE, "Field will be overwritten. Field is %s, overwrite is %s", isFieldOccupied(pos) ? "occupied" : "free", overwrite ? "set" : "not set");
    }
    _board[_index(pos)] = {
        .piece = piece,
        .timestamp = _turn
    };
    ESP_LOGV(TAG_MOD_SNAKE, "setGamingPiece: Set field (%d,%d / field index: %d) to new values: piece: %s, timestamp: %lu.", pos.x, pos.y, _index(pos), _board[_index(pos)].piece.getName(), _board[_index(pos)].timestamp);
    return MOD_SNAKE_OK;
}

bcd_mod_snake::gamingPieces bcd_mod_snake::gameBoard::getGamingPiece(gridPosition pos) {
    if(pos.x >= _board_width || pos.y >= _board_height) {
        _lasterror = MOD_SNAKE_INDEX_OUT_OF_BOUNDS;
        return gamingPieces::none;
    }
    return _board[_index(pos)].piece;
}

void bcd_mod_snake::gameBoard::nextTurn() {
    if(_turn == std::numeric_limits<uint32_t>::max()) {
        ESP_LOGV(TAG_MOD_SNAKE, "Turn counter wrapping.");
        // Need to wrap turn counter and clean board
        // Initialise the board to empty. We restart turn at 2 to be able to
        // move last generation of turn to 1
        for(int x = 0; x < _board_width; x++ ) {
            for(int y = 0; y < _board_height; y++) {
                if(_board[_index(x,y)].timestamp == _turn) {
                    // Givr it a turn of 1 to indicate last gen move
                    _board[_index(x,y)].timestamp = 1;
                } else {
                    // Give it a turn of 0 to indicate older generation turn.
                    _board[_index(x,y)].timestamp = 0;     
                }
            }
        }
        _turn = 2;
    } else {
        _turn++;
    }
}

void bcd_mod_snake::gameBoard::refreshField(gridPosition pos) {
    _board[_index(pos)].timestamp = _turn;
}

mod_snake_err_t bcd_mod_snake::gameBoard::findNextFreeField(gridPosition &pos) {
    gridPosition initialPosition = pos;
    while(isFieldOccupied(pos)) {
        if(pos.x < (_board_width-1)) {
            pos.x++;
        } else if(pos.y < (_board_height-1)) {
            pos.x = 0;
            pos.y++;
        } else {
            pos.x = 0;
            pos.y = 0;
        }
        if(pos == initialPosition) {
            // We could not find a free field
            return MOD_SNAKE_BOARD_NO_MORE_FREE_FIELDS;
        }
    }
    return MOD_SNAKE_OK;
}

mod_snake_err_t bcd_mod_snake::gameBoard::getLastError() {
    return _lasterror;
}

inline uint16_t bcd_mod_snake::gameBoard::_index(gridPosition pos) {
    return (pos.x + pos.y * _board_width);
}

inline uint16_t bcd_mod_snake::gameBoard::_index(uint8_t x, uint8_t y) {
    return (x + y * _board_width);
}

////////////////////////////////////////////////////////////////////////////////
// Snake stuff implementation
////////////////////////////////////////////////////////////////////////////////

sprite_type *bcd_mod_snake::snakeSegment::getSprite() {
    sprite_type *retval = NULL;
    switch(this->direction) {
        case movementDirection::none:
            retval = this->sprites.right;
            break;
        case movementDirection::up:
            retval = this->sprites.up;
            break;
        case movementDirection::down:
            retval = this->sprites.down;
            break;
        case movementDirection::left:
            retval = this->sprites.left;
            break;
        case movementDirection::right:
            retval = this->sprites.right;
            break;
    }

    return retval;
}

sprite_type *bcd_mod_snake::snakeSegmentChainElement::getSprite() {
    return segment.getSprite(); 
}

bcd_mod_snake::snake::snake(gridPosition pos, movementDirection dir, 
    directionalSprites head_sprites, directionalSprites segments_sprites) {

    _head.segment.position = pos;
    _head.segment.direction = dir;
    _head.segment.sprites = head_sprites;
    _head.previous = NULL;
    _head.next = &_tail;

    _segment_sprites = segments_sprites;
}

bcd_mod_snake::snake::~snake() {
    // Free all the snake segments (_head and _tail are not dynamically 
    // allocated and must not be freed).
    snakeSegmentChainElement *tmp = _head.next, *tmp2 = NULL;
    while(tmp != &_tail) {
        tmp2 = tmp->next;
        free(tmp);
        tmp = tmp2;
    }
}

void bcd_mod_snake::snake::move(movementDirection dir, int distance) {
    
    // Adjust direction of the head
    _head.segment.direction = dir;

    // If we need to add a segment, then we introduce a new segment
    if(_add_segment && dir != movementDirection::none) {

        // Create a new element between head and the next one, put it to the
        // position of the current head and give it the direction of movement
        snakeSegmentChainElement *tmp = (snakeSegmentChainElement *)malloc(sizeof(snakeSegmentChainElement));
        if(tmp != NULL) {
            tmp->segment.position = _head.segment.position;
            tmp->segment.direction = dir;
            tmp->segment.sprites = _segment_sprites;

            tmp->previous = &_head;
            tmp->next = _head.next;
            _head.next = tmp;
            tmp->next->previous = tmp;
            _num_segments++; // TODO maybe not needed 

            _add_segment = false;   
        } else {
            ESP_LOGE(TAG_MOD_SNAKE, "Could not grow snake. Out of memory.");
        }
    } else {

        // Move the last segment to the previous head position if there is
        // a segment and if some sort of movement occured
        if(_tail.previous != &_head && dir != movementDirection::none) {

            snakeSegmentChainElement *tmp = _tail.previous;
            tmp->segment.position = _head.segment.position;
            tmp->segment.direction = dir;

            if(tmp->previous != &_head) {

                // We have more than one segment, need to adjust tail and new 
                // last element
                tmp->previous->next = &_tail;       
                _tail.previous = tmp->previous;

                tmp->next = _head.next;
                tmp->previous = &_head;
                _head.next->previous = tmp;
                _head.next = tmp;
            }
        }
    }

    switch(dir) {
        case movementDirection::up:       
            // Adjust head to new position
            _head.segment.position.y -= distance;
            break;

        case movementDirection::down:
            _head.segment.position.y += distance;
            break;
  
        case movementDirection::left:
            _head.segment.position.x -= distance;
            break;

        case movementDirection::right:
            _head.segment.position.x += distance;
            break;
        case movementDirection::none:
            // not moving
            break;
    }
}

mod_snake_err_t bcd_mod_snake::snake::grow() {
    if(_add_segment) {
        return MOD_SNAKE_PENDING;
    }
    _add_segment = true;
    return MOD_SNAKE_OK;
}

bcd_mod_snake::gridPosition bcd_mod_snake::snake::getHeadPosition() {
    return _head.segment.position;
}

//////////////////////////////////
// Mouse stuff implementation
//////////////////////////////////
bcd_mod_snake::mouse::mouse(gridPosition pos, movementDirection dir, directionalSprites sprites) {
    _pos = pos;
    _dir = dir;
    _sprites = sprites;
}

void bcd_mod_snake::mouse::setPosition(gridPosition pos) {
    _pos = pos;
}

bcd_mod_snake::gridPosition bcd_mod_snake::mouse::getPosition() {
    return _pos;
}

sprite_type *bcd_mod_snake::mouse::getSprite() {
    sprite_type *retval = NULL;
    switch(_dir) {
        case movementDirection::none:
            retval = _sprites.right;
            break;
        case movementDirection::up:
            retval = _sprites.up;
            break;
        case movementDirection::down:
            retval = _sprites.down;
            break;
        case movementDirection::left:
            retval = _sprites.right;
            break;
        case movementDirection::right:
            retval = _sprites.right;
            break;
    }
    return retval;
}