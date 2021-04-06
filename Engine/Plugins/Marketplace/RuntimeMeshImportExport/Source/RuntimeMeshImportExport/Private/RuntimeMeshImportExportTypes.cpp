// MIT License
//
// Copyright (c) 2019 Lucid Layers

#include "RuntimeMeshImportExportTypes.h"
#include "RuntimeMeshImportExportLibrary.h"
#include "assimp/cexport.h"

void FExportableMeshSection::Append(FExportableMeshSection&& other)
{
    check(material == other.material);


    int32 triangleOffset = vertices.Num();

    vertices.Append(other.vertices);
    normals.Append(other.normals);
    tangents.Append(other.tangents);
    textureCoordinates.Append(other.textureCoordinates);
    vertexColors.Append(other.vertexColors);

    triangles.Reserve(triangles.Num() + other.triangles.Num());

    for (int32& tri : other.triangles)
    {
        triangles.Add(tri + triangleOffset);
    }
}

void FRuntimeMeshImportSectionInfo::Append_Move(FRuntimeMeshImportSectionInfo&& other)
{
    URuntimeMeshImportExportLibrary::OffsetTriangleArray(vertices.Num(), other.triangles);
    vertices.Append(MoveTemp(other.vertices));
    normals.Append(MoveTemp(other.normals));
    tangents.Append(MoveTemp(other.tangents));
    vertexColors.Append(MoveTemp(other.vertexColors));
    uv0.Append(MoveTemp(other.uv0));
    triangles.Append(MoveTemp(other.triangles));

    // Retain the material data if it is the same
    materialName = materialName == other.materialName ? materialName : FName();
    materialIndex = materialIndex == other.materialIndex? materialIndex : INDEX_NONE;
    // Clear the other material data
    other.materialName = FName();
    other.materialIndex = INDEX_NONE;
}

FAssimpExportFormat::FAssimpExportFormat(const aiExportFormatDesc* desc) : id(FString(desc->id)), description(FString(desc->description))
, fileExtension(FString(desc->fileExtension))
{

}

unsigned int FAssimpImportPostProcessFlags::GetAsInt() const
{
	unsigned int flags = 0;

	if (bCalculateTangentSpace)
	{
		flags |= aiProcess_CalcTangentSpace;
	}

	if (bJoinIdenticalVertices)
	{
		flags |= aiProcess_JoinIdenticalVertices;
	}

	if (bGenerateSmoothNormals)
	{
		flags |= aiProcess_GenSmoothNormals;
	}

	if (bImproveCacheLocality)
	{
		flags |= aiProcess_ImproveCacheLocality;
	}

	if (bFixInfacingNormals)
	{
		flags |= aiProcess_FixInfacingNormals;
	}

	if (bFindInstances)
	{
		flags |= aiProcess_FindInstances;
	}

	if (bOptimizeMeshes)
	{
		flags |= aiProcess_OptimizeMeshes;
	}

	if (bDropNormals)
	{
		flags |= aiProcess_DropNormals;
	}

	return flags;
}
