#pragma once

#include "Slate/SceneViewport.h"

class FRendererToolViewport : public FSceneViewport
{
public:

	FRendererToolViewport(FViewportClient* InViewportClient, TSharedPtr<SViewport> InViewportWidget);

	~FRendererToolViewport();

	virtual void ProcessInput(float InDeltaTime) override;

};