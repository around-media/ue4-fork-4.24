// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

class FBABlueprintHandler
{
public:
	~FBABlueprintHandler();

	void BindBlueprintChanged(UBlueprint* Blueprint);

	void UnbindBlueprintChanged(UBlueprint* Blueprint);

	TArray<FBPVariableDescription> LastVariables;

	void OnBlueprintChanged(UBlueprint* Blueprint);

	void OnVariableAdded(UBlueprint* Blueprint, FBPVariableDescription& Variable);

	void OnVariableRenamed(UBlueprint* Blueprint, const FBPVariableDescription& OldVariable, FBPVariableDescription& NewVariable);

	void RenameGettersAndSetters(UBlueprint* Blueprint, const FBPVariableDescription& OldVariable, FBPVariableDescription& NewVariable);

private:
	TWeakObjectPtr<UBlueprint> BlueprintPtr;
};
