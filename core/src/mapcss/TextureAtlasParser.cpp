#include "mapcss/TextureAtlasParser.hpp"

using namespace utymap::mapcss;

TextureAtlas TextureAtlasParser::parse(const std::string& content)
{
    // TODO
    return {};
}

TextureAtlas TextureAtlasParser::parse(std::istream& istream)
{
    std::string atlasContent((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());
    return parse(atlasContent);
}
