
namespace tetrics_module
{
    class board
    {
    public:
        void frame();
        void clear();
        void moveLeft();
        void moveRight();
        void moveDown();
        void makeNewShape();
        int getTile(int x, int y);
        const int width=10;
        const int height=22;
        int board[10][22];
    private:
        void createShape();
        int shapeIndex;
        int currentShape[4][4];
        int currentShapeX;
        int currentShapeY;
        int currentShapeColor;

    };
}