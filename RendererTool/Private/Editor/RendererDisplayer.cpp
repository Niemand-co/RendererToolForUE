#include "Editor/RendererDisplayer.h"
#include "Framework/Application/MenuStack.h"

FRendererDisplayer::FRendererDisplayer()
{

	World = UWorld::CreateWorld( EWorldType::Editor, true, TEXT("Renderer Tool World"));

	
}

FRendererDisplayer::~FRendererDisplayer()
{
}

void FRendererDisplayer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	World->Tick(ELevelTick::LEVELTICK_All, InDeltaTime);
}