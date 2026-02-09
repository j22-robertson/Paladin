cmake_minimum_required(VERSION 4.0.2)
set(PALADIN_CORE_SOURCES
        paladin-core/profiling/Profiling.h
        paladin-core/Logger.cpp
        paladin-core/Logger.h
        paladin-core/Camera.h
        paladin-core/Transform.h
        paladin-core/utility/StringHash.h
        paladin-core/utility/GenIndices.h
        paladin-core/asset-pipeline/interfaces/IAssetManager.h
        paladin-core/asset-pipeline/interfaces/IAssetImporter.h
        paladin-core/asset-pipeline/interfaces/IAssetImporterBase.h
        paladin-core/asset-pipeline/importers/ModelImporter.h
        paladin-core/asset-pipeline/importers/ModelImporter.cpp
        paladin-core/asset-pipeline/AssetRegistry.h
        paladin-core/asset-pipeline/importers/TextureImporter.h

        paladin-core/asset/Asset.h
        paladin-core/asset/Material.h
        paladin-core/asset/Mesh.h
        paladin-core/asset/Model.h
        paladin-core/asset/Texture2D.h
        paladin-core/Vertex.h
        PARENT_SCOPE)