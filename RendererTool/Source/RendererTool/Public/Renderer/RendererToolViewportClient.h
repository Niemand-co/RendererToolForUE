#pragma once

class FRendererToolViewportClient : public FEditorViewportClient
{
public:

	FRendererToolViewportClient(UWorld* InWorld, TSharedPtr<SViewport> InViewportWidget);

	~FRendererToolViewportClient();

	virtual UWorld* GetWorld() const override { return World.Get(); }

	TSharedPtr<FSceneViewport> GetViewport() const { return LocalViewport; }

	//virtual void Draw(FViewport* InViewport, FCanvas* InCanvas);

	virtual void Tick(float DeltaSeconds) override;

protected:

	TObjectPtr<UWorld> World;

	TSharedPtr<class FSceneViewport> LocalViewport;
};