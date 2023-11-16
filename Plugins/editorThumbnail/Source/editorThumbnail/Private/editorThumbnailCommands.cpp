// Copyright Epic Games, Inc. All Rights Reserved.

#include "editorThumbnailCommands.h"

#define LOCTEXT_NAMESPACE "FeditorThumbnailModule"

void FeditorThumbnailCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "editorThumbnail", "Execute editorThumbnail action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
