// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitCBMenuExtention();
}


//generates a whole block that can be minimized
#pragma region ContentBrowserMenuExtention 
void FSuperManagerModule::InitCBMenuExtention()
{
	//Loading ContenBrowser Module
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	//Getting array with all path of DELEGATES of the actual view context menu right click
	//by placing "&" when declaring means that we are creating a reference to the object instead of creating a new "variable" otherwise
	// if we dont use "&" we create a new variable which will be a copy of the object's address
	//so it's really important to use "&" here cause later we are gonna modified this array to add our custom path with our function
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	
	//GENERAL RULE
	//from now on if my module function generate a "const" or a "&" (reference) we should declare it as "const" or "&"

	/*
	// this will be my custom delegate, right now is empty, needs to be binded
	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);
	// Adding my custom delegate to the array of delegates of the context menu that we got before
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);
	*/


	//First Binding
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));

}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry));//Second Binding
			
	}
	return MenuExtender;
}
void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely delete all unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnsuedAssetButtonClicked)//Third binding
	);
}
void FSuperManagerModule::OnDeleteUnsuedAssetButtonClicked()
{

}
#pragma endregion

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)