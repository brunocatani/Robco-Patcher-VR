#include "EngineAdapters.h"

#include <array>
#include <cmath>
#include <cstring>
#include <mutex>

namespace EngineAdapters
{
	namespace
	{
		constexpr std::uintptr_t kLeveledObjectDestructorRva = 0x43900;
		constexpr std::uintptr_t kEffectItemInitializerRva = 0x6D910;
		constexpr std::uintptr_t kEffectItemDestructorRva = 0x6D9D0;
		constexpr std::uintptr_t kOmodSetDataRva = 0x4B310;
		constexpr std::uintptr_t kOmodFreeBufferRva = 0x4F280;
		constexpr std::uintptr_t kOmodGetDataRva = 0x4F3C0;

		constexpr std::array<std::uint8_t, 17> kLeveledObjectDestructorSignature{
			0x48, 0x89, 0x5C, 0x24, 0x18, 0x56, 0x48, 0x83, 0xEC,
			0x20, 0x8B, 0xF2, 0x48, 0x8B, 0xD9, 0xF6, 0xC2
		};
		constexpr std::array<std::uint8_t, 20> kEffectItemInitializerSignature{
			0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10,
			0x57, 0x48, 0x83, 0xEC, 0x30, 0x33, 0xC0, 0x48, 0x8B, 0xF1
		};
		constexpr std::array<std::uint8_t, 12> kEffectItemDestructorSignature{
			0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8D, 0x59, 0x20, 0x48, 0x8B, 0xCB
		};
		constexpr std::array<std::uint8_t, 21> kOmodSetDataSignature{
			0x48, 0x89, 0x5C, 0x24, 0x18, 0x55, 0x56, 0x57, 0x48, 0x83,
			0xEC, 0x20, 0x8B, 0x5A, 0x10, 0x48, 0x8B, 0xF2, 0x8B, 0x52, 0x14
		};
		constexpr std::array<std::uint8_t, 16> kOmodFreeBufferSignature{
			0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83,
			0xEC, 0x20, 0x48, 0x8B, 0x39, 0x48, 0x8B, 0xD9
		};
		constexpr std::array<std::uint8_t, 13> kOmodGetDataSignature{
			0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x0F,
			0xB6, 0x81, 0xC6, 0x00, 0x00, 0x00
		};

		template <std::size_t N>
		bool VerifyTarget(
			std::uintptr_t resolvedAddress,
			std::uintptr_t expectedRva,
			const std::array<std::uint8_t, N>& signature,
			std::string_view targetName) noexcept
		{
			if (!IsSupportedRuntime()) {
				logger::critical(FMT_STRING("{} rejected: unsupported executable"), targetName);
				return false;
			}

			const auto expectedAddress = REL::Module::get().base() + expectedRva;
			if (resolvedAddress != expectedAddress) {
				logger::critical(
					FMT_STRING("{} rejected: address library resolved {:016X}, expected {:016X}"),
					targetName,
					resolvedAddress,
					expectedAddress);
				return false;
			}

			if (std::memcmp(reinterpret_cast<const void*>(resolvedAddress), signature.data(), signature.size()) != 0) {
				logger::critical(FMT_STRING("{} rejected: live executable signature mismatch"), targetName);
				return false;
			}

			logger::info(FMT_STRING("Verified FO4VR engine adapter: {}"), targetName);
			return true;
		}
	}

	bool IsSupportedRuntime() noexcept
	{
		return REL::Module::IsVR() && REL::Module::get().version() == F4SE::RUNTIME_VR_1_2_72;
	}

	bool VerifyLeveledObjectDestructor() noexcept
	{
		static std::once_flag once;
		static bool verified = false;
		std::call_once(once, [] {
			const REL::Relocation<std::uintptr_t> target{ REL::ID(296092) };
			verified = VerifyTarget(
				target.address(),
				kLeveledObjectDestructorRva,
				kLeveledObjectDestructorSignature,
				"LEVELED_OBJECT vector destructor");
		});
		return verified;
	}

	bool VerifyEffectItemInitializer() noexcept
	{
		static std::once_flag once;
		static bool verified = false;
		std::call_once(once, [] {
			const REL::Relocation<std::uintptr_t> target{ REL::ID(1219158) };
			verified = VerifyTarget(
				target.address(),
				kEffectItemInitializerRva,
				kEffectItemInitializerSignature,
				"EffectItem initializer") &&
				VerifyTarget(
					REL::Module::get().base() + kEffectItemDestructorRva,
					kEffectItemDestructorRva,
					kEffectItemDestructorSignature,
					"EffectItem destructor");
		});
		return verified;
	}

	bool VerifyOmodBufferFunctions() noexcept
	{
		static std::once_flag once;
		static bool verified = false;
		std::call_once(once, [] {
			const auto base = REL::Module::get().base();
			verified = VerifyTarget(
				base + kOmodGetDataRva,
				kOmodGetDataRva,
				kOmodGetDataSignature,
				"OMOD data decoder") &&
				VerifyTarget(
					base + kOmodSetDataRva,
					kOmodSetDataRva,
					kOmodSetDataSignature,
					"OMOD data rebuilder") &&
				VerifyTarget(
					base + kOmodFreeBufferRva,
					kOmodFreeBufferRva,
					kOmodFreeBufferSignature,
					"OMOD buffer destructor");
		});
		return verified;
	}

