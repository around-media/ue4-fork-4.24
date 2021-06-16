// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistAssetEditorHandler.h"

#include "BlueprintAssistInputProcessor.h"
#include "BlueprintAssistTabHandler.h"
#include "BlueprintEditor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/LazySingleton.h"

#if ENGINE_MINOR_VERSION < 24 && ENGINE_MAJOR_VERSION == 4
#include "Toolkits/AssetEditorManager.h"
#endif

FBAAssetEditorHandler& FBAAssetEditorHandler::Get()
{
	return TLazySingleton<FBAAssetEditorHandler>::Get();
}

void FBAAssetEditorHandler::Init()
{
	BindAssetOpenedDelegate();
}

void FBAAssetEditorHandler::Cleanup()
{
	UnbindAssetOpenedDelegate();
	BlueprintHandlers.Empty();
	AssetsByTab.Empty();
}

void FBAAssetEditorHandler::Tick()
{
	CheckInvalidAssetEditors();
}

IAssetEditorInstance* FBAAssetEditorHandler::GetEditorFromTab(const TSharedPtr<SDockTab> Tab) const
{
	if (const TWeakObjectPtr<UObject>* FoundAsset = AssetsByTab.Find(Tab))
	{
		if (FoundAsset->IsValid())
		{
			if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				return AssetEditorSubsystem->FindEditorForAsset(FoundAsset->Get(), false);
			}
		}
	}

	return nullptr;
}

template <class AssetClass, class EditorClass>
EditorClass* FBAAssetEditorHandler::GetEditorFromTabCasted(const TSharedPtr<SDockTab> Tab) const
{
	if (const TWeakObjectPtr<UObject>* FoundAsset = AssetsByTab.Find(Tab))
	{
		if (FoundAsset->IsValid() && FoundAsset->Get()->IsA(AssetClass::StaticClass()))
		{
			if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				return static_cast<EditorClass*>(AssetEditorSubsystem->FindEditorForAsset(FoundAsset->Get(), false));
			}
		}
	}

	return nullptr;
}

template<class AssetClass>
AssetClass* FBAAssetEditorHandler::GetAssetFromTab(const TSharedPtr<SDockTab> Tab) const
{
	if (const TWeakObjectPtr<UObject>* FoundAsset = AssetsByTab.Find(Tab))
	{
		if (FoundAsset->IsValid())
		{
			return Cast<AssetClass>(FoundAsset->Get());
		}
	}

	return nullptr;
}

void FBAAssetEditorHandler::BindAssetOpenedDelegate()
{
	// TODO: OnAssetEditorRequestClose is not being properly called in 4.26, maybe this will work in the future?

#if ENGINE_MINOR_VERSION < 24 && ENGINE_MAJOR_VERSION == 4
	OnAssetOpenedDelegateHandle = FAssetEditorManager::Get().OnAssetOpenedInEditor().AddRaw(this, &FBAAssetEditorHandler::OnAssetOpenedInEditor);
#else
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	OnAssetOpenedDelegateHandle = AssetEditorSubsystem->OnAssetOpenedInEditor().AddRaw(this, &FBAAssetEditorHandler::OnAssetOpenedInEditor);
#endif
}

void FBAAssetEditorHandler::UnbindAssetOpenedDelegate()
{
	if (!GEditor)
	{
		return;
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

	if (OnAssetOpenedDelegateHandle.IsValid())
	{
#if ENGINE_MINOR_VERSION < 24 && ENGINE_MAJOR_VERSION == 4
		FAssetEditorManager::Get().OnAssetOpenedInEditor().Remove(OnAssetOpenedDelegateHandle);
#else
		AssetEditorSubsystem->OnAssetOpenedInEditor().Remove(OnAssetOpenedDelegateHandle);
#endif
		OnAssetOpenedDelegateHandle.Reset();
	}
}

void FBAAssetEditorHandler::OnAssetOpenedInEditor(UObject* Asset, IAssetEditorInstance* AssetEditor)
{
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("AssetEditor opened %s (%s)"), *Asset->GetName(), *AssetEditor->GetEditorName().ToString());

	TSharedPtr<SDockTab> Tab = AssetEditor->GetAssociatedTabManager()->GetOwnerTab();
	AssetsByTab.Add(TWeakPtr<SDockTab>(Tab), Asset);

	if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
	{
		BlueprintHandlers.FindOrAdd(Blueprint->GetBlueprintGuid()).BindBlueprintChanged(Blueprint);
	}
}

void FBAAssetEditorHandler::OnAssetClosed(UObject* Asset)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
	{
		const FGuid& BPGuid = Blueprint->GetBlueprintGuid();
		if (FBABlueprintHandler* FoundHandler = BlueprintHandlers.Find(BPGuid))
		{
			FoundHandler->UnbindBlueprintChanged(Blueprint);
			BlueprintHandlers.Remove(BPGuid);
		}
	}
}

void FBAAssetEditorHandler::CheckInvalidAssetEditors()
{
	TArray<TWeakPtr<SDockTab>> Tabs;
	AssetsByTab.GetKeys(Tabs);

	const auto IsTabInvalid = [](const TWeakPtr<SDockTab> Tab) { return !Tab.IsValid(); };
	TArray<TWeakPtr<SDockTab>> InvalidTabs = Tabs.FilterByPredicate(IsTabInvalid);

	for (TWeakPtr<SDockTab> Tab : InvalidTabs)
	{
		if (AssetsByTab[Tab].IsValid())
		{
			OnAssetClosed(AssetsByTab[Tab].Get());
		}

		AssetsByTab.Remove(Tab);
	}
}
