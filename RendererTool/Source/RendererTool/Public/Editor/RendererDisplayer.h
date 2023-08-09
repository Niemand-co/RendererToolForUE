#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "TickableEditorObject.h"

class FRendererDisplayerModule
{
public:

	static TSharedRef<SWidget> GetDispalyerContent();

};

class FRendererDisplayer : public SWindow
{
public:

	FRendererDisplayer();

	virtual ~FRendererDisplayer();

	virtual void Tick(float InDeltaTime);

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