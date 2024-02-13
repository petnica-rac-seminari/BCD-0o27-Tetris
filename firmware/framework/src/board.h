
namespace tetrics_module
{
    class board
    {
    public:
        void frame();
        void moveLeft();
        void moveRight();
        void moveDown();
        int getTile(int x, int y);
        const int width=10;
        const int height=22;
        int board[10][22];
    private:
    };
}