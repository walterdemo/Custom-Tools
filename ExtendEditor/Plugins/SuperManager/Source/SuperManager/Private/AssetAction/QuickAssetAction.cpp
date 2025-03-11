// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/QuickAssetAction.h"
#include "DebugHeader.h"

#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"

#include "ObjectTools.h"

#include "AssetRegistry/AssetRegistryModule.h"  //for Loading AssetRegistryModule, to filter out all the redirectors
#include "AssetToolsModule.h" //Loading AssetTools Module , for fixing up the redirectors 



void UQuickAssetAction::DuplicateAsset(int32 NumOfDuplicates)
{
	//checking a valid number input for duplicate
	if (NumOfDuplicates <= 0)
	{
		/*Print(TEXT("Please enter a VALID number"), FColor::Red);*/
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"));
		return;
	}

	//Get a list of selected objects
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	//initialize counter
	uint32 Counter = 0;

	//for every object in lista
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		//for every object we duplicated i times
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{

			//objectPath  :: Game/Public/object.object
			//packagePath :: Game/Publi/
			//AssetName :: object.object
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();//getting the objectPath name
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);//getting the object name and adding an identifier number
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);//generating a new objectPath by merging the packagePath and the new name
			//duplicating the asset (it requires objectPath and )
			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName)) //both inputs are objectpath, source and goal
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);//saving new assets
				++Counter;
			}
		}
	}
	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"));
		//Print(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"), FColor::Green);
	}
}


void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;//this will keep track of how many assets are changing
	for (UObject* SelectedObject : SelectedObjects)
	{
		if (!SelectedObject) continue;
		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());//PrefixFound is a pointer, means its an address to the string that we are calling
		
		//we always have to check the pointer 
		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}
		FString OldName = SelectedObject->GetName();
		//the * is there because we want the name of the pointer (not the address)
		if (OldName.StartsWith(*PrefixFound))// because this is a pointer, in order to get the value of it,  we need to referenc it with the *
		{
			DebugHeader::Print(OldName + TEXT(" already has prefix added"), FColor::Red);
			continue;
		}

		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);
		++Counter;
	}
	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(Counter) + " assets"));
	}
}


void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	FixUpRedirectors();//in case we have moved any object to other folder

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		TArray<FString> AssetRefrencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.ObjectPath.ToString());
		if (AssetRefrencers.Num() == 0)
		{
			UnusedAssetsData.Add(SelectedAssetData);//if no ona is using thius data
		}
	}
	if (UnusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets"), false);
		return;
	}
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);
	if (NumOfAssetsDeleted == 0) return;
	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets")));
}

void UQuickAssetAction::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray; //creating Array for store redirectors that are in content folder, array is empty in this line
	//this array has to be of UObjectRedirector pointer type
	
	//we use & because this module need to be loaded only once
	//FModule is used to load different modules
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")); //Loading AssetRegistryModule, to filter out all the redirectors
	
	FARFilter Filter; //creating a Filter of FARFilter type, is unset in this line
	Filter.bRecursivePaths = true; //assigning property to look in subfolders
	Filter.PackagePaths.Emplace("/Game");// To tell it to look in the Game Folder (Content)
	Filter.ClassNames.Emplace("ObjectRedirector");//Tell it look objects of ObjectRedirector type
	
	TArray<FAssetData> OutRedirectors; //Create an empty Array for store Redirectors, this redirectors will be store in next line but they will be AssetDatatype
	
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);// Using the module to get the assets according our Filter and saving them into OutRedirectors Array

	for (const FAssetData& RedirectorData : OutRedirectors) // Since we need our Array made of UObjectRedirector type
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset())) // If the assetData can be casted as UObjectRedirector type
		{
			RedirectorsToFixArray.Add(RedirectorToFix);//we add this Redirector objects into our Array (RedirectorsToFixArray)
		}
	}

	//we use & because this module need to be loaded only once
	//FModule is used to load different modules
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));//Loading AssetTools Module , for fixing up the redirectors 

	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);//Using AssetTool Module to Fix our Redirectors
}