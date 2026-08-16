//
// Created by roman on 11/19/2024.
//

#include "CUtil.h"
#include "game/Textures/TextureDatabaseRuntime.h"
#include "platform/api.h"


uintptr_t CUtil::FindLib(const char* libname)
{
    // How a module base is found differs completely between platforms — a
    // dlopen/dladdr pair here, a dyld image walk on iOS — so the mechanism
    // lives behind the platform layer and this stays a name lookup.
    return Platform::ModuleBase(libname);
}

RwTexture* CUtil::LoadTextureFromDB(const char* dbname, const char* texture)
{
    TextureDatabaseRuntime* db_handle = TextureDatabaseRuntime::GetDatabase(dbname);
    if(!db_handle)
    {
        Log("Error: Database not found! (%s)", dbname);
        return nullptr;
    }

    TextureDatabaseRuntime::Register(db_handle);

    auto tex = CUtil::GetTexture(texture);
    if(!tex)
    {
        Log("Error: Texture (%s) not found in database (%s)", dbname, texture);
        return nullptr;
    }

    TextureDatabaseRuntime::Unregister(db_handle);

    return tex;
}

RwTexture* CUtil::GetTexture(const char* name)
{
    auto tex = TextureDatabaseRuntime::GetTexture(name);
    if (!tex)
    {
        //tex = CUtil::LoadTextureFromDB("gta3", "ahoodfence2");
        Log("WARNING! No tex = %s", name);
        return nullptr;
    }
    ++tex->refCount;

    return tex;
}

void __fastcall CUtil::TransformPoint(RwV3d &result, const CSimpleTransform &t, const RwV3d &v)
{
    float cos_heading = cosf(t.m_fHeading);
    float sin_heading = sinf(t.m_fHeading);

    result = {
            t.m_vPosn.x + cos_heading * v.x - sin_heading * v.y,
            t.m_vPosn.y + sin_heading * v.x + cos_heading * v.y,
            v.z + t.m_vPosn.z
    };
}

float CUtil::GetDistanceBetween3DPoints(const RwV3d f, const RwV3d s)
{
    return sqrt(pow(s.x - f.x, 2) + pow(s.y - f.y, 2) + pow(s.z - f.z, 2));
}