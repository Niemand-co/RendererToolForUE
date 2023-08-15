#pragma once

struct FViewportTransform
{
public:

	FViewportTransform();

	void SetLocation(const FVector& InLocation)
	{
		Location = InLocation;
	}

	void SetRotation(const FRotator& InRotation)
	{
		Rotation = InRotation;
	}

	FVector& GetLocation()
	{
		return Location;
	}

	const FVector& GetLocation() const
	{
		return Location;
	}

	FRotator& GetRotation()
	{
		return Rotation;
	}

	const FRotator& GetRotation() const
	{
		return Rotation;
	}

private:

	FVector Location;

	FRotator Rotation;

};

class FRendererToolViewportClient : public FViewportClient, public FViewElementDrawer, public FGCObject
{
public:

	FRendererToolViewportClient(UWorld* InWorld, TSharedPtr<SViewport> InViewportWidget);

	~FRendererToolViewportClient();

	virtual UWorld* GetWorld() const override { return World.Get(); }

	TSharedPtr<FSceneViewport> GetViewport() const { return Viewport; }

	FSceneInterface* GetScene() const;

	virtual void Draw(FViewport* InViewport, FCanvas* InCanvas);

	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;

	bool SupportsPreviewResolutionFraction() const;

public:

	FORCEINLINE bool IsPerspective() const
	{
		return (ViewType == ELevelViewportType::LVT_Perspective);
	}

	FORCEINLINE bool IsOrtho() const
	{
		return !IsPerspective();
	}

	FORCEINLINE FViewportTransform& GetViewTransform()
	{
		return IsPerspective() ? PerspectiveViewTransform : OrthographicViewTransform;
	}

	FORCEINLINE const FViewportTransform& GetViewTransform() const
	{
		return IsPerspective() ? PerspectiveViewTransform : OrthographicViewTransform;
	}

	FORCEINLINE float GetNearPlane() const
	{
		return (NearPlane < 0.0f) ? GNearClippingPlane : NearPlane;
	}

	FORCEINLINE EViewModeIndex GetViewMode() const
	{
		return IsPerspective() ? PerspViewModexIndex : OrthoViewModeIndex;
	}

private:

	FSceneView* CalcSceneView(FSceneViewFamily *InViewFamily);

protected:

	TObjectPtr<UWorld> World;

	TSharedPtr<class FSceneViewport> Viewport;

	FEngineShowFlags EngineShowFlags;

	FSceneViewStateReference ViewState;

	ELevelViewportType ViewType;

	int32 ViewIndex;

	float ViewFOV;

	float AspectRatio;

	float NearPlane;

	float FarPlane;

	FViewportTransform PerspectiveViewTransform;

	FViewportTransform OrthographicViewTransform;

	uint32 LastMouseX;
	uint32 LastMouseY;

	FIntPoint CurrentCursorPos;

	EViewModeIndex PerspViewModexIndex;

	EViewModeIndex OrthoViewModeIndex;

};