// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiHeatMap.h"


#include "imgui.h"
#include "imgui_internal.h"
#include "UnrealImGuiStat.h"

namespace UnrealImGui
{
DECLARE_MEMORY_STAT(TEXT("UnrealImGuiHeatMap_UnitSize"), Stat_UnrealImGuiHeatMap_UnitSize, STATGROUP_ImGui);

template <typename T>
struct MaxIdx { static const unsigned int Value; };
template <> const unsigned int MaxIdx<unsigned short>::Value = 65535;
template <> const unsigned int MaxIdx<unsigned int>::Value   = 4294967295;

struct FRectInfo
{
	ImVec2 Min, Max;
	ImU32 Color[4];
};

template <typename TGetter>
struct RectRenderer
{
    RectRenderer(const TGetter& getter) :
        Getter(getter),
        Prims(Getter.Count)
    {}
    bool operator()(ImDrawList& DrawList, const ImRect& cull_rect, const ImVec2& uv, unsigned int prim) const
	{
        const FRectInfo rect = Getter(prim);
        const ImVec2 P1 = rect.Min;
        const ImVec2 P2 = rect.Max;

		if ((!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))))
            return false;

        DrawList._VtxWritePtr[0].pos   = P1;
        DrawList._VtxWritePtr[0].uv    = uv;
        DrawList._VtxWritePtr[0].col   = rect.Color[0];
        DrawList._VtxWritePtr[1].pos.x = P1.x;
        DrawList._VtxWritePtr[1].pos.y = P2.y;
        DrawList._VtxWritePtr[1].uv    = uv;
        DrawList._VtxWritePtr[1].col   = rect.Color[1];
        DrawList._VtxWritePtr[2].pos   = P2;
        DrawList._VtxWritePtr[2].uv    = uv;
        DrawList._VtxWritePtr[2].col   = rect.Color[2];
        DrawList._VtxWritePtr[3].pos.x = P2.x;
        DrawList._VtxWritePtr[3].pos.y = P1.y;
        DrawList._VtxWritePtr[3].uv    = uv;
        DrawList._VtxWritePtr[3].col   = rect.Color[3];
        DrawList._VtxWritePtr += 4;
        DrawList._IdxWritePtr[0] = DrawList._VtxCurrentIdx;
        DrawList._IdxWritePtr[1] = DrawList._VtxCurrentIdx + 1;
        DrawList._IdxWritePtr[2] = DrawList._VtxCurrentIdx + 3;
        DrawList._IdxWritePtr[3] = DrawList._VtxCurrentIdx + 1;
        DrawList._IdxWritePtr[4] = DrawList._VtxCurrentIdx + 2;
        DrawList._IdxWritePtr[5] = DrawList._VtxCurrentIdx + 3;
        DrawList._IdxWritePtr   += 6;
        DrawList._VtxCurrentIdx += 4;
        return true;
    }
    const TGetter& Getter;
    const unsigned int Prims;
    static constexpr int IdxConsumed = 6;
    static constexpr int VtxConsumed = 4;
};

template <typename Renderer>
void RenderPrimitives(const Renderer& renderer, ImDrawList& DrawList, const ImRect& cull_rect) {
	unsigned int prims        = renderer.Prims;
	unsigned int prims_culled = 0;
	unsigned int idx          = 0;
	const ImVec2 uv = DrawList._Data->TexUvWhitePixel;
	while (prims) {
		// find how many can be reserved up to end of current draw command's limit
		unsigned int cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - DrawList._VtxCurrentIdx) / Renderer::VtxConsumed);
		// make sure at least this many elements can be rendered to avoid situations where at the end of buffer this slow path is not taken all the time
		if (cnt >= ImMin(64u, prims)) {
			if (prims_culled >= cnt)
				prims_culled -= cnt; // reuse previous reservation
			else {
				DrawList.PrimReserve((cnt - prims_culled) * Renderer::IdxConsumed, (cnt - prims_culled) * Renderer::VtxConsumed); // add more elements to previous reservation
				prims_culled = 0;
			}
		}
		else
		{
			if (prims_culled > 0) {
				DrawList.PrimUnreserve(prims_culled * Renderer::IdxConsumed, prims_culled * Renderer::VtxConsumed);
				prims_culled = 0;
			}
			cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - 0/*DrawList._VtxCurrentIdx*/) / Renderer::VtxConsumed);
			DrawList.PrimReserve(cnt * Renderer::IdxConsumed, cnt * Renderer::VtxConsumed); // reserve new draw command
		}
		prims -= cnt;
		for (unsigned int ie = idx + cnt; idx != ie; ++idx) {
			if (!renderer(DrawList, cull_rect, uv, idx))
				prims_culled++;
		}
	}
	if (prims_culled > 0)
		DrawList.PrimUnreserve(prims_culled * Renderer::IdxConsumed, prims_culled * Renderer::VtxConsumed);
}

constexpr ImU32 GetHeatColor(float T)
{
	constexpr ImU32 Jet[] = {4289331200, 4294901760, 4294923520, 4294945280, 4294967040, 4289396565, 4283826090, 4278255615, 4278233855, 4278212095, 4278190335 };
	return Jet[(int)((UE_ARRAY_COUNT(Jet) - 1) * T + 0.5f)];
}

FHeatMapBase::~FHeatMapBase()
{
#if STATS
	const int64 ReleaseUnitSize = Cells.Num() * PreCellUnitCount * PreCellUnitCount * UnitTypeSize;
	DEC_MEMORY_STAT_BY(Stat_UnrealImGuiHeatMap_UnitSize, ReleaseUnitSize);
#endif
}

void FHeatMapBase::DrawHeatMap(const FBox2D& ViewBounds, const FBox2D& CullRect, const float Zoom, const FTransform2D& WorldToMapTransform) const
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UnrealImGuiHeatMap_Draw"), STAT_UnrealImGuiHeatMap_Draw, STATGROUP_ImGui);

	struct FToDrawCell
	{
		FIntVector2 Location;
		const FCell* Cell;
	};
	TArray64<FToDrawCell> ToDrawCells;

	ImDrawList& DrawList = *ImGui::GetWindowDrawList();

	const FIntVector2 CellMin = ToCellLocation(ViewBounds.Min);
	const FIntVector2 CellMax = ToCellLocation(ViewBounds.Max);
	const FVector2D OffsetPixel = WorldToMapTransform.TransformPoint(ViewBounds.GetCenter()) - ViewBounds.GetCenter() * Zoom;

	for (int32 CellX = CellMin.X; CellX <= CellMax.X; ++CellX)
	{
		for (int32 CellY = CellMin.Y; CellY <= CellMax.Y; ++CellY)
		{
			if (const FCell* Cell = Cells.Find({ CellX, CellY }))
			{
				ToDrawCells.Add({ FIntVector2{ CellX, CellY }, Cell });
			}
			else
			{
				DrawList.AddRectFilled({ (float)OffsetPixel.X + CellX * CellSize * Zoom, (float)OffsetPixel.Y + CellY * CellSize * Zoom },
						{(float)OffsetPixel.X + (CellX + 1) * CellSize * Zoom, (float)OffsetPixel.Y + (CellY + 1) * CellSize * Zoom }, GetHeatColor(0.f));
			}
		}
	}
	struct FUnitGetter
	{
		FUnitGetter(const FHeatMapBase& HeatMap, const TArray64<FToDrawCell>& ToDrawCells, int32 PreCellUnitCount, const float CellSize, const float UnitSize, const FVector2D& OffsetPixel)
			: HeatMap(HeatMap)
			, ToDrawCells(ToDrawCells)
			, PreCellUnitCount(PreCellUnitCount)
			, PreCellUnitCountSquared(PreCellUnitCount * PreCellUnitCount)
			, Count(ToDrawCells.Num() * PreCellUnitCountSquared)
			, CellSize(CellSize)
			, UnitSize(UnitSize)
			, OffsetPixel(OffsetPixel.X, OffsetPixel.Y)
		{}

		const FHeatMapBase& HeatMap;
		const TArray64<FToDrawCell>& ToDrawCells;
		const uint32 PreCellUnitCount;
		const uint32 PreCellUnitCountSquared;
		const uint32 Count;
		const float CellSize;
		const float UnitSize;
		const FVector2f OffsetPixel;
		FRectInfo operator()(uint32 TotalUnitIndex) const
		{
			const uint32 CellIndex = TotalUnitIndex / PreCellUnitCountSquared;
			const uint32 UnitIndex = TotalUnitIndex % PreCellUnitCountSquared;
			const uint32 UnitX = UnitIndex % PreCellUnitCount;
			const uint32 UnitY = UnitIndex / PreCellUnitCount;
			const FToDrawCell& ToDrawCell = ToDrawCells[CellIndex];
			const float T = HeatMap.GetUnitValueT_Impl((FUnit&)reinterpret_cast<const uint8*>(ToDrawCell.Cell->Units)[UnitIndex * HeatMap.UnitTypeSize]);

			// TODO：处理跨Cell的Lerp
			auto GetUnitT = [this, &ToDrawCell](uint32 UnitX, uint32 UnitY)
			{
				const int32 Index = UnitY * PreCellUnitCount + UnitX;
				return HeatMap.GetUnitValueT_Impl((FUnit&)reinterpret_cast<const uint8*>(ToDrawCell.Cell->Units)[Index * HeatMap.UnitTypeSize]);
			};
			const float RightT = UnitX + 1 < PreCellUnitCount ? GetUnitT(UnitX + 1, UnitY) : T;
			const float TopT = UnitY >= 1 ? GetUnitT(UnitX, UnitY - 1) : T;
			const float TopRightT = UnitX + 1 < PreCellUnitCount && UnitY >= 1 ? GetUnitT(UnitX + 1, UnitY - 1) : T;

			const ImVec2 Min{ OffsetPixel.X + ToDrawCell.Location.X * CellSize + UnitX * UnitSize, OffsetPixel.Y + ToDrawCell.Location.Y * CellSize + UnitY * UnitSize };
			const ImVec2 Max{ Min.x + UnitSize, Min.y + UnitSize };
			return FRectInfo{ Min, Max, { GetHeatColor(TopT), GetHeatColor(T), GetHeatColor(RightT), GetHeatColor(TopRightT) } };
		}
	};
	const FUnitGetter UnitGetter{ *this, ToDrawCells, PreCellUnitCount, CellSize * Zoom, UnitSize * Zoom, OffsetPixel };
	RenderPrimitives(RectRenderer{ UnitGetter }, DrawList, ImRect{ { (float)CullRect.Min.X, (float)CullRect.Min.Y }, { (float)CullRect.Max.X, (float)CullRect.Max.Y } });
}

void FHeatMapBase::StatMemoryInc(int64 Value)
{
	INC_MEMORY_STAT_BY(Stat_UnrealImGuiHeatMap_UnitSize, Value);
}
}

