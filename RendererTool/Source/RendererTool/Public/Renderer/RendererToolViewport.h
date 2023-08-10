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

	FSceneInterface* GetScene() const;

	virtual void Draw(FViewport* InViewport, FCanvas* InCanvas);

protected:

	TObjectPtr<UWorld> World;

	TSharedPtr<FSceneViewport> Viewport;

	class UTextureRenderTarget2D* RenderTarget;

};