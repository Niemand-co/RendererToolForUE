// Copyright Epic Games, Inc. All Rights Reserved.

#include "RendererTool.h"
#include "ToolMenus.h"
#include "RendererToolActions.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "FRendererToolModule"

void FRendererToolModule::StartupModule()
{
	RegisterRendererToolMenu();
}

void FRendererToolModule::ShutdownModule()
{
	UnregisterRendererToolMenu();
}

void FRendererToolModule::RegisterRendererToolMenu()
{
	FRendererToolCommands::Register();
	ActionList = MakeShareable(new FUICommandList);

	ActionList->MapAction(
		FRendererToolCommands::Get().CreateDisplayer,
		FExecuteAction::CreateStatic(&FRendererToolActionCallbacks::CreateNewDisplayer),
		FCanExecuteAction()
	);

	//FInputBindingManager::Get().RegisterCommandList(FRendererToolCommands::Get().GetContextName(), ActionList);

	UToolMenu *MenuBar = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu");
	MenuBar->AddSubMenu(
		"LevelEditor.MainMenu",
		NAME_None,
		"Renderer",
		LOCTEXT("RendererMenu", "Renderer"),
		LOCTEXT("RendererMenu_ToolTip", "Open the renderer menu")
	);

	UToolMenu* Menu = UToolMenus::Get()->RegisterMenu("LevelEditor.MainMenu.Renderer");
	{
		FToolMenuSection& Section = Menu->AddSection("Displayer", LOCTEXT("DisplayerHeading", "Displayer"));
		Section.AddMenuEntryWithCommandList(FRendererToolCommands::Get().CreateDisplayer, ActionList);
	}
}

void FRendererToolModule::UnregisterRendererToolMenu()
{
	FRendererToolCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRendererToolModule, RendererTool)