#pragma once

struct FViewportTransform
{

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

public:

	FORCEINLINE bool IsPerspective() const
	{
		return (ViewType == ELevelViewportType::LVT_Perspective);
	}

	FORCEINLINE bool IsOrtho() const
	{
		return !IsPerspective();
	}

	FORCEINLINE FViewportTransform& GetViewTrasnform()
	{
		return IsPerspective() ? PerspectiveViewTransform : OrthographicViewTransform;
	}

	FORCEINLINE const FViewportTransform& GetViewTrasnform() const
	{
		return IsPerspective() ? PerspectiveViewTransform : OrthographicViewTransform;
	}

private:

	FSceneView* CalcSceneView(FSceneViewFamily *InViewFamily);

protected:

	TObjectPtr<UWorld> World;

	TSharedPtr<class FSceneViewport> Viewport;

	FEngineShowFlags EngineShowFlags;

	ELevelViewportType ViewType;

	FViewportTransform PerspectiveViewTransform;

	FViewportTransform OrthographicViewTransform;

};