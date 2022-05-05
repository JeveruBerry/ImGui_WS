// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPropertyDetails.h"

/**
 * 
 */
namespace UnrealImGui
{
	struct IMGUI_API FPropertyDisableScope
	{
		FPropertyDisableScope(bool IsDisable);
		~FPropertyDisableScope();
	private:
		bool Disable;
	};

	struct IMGUI_API FBoolPropertyCustomization : public IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_API FNumericPropertyCustomization : public IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_API FObjectPropertyCustomization : public IUnrealPropertyCustomization
	{
		FObjectPropertyCustomization()
		{
			bOverrideHasChildProperties = true;
		}

		bool HasChildPropertiesOverride(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateChildrenWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	private:
		mutable TWeakObjectPtr<UClass> CachedAssetClass;
		mutable TArray<FAssetData> CachedAssetList;

		mutable TWeakObjectPtr<UClass> CachedInstancedClass;
		mutable TArray<TWeakObjectPtr<UClass>> CachedClassList;
	};

	struct IMGUI_API FSoftObjectPropertyCustomization : public IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	private:
		mutable TWeakObjectPtr<UClass> CachedAssetClass;
		mutable TArray<FAssetData> CachedAssetList;

		mutable TWeakObjectPtr<UClass> CachedActorClass;
		mutable TArray<TWeakObjectPtr<AActor>> CachedActorList;
	};

	struct IMGUI_API FClassPropertyCustomization : public IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	private:
		mutable TWeakObjectPtr<UClass> CachedInstancedClass;
		mutable TArray<TWeakObjectPtr<UClass>> CachedClassList;
	};

	struct IMGUI_API FSoftClassPropertyCustomization : public IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	private:
		mutable TWeakObjectPtr<UClass> CachedInstancedClass;
		mutable TArray<TWeakObjectPtr<UClass>> CachedClassList;
	};

	struct IMGUI_API FStringPropertyCustomization : public IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_API FNamePropertyCustomization : public IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_API FTextPropertyCustomization : public IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_API FEnumPropertyCustomization : public IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_API FArrayPropertyCustomization : public IUnrealPropertyCustomization
	{
		FArrayPropertyCustomization()
		{
			bOverrideHasChildProperties = true;
		}

		bool HasChildPropertiesOverride(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;

		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;

		void CreateChildrenWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_API FSetPropertyCustomization : public IUnrealPropertyCustomization
	{
		FSetPropertyCustomization()
		{
			bOverrideHasChildProperties = true;
		}

		bool HasChildPropertiesOverride(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;

		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;

		void CreateChildrenWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_API FMapPropertyCustomization : public IUnrealPropertyCustomization
	{
		FMapPropertyCustomization()
		{
			bOverrideHasChildProperties = true;
		}

		bool HasChildPropertiesOverride(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;

		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;

		void CreateChildrenWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_API FStructPropertyCustomization : public IUnrealPropertyCustomization
	{
		FStructPropertyCustomization()
		{
			bHasChildProperties = true;
		}

		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;

		void CreateChildrenWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override;
	};
}