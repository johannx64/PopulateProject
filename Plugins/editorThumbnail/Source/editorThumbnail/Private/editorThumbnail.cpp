#include "editorThumbnail.h"
#include "editorThumbnailStyle.h"
#include "editorThumbnailCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "AssetRegistryModule.h"

#include "Engine/Texture2D.h"  

#include "FileHelpers.h"
#include "ObjectTools.h"
#include "Misc/FileHelper.h"

#include "Engine/Texture2D.h"
#include "PackageTools.h"
#include "AssetRegistryModule.h"

#include "FileHelpers.h"
#include "ObjectTools.h"

#include "IImageWrapper.h" 
#include "IImageWrapperModule.h"
#include "UObject/SavePackage.h"

#include "Components/Image.h"
#include "ImageUtils.h"

#include "UObject/ConstructorHelpers.h"  // Include for ConstructorHelpers
#include "Materials/MaterialInstanceConstant.h"  // Include the appropriate header for material instances
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"

#include "UObject/Package.h"

#include "Async/Async.h"

static const FName editorThumbnailTabName("editorThumbnail");

#define LOCTEXT_NAMESPACE "FeditorThumbnailModule"

// Declare variables to store original values for Thumbnail background color
FVector OriginalBaseColor1;
float OriginalEmissiveStrength1;

FVector OriginalBaseColor2;
float OriginalEmissiveStrength2;


void FeditorThumbnailModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

    FeditorThumbnailStyle::Initialize();
    FeditorThumbnailStyle::ReloadTextures();

    FeditorThumbnailCommands::Register();

    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(
        FeditorThumbnailCommands::Get().PluginAction,
        FExecuteAction::CreateRaw(this, &FeditorThumbnailModule::PluginButtonClicked),
        FCanExecuteAction());

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FeditorThumbnailModule::RegisterMenus));

}

void ModifyFloorMat() {
  // Paths
  FString EngineDir = FPaths::EngineDir();
  FString EngineMaterialsDir = EngineDir / "EditorMaterials/Thumbnails";
  
  FString ProjectDir = FPaths::ProjectDir();
  FString ContentDir = ProjectDir / "Content";
  
  FString HelixDir = ContentDir / "HELIX";
  FString OldDir = ContentDir / "Old";

  // Source asset paths
  FString FloorSource = EngineMaterialsDir / "FloorPlaneMaterial.FloorPlaneMaterial";
  FString SkySource = EngineMaterialsDir / "SkySphereMaterial.SkySphereMaterial";

  // Old asset paths  
  FString OldFloor = OldDir / "FloorPlaneMaterial.FloorPlaneMaterial";
  FString OldSky = OldDir / "SkySphereMaterial.SkySphereMaterial";

  // Create Old folder
  if (!FPaths::DirectoryExists(OldDir))
  {
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*OldDir);
    UPackage* Pkg = CreatePackage(*OldDir);
    if (Pkg) 
    {
      Pkg->FullyLoad();
      Pkg->MarkPackageDirty();
    }
  }

  // Copy source assets to Old
  TArray<uint8> FloorData, SkyData;
  FFileHelper::LoadFileToArray(FloorData, *FloorSource);
  FFileHelper::LoadFileToArray(SkyData, *SkySource);
  
  // Create package for Old folder
  UPackage* OldPackage = CreatePackage(*OldDir);

  // Create dummy object 
  UObject* OldObject = NewObject<UObject>(OldPackage, "TempAsset");

  FSavePackageArgs SaveArgs;
  SaveArgs.TopLevelFlags = RF_Standalone;

  // Save object
  UPackage::SavePackage(OldPackage, OldObject, SaveArgs);

  // Delete source assets
  IFileManager::Get().Delete(*FloorSource); 
  IFileManager::Get().Delete(*SkySource);

  // Assets to copy
  FString FloorCopy = HelixDir / "Engine/Thumbnails/FloorPlaneMaterial.uasset";
  FString SkyCopy = HelixDir / "Engine/Thumbnails/SkySphereMaterial.uasset";

  // Dest paths 
  FString EngineFloorDest = EngineMaterialsDir / "FloorPlaneMaterial.FloorPlaneMaterial";
  FString EngineSkyDest = EngineMaterialsDir / "SkySphereMaterial.SkySphereMaterial";  

  // Copy to Engine
  TArray<uint8> FloorCopyData, SkyCopyData;

  FFileHelper::LoadFileToArray(FloorCopyData, *FloorCopy);
  FFileHelper::LoadFileToArray(SkyCopyData, *SkyCopy);
  
  // Create package for Engine folder
  UPackage* EnginePackage = CreatePackage(*EngineDir);

  // Create dummy object
  UObject* EngineObject = NewObject<UObject>(EnginePackage, "TempAsset");

  FSavePackageArgs SaveArgs;
  SaveArgs.TopLevelFlags = RF_Standalone;

  // Save object  
  UPackage::SavePackage(EnginePackage, EngineObject, SaveArgs);

  // Log results
  UE_LOG(LogTemp, Display, TEXT("Copied materials to Old folder"));
  UE_LOG(LogTemp, Display, TEXT("Pasted materials to Engine folder"));


    /*
    //Load sky material 
    //TObjectPtr<UMaterial> SkyMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EditorMaterials/Thumbnails/SkySphereMaterial.SkySphereMaterial"));

    // Load floor material
    TObjectPtr<UMaterial> FloorMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EditorMaterials/Thumbnails/FloorPlaneMaterial.FloorPlaneMaterial"));

    if (FloorMat)
    {
        // Get editor only data
        UMaterialEditorOnlyData* EditorOnlyData = FloorMat->GetEditorOnlyData();

        // Get existing base color expression
        UMaterialExpressionConstant3Vector* BaseColorExpression = Cast<UMaterialExpressionConstant3Vector>(EditorOnlyData->BaseColor.Expression);

        // Modify base color value
        if (BaseColorExpression)
        {
            BaseColorExpression->Constant = FLinearColor(0.0f, 0.0f, 0.0f); // Green
        }

        // Update material
        FloorMat->PostEditChange();
       // FloorMat->MarkPackageDirty();
        // Delay for a frame to apply changes


    }*/
}

