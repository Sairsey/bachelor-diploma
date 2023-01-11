#pragma once

#ifndef __cplusplus

struct D3D12_DRAW_INDEXED_ARGUMENTS
{
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int BaseVertexLocation;
    uint StartInstanceLocation;
};

struct D3D12_VERTEX_BUFFER_VIEW
{
    uint2 BufferLocation;
    uint SizeInBytes;
    uint StrideInBytes;
};

struct D3D12_INDEX_BUFFER_VIEW
{
    uint2 BufferLocation;
    uint SizeInBytes;
    uint Format;
};

#endif // __cplusplus
