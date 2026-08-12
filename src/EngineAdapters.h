#pragma once

#include "PCH.h"

namespace EngineAdapters
{
	bool IsSupportedRuntime() noexcept;
	bool VerifyLeveledObjectDestructor() noexcept;
	bool VerifyEffectItemInitializer() noexcept;
	bool VerifyOmodBufferFunctions() noexcept;

	bool DestroyLeveledObjectArray(RE::LEVELED_OBJECT* objects) noexcept;
	RE::EffectItem* CreateEffectItem(
		RE::EffectSetting* setting,
		float magnitude,
		std::int32_t duration,
		std::int32_t area) noexcept;
	void DestroyEffectItem(RE::EffectItem* item) noexcept;

	bool ReadOmodData(
		const RE::BGSMod::Attachment::Mod* omod,
		RE::BGSMod::Attachment::Mod::Data& data) noexcept;
	bool ReplaceOmodData(
		RE::BGSMod::Attachment::Mod* omod,
		const RE::BGSMod::Attachment::Mod::Data& data) noexcept;
}
