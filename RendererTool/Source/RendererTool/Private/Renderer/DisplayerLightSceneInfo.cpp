#include "Renderer/DisplayerLightSceneInfo.h"
#include "Renderer/DisplayerLightSceneProxy.h"

FDisplayerLightSceneInfo::FDisplayerLightSceneInfo(FDisplayerLightSceneProxy* InProxy, bool InbVisible)
	: Proxy(InProxy)
	, Id(INDEX_NONE)
	, bVisible(InbVisible)
	, Scene(InProxy->GetScene())
{
}

FDisplayerLightSceneInfo::~FDisplayerLightSceneInfo()
{
}
