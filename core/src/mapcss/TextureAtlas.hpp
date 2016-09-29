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
    TextureRegion(std::uint16_t atlasWidth,
                  std::uint16_t atlasHeight,
                  std::uint16_t x,
                  std::uint16_t y,
                  std::uint16_t width,
                  std::uint16_t height) :
            atlasWidth_(atlasWidth), atlasHeight_(atlasHeight),
            x_(x), y_(y), width_(width), height_(height)
    {
    }

    /// Maps relative uv coordinate to absolute one in texture atlas.
    utymap::meshing::Vector2 map(const utymap::meshing::Vector2& uv) const
    {
        return utymap::meshing::Vector2(
                (x_ + width_ * uv.x) / atlasWidth_,
                (y_ + height_ * uv.y) / atlasHeight_);
    }

    bool isEmpty() const {
        return atlasWidth_ == 0 || atlasHeight_ == 0;
    }

private:
    std::uint16_t atlasWidth_, atlasHeight_, x_, y_, width_, height_;
};

/// Holds list of textures which represent the same material.
struct TextureGroup final
{
    /// Adds specific region to the group.
    void add(std::uint16_t width, std::uint16_t height, utymap::meshing::Rectangle rect)
    {
        regions_.emplace_back(width, height, rect.xMin, rect.yMin, rect.width(), rect.height());
    }

    /// Returns pseudo random region.
    TextureRegion random(std::uint32_t seed) const
    {
        return regions_[seed % regions_.size()];
    }

private:
    std::vector<TextureRegion> regions_;
};

struct TextureAtlas final
{
    using Groups = std::unordered_map<std::string, TextureGroup>;

    TextureAtlas() : TextureAtlas(Groups())
    {
    }

    TextureAtlas(const Groups& textureGroups) :
        textureGroups_(std::move(textureGroups)),
        emptyGroup_()
    {
        emptyGroup_.add(0, 0, utymap::meshing::Rectangle());
    }

    /// Returns a reference to texture group.
    /// Note: returns raw reference from map.
    const TextureGroup& get(const std::string& key) const
    {
        auto group = textureGroups_.find(key);
        if (group == textureGroups_.end()) {
            return emptyGroup_;
        }

        return group->second;
    }

private:
    Groups textureGroups_;
    TextureGroup emptyGroup_;
};

}}

#endif  // MAPCSS_TEXTUREATLAS_HPP_INCLUDED