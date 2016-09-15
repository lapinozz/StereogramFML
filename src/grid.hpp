#ifndef GRID_H
#define GRID_H

#include "datas.hpp"
#include "tilemap.hpp"

class Grid
{
    public:

        struct Cell
        {
            std::vector<int32_t> entityIds;

            Cell* n = nullptr;
            Cell* s = nullptr;
            Cell* w = nullptr;
            Cell* e = nullptr;
        };

        Grid(sf::Vector2i size)
        {
            mSize = size;
            mCells.setSize(size);

            for(int x = 0; x < size.x; x++)
            {
                for(int y = 0; y < size.y; y++)
                {
                    if(x != 0)
                        mCells.at({x, y}).w = &mCells.at({x - 1, y});
                    if(x != size.x - 1)
                        mCells.at({x, y}).e = &mCells.at({x + 1, y});
                    if(y != 0)
                        mCells.at({x, y}).n = &mCells.at({x, y - 1});
                    if(y != size.y - 1)
                        mCells.at({x, y}).s = &mCells.at({x, y + 1});
                }
            }
        }

        virtual ~Grid() = default;

        void addEntity(int32_t id)
        {

        }

        void removeEntity(int32_t id)
        {

        }

    private:
        TileMap<Cell> mCells;
        sf::Vector2i mSize;
};

#endif // GRID_H