void RestoreFloorMat() {
    // Load floor material
    TObjectPtr<UMaterial> FloorMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EditorMaterials/Thumbnails/FloorPlaneMaterial.FloorPlaneMaterial"));

    if (FloorMat)
    {
        // Get editor only data
        UMaterialEditorOnlyData* EditorOnlyData = FloorMat->GetEditorOnlyData();

        // Get existing base color expression
        UMaterialExpressionConstant3Vector* BaseColorExpression = Cast<UMaterialExpressionConstant3Vector>(EditorOnlyData->BaseColor.Expression);

        // Modify base color value
        if (BaseColorExpression)
        {
            BaseColorExpression->Constant = FLinearColor(0.0f, 0.0f, 0.0f); // Green
        }

        // Update material
        FloorMat->PostEditChange();
        FloorMat->MarkPackageDirty();
        // Delay for a frame to apply changes

    }
}


//Method 1
void SaveThumbnail(FString AssetPath, FString OutputFile)
{
    FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(*AssetPath);
    UTexture2D* CreatedTexture = nullptr;

    UObject* Asset = AssetData.GetAsset();
    if (!Asset)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid asset %s"), *AssetPath);
        return;
    }


    // Render thumbnail 
    ThumbnailTools::RenderThumbnail(
        Asset,
        256, 256,
        ThumbnailTools::EThumbnailTextureFlushMode::NeverFlush,
        nullptr,
        nullptr
    );

    FObjectThumbnail* Thumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(Asset);    

    if (!Thumbnail)
    {
        UE_LOG(LogTemp, Error, TEXT("No thumbnail generated for %s"), *AssetPath);
        return;
    }

    TArray64<uint8> PNGData;

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");

    //TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
    EImageFormat Format = EImageFormat::PNG;

    TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(Format);

    if (Wrapper.IsValid())
    {
        Wrapper->SetRaw(Thumbnail->GetUncompressedImageData().GetData(), Thumbnail->GetUncompressedImageData().Num(), Thumbnail->GetImageWidth(), Thumbnail->GetImageHeight(), ERGBFormat::BGRA, 8);
        PNGData = Wrapper->GetCompressed();
    }

    if (PNGData.Num() > 0)
    {

        FFileHelper::SaveArrayToFile(PNGData, *OutputFile);
        UE_LOG(LogTemp, Log, TEXT("SAVED AS PNG for %s"), *OutputFile);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to compress PNG for %s"), *AssetPath);
    }

}

