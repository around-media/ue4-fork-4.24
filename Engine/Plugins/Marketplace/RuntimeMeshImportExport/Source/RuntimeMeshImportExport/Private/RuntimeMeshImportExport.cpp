// MIT License
//
// Copyright (c) 2019 Lucid Layers

#include "RuntimeMeshImportExport.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformProcess.h"

#define LOCTEXT_NAMESPACE "FRuntimeMeshImportExportModule"

void FRuntimeMeshImportExportModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginBaseDir = IPluginManager::Get().FindPlugin("RuntimeMeshImportExport")->GetBaseDir();
	FString configString;

	//AMCHANGE_begin
	//#AMCHANGE Always use the release DLLs, not all testers and QA always have the debug dll's required to load the debug assimp dll.
	configString = "Release";
	//AMCHANGE_end

#if PLATFORM_WINDOWS
#if PLATFORM_32BITS
	FString platformString = "Win32";
#elif PLATFORM_64BITS
	FString platformString = "x64";
#endif
#elif PLATFORM_MAC
	FString platformString = "Mac";
#endif

	FString dllFileName = FString(TEXT("assimp-vc141-mt")) + (UE_BUILD_SHIPPING ? TEXT("") : TEXT("d")) + TEXT(".dll");
	FString dllFile = FPaths::Combine(PluginBaseDir, FString("Source/ThirdParty/assimp/bin"), platformString, configString, dllFileName);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*dllFile))
	{
		RMIE_LOG(Fatal, "Missing file: %s", *dllFile);
	}

	//AMCHANGE_begin
	//#AMCHANGE Add validation to detect when the assimp dll failed to load
	//			One of the reason making this fail is the error 206: the path to the dll is too long. It can be other causes though.
	//			In all cases, it's better to detect this problem early than to crash when trying to use the dll.
	dllHandle_assimp = FPlatformProcess::GetDllHandle(*dllFile);
	if (!dllHandle_assimp)
	{
		RMIE_LOG(Fatal, "Failed to load the required '%s' dll. The application will now close.", *dllFileName);
	}
	//AMCHANGE_end
}

void FRuntimeMeshImportExportModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FPlatformProcess::FreeDllHandle(dllHandle_assimp);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRuntimeMeshImportExportModule, RuntimeMeshImportExport)