#ifndef MAPCSS_TEXTUREATLAS_HPP_INCLUDED
#define MAPCSS_TEXTUREATLAS_HPP_INCLUDED

#include "meshing/MeshTypes.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace utymap { namespace mapcss {

/// Represents a single texture region inside texture atlas.
struct TextureRegion final
{
    TextureRegion(double x, double y, double width, double height) :
        x_(x), y_(y), width_(width), height_(height)
    {
    }

    /// Maps relative uv coordinate to absolute one in texture atlas.
    utymap::meshing::Vector2 map(const utymap::meshing::Vector2& uv) const
    {
        return utymap::meshing::Vector2(x_ + width_ * uv.x, y_ + height_ * uv.y);
    }

private:
    double x_, y_, width_, height_;
};

/// Holds list of textures which represent the same material.
struct TextureGroup final
{
    /// Adds specific region to the group.
    void add(int x, int y, int width, int height)
    {
        regions_.emplace_back(x, y, width, height);
    }

    /// Returns random region.
    TextureRegion random(std::uint32_t seed) const
    {
        return regions_[regions_.size() % seed];
    }

private:
    std::vector<TextureRegion> regions_;
};

struct TextureAtlas final
{
    /// Adds texture group to atlas.
    void add(const std::string& key, const TextureGroup& pack)
    {
        texturePack_.emplace(key, pack);
    }

    /// Returns a reference to texture group.
    /// Note: returns raw reference from map.
    const TextureGroup& get(const std::string& key) const
    {
        return texturePack_.find(key)->second;
    }

private:
    std::unordered_map<std::string, TextureGroup> texturePack_;
};

}}

#endif  // MAPCSS_TEXTUREATLAS_HPP_INCLUDED