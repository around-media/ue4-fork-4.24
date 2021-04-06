// MIT License
//
// Copyright (c) 2017 Eric Liu
// Copyright (c) 2019 Lucid Layers

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "LatentActions.h"
#include "Engine/LatentActionManager.h"
#include "Interface/MeshExportable.h"
#include "assimp/matrix4x4.h"
#include "RuntimeMeshImportExportTypes.h"
#include "ProceduralMeshComponent.h"
#include "RuntimeMeshImportExportLibrary.generated.h"


/**
 * Library to import meshes from disk at runtime using Assimp library.
 */
UCLASS()
class RUNTIMEMESHIMPORTEXPORT_API URuntimeMeshImportExportLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     *	Import a mesh from various scene description files like fbx, gltf, obj, ... . @see GetSupportedExtensionsImport
     *	Note: The hierarchy of the scene is not retained on import
     *
     *	@param params		Import parameter
     *	@param result		Result of the import
     */
    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshImportExport|Import")
    static void ImportScene(const FRuntimeMeshImportParameter& params, FRuntimeMeshImportResult& result);

    /**
     *	Import a mesh from various scene description files like fbx, gltf, obj, ... . @see GetSupportedExtensionsImport
     *	Note: The hierarchy of the scene is not retained on import
     *
     *  @param params				Import parameter
     *	@param progressDelegate		Callback for a progress update of the import
     */
    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshImportExport|Import", meta = (Latent, WorldContext = "WorldContextObject", LatentInfo = "latentInfo"))
    static void	ImportScene_Async(UObject* worldContextObject, FLatentActionInfo latentInfo
                                  , const FRuntimeMeshImportParameter& params
                                  , FRuntimeMeshImportExportProgressUpdateDyn progressDelegate
                                  , FRuntimeMeshImportResult& result);

    /**
     *	Import a mesh from various scene description files like fbx, gltf, obj, ... . @see GetSupportedExtensionsImpor
     *	Note: The hierarchy of the scene is not retained on import
     *
     *	@param callbackFinished     Called when the Import is finished
     *  @param progressDelegate		Callback for a progress update of the import
     */
    static void ImportScene_Async_Cpp(const FRuntimeMeshImportParameter& params
                                      , FRuntimeImportFinished callbackFinished
                                      , FRuntimeMeshImportExportProgressUpdate callbackProgress);

    // Returns all supported extensions for import
    UFUNCTION(BlueprintPure, Category = "RuntimeMeshImportExport|Import")
    static bool GetIsExtensionSupportedImport(FString extension);

    // Returns the extensions that are supported for Import
    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshImportExport|Import")
    static void GetSupportedExtensionsImport(TArray<FString>& extensions);

    // Returns all supported extensions for export
    UFUNCTION(BlueprintPure, Category = "RuntimeMeshImportExport|Export")
    static bool GetIsExtensionSupportedExport(FString extension);

    // Returns the extensions that are supported for export
    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshImportExport|Export")
    static void GetSupportedExtensionsExport(TArray<FAssimpExportFormat>& formats);

    // Returns presets for transform corrections that can be used for export
    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshImportExport|Export")
    static void GetTransformCorrectionPresetsExport(TMap<FString, FTransformCorrection>& corrections);

    UFUNCTION(BlueprintPure, Category = "RuntimeMeshImportExport")
    static FTransform TransformCorrectionToTransform(const FTransformCorrection& correction);

    UFUNCTION(BlueprintPure, Category = "RuntimeMeshImportExport")
    static float RotationCorrectionToValue(const ERotationCorrection correction);

    UFUNCTION(BlueprintPure, Category = "RuntimeMeshImportExport")
    static void ConvertVectorToProceduralMeshTangent(const TArray<FVector>& tangents, const bool bFlipTangentY, TArray<FProcMeshTangent>& procTangents);
    UFUNCTION(BlueprintPure, Category = "RuntimeMeshImportExport")
    static void ConvertProceduralMeshTangentToVector(const TArray<FProcMeshTangent>& procTangents, TArray<FVector>& tangents);

    // Converts the whole MaterialInfo to a string for debugging purpose.
    UFUNCTION(BlueprintPure, Category = "RuntimeMeshImportExport")
    static FString MaterialInfoToLogString(const FRuntimeMeshImportMaterialInfo& materialInfo);

    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshImportExport")
    static UTexture2D* MaterialParamTextureToTexture2D(const FRuntimeMeshImportExportMaterialParamTexture& textureParam);

    /**
     * Create a DynamicMaterialInstance from a given SourceMaterial and pass in parameters from MaterialInfo.
     * Only parameters from MaterialInfo that also exist in SourceMaterial can be assigned.
     * TextureParameter from MaterialInfo will only be loaded as UTexture2D if the parameter is present in SourceMaterial.
     * The Dnamic material will be name: SOURCEMATERIALNAME_MATERIALINFONAME
     */
    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshImportExport", meta = (WorldContext = "worldContextObject"))
    static UMaterialInstanceDynamic* MaterialInfoToDynamicMaterial(UObject* worldContextObject, const FRuntimeMeshImportMaterialInfo& materialInfo, UMaterialInterface* sourceMaterial);

    // Append 'append' to 'appendTo'. Add a newline before appending if last character of 'appendTo' is not already a newline
    static void NewLineAndAppend(FString& appendTo, const FString& append);

    static void OffsetTriangleArray(int32 offset, TArray<int32>& triangles);

    static FTransform AiTransformToFTransform(const aiMatrix4x4& transform);
    static aiMatrix4x4 FTransformToAiTransform(const FTransform& transform);

    static void SendProgress_AnyThread(FRuntimeMeshImportExportProgressUpdate delegateProgress, FRuntimeMeshImportExportProgress progress);

private:
    /**
    *	Import a mesh from various scene description files like fbx, gltf, obj, ... . @see GetSupportedExtensionsImport
    *	Note: The hierarchy of the scene is not retained on import
    *
    *	@param params				Import parameter
    *	@param callbackProgress     Callback for a progress update of the import
    */
	static void ImportScene_AnyThread(const FRuntimeMeshImportParameter& params
                                      , FRuntimeMeshImportExportProgressUpdate callbackProgress
                                      , FRuntimeMeshImportResult& result);
};
