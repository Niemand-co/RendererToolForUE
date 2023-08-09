#pragma once

#include "Slate/SceneViewport.h"

class FRendererToolViewport : public FSceneViewport
{
public:

	FRendererToolViewport(FViewportClient* InViewportClient, TSharedPtr<SViewport> InViewportWidget);

	~FRendererToolViewport();

};

class FRendererToolViewportClient : public FViewportClient
{
public:

	FRendererToolViewportClient(UWorld* InWorld, TSharedPtr<SViewport> InViewportWidget);

	~FRendererToolViewportClient();

	virtual UWorld* GetWorld() const override { return World.Get(); }

	TSharedPtr<FViewport> GetViewport() const { return Viewport; }

protected:

	TObjectPtr<UWorld> World;

	TSharedPtr<FViewport> Viewport;

};