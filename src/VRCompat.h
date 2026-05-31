#pragma once

// VRCompat.h — Compatibility shims for CommonLibF4VR
// CommonLibF4VR is missing some enum definitions and helper methods
// that exist in the flat FO4 CommonLibF4. This header fills those gaps.

#include "RE/Fallout.h"

namespace RE
{
	// =========================================================================
	// Missing enum: BIPED_MODEL::BipedObjectSlot
	// In VR headers, BIPED_MODEL is a struct (not a namespace), so we cannot
	// reopen it. Define BipedObjectSlot inside BIPED_MODEL via a nested
	// workaround: we add it as a free enum in RE and alias it.
	// =========================================================================
	enum class BipedObjectSlot : std::uint32_t
	{
		kNone = 0,
		kHairTop = 1u << 0,
		kHairlong = 1u << 1,
		kBodyFaceGenHead = 1u << 2,
		kBody = 1u << 3,
		kLleftHand = 1u << 4,
		kRightHand = 1u << 5,
		kUTorso = 1u << 6,
		kULeftArm = 1u << 7,
		kURrightArm = 1u << 8,
		kULeftLeg = 1u << 9,
		kURirghtLeg = 1u << 10,
		kATorso = 1u << 11,
		kALeftArm = 1u << 12,
		kARrightArm = 1u << 13,
		kALeftLeg = 1u << 14,
		kARightLeg = 1u << 15,
		kHeadband = 1u << 16,
		kEyes = 1u << 17,
		kBeard = 1u << 18,
		kMouth = 1u << 19,
		kNeck = 1u << 20,
		kRing = 1u << 21,
		kScalp = 1u << 22,
		kDecapitation = 1u << 23,
		kUnnamed1 = 1u << 24,
		kUnnamed2 = 1u << 25,
		kUnnamed3 = 1u << 26,
		kUnnamed4 = 1u << 27,
		kUnnamed5 = 1u << 28,
		kShield = 1u << 29,
		kPipboy = 1u << 30,
		kFX01 = 1u << 31
	};

	// =========================================================================
	// Missing enum: WEAPONHITBEHAVIOR
	// Forward-declared in VR headers but not defined.
	// =========================================================================
	enum class WEAPONHITBEHAVIOR : std::int32_t
	{
		kNormal = 0,
		kDismemberOnly = 1,
		kExplodeOnly = 2,
		kNoDismemberOrExplode = 3
	};

	// =========================================================================
	// Missing enum: SOUND_LEVEL
	// Forward-declared in VR headers but not defined.
	// =========================================================================
	enum class SOUND_LEVEL : std::int32_t
	{
		kLoud = 0,
		kNormal = 1,
		kSilent = 2,
		kVeryLoud = 3,
		kQuiet = 4
	};

	// Forward-declared in CommonLibF4VR but not defined in the public headers.
	enum class STAGGER_MAGNITUDE : std::int32_t
	{
		kNone = 0,
		kSmall = 1,
		kMedium = 2,
		kLarge = 3,
		kExtraLarge = 4
	};
}

// =========================================================================
// fmt formatters for RE:: types that lack them
// =========================================================================
template <typename CharT>
struct fmt::formatter<RE::BGSLocalizedString, CharT> : formatter<const char*, CharT>
{
	template <class FormatContext>
	auto format(const RE::BGSLocalizedString& a_str, FormatContext& a_ctx) const
	{
		return formatter<const char*, CharT>::format(a_str.c_str(), a_ctx);
	}
};

template <bool CS, typename CharT>
struct fmt::formatter<RE::detail::BSFixedString<char, CS>, CharT> : formatter<const char*, CharT>
{
	template <class FormatContext>
	auto format(const RE::detail::BSFixedString<char, CS>& a_str, FormatContext& a_ctx) const
	{
		return formatter<const char*, CharT>::format(a_str.c_str(), a_ctx);
	}
};

// =========================================================================
// Helper functions replacing missing member functions
// =========================================================================
namespace VRCompat
{
	// Replaces BGSAttachParentArray::RemoveKeyword
	// BGSTypedKeywordValueArray uses raw array/size, not BSTArray
	inline void RemoveKeyword(RE::BGSAttachParentArray& arr, RE::BGSKeyword* keyword)
	{
		for (std::uint32_t i = 0; i < arr.size; ++i) {
			const auto kywd = RE::detail::BGSKeywordGetTypedKeywordByIndex(
				RE::KeywordType::kAttachPoint, arr.array[i].keywordIndex);
			if (kywd == keyword) {
				// Shift remaining elements
				for (std::uint32_t j = i; j < arr.size - 1; ++j) {
					arr.array[j] = arr.array[j + 1];
				}
				arr.size--;
				return;
			}
		}
	}

