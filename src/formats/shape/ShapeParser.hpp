#ifndef FORMATS_SHAPE_SHAPEPARSER_HPP_INCLUDED
#define FORMATS_SHAPE_SHAPEPARSER_HPP_INCLUDED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/OsmTypes.hpp"
#include "shapefil.h"

#include <istream>
#include <string>
#include <sstream>
#include <vector>

template<typename Visitor>
class ShapeParser
{
public:

    void parse(const std::string& path, Visitor& visitor)
    {
        SHPHandle shpFile = SHPOpen(argv[1], "rb");
        if (shpFile == NULL)
            throw std::domain_error("Cannot open shp file.");

        int shapeType, entityCount;
        double adfMinBound[4], adfMaxBound[4];
        SHPGetInfo(shpFile, &entityCount, &shapeType, adfMinBound, adfMaxBound);

        DBFHandle dbfFile = DBFOpen(pszFilename, "rb");
        if (hDBF == NULL)
            throw std::domain_error("Cannot open dbf file.");

        if (DBFGetFieldCount(hDBF) == 0)
            throw std::domain_error("There are no fields in dbf table.");

        if (entityCount != DBFGetRecordCount(dbfFile))
            throw std::domain_error("dbf file has different entity count.");

        char szTitle[12];
        for (int k = 0; k < entityCount; k++)
        {
            SHPObject* shape = SHPReadObject(shpFile, k);
            if (shape == NULL)
                throw std::domain_error("Unable to read shape:" + to_string(k));
            
            visitShape(shape, parseTags(dbfFile, k), visitor);

            SHPDestroyObject(shape);
        }

        DBFClose(hDBF);
        SHPClose(hSHP);
    }

private:
    // NOTE: Workaround due to MinGW g++ compiler
    template <typename T>
    std::string to_string(T const& value)
    {
        std::stringstream sstr;
        sstr << value;
        return sstr.str();
    }

    inline std::vector<utymap::formats::Tag> parseTags(DBFHandle dbfFile, int k)
    {
        int fieldCount = DBFGetFieldCount(dbfFile);
        std::vector<utymap::formats::Tag> tags;
        tags.reserve(fieldCount);
        for (int i = 0; i < fieldCount; i++)
        {
            if (DBFIsAttributeNULL(dbfFile, k, i))
                continue;

            utymap::formats::Tag tag;
            int width, decimals;
            DBFFieldType eType = DBFGetFieldInfo(dbfFile, i, szTitle, &width, &decimals);
            tag.key = std::string(szTitle);
            {
                switch (eType)
                {
                    case FTString:
                        tag.value = DBFReadStringAttribute(dbfFile, k, i);
                        break;
                    case FTInteger:
                        tag.value = to_string(DBFReadIntegerAttribute(dbfFile, k, i));
                        break;
                    case FTDouble:
                        tag.value = to_string(DBFReadDoubleAttribute(dbfFile, k, i));
                        break;
                    default:
                        break;
                }
            }
            tags.push_back(tag);
        }
        return std::move(tags);
    }

    inline void visitShape(SHPObject* shape, std::vector<utymap::formats::Tag>& tags, Visitor& visitor)
    {
        // parse coordinates
        std::vector<utymap::GeoCoordinate> coordinates;
        coordinates.reserve(shape->nVertices);
        std::string type = SHPTypeName(psShape->nSHPType);
        for (int j = 0; j < shape->nVertices; j++)
            coordinates.push_back(utymap::GeoCoordinate(shape->padfX[j], shape->padfY[j]));

        /*switch (nSHPType)
        {
        case SHPT_POINT:
        case SHPT_POINTM:
        case SHPT_POINTZ:
            return "Point";

        case SHPT_ARC:
        case SHPT_ARCZ:
        case SHPT_ARCM:
            return "Arc";

        case SHPT_POLYGON:
        case SHPT_POLYGONZ:
        case SHPT_POLYGONM:
            return "Polygon";

        case SHPT_MULTIPOINT:
        case SHPT_MULTIPOINTZ:
        case SHPT_MULTIPOINTM:
            return "MultiPoint";

        case SHPT_MULTIPATCH:
            return "MultiPatch";

        default:
            return "UnknownShapeType";
        }*/

    }

};

#endif  // FORMATS_SHAPE_SHAPEPARSER_HPP_INCLUDED