//Method 2
void SaveThumbnailAsTexture(FAssetData Asset)
{
    int32 pathSeparatorIdx;
    FAssetData obj = Asset;
    FString GamePath = obj.GetAsset()->GetPathName();
    FString AssetName;
    int32 pathEnd;
    if (GamePath.FindLastChar('/', pathEnd)) {
        ++pathEnd;
        AssetName += GamePath;
        AssetName.RightChopInline(pathEnd);
        int32 extensionIdx;
        if (AssetName.FindChar('.', extensionIdx)) {
            AssetName.LeftInline(extensionIdx);
        }
        GamePath.LeftInline(pathEnd);
        FString Suffix = "T_";
        FString NameWithSuffix = Suffix + AssetName;
        AssetName = NameWithSuffix;
    }
    else {
        AssetName = "T_Thumbnail";
    }

    if (AssetName.FindChar('/', pathSeparatorIdx)) {
        // TextureName should not have any path separators in it
        return;
    }

    FObjectThumbnail* thumb = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(obj.GetAsset());
    if (!thumb) {
        return;
    }

    FString PackageName = TEXT("/Game/ProceduralTextures/");
    if (!PackageName.EndsWith("/")) {
        PackageName += "/";
    }
    PackageName += AssetName;

    UPackage* Package = CreatePackage(*PackageName);
    Package->FullyLoad();

    UTexture2D* NewTexture = NewObject<UTexture2D>(Package, *AssetName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
    NewTexture->AddToRoot();
    FTexturePlatformData* platformData = new FTexturePlatformData();
    platformData->SizeX = thumb->GetImageWidth();
    platformData->SizeY = thumb->GetImageHeight();
    //platformData->NumSlices = 1;
    platformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;
    NewTexture->SetPlatformData(platformData);

    FTexture2DMipMap* Mip = new FTexture2DMipMap();
    platformData->Mips.Add(Mip);
    Mip->SizeX = thumb->GetImageWidth();
    Mip->SizeY = thumb->GetImageHeight();

    Mip->BulkData.Lock(LOCK_READ_WRITE);
    uint8* TextureData = (uint8*)Mip->BulkData.Realloc(thumb->GetUncompressedImageData().Num() * 4);
    FMemory::Memcpy(TextureData, thumb->GetUncompressedImageData().GetData(), thumb->GetUncompressedImageData().Num());
    Mip->BulkData.Unlock();

    NewTexture->Source.Init(thumb->GetImageWidth(), thumb->GetImageHeight(), 1, 1, ETextureSourceFormat::TSF_BGRA8, thumb->GetUncompressedImageData().GetData());
    NewTexture->LODGroup = TEXTUREGROUP_UI;
    NewTexture->UpdateResource();
    Package->MarkPackageDirty();
    Package->FullyLoad();
    FAssetRegistryModule::AssetCreated(NewTexture);

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    SaveArgs.bForceByteSwapping = true;
    
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
    UE_LOG(LogTemp, Log, TEXT("############# SAVED TO: %s"), *PackageFileName);
    UPackage::SavePackage(Package, NewTexture, *PackageFileName, SaveArgs);


}

//Thumbnail Background

UMaterialInstanceConstant* LoadMaterialInstance(const FString& MaterialPath)
{
    UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(
        StaticLoadObject(UMaterialInstanceConstant::StaticClass(), nullptr, *MaterialPath)
        );

    if (MaterialInstance)
    {
        UE_LOG(LogTemp, Log, TEXT("Material instance loaded successfully for %s"), *MaterialPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load material instance for %s"), *MaterialPath);
    }

    return MaterialInstance;
}

void ModifyMaterialInstance(UMaterialInstanceConstant* MaterialInstance)
{
    if (MaterialInstance)
    {
        // Modify the material properties here

        // Set the emissive color to black
        MaterialInstance->SetVectorParameterValueEditorOnly(FName("EmissiveColor"), FLinearColor::Black);

        // Access the base color node
        FLinearColor BaseColor;
        MaterialInstance->GetVectorParameterValue(FName("BaseColor"), BaseColor);

        // Change the color to green (setting G to 1, other components to 0)
        BaseColor = FLinearColor(0.0f, 1.0f, 0.0f);

        // Set the updated base color
        MaterialInstance->SetVectorParameterValueEditorOnly(FName("BaseColor"), BaseColor);

        // Print a debug log indicating that the material was successfully changed to green
        UE_LOG(LogTemp, Log, TEXT("Material %s changed to green"), *MaterialInstance->GetName());

        // Save the modified material
        MaterialInstance->PostEditChange();
        MaterialInstance->PostLoad();
        MaterialInstance->MarkPackageDirty();
        FAssetRegistryModule::AssetCreated(MaterialInstance);
        FAssetRegistryModule::AssetCreated(MaterialInstance->GetClass());
    }
}

//thumbnail end



void FeditorThumbnailModule::FindAvailableGameMods(TArray<TSharedRef<IPlugin>>& OutAvailableGameMods)
{
    OutAvailableGameMods.Empty();
    

    for (TSharedRef<IPlugin> Plugin : IPluginManager::Get().GetDiscoveredPlugins())
    {
        if (Plugin->GetLoadedFrom() == EPluginLoadedFrom::Project &&
            Plugin->GetType() == EPluginType::Mod)
        {
            FString ModPath = Plugin->GetBaseDir();

            // Iterate assets in plugin directory
            TArray<FString> AssetFileNames;
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
            PlatformFile.FindFilesRecursively(AssetFileNames, *ModPath, *FString(".uasset"));

            for (FString& FileName : AssetFileNames)
            {
               
                TArray<FString> Parts;
                FileName.ParseIntoArray(Parts, TEXT("/"));

                int32 ModsIndex;
                for (int32 i = 0; i < Parts.Num(); i++)
                {
                    if (Parts[i] == "Mods")
                    {
                        ModsIndex = i;
                        break;
                    }
                }

                FString ObjectPath = "/";

                for (int32 i = ModsIndex + 1; i < Parts.Num(); i++)
                {
                    if (Parts[i] != "Content") {
                        ObjectPath += Parts[i] + "/";
                    }                    
                }
                // Remove trailing slash
                
                ObjectPath = ObjectPath.LeftChop(1);
                FString AssetName = ObjectPath.Replace(TEXT(".uasset"), TEXT(""));
                AssetName = AssetName + "." + FPaths::GetCleanFilename(AssetName);

                // Split by '/'
                TArray<FString> PathParts;
                FileName.ParseIntoArray(PathParts, TEXT("/"), true);

                UE_LOG(LogTemp, Log, TEXT("Processing mod asset: %s"), *AssetName);

                // Build thumbnail file name
                FString ThumbnailFilename = ModPath + "/Saved/Thumbnails/" + FPaths::GetCleanFilename(AssetName) + ".png";

                // Save thumbnail
                SaveThumbnail(AssetName, ThumbnailFilename);
                // Create a filter to search for the asset by its path
                
                /*Method 2*/
                /*
                FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
                FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(*AssetName);

                SaveThumbnailAsTexture(AssetData);
                */
                
            }
            //UE_LOG(LogTemp, Log, TEXT("Thumbnail Debug"));
        }
    }
}


void FeditorThumbnailModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    UToolMenus::UnRegisterStartupCallback(this);

    UToolMenus::UnregisterOwner(this);

    FeditorThumbnailStyle::Shutdown();

    FeditorThumbnailCommands::Unregister();
}

//asset scan method
void FeditorThumbnailModule::ScanAssetsAndLog()
{
    // Print a message indicating the module is starting
    UE_LOG(LogTemp, Log, TEXT("cookThumbnails module has started!"));
    // Get the Asset Registry module
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    // Define a TArray to hold the asset dataModifyFloorMat
    TArray<FAssetData> AssetData;

    // Use GetAssets with the specified filter to retrieve the assets
    AssetRegistryModule.Get().GetAllAssets(AssetData, true);

    // Log the asset paths
    for (const FAssetData& Asset : AssetData)
    {
        FString AssetPath = Asset.GetSoftObjectPath().ToString();

        // Check if asset path contains "Mods"
        if (AssetPath.Contains("Basket"))
        {
            UE_LOG(LogTemp, Log, TEXT("Processing mod asset: %s"), *AssetPath);

            // Build thumbnail file name
            FString ThumbnailFilename = FPaths::ProjectDir() + "Thumbnails/" + Asset.AssetName.ToString() + ".png";

            UE_LOG(LogTemp, Log, TEXT("Found asset: %s"), *AssetPath);
            // Save thumbnail
            SaveThumbnail(AssetPath, ThumbnailFilename);
        }

    }
}


void FeditorThumbnailModule::PluginButtonClicked()
{    

    //AsyncTask(ENamedThreads::GameThread, [this]() {
//       ModifyFloorMat();
    //});   
    ModifyFloorMat();

    // Find and print available game mods
    TArray<TSharedRef<IPlugin>> AvailableGameMods;
    FindAvailableGameMods(AvailableGameMods);

    AsyncTask(ENamedThreads::GameThread, [this]() {
        RestoreFloorMat();
        });
    //RestoreFloorMat();

}

void FeditorThumbnailModule::RegisterMenus()
{
    // Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
    FToolMenuOwnerScoped OwnerScoped(this);

    {
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        {
            FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
            Section.AddMenuEntryWithCommandList(FeditorThumbnailCommands::Get().PluginAction, PluginCommands);
        }
    }

    {
        UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
        {
            FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
            {
                FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FeditorThumbnailCommands::Get().PluginAction));
                Entry.SetCommandList(PluginCommands);
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FeditorThumbnailModule, editorThumbnail)
