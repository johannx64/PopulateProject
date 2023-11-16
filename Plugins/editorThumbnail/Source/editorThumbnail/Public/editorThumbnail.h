#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"


#include "Engine/TextureRenderTarget2D.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
class FeditorThumbnailModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    // Function to find and print available game mods
    
    void FindAvailableGameMods(TArray<TSharedRef<IPlugin>>& OutAvailableGameMods);
    void ScanAssetsAndLog();
    void RenderThumbnailForAsset(const FString& AssetPath);


    // Function triggered on button click
    void PluginButtonClicked();

    // Function to register menus
    void RegisterMenus();

private:
    // Pointer to the UI command list
    TSharedPtr<FUICommandList> PluginCommands;
};
