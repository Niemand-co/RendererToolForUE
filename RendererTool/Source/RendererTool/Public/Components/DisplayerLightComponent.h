#pragma once

static class FDisplayerLightSceneProxy* CreateDisplayerSceneProxy(class ULightComponent* Light);

static class FDisplayerLightSceneProxy* GetDisplayerSceneProxy(class ULightComponent* Light);

static void EmptyDisplayerSceneProxy(class ULightComponent* Light);