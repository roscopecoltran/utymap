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

        ::triangulate("pzBQ", &in, &mid, nullptr);

        mid.trianglearealist = (REAL *)malloc(mid.numberoftriangles * sizeof(REAL));
        for (int i = 0; i < mid.numberoftriangles; ++i) {
            mid.trianglearealist[i] = options.area;
        }

        out.pointlist = nullptr;
        out.pointattributelist = nullptr;
        out.trianglelist = nullptr;
        out.triangleattributelist = nullptr;

        ::triangulate("praqzBPQ", &mid, &out, nullptr);

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
        // TODO
    }

    // for debug only
    void printData(triangulateio* io)
    {
        for (int i = 0; i < io->numberofpoints; i++) {
            printf("Point %4d:", i);
            for (int j = 0; j < 2; j++) {
                printf(" %.6g", io->pointlist[i * 2 + j]);
            }
            printf("\n");
        }

        for (int i = 0; i < io->numberoftriangles; i++) {
            printf("Triangle %4d:", i);
            for (int j = 0; j < io->numberofcorners; j++) {
                printf(" %d", io->trianglelist[i * io->numberofcorners + j]);
            }
            printf("\n");
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
