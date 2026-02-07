cmake_minimum_required(VERSION 4.0.2)
set(PALADIN_CORE_SOURCES
        paladin-core/profiling/Profiling.h
        paladin-core/Logger.cpp
        paladin-core/Logger.h
        paladin-core/Camera.h
        paladin-core/Transform.h
        paladin-core/utility/StringHash.h
        paladin-core/asset-pipeline/AssetManager.cpp
        paladin-core/asset-pipeline/AssetManager.h
        PARENT_SCOPE)