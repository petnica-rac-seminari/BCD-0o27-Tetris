#include <FreeRTOSConfig.h>
#include <freertos/portmacro.h>
#include <freertos/projdefs.h>
#include <array>
namespace tetrics_module
{
        class board
        {
        public:
                void start();
                bool frame(TickType_t currTick);
                void clear();
                void rotate();
                void moveLeft();
                void moveRight();
                void moveDown();
                void updateScore(int increase);

                const int width = 10;
                const int height = 22;
                std::array<std::array<int, 22>, 10> board = {};

        private:
                using piece = std::array<std::array<int, 4>, 4>;
                int currentRotation; // has a value of 0, 1, 2 or 3 depending on the rotation of the figure

                piece I_shape[4] = {
                    piece({{{0, 0, -1, 0},
                            {0, 0, -1, 0},
                            {0, 0, -1, 0},
                            {0, 0, -1, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {-1, -1, -1, -1},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, -1, 0},
                            {0, 0, -1, 0},
                            {0, 0, -1, 0},
                            {0, 0, -1, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {-1, -1, -1, -1},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}}})};

                piece L_shape[4] = {
                    piece({{{0, -1, 0, 0},
                            {0, -1, 0, 0},
                            {0, -1, -1, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, 0, -1, 0},
                            {-1, -1, -1, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, -1, 0},
                            {0, 0, -1, 0},
                            {0, 0, -1, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, -1, -1},
                            {0, -1, 0, 0},
                            {0, 0, 0, 0}}})};

                piece J_shape[4] = {
                    piece({{{0, 0, -1, 0},
                            {0, 0, -1, 0},
                            {0, -1, -1, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {-1, -1, -1, 0},
                            {0, 0, -1, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, -1, 0},
                            {0, -1, 0, 0},
                            {0, -1, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, 0, 0},
                            {0, -1, -1, -1},
                            {0, 0, 0, 0}}})};

                piece Z_shape[4] = {
                    piece({{{0, 0, 0, 0},
                            {-1, -1, 0, 0},
                            {0, -1, -1, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, 0, -1, 0},
                            {0, -1, -1, 0},
                            {0, -1, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, -1, 0},
                            {0, 0, -1, -1},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, -1, 0},
                            {0, -1, -1, 0},
                            {0, -1, 0, 0},
                            {0, 0, 0, 0}}})};

                piece T_shape[4] = {
                    piece({{{0, 0, 0, 0},
                            {-1, -1, -1, 0},
                            {0, -1, 0, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, 0, 0},
                            {0, -1, -1, 0},
                            {0, -1, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, 0, -1, 0},
                            {0, -1, -1, -1},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, -1, 0},
                            {0, -1, -1, 0},
                            {0, 0, -1, 0},
                            {0, 0, 0, 0}}})};

                piece S_shape[4] = {
                    piece({{{0, 0, 0, 0},
                            {0, -1, -1, 0},
                            {-1, -1, 0, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, 0, 0},
                            {0, -1, -1, 0},
                            {0, 0, -1, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, 0, -1, -1},
                            {0, -1, -1, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, -1, 0, 0},
                            {0, -1, -1, 0},
                            {0, 0, -1, 0},
                            {0, 0, 0, 0}}})};

                piece O_shape[4] = {
                    piece({{{0, 0, 0, 0},
                            {0, -1, -1, 0},
                            {0, -1, -1, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, -1, 0},
                            {0, -1, -1, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, -1, 0},
                            {0, -1, -1, 0},
                            {0, 0, 0, 0}}}),
                    piece({{{0, 0, 0, 0},
                            {0, -1, -1, 0},
                            {0, -1, -1, 0},
                            {0, 0, 0, 0}}})};

                int inc;
                int speedUp = 5;
                int downDifMS;
                int checkDifMS;
                TickType_t lastTick = 0;

                bool createShape();
                bool checkCollision();
                piece getShape(int shapeIndex, int rotation);

        public:
                int score;
                int shapeIndex;
                piece currentShape;
                piece nextShape;
                int currentShapeX;
                int currentShapeY;
                int currentShapeColor;
                int nextShapeColor;
                int nextShapeIndex;
        };
}