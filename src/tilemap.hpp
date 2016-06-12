#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <vector>

template <typename TileType>
struct TileMap
{
//    using TileType = bool;

    void setSize(const sf::Vector2i& size)
    {
        tiles.resize(size.x * size.y);
        width = size.x;
    }

    sf::Vector2i getSize() const
    {
        return {width, tiles.size()/width};
    }

    void setTile(const sf::Vector2i& pos, const TileType tile)
    {
        tiles[pos.y * width + pos.x] = tile;
    }

    TileType getTile(const sf::Vector2i& pos) const
    {
        return tiles[pos.y * width + pos.x];
    }

    TileType& at(const sf::Vector2i& pos)
    {
        return tiles[pos.y * width + pos.x];
    }

    static sf::FloatRect tileToRect(const sf::Vector2i& pos, const sf::Vector2f& tileSize)
    {
        sf::FloatRect rect;

        rect.left = pos.x * tileSize.x;
        rect.top = pos.y * tileSize.y;
        rect.width = tileSize.x;
        rect.height = tileSize.y;

        return rect;
    }

    sf::VertexArray toVertexArray(const sf::Vector2f& tileSize) const
    {
        sf::VertexArray array;
//        array.setPrimitiveType(sf::PrimitiveType::TrianglesStrip);
        array.setPrimitiveType(sf::PrimitiveType::Quads);
        array.resize(tiles.size() * 4);
        for(size_t i = 0; i < tiles.size(); i++)
        {
            const auto& tile = tiles[i];
            const sf::Vector2f pos = sf::Vector2f{i % width, i / width} * tileSize.x;
            const sf::Color color = tile ? sf::Color::White : sf::Color::Black;

            array[i*4 + 0] = {pos, color};
            array[i*4 + 1] = {pos + sf::Vector2f{tileSize.x , 0}, color};
            array[i*4 + 2] = {pos + sf::Vector2f{tileSize.x , tileSize.y}, color};
            array[i*4 + 3] = {pos + sf::Vector2f{0, tileSize.y}, color};
        }

        return array;
    }

    std::vector<TileType> tiles;

private:
    size_t width;
};
