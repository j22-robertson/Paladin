//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_VERTEX_H
#define PALADIN_VERTEX_H

namespace Paladin
{
    struct alignas(16)Vertex
    {
        float x,y,z; //Positions
        //float nx,ny,nz; // Normals
       // float tx,ty,tz; // Tangents
       // float btx,bty,btz; // Bitangents
       // float u,v; // TexUV
       // float r,g,b,a; // Colors
    };
}


#endif //PALADIN_VERTEX_H