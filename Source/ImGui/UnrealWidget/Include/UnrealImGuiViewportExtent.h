﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Math/TransformCalculus2D.h"
#include "UnrealImGuiViewportExtent.generated.h"

class UUnrealImGuiViewportBase;
struct FUnrealImGuiViewportContext;

UCLASS(Abstract, config = ImGuiPanelConfig, PerObjectConfig)
class IMGUI_API UUnrealImGuiViewportExtentBase : public UObject
{
	GENERATED_BODY()
public:
	UUnrealImGuiViewportExtentBase()
		: bEnable(true)
	{}

	int32 Priority = 0;
	FText ExtentName;

	UPROPERTY(Config)
	uint8 bEnable : 1;

	virtual bool ShouldCreateExtent(UObject* Owner, UUnrealImGuiViewportBase* Viewport) const { return true; }
	virtual void Register(UObject* Owner, UUnrealImGuiViewportBase* Viewport) {}
	virtual void Unregister(UObject* Owner, UUnrealImGuiViewportBase* Viewport) {}
	virtual void WhenEnable(UObject* Owner, UUnrealImGuiViewportBase* Viewport) {}
	virtual void WhenDisable(UObject* Owner, UUnrealImGuiViewportBase* Viewport) {}
	virtual void DrawViewportMenu(UObject* Owner, bool& bIsConfigDirty) {}
	virtual void DrawViewportContent(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext) {}

	struct FPassDrawer
	{
		int32 Priority = 0;
		TFunction<void(UObject* Owner, const FUnrealImGuiViewportContext& ViewportContext)> Drawer;
	};
	virtual TArray<FPassDrawer> GetPassDrawers(UObject* Owner, UUnrealImGuiViewportBase* Viewport) { return {}; }

	virtual void ResetSelection() {}

	virtual void WhenFilterStringChanged(UUnrealImGuiViewportBase* Viewport, const FString& FilterString) {}
	virtual void DrawFilterTooltip(UUnrealImGuiViewportBase* Viewport) {}
	virtual void DrawFilterPopup(UUnrealImGuiViewportBase* Viewport) {}
	virtual void FocusEntitiesByFilter(UUnrealImGuiViewportBase* Viewport) {}

	UUnrealImGuiViewportBase* GetViewport() const;
};

struct IMGUI_API FUnrealImGuiViewportContext
{
	FUnrealImGuiViewportContext(UUnrealImGuiViewportBase* Viewport, struct ImDrawList* DrawList, const FVector2D& ContentMin, const FVector2D& ContentSize, const FVector2D& ViewLocation, float Zoom, const FVector2D& MousePos, bool bIsContentHovered, bool bIsContentActive, bool bIsViewDragEnd, bool IsSelectDragging, const FVector2D& MouseClickedPos, float DeltaSeconds)
		: Viewport(Viewport)
		, DrawList(DrawList)
		, ContentMin(ContentMin)
		, ContentSize(ContentSize)
		, Zoom(Zoom)
		, WorldToScreenTransform{ FScale2D{Zoom}, -ViewLocation * Zoom + ContentSize / 2.f + ContentMin }
		, ScreenToWorldTransform{ WorldToScreenTransform.Inverse() }
		, ViewBounds{ ScreenToWorldTransform.TransformPoint(ContentMin), ScreenToWorldTransform.TransformPoint(ContentSize + ContentMin) }
		, MousePos(MousePos)
		, MouseWorldPos(ScreenToWorldTransform.TransformPoint(MousePos))
		, bIsContentHovered(bIsContentHovered)
		, bIsContentActive(bIsContentActive)
		, bIsViewDragEnd(bIsViewDragEnd)
		, bIsSelectDragging(IsSelectDragging)
		, SelectDragBounds{ IsSelectDragging ? TArray<FVector2D>{ ScreenToWorldTransform.TransformPoint(MouseClickedPos), ScreenToWorldTransform.TransformPoint(MousePos) } : FBox2D() }
		, DeltaSeconds(DeltaSeconds)
		, bIsSelected(false)
		, bIsTopSelected(false)
		, bIsConfigDirty(false)
	{}

	UUnrealImGuiViewportBase* Viewport;
	struct ImDrawList* DrawList;
	const FVector2D ContentMin;
	const FVector2D ContentSize;
	const FVector2D ViewLocation;
	const float Zoom;
	const FTransform2D WorldToScreenTransform;
	const FTransform2D ScreenToWorldTransform;
	const FBox2D ViewBounds;
	const FVector2D MousePos;
	const FVector2D MouseWorldPos;
	const uint8 bIsContentHovered : 1;
	const uint8 bIsContentActive : 1;
	const uint8	bIsViewDragEnd : 1;
	const uint8 bIsSelectDragging : 1;
	const FBox2D SelectDragBounds;
	const float DeltaSeconds;

	mutable uint8 bIsSelected : 1;
	mutable uint8 bIsTopSelected : 1;

	FBox GetViewBounds3D() const
	{
		return FBox{ FVector{ ViewBounds.Min, -MAX_flt }, FVector{ ViewBounds.Max, MAX_FLT } };
 	}
	FVector2D GetScreenCenter() const
	{
		return ContentMin + ContentMin / 2.f;
	}
	FVector2D WorldToContentLocation(const FVector2D& Location) const
	{
		return WorldToScreenTransform.TransformPoint(Location) - ContentMin;
	}
	FVector2D WorldToScreenLocation(const FVector2D& Location) const
	{
		return WorldToScreenTransform.TransformPoint(Location);
	}
	FVector2D ContentToWorldLocation(const FVector2D& Location) const
	{
		return ScreenToWorldTransform.TransformPoint(Location + ContentMin);
	}
	FVector2D ScreenToWorldLocation(const FVector2D& Location) const
	{
		return ScreenToWorldTransform.TransformPoint(Location);
	}
	FVector2D AddScreenOffset(const FVector2D& Position, const FVector2D& ScreenOffset) const
	{
		return Position + ScreenOffset / Zoom;
	}

	void DrawLine(const FVector2D& Start, const FVector2D& End, const FColor& Color, float Thickness = 1.f) const;
	void DrawTriangle(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FColor& Color, float Thickness = 1.f) const;
	void DrawTriangleFilled(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FColor& Color) const;
	void DrawRect(const FBox2D& Box, const FColor& Color, float Rounding = 0.f, float Thickness = 1.f) const;
	void DrawRectFilled(const FBox2D& Box, const FColor& Color, float Rounding = 0.f) const;
	void DrawCircle(const FVector2D& Center, float Radius, const FColor& Color, int NumSegments = 0, float Thickness = 1.0f) const;
	void DrawCircleFilled(const FVector2D& Center, float Radius, const FColor& Color, int NumSegments = 0) const;
	void DrawQuad(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FVector2D& P4, const FColor& Color, float Thickness = 1.0f) const;
	void DrawQuadFilled(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FVector2D& P4, const FColor& Color) const;
	void DrawText(const FVector2D& Position, const FString& Text, const FColor& Color) const;
	void AddMessageText(const FString& Message, const FColor& Color) const { Messages.Add(FMessage{ Message, Color }); }
	void MarkConfigDirty() const { bIsConfigDirty |= true; }
private:
	friend class UUnrealImGuiViewportBase;

	using ImU32 = uint32;
	static ImU32 FColorToU32(const FColor& Color);

	mutable float MessageHeight = 0.f;
	struct FMessage
	{
		FString Text;
		FColor Color;
	};
	mutable TArray<FMessage> Messages;
	mutable uint8 bIsConfigDirty : 1;
};
