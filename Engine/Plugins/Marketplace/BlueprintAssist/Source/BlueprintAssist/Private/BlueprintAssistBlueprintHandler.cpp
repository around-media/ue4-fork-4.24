// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistBlueprintHandler.h"

#include "BlueprintAssistSettings.h"
#include "BlueprintAssistUtils.h"
#include "Kismet2/BlueprintEditorUtils.h"

FBABlueprintHandler::~FBABlueprintHandler()
{
	// we shouldn't need this but for safety...
	if (BlueprintPtr.IsValid())
	{
		BlueprintPtr->OnChanged().RemoveAll(this);
	}
}

void FBABlueprintHandler::BindBlueprintChanged(UBlueprint* Blueprint)
{
	if (!Blueprint->OnChanged().IsBoundToObject(this))
	{
		LastVariables = Blueprint->NewVariables;
		Blueprint->OnChanged().AddRaw(this, &FBABlueprintHandler::OnBlueprintChanged);
	}

	BlueprintPtr = Blueprint;
}

void FBABlueprintHandler::UnbindBlueprintChanged(UBlueprint* Blueprint)
{
	Blueprint->OnChanged().RemoveAll(this);
}

// See UControlRigBlueprint::OnPostVariableChange
void FBABlueprintHandler::OnBlueprintChanged(UBlueprint* Blueprint)
{
	TMap<FGuid, int32> NewVariablesByGuid;
	for (int32 VarIndex = 0; VarIndex < Blueprint->NewVariables.Num(); VarIndex++)
	{
		// we use the storage within the CDO for the default values,
		// no need to maintain the default value as a string
		NewVariablesByGuid.Add(Blueprint->NewVariables[VarIndex].VarGuid, VarIndex);
	}

	TMap<FGuid, int32> OldVariablesByGuid;
	for (int32 VarIndex = 0; VarIndex < LastVariables.Num(); VarIndex++)
	{
		OldVariablesByGuid.Add(LastVariables[VarIndex].VarGuid, VarIndex);
	}

	TArray<FBPVariableDescription> SavedLastVariables = LastVariables;
	LastVariables = Blueprint->NewVariables;

	for (FBPVariableDescription& NewVariable : Blueprint->NewVariables)
	{
		if (!OldVariablesByGuid.Contains(NewVariable.VarGuid))
		{
			OnVariableAdded(Blueprint, NewVariable);
			continue;
		}

		const int32 OldVarIndex = OldVariablesByGuid.FindChecked(NewVariable.VarGuid);
		const FBPVariableDescription& OldVariable = SavedLastVariables[OldVarIndex];

		// Make set instance editable to true when you set expose on spawn to true
		if (FBAUtils::HasMetaDataChanged(OldVariable, NewVariable, FBlueprintMetadata::MD_ExposeOnSpawn))
		{
			if (NewVariable.HasMetaData(FBlueprintMetadata::MD_ExposeOnSpawn) && NewVariable.GetMetaData(FBlueprintMetadata::MD_ExposeOnSpawn) == TEXT("true"))
			{
				FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, NewVariable.VarName, false);
			}
		}

		// Check if a variable has been renamed
		if (OldVariable.VarName != NewVariable.VarName)
		{
			OnVariableRenamed(Blueprint, OldVariable, NewVariable);
		}
	}
}

void FBABlueprintHandler::OnVariableAdded(UBlueprint* Blueprint, FBPVariableDescription& Variable)
{
	const UBASettings* BASettings = GetDefault<UBASettings>();
	if (BASettings->bEnableVariableDefaults)
	{
		if (BASettings->bDefaultInstanceEditable)
		{
			FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, Variable.VarName, false);
		}

		if (BASettings->bDefaultBlueprintReadOnly)
		{
			FBlueprintEditorUtils::SetBlueprintPropertyReadOnlyFlag(Blueprint, Variable.VarName, true);
		}

		if (BASettings->bDefaultExposeOnSpawn)
		{
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, Variable.VarName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));
		}

		if (BASettings->bDefaultPrivate)
		{
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, Variable.VarName, nullptr, FBlueprintMetadata::MD_Private, TEXT("true"));
		}

		if (BASettings->bDefaultExposeToCinematics)
		{
			FBlueprintEditorUtils::SetInterpFlag(Blueprint, Variable.VarName, true);
		}

		FBlueprintEditorUtils::SetBlueprintVariableCategory(Blueprint, Variable.VarName, nullptr, BASettings->DefaultCategory);

		FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, Variable.VarName, nullptr, FBlueprintMetadata::MD_Tooltip, BASettings->DefaultTooltip.ToString());
	}
}

void FBABlueprintHandler::OnVariableRenamed(UBlueprint* Blueprint, const FBPVariableDescription& OldVariable, FBPVariableDescription& NewVariable)
{
	if (GetDefault<UBASettings>()->bAutoRenameGettersAndSetters)
	{
		RenameGettersAndSetters(Blueprint, OldVariable, NewVariable);
	}
}

void FBABlueprintHandler::RenameGettersAndSetters(UBlueprint* Blueprint, const FBPVariableDescription& OldVariable, FBPVariableDescription& NewVariable)
{
	const FString GetterName = FString::Printf(TEXT("Get%s"), *OldVariable.VarName.ToString());
	const FString SetterName = FString::Printf(TEXT("Set%s"), *OldVariable.VarName.ToString());

	for (UEdGraph* FunctionGraph : Blueprint->FunctionGraphs)
	{
		if (FunctionGraph->GetName() == GetterName)
		{
			const FString NewName = FString::Printf(TEXT("Get%s"), *NewVariable.VarName.ToString());
			FBlueprintEditorUtils::RenameGraph(FunctionGraph, NewName);
		}
		else if (FunctionGraph->GetName() == SetterName)
		{
			const FString NewName = FString::Printf(TEXT("Set%s"), *NewVariable.VarName.ToString());
			FBlueprintEditorUtils::RenameGraph(FunctionGraph, NewName);
		}
	}
}
