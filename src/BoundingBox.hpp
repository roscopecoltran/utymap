#ifndef BOUNDINGBOX_HPP_DEFINED
#define BOUNDINGBOX_HPP_DEFINED

namespace utymap {

// Represents geo bounding box
struct BoundingBox
{
    // Minimal latitude.
    double minLatitude;
    // Minimal longitude.
    double minLongitude;
    // Maximum latitude.
    double maxLatitude;
    // Maxumum longitude.
    double maxLongitude;

    // Expands bounding box by given.
    void expand(const BoundingBox&);
    // Checks whether given bounding box inside the current one.
    void contains(const BoundingBox&);
    // Checks whether given bounding box intersects the current one.
    void intersects(const BoundingBox&);
};

}
#endif // BOUNDINGBOX_HPP_DEFINED