	bool DestroyLeveledObjectArray(RE::LEVELED_OBJECT* objects) noexcept
	{
		if (!objects || !VerifyLeveledObjectDestructor()) {
			return objects == nullptr;
		}

		using Function = void (*)(RE::LEVELED_OBJECT*, std::uint32_t);
		const REL::Relocation<Function> destroy{ REL::ID(296092) };
		destroy(objects, 0x3);
		return true;
	}

	static RE::EffectItem* InitializeEffectItem(
		RE::EffectItem* item,
		RE::EffectSetting* setting,
		float magnitude,
		std::int32_t duration,
		std::int32_t area) noexcept
	{
		if (!item || !setting || !std::isfinite(magnitude) || !VerifyEffectItemInitializer()) {
			return nullptr;
		}

		using Function = RE::EffectItem* (*)(RE::EffectItem*, RE::EffectSetting*, float, std::int32_t, std::int32_t);
		const REL::Relocation<Function> initialize{ REL::ID(1219158) };
		return initialize(item, setting, magnitude, duration, area);
	}

	RE::EffectItem* CreateEffectItem(
		RE::EffectSetting* setting,
		float magnitude,
		std::int32_t duration,
		std::int32_t area) noexcept
	{
		if (!setting || !std::isfinite(magnitude) || !VerifyEffectItemInitializer()) {
			return nullptr;
		}

		auto& memoryManager = RE::MemoryManager::GetSingleton();
		auto* storage = static_cast<RE::EffectItem*>(memoryManager.Allocate(sizeof(RE::EffectItem), 0, false));
		if (!storage) {
			return nullptr;
		}

		if (auto* initialized = InitializeEffectItem(storage, setting, magnitude, duration, area)) {
			return initialized;
		}
		memoryManager.Deallocate(storage, false);
		return nullptr;
	}

	void DestroyEffectItem(RE::EffectItem* item) noexcept
	{
		if (!item) {
			return;
		}
		if (!VerifyEffectItemInitializer()) {
			logger::critical("EffectItem cleanup rejected because the verified destructor is unavailable");
			return;
		}

		using Function = void (*)(RE::EffectItem*);
		const REL::Relocation<Function> destroy{ REL::Offset(kEffectItemDestructorRva) };
		destroy(item);
		RE::MemoryManager::GetSingleton().Deallocate(item, false);
	}

	bool ReadOmodData(
		const RE::BGSMod::Attachment::Mod* omod,
		RE::BGSMod::Attachment::Mod::Data& data) noexcept
	{
		data = {};
		if (!omod || !VerifyOmodBufferFunctions()) {
			return false;
		}

		using Function = RE::BGSMod::Attachment::Mod::Data* (*)(
			const RE::BGSMod::Attachment::Mod*,
			RE::BGSMod::Attachment::Mod::Data*);
		const REL::Relocation<Function> getData{ REL::Offset(kOmodGetDataRva) };
		return getData(omod, std::addressof(data)) == std::addressof(data);
	}

	bool ReplaceOmodData(
		RE::BGSMod::Attachment::Mod* omod,
		const RE::BGSMod::Attachment::Mod::Data& data) noexcept
	{
		if (!omod || !VerifyOmodBufferFunctions() ||
			(data.attachmentCount > 0 && !data.attachments) ||
			(data.propertyModCount > 0 && !data.propertyMods)) {
			return false;
		}

		constexpr auto kElementSize = sizeof(RE::BGSMod::Property::Mod);
		static_assert(kElementSize == sizeof(RE::BGSMod::Attachment::Instance));
		const auto totalCount = static_cast<std::uint64_t>(data.attachmentCount) + data.propertyModCount;
		if (totalCount > ((std::numeric_limits<std::uint32_t>::max)() - 8u) / kElementSize) {
			return false;
		}

		using SetDataFunction = void (*)(RE::BGSMod::Container*, const RE::BGSMod::Container::Data*);
		using FreeBufferFunction = void (*)(RE::BGSMod::Container*);
		const REL::Relocation<SetDataFunction> setData{ REL::Offset(kOmodSetDataRva) };
		const REL::Relocation<FreeBufferFunction> freeBuffer{ REL::Offset(kOmodFreeBufferRva) };

		RE::BGSMod::Container replacement{};
		const auto& containerData = static_cast<const RE::BGSMod::Container::Data&>(data);
		setData(std::addressof(replacement), std::addressof(containerData));
		if (totalCount > 0 && !replacement.buffer) {
			freeBuffer(std::addressof(replacement));
			return false;
		}

		auto* target = static_cast<RE::BGSMod::Container*>(omod);
		RE::BGSMod::Container previous{};
		previous.buffer = target->buffer;
		previous.size = target->size;
		target->buffer = replacement.buffer;
		target->size = replacement.size;
		replacement.buffer = nullptr;
		replacement.size = 0;
		freeBuffer(std::addressof(previous));
		return true;
	}
}