	// Replaces BGSAttachParentArray::HasKeyword2 (same as HasKeyword)
	inline bool HasKeyword2(RE::BGSAttachParentArray& arr, RE::BGSKeyword* keyword)
	{
		return arr.HasKeyword(keyword);
	}

	// Manual AddKeyword implementation — the VR header's AddKeyword is broken
	// (references BGSKeyword::KeywordType which doesn't exist in VR).
	// This generic version works for any BGSTypedKeywordValueArray.
	template <RE::KeywordType TYPE, typename T>
	inline void AddKeywordTyped(T& arr, RE::BGSKeyword* keyword)
	{
		if (!keyword || arr.HasKeyword(keyword))
			return;
		RE::MemoryManager& mm = RE::MemoryManager::GetSingleton();
		auto* newArray = (RE::BGSTypedKeywordValue<TYPE>*)mm.Allocate(
			sizeof(RE::BGSTypedKeywordValue<TYPE>) * (arr.size + 1), 0, false);
		for (std::uint32_t i = 0; i < arr.size; ++i) {
			newArray[i] = arr.array[i];
		}
		RE::BGSTypedKeywordValue<TYPE> newValue;
		newValue.keywordIndex = RE::detail::BGSKeywordGetIndexForTypedKeyword(keyword, TYPE);
		newArray[arr.size] = newValue;
		mm.Deallocate(arr.array, false);
		arr.array = newArray;
		++arr.size;
	}

	// Replaces BGSAttachParentArray::AddKeyword
	inline void AddKeywordAttachPoint(RE::BGSAttachParentArray& arr, RE::BGSKeyword* keyword)
	{
		AddKeywordTyped<RE::KeywordType::kAttachPoint>(arr, keyword);
	}

	// Replaces BGSTypedKeywordValueArray<kRecipeFilter>::AddKeywordRecipe
	template <typename T>
	inline void AddKeywordRecipe(T& arr, RE::BGSKeyword* keyword)
	{
		AddKeywordTyped<RE::KeywordType::kRecipeFilter>(arr, keyword);
	}

	// Replaces BGSTypedKeywordValueArray<kRecipeFilter>::RemoveKeywordRecipe
	template <typename T>
	inline void RemoveKeywordRecipe(T& arr, RE::BGSKeyword* keyword)
	{
		for (std::uint32_t i = 0; i < arr.size; ++i) {
			const auto kywd = RE::detail::BGSKeywordGetTypedKeywordByIndex(
				RE::KeywordType::kRecipeFilter, arr.array[i].keywordIndex);
			if (kywd == keyword) {
				for (std::uint32_t j = i; j < arr.size - 1; ++j) {
					arr.array[j] = arr.array[j + 1];
				}
				arr.size--;
				return;
			}
		}
	}

	// Replaces TESNPC::hasFaction
	inline bool hasFaction(RE::TESNPC* npc, RE::TESFaction* faction)
	{
		return npc->IsInFaction(faction);
	}

	// Replaces TESNPC::AddFaction
	inline void AddFaction(RE::TESNPC* npc, RE::TESFaction* faction, std::int8_t rank)
	{
		RE::FACTION_RANK newRank;
		newRank.faction = faction;
		newRank.rank = rank;
		npc->factions.push_back(newRank);
	}

	// Replaces TESNPC::RemoveFaction
	inline void RemoveFaction(RE::TESNPC* npc, RE::TESFaction* faction)
	{
		for (std::uint32_t i = 0; i < npc->factions.size(); ++i) {
			if (npc->factions[i].faction == faction) {
				npc->factions.erase(npc->factions.begin() + i);
				return;
			}
		}
	}

	// Replaces TESNPC::RemoveObject / TESObjectCONT::RemoveObject
	// These types inherit TESContainer which has containerObjects array
	inline void RemoveObject(RE::TESContainer* container, RE::TESBoundObject* obj)
	{
		for (std::uint32_t i = 0; i < container->numContainerObjects; ++i) {
			if (container->containerObjects[i]->obj == obj) {
				for (std::uint32_t j = i; j < container->numContainerObjects - 1; ++j) {
					container->containerObjects[j] = container->containerObjects[j + 1];
				}
				container->numContainerObjects--;
				return;
			}
		}
	}

	// Replaces BGSMod::Property::Mod::RemoveData()
	// Zeroes out the mod entry data
	inline void RemoveData(RE::BGSMod::Property::Mod& mod)
	{
		mod.data.mm.min.i = 0;
		mod.data.mm.max.i = 0;
	}
}
