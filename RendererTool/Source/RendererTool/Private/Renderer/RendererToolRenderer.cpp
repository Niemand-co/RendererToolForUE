#include "Renderer/RendererToolRenderer.h"
#include "Renderer/Private/DynamicPrimitiveDrawing.h"
#include "ClearQuad.h"

template <typename T>
inline T* CheckPointer(T* Ptr)
{
	check(Ptr != nullptr);
	return Ptr;
}

FDisplayerViewInfo::FDisplayerViewInfo(const FSceneView* InSceneView)
	: FSceneView(*InSceneView)
{
}

FDisplayerViewInfo::~FDisplayerViewInfo()
{
}

FDisplayerViewFamilyInfo::FDisplayerViewFamilyInfo(const FSceneViewFamily* InViewFamily)
	: Scene(CheckPointer(InViewFamily->Scene)->GetRenderScene())
	, RenderTarget(InViewFamily->RenderTarget)
	, EngineShowFlags(InViewFamily->EngineShowFlags)
{
	Views.SetNumZeroed(InViewFamily->Views.Num());
}

FDisplayerViewFamilyInfo::~FDisplayerViewFamilyInfo()
{
}

FDisplayerSceneRenderer::FDisplayerSceneRenderer(const FSceneViewFamily* InViewFamily, FHitProxyConsumer* InHitProxyConsumer)
	: Scene(CheckPointer(InViewFamily->Scene)->GetRenderScene())
	, ViewFamily(CheckPointer(InViewFamily))
	, FeatureLevel(CheckPointer(InViewFamily)->GetFeatureLevel())
{
	check(Scene != nullptr);
	check(IsInGameThread());

	Views.Empty(InViewFamily->Views.Num());
	for (int32 ViewIndex = 0; ViewIndex < InViewFamily->Views.Num(); ++ViewIndex)
	{
		FDisplayerViewInfo* ViewInfo = &Views.Emplace_GetRef(InViewFamily->Views[ViewIndex]);
		ViewInfo->Family = InViewFamily;
		ViewFamily.Views[ViewIndex] = ViewInfo;
	}
}

FDisplayerSceneRenderer* FDisplayerSceneRenderer::CreateDisplayerSceneRenderer(const FSceneViewFamily* InViewFamily, FHitProxyConsumer* InHitProxyConsumer)
{
	TArray<FDisplayerSceneRenderer*> Renderers;

	TArray<const FSceneViewFamily*> ViewFamilies;
	ViewFamilies.Add(InViewFamily);

	CreateDisplayerSceneRenderers(ViewFamilies, InHitProxyConsumer, Renderers);

	return Renderers[0];
}

void FDisplayerSceneRenderer::CreateDisplayerSceneRenderers(TArrayView<const FSceneViewFamily*> InViewFamilies, FHitProxyConsumer* InHitProxyConsumer, TArray<FDisplayerSceneRenderer*>& OutRenderers)
{
	EDisplayerShadingPath ShadingPath = EDisplayerShadingPath::BlueProtocol;

	OutRenderers.SetNum(InViewFamilies.Num());
	for (int32 ViewFamilyIndex = 0; ViewFamilyIndex < InViewFamilies.Num(); ++ViewFamilyIndex)
	{
		const FSceneViewFamily* ViewFamily = InViewFamilies[ViewFamilyIndex];
		if (ShadingPath == EDisplayerShadingPath::BlueProtocol)
		{
			OutRenderers[ViewFamilyIndex] = new FBlueProtocolRenderer(ViewFamily, InHitProxyConsumer);
		}
	}
}

void FDisplayerSceneRenderer::RenderThreadBegin(FRHICommandListImmediate& RHICmdList, const TArray<FDisplayerSceneRenderer*>& SceneRenderers)
{
	CleanUp(RHICmdList);
}

static void DeleteSceneRenderers(const TArray<FDisplayerSceneRenderer*>& SceneRenderers, FParallelMeshDrawCommandPass::EWaitThread WaitThread)
{
	SCOPED_NAMED_EVENT_TEXT("DeleteSceneRenderer", FColor::Red);

	//for (FDisplayerSceneRenderer* SceneRenderer : SceneRenderers)
	//{
	//	//// Wait for all dispatched shadow mesh draw tasks.
	//	//for (int32 PassIndex = 0; PassIndex < SceneRenderer->DispatchedShadowDepthPasses.Num(); ++PassIndex)
	//	//{
	//	//	SceneRenderer->DispatchedShadowDepthPasses[PassIndex]->WaitForTasksAndEmpty(WaitThread);
	//	//}

	//	for (FViewInfo& View : SceneRenderer->Views)
	//	{
	//		View.WaitForTasks(WaitThread);
	//	}
	//}

	for (FDisplayerSceneRenderer* SceneRenderer : SceneRenderers)
	{
		delete SceneRenderer;
	}
}

static void WaitForTasksAndDeleteSceneRenderers(FRHICommandListImmediate& RHICmdList, const TArray<FDisplayerSceneRenderer*>& SceneRenderers)
{
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_DeleteSceneRenderer_WaitForTasks);
		RHICmdList.ImmediateFlush(EImmediateFlushType::WaitForOutstandingTasksOnly);
	}

	DeleteSceneRenderers(SceneRenderers, FParallelMeshDrawCommandPass::EWaitThread::Render);
}

static void FinishCleanUp(FRHICommandListImmediate& RHICmdList)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FinishCleanUp);
}

void FDisplayerSceneRenderer::RenderThreadEnd(FRHICommandListImmediate& RHICmdList, const TArray<FDisplayerSceneRenderer*>& SceneRenderers)
{
	// We need to sync async uniform expression cache updates since we're about to start deleting material proxies.
	FUniformExpressionCacheAsyncUpdateScope::WaitForTask();

	WaitForTasksAndDeleteSceneRenderers(RHICmdList, SceneRenderers);
	FinishCleanUp(RHICmdList);
}

void FDisplayerSceneRenderer::CleanUp(FRHICommandListImmediate& RHICmdList)
{
	return;

	FinishCleanUp(RHICmdList);
}

FBlueProtocolRenderer::FBlueProtocolRenderer(const FSceneViewFamily* InViewFamily, FHitProxyConsumer* HitProxyConsumer)
	: FDisplayerSceneRenderer(InViewFamily, HitProxyConsumer)
{
}

FBlueProtocolRenderer::~FBlueProtocolRenderer()
{
}

BEGIN_SHADER_PARAMETER_STRUCT(FBlueProtocolRenderPassParameter, )
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

void FBlueProtocolRenderer::Render(FRDGBuilder& GraphBuilder)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FBlueProtocalRenderer_Render);

	FRDGTextureRef SceneColor = nullptr;
	if (const FRenderTarget* RT = ViewFamily.RenderTarget)
	{
		SceneColor = RegisterExternalTexture(GraphBuilder, RT->GetRenderTargetTexture(), TEXT("ViewFamilyTexture"));
	}

	FBlueProtocolRenderPassParameter* Parameters = GraphBuilder.AllocParameters<FBlueProtocolRenderPassParameter>();
	Parameters->RenderTargets[0] = FRenderTargetBinding(SceneColor, ERenderTargetLoadAction::ENoAction);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("SceneColorRendering"),
		Parameters,
		ERDGPassFlags::Raster | ERDGPassFlags::NeverMerge,
		[this](FRHICommandListImmediate& RHICmdList)
	{
		DrawClearQuad(RHICmdList, FLinearColor(0, 0, 0, 1));
	});
}


