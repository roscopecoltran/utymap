#include "meshing/clipper.hpp"
#include "terrain/TerraBuilder.hpp"

using namespace utymap::meshing;
using namespace utymap::terrain;
using namespace ClipperLib;

Paths clipByRect(Clipper clipper, const Path clipRect, const Paths subjects)
{
    Paths solution;
    clipper.AddPaths(subjects, ptSubject, true);
    clipper.AddPath(clipRect, ptClip, true);
    clipper.Execute(ctIntersection, solution);
    clipper.Clear();
    return std::move(solution);
}

Mesh<double> TerraBuilder::build(const Rectangle<double> tileRect)
{
    Mesh<double> mesh;

    return std::move(mesh);
}
