#pragma once

#include "CoreMinimal.h"
#include "TickableEditorObject.h"
#include "Renderer/RendererToolViewport.h"

class FRendererDisplayer;

class FRendererDisplayerModule
{
public:

	static TSharedPtr<FRendererDisplayer> CreateDefaultRendererDisplayer();

};

class FRendererToolViewportClient;

class FRendererDisplayer : public SWindow
{
public:

	FRendererDisplayer() = default;

	FRendererDisplayer(TSharedPtr<SViewport> InViewportWidget);

	virtual ~FRendererDisplayer();

	void Construct(const FArguments& Arguments, TSharedPtr<SViewport> InViewportWidget);

	virtual void Tick(float InDeltaTime);

	FORCEINLINE TSharedPtr<FViewport> GetViewportClient() const
	{
		return ViewportClient->GetViewport();
	}

private:

	TSharedPtr<FRendererToolViewportClient> ViewportClient = nullptr;

	TObjectPtr<UWorld> World = nullptr;

	friend class FRendererDisplayerSystem;

};

class FRendererDisplayerSystem : public FTickableEditorObject
{
public:

	FRendererDisplayerSystem() = default;

	~FRendererDisplayerSystem() = default;

	virtual void Tick(float InDeltaTime) override;

	virtual ETickableTickType GetTickableTickType() const override;

	virtual TStatId GetStatId() const override;

	static FRendererDisplayerSystem& Get()
	{
		static FRendererDisplayerSystem Singleton;
		return Singleton;
	}

	static TSharedPtr<FRendererDisplayer> CreateRendererDisplayer();

	static void RemoveRendererDisplayer(TSharedPtr<FRendererDisplayer> InDisplayer);

private:

	TArray<TSharedPtr<FRendererDisplayer>> Displayers;

};