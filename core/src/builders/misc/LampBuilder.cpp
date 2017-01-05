#include <mapcss/StyleConsts.hpp>
#include "builders/generators/TreeGenerator.hpp"
#include "builders/misc/LampBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"
#include "lsys/LSystem.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/MeshUtils.hpp"

using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
    const std::string NodeMeshNamePrefix = "lamp:";
    const std::string WayMeshNamePrefix = "lamps:";

    const std::string LampStep = "lamp-step";
}

void LampBuilder::visitNode(const utymap::entities::Node& node)
{
    Style style = context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail);

    const auto& lsystem = context_.styleProvider.getLsystem(style.getString(StyleConsts::LSystemKey()));
    double elevation = context_.eleProvider.getElevation(context_.quadKey, node.coordinate);

    Mesh lampMesh(utymap::utils::getMeshName(NodeMeshNamePrefix, node));
    TreeGenerator(context_, style, lampMesh)
        .setPosition(Vector3(node.coordinate.longitude, elevation, node.coordinate.latitude))
        .run(lsystem);
    
    context_.meshCallback(lampMesh);
}

void LampBuilder::visitWay(const utymap::entities::Way& way)
{
    Style style = context_.styleProvider.forElement(way, context_.quadKey.levelOfDetail);

    const auto& lsystem = context_.styleProvider.getLsystem(style.getString(StyleConsts::LSystemKey()));
    double width = style.getValue(StyleConsts::WidthKey(), context_.boundingBox);
    double stepInMeters = style.getValue(LampStep);

    Mesh lampMesh("");
    Mesh newMesh(utymap::utils::getMeshName(WayMeshNamePrefix, way));

    TreeGenerator(context_, style, lampMesh)
        .setPosition(Vector3(0, 0, 0))
        .run(lsystem);

    for (std::size_t i = 0; i < way.coordinates.size() - 1; ++i) {
        const auto& p0 = way.coordinates[i];
        const auto& p1 = way.coordinates[i + 1];

        // way is transformed to area with offsetting.
        if (width > 0) {
            auto direction = (Vector2(p1.longitude, p1.latitude) - Vector2(p0.longitude, p0.latitude)).normalized();
            Vector2 normal(-direction.y, direction.x);
            auto leftP0 = Vector2(p0.longitude, p0.latitude) + normal * width;
            auto leftP1 = Vector2(p1.longitude, p1.latitude) + normal * width;
            auto rightP0 = Vector2(p0.longitude, p0.latitude) - normal * width;
            auto rightP1 = Vector2(p1.longitude, p1.latitude) - normal * width;

            double distanceInMeters = GeoUtils::distance(p0, p1);
            int count = static_cast<int>(distanceInMeters / stepInMeters);

            for (int j = 1; j < count - 1; ++j) {
                double offset = static_cast<double>(j) / count;
                GeoCoordinate position = j % 2 == 0 
                    ? GeoUtils::newPoint(GeoCoordinate(leftP0.y, leftP0.x), GeoCoordinate(leftP1.y, leftP1.x), offset)
                    : GeoUtils::newPoint(GeoCoordinate(rightP0.y, rightP0.x), GeoCoordinate(rightP1.y, rightP1.x), offset);

                double elevation = context_.eleProvider.getElevation(context_.quadKey, position);
                utymap::utils::copyMesh(utymap::math::Vector3(position.longitude, elevation, position.latitude), lampMesh, newMesh);
            }

        } else
            utymap::utils::copyMeshAlong(context_.quadKey, p0, p1, lampMesh, newMesh, stepInMeters, context_.eleProvider);
    }

    context_.meshCallback(newMesh);
}

void LampBuilder::visitRelation(const utymap::entities::Relation& relation)
{
    for (const auto& element : relation.elements)
        element->accept(*this);
}
