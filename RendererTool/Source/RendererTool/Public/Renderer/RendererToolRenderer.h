#pragma once

#include "Runtime/Renderer/Private/SceneRendering.h"

enum class EDisplayerShadingPath : uint8
{
	BlueProtocol = 0,
	GenshinImpact = 1,
};

class FDisplayerViewInfo : public FSceneView
{
public:

	FDisplayerViewInfo(const FSceneView* InSceneView);
	virtual ~FDisplayerViewInfo();

};

class FDisplayerViewFamilyInfo
{
public:

	const FRenderTarget* RenderTarget;

	FScene* Scene;

	TArray<FDisplayerViewInfo*> Views;

	FEngineShowFlags EngineShowFlags;

	FDisplayerViewFamilyInfo(const FSceneViewFamily* InViewFamily);
	~FDisplayerViewFamilyInfo();

};

class FDisplayerSceneRenderer
{
public:

	FScene* Scene;

	/** The view family being rendered.  This references the Views array. */
	FDisplayerViewFamilyInfo ViewFamily;

	/** The views being rendered. */
	TArray<FDisplayerViewInfo> Views;

	ERHIFeatureLevel::Type FeatureLevel;


public:

	FDisplayerSceneRenderer(const FSceneViewFamily* InViewFamily, FHitProxyConsumer* InHitProxyConsumer);
	virtual ~FDisplayerSceneRenderer() = default;

	static FDisplayerSceneRenderer* CreateDisplayerSceneRenderer(const FSceneViewFamily* InViewFamily, FHitProxyConsumer* InHitProxyConsumer);
	static void CreateDisplayerSceneRenderers(TArrayView<const FSceneViewFamily*> InViewFamilies, FHitProxyConsumer* InHitProxyConsumer, TArray<FDisplayerSceneRenderer*>& OutRenderers);

	// Initializes the scene renderer on the render thread.
	static void RenderThreadBegin(FRHICommandListImmediate& RHICmdList, const TArray<FDisplayerSceneRenderer*>& SceneRenderers);
	static void RenderThreadEnd(FRHICommandListImmediate& RHICmdList, const TArray<FDisplayerSceneRenderer*>& SceneRenderers);

	virtual void Render(FRDGBuilder& GraphBuilder) = 0;
	virtual void RenderHitProxies(FRDGBuilder& GraphBuilder) {}

	static void CleanUp(FRHICommandListImmediate& RHICmdList);
};

class FBlueProtocolRenderer : public FDisplayerSceneRenderer
{
public:

	FBlueProtocolRenderer(const FSceneViewFamily* InViewFamily, FHitProxyConsumer* InHitProxyConsumer);

	virtual ~FBlueProtocolRenderer();

	virtual void Render(FRDGBuilder& GraphBuilder) override;

private:

	void RenderBasePass(FRDGBuilder& GraphBuilder);

};