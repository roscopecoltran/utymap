#ifndef HEIGHTMAP_SRTMELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_SRTMELEVATIONPROVIDER_HPP_DEFINED

#include "heightmap/ElevationProvider.hpp"

#include <cstdio>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <string>
#include <stdexcept>
#include <map>
#include <memory>

namespace utymap { namespace heightmap {

// Provides the way to get elevation for given location from SRTM data.
class SrtmElevationProvider : public ElevationProvider
{
    typedef std::shared_ptr<char*> Data;

    struct HgtCellKey
    {
        int lat, lon;

        HgtCellKey(int lat, int lon) : lat(lat), lon(lon) { }

        inline bool operator<(const HgtCellKey& other) { return lat < other.lat || (!(other.lat < lat) && lon < other.lon); }
    };

    struct HgtCell
    {
        int totalPx, secondsPerPx;
        int offset;
        Data data;
        int size;
       
        HgtCell(int totalPx, int secondsPerPx, Data data, int size) :
            totalPx(totalPx), secondsPerPx(secondsPerPx), 
            offset((totalPx * totalPx - totalPx) * 2), data(data), size(size)
        {
        }
    };

    typedef std::shared_ptr<HgtCell> CellPtr;

public:

    SrtmElevationProvider(std::string dataDirectory, std::string dataSchema, int maxCacheSize = 4):
        dataDirectory_(dataDirectory), dataSchema_(dataSchema), maxCacheSize_(maxCacheSize)
    {
    }

    void preload(const utymap::BoundingBox& bbox)
    {
        int minLat = (int)bbox.minPoint.latitude;
        int minLon = (int)bbox.minPoint.longitude;

        int maxLat = (int)bbox.maxPoint.latitude;
        int maxLon = (int)bbox.maxPoint.longitude;

        int latDiff = maxLat - minLat;
        int lonDiff = maxLon - minLon;

        for (int j = 0; j <= latDiff; j++)
            for (int i = 0; i <= lonDiff; i++) {
                HgtCellKey cellKey(minLat + j, minLon + i);

                if (cells_.find(cellKey) != cells_.end()) 
                    continue;

                std::string path = getFilePath(cellKey);
                CellPtr cell = readCell(path);
                cells_[cellKey] = cell;
            }
    }

    double getElevation(const utymap::GeoCoordinate& coordinate) const { return getElevationImpl(coordinate.latitude, coordinate.longitude); };

    double getElevation(double latitude, double longitude) const { return getElevationImpl(latitude, longitude); };

private:

    inline double getElevationImpl(double latitude, double longitude) const
    {
        int latDec = (int) latitude;
        int lonDec = (int) longitude;

        double secondsLat = (latitude - latDec) * 3600;
        double secondsLon = (longitude - lonDec) * 3600;

        auto cell = cells_.find(HgtCellKey(latDec, lonDec))->second;

        // load tile
        //X coresponds to x/y values,
        //everything easter/norhter (< S) is rounded to X.
        //
        //  y   ^
        //  3   |       |   S
        //      +-------+-------
        //  0   |   X   |
        //      +-------+-------->
        // (sec)    0        3   x  (lon)

        //both values are [0; totalPx - 1] (totalPx reserved for interpolating)
        int y = (int)(secondsLat / cell->secondsPerPx);
        int x = (int)(secondsLon / cell->secondsPerPx);

        //get norther and easter points
        int height2 = readPx(cell, y, x);
        int height0 = readPx(cell, y + 1, x);
        int height3 = readPx(cell, y, x + 1);
        int height1 = readPx(cell, y + 1, x + 1);

        //ratio where X lays
        double dy = std::fmod(secondsLat, cell->secondsPerPx) / cell->secondsPerPx;
        double dx = std::fmod(secondsLon, cell->secondsPerPx) / cell->secondsPerPx;

        // Bilinear interpolation
        // h0------------h1
        // |
        // |--dx-- .
        // |       |
        // |      dy
        // |       |
        // h2------------h3   

        return height0*dy*(1 - dx) + height1*dy*(dx)+height2*(1 - dy)*(1 - dx) + height3*(1 - dy)*dx;
    }

    inline int readPx(CellPtr cell, int y, int x) const
    {
        int pos = cell->offset + 2 * (x - cell->totalPx*y);
        // TODO ensure that it works on all platforms
        return *((char*)(*cell->data + pos)) << 8 |
               *((char*)(*cell->data + pos + 1));
    }

    inline CellPtr readCell(const std::string& path) const
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        file.seekg(0, std::ios::end);
        std::uint32_t size = file.tellg();

        int totalPx, secondsPerPx;
        switch (size)
        {
            case 1201 * 1201 * 2: // SRTM-3
                totalPx = 1201;
                secondsPerPx = 3;
                break;
            case 3601 * 3601 * 2: // SRTM-1
                totalPx = 3601;
                secondsPerPx = 1;
                break;
            default:
                file.close();
                throw std::domain_error(std::string("Cannot load srtm file:") + path);
        }

        Data data(new char[size]);
        file.seekg(0, std::ios::beg);
        file.read(*data, size);
        file.close();

        return CellPtr(new HgtCell(totalPx, secondsPerPx, data, size));
    }

    std::string getFilePath(const HgtCellKey& key) const
    {
        auto directoryLength = dataDirectory_.size();
        std::shared_ptr<char*> buffer(new char[directoryLength + 1 + 2 + 1 + 4 + 4]);

        std::sprintf(*buffer, "%s/%c%.2i%s%.4i", dataDirectory_, 
            key.lat > 0 ? 'N' : 'S', std::abs(key.lat),
            key.lon > 0 ? 'E' : 'W', std::abs(key.lon));

        return std::string(*buffer);
    }

    std::map<HgtCellKey, CellPtr> cells_;
    std::string dataDirectory_;
    std::string dataSchema_;
    int maxCacheSize_;
};

}}

#endif // HEIGHTMAP_SRTMELEVATIONPROVIDER_HPP_DEFINED
