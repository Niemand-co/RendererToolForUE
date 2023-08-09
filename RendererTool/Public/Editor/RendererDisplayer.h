#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "TickableEditorObject.h"

class FRendererDisplayer : public SWindow
{
public:

	FRendererDisplayer();

	~FRendererDisplayer();

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:

	UWorld *World;


};