// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "editorThumbnailStyle.h"

class FeditorThumbnailCommands : public TCommands<FeditorThumbnailCommands>
{
public:

	FeditorThumbnailCommands()
		: TCommands<FeditorThumbnailCommands>(TEXT("editorThumbnail"), NSLOCTEXT("Contexts", "editorThumbnail", "editorThumbnail Plugin"), NAME_None, FeditorThumbnailStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
