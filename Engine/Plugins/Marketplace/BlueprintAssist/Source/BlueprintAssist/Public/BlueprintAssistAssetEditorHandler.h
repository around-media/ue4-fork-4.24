// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "BlueprintAssistBlueprintHandler.h"
#include "BlueprintAssistToolbar.h"

class BLUEPRINTASSIST_API FBAAssetEditorHandler
{
public:
	static FBAAssetEditorHandler& Get();

	void Init();

	void Cleanup();

	void Tick();

	IAssetEditorInstance* GetEditorFromTab(const TSharedPtr<SDockTab> Tab) const;

	template<class AssetClass, class EditorClass>
	EditorClass* GetEditorFromTabCasted(const TSharedPtr<SDockTab> Tab) const;

	template<class AssetClass>
	AssetClass* GetAssetFromTab(const TSharedPtr<SDockTab> Tab) const;

protected:
	void BindAssetOpenedDelegate();

	void UnbindAssetOpenedDelegate();

	void OnAssetOpenedInEditor(UObject* Asset, class IAssetEditorInstance* AssetEditor);

	void OnAssetClosed(UObject* Asset);

	void CheckInvalidAssetEditors();

private:
	TMap<FGuid, FBABlueprintHandler> BlueprintHandlers;
	TMap<TWeakPtr<SDockTab>, TWeakObjectPtr<UObject>> AssetsByTab;

	FDelegateHandle OnAssetOpenedDelegateHandle;
};
