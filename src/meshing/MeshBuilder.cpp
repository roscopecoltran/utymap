#define REAL double
#define ANSI_DECLARATORS

#include "meshing/MeshBuilder.hpp"
#include "meshing/Triangle.h"

using namespace utymap::heightmap;
using namespace utymap::meshing;

class MeshBuilder::MeshBuilderImpl
{
public:
    MeshBuilderImpl(ElevationProvider<double>& eleProvider)
    : eleProvider_(eleProvider) { }

    Mesh<double> build(Polygon<double>& polygon, const MeshBuilder::Options& options)
    {
        triangulateio in, mid, out;

        in.numberofpoints = polygon.points.size() / 2;
        in.numberofholes = polygon.holes.size() / 2;
        in.numberofpointattributes = 0;
        in.numberofregions = 0;
        in.numberofsegments = polygon.segments.size() / 2;

        in.pointlist = polygon.points.data();
        in.holelist = polygon.holes.data();
        in.segmentlist = polygon.segments.data();
        in.segmentmarkerlist = nullptr;
        in.pointmarkerlist = nullptr;

        mid.pointlist = nullptr;
        mid.pointmarkerlist = nullptr;
        mid.trianglelist = nullptr;
        mid.segmentlist = nullptr;
        mid.segmentmarkerlist = nullptr;

        ::triangulate(const_cast<char*>("pzBQ"), &in, &mid, nullptr);

        mid.trianglearealist = (REAL *)malloc(mid.numberoftriangles * sizeof(REAL));
        for (int i = 0; i < mid.numberoftriangles; ++i) {
            mid.trianglearealist[i] = options.area;
        }

        out.pointlist = nullptr;
        out.pointattributelist = nullptr;
        out.trianglelist = nullptr;
        out.triangleattributelist = nullptr;

        ::triangulate(const_cast<char*>("praqzBPQ"), &mid, &out, nullptr);

        Mesh<double> mesh;
        fillMesh(&out, mesh);

        free(in.pointmarkerlist);

        free(mid.pointlist);
        free(mid.pointmarkerlist);
        free(mid.trianglelist);
        free(mid.trianglearealist);
        free(mid.segmentlist);
        free(mid.segmentmarkerlist);

        free(out.pointlist);
        free(out.pointattributelist);
        free(out.trianglelist);
        free(out.triangleattributelist);

        return std::move(mesh);
    }

private:
    ElevationProvider<double>& eleProvider_;

    void fillMesh(triangulateio* io, Mesh<double>& mesh)
    {
        mesh.vertices.reserve(io->numberofpoints / 2);
        mesh.triangles.reserve(io->numberoftriangles);

        for (int i = 0; i < io->numberofpoints; i++) {
            double x = io->pointlist[i * 2 + 0];
            double y = io->pointlist[i * 2 + 1];
            double elevation = eleProvider_.getElevation(x, y);
            Vertex<double> vertex(x, y, elevation);

            mesh.vertices.push_back(vertex);
        }

        for (int i = 0; i < io->numberoftriangles; i++) {
            int v0 = io->trianglelist[i * io->numberofcorners + 0];
            int v1 = io->trianglelist[i * io->numberofcorners + 1];
            int v2 = io->trianglelist[i * io->numberofcorners + 2];
            Triangle tri(v0, v1, v2);

            mesh.triangles.push_back(tri);
        }
    }
};

MeshBuilder::MeshBuilder(ElevationProvider<double>& eleProvider) :
    pimpl_(std::unique_ptr<MeshBuilder::MeshBuilderImpl>(new MeshBuilder::MeshBuilderImpl(eleProvider)))
{
}

MeshBuilder::~MeshBuilder() { }

Mesh<double> utymap::meshing::MeshBuilder::build(Polygon<double>& polygon, const MeshBuilder::Options& options)
{
    return pimpl_->build(polygon, options);
}
