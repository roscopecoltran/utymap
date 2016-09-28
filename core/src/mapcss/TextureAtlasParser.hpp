#ifndef MAPCSS_TEXTUREATLASPARSER_HPP_INCLUDED
#define MAPCSS_TEXTUREATLASPARSER_HPP_INCLUDED

#include "mapcss/TextureAtlas.hpp"

#include <string>
#include <istream>

namespace utymap { namespace mapcss {

/// Provides the way to parse texture atlas.
class TextureAtlasParser final
{
public:
    /// Parses texture atlas from string.
    static TextureAtlas parse(const std::string& content);

    /// Parses texture atlas from input stream.
    static TextureAtlas parse(std::istream& istream);
};
}}

#endif  // MAPCSS_TEXTUREATLASPARSER_HPP_INCLUDED