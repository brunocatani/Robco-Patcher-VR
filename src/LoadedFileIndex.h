#pragma once
#include <cstdint>
#include <optional>

namespace LoadedFiles
{
	enum class Domain { inactive, full, light, invalid };
	constexpr Domain Classify(std::uint8_t index, std::uint16_t smallIndex, bool small) noexcept
	{
		if (index == 0xFF) return Domain::inactive;
		if (!small && index < 0xFE) return Domain::full;
		if (small && index == 0xFE && smallIndex < 0x1000) return Domain::light;
		return Domain::invalid;
	}
	constexpr std::optional<std::uint32_t> FormID(std::uint8_t index, std::uint16_t smallIndex,
		bool small, std::uint32_t localID) noexcept
	{
		switch (Classify(index, smallIndex, small)) {
		case Domain::full:
			if (localID <= 0xFFFFFF) return (static_cast<std::uint32_t>(index) << 24) | localID;
			break;
		case Domain::light:
			if (localID <= 0xFFF) return 0xFE000000u | (static_cast<std::uint32_t>(smallIndex) << 12) | localID;
			break;
		default: break;
		}
		return std::nullopt;
	}
}
