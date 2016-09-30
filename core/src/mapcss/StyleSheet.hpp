#ifndef MAPCSS_STYLESHEET_HPP_DEFINED
#define MAPCSS_STYLESHEET_HPP_DEFINED

#include "meshing/MeshTypes.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace utymap { namespace mapcss {

/// Represents a single texture region inside texture atlas.
struct TextureRegion final
{
    TextureRegion() : TextureRegion(0, 0, 0, 0, 0, 0)
    {
    }

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
        regions_.emplace_back(
            width,
            height,
            static_cast<std::uint16_t>(rect.xMin),
            static_cast<std::uint16_t>(rect.yMin),
            static_cast<std::uint16_t>(rect.width()),
            static_cast<std::uint16_t>(rect.height()));
    }

    /// Returns pseudo random region.
    TextureRegion random(std::uint32_t seed) const
    {
        return regions_[seed % regions_.size()];
    }

private:
    std::vector<TextureRegion> regions_;
};

/// Represents texture atlas.
struct TextureAtlas final
{
    using Groups = std::unordered_map<std::string, TextureGroup>;

    TextureAtlas() : TextureAtlas("", Groups())
    {
    }

    TextureAtlas(const std::string& name, const Groups& textureGroups) :
        name_(name),
        textureGroups_(std::move(textureGroups)),
        emptyGroup_()
    {
        emptyGroup_.add(0, 0, utymap::meshing::Rectangle());
    }

    /// Returns name of atlas.
    const std::string& name() const
    {
        return name_;
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
    const std::string name_;
    Groups textureGroups_;
    TextureGroup emptyGroup_;
};

// Represents condition.
struct Condition final
{
    std::string key;
    std::string operation;
    std::string value;
};

// Represents map zoom level.
struct Zoom final
{
    std::uint8_t start;
    std::uint8_t end;
};

// Represents selector.
struct Selector final
{
    std::vector<std::string> names;
    Zoom zoom;
    std::vector<Condition> conditions;
};

// Represents declaration.
struct Declaration final
{
    std::string key;
    std::string value;
};

// Represents rule.
struct Rule final
{
    std::vector<Selector> selectors;
    std::vector<Declaration> declarations;
};

// Represents stylesheet.
struct StyleSheet final
{
    std::vector<TextureAtlas> textures;
    std::vector<Rule> rules;
};

std::ostream& operator<<(std::ostream &stream, const Condition &c);
std::ostream& operator<<(std::ostream &stream, const Zoom &z);
std::ostream& operator<<(std::ostream &stream, const Selector &s);
std::ostream& operator<<(std::ostream &stream, const Declaration &d);
std::ostream& operator<<(std::ostream &stream, const Rule &r);
std::ostream& operator<<(std::ostream &stream, const StyleSheet &s);

}}
#endif // MAPCSS_STYLESHEET_HPP_DEFINED
