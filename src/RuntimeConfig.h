#pragma once

#include <filesystem>

namespace RuntimeConfig
{
	std::filesystem::path PathFromGameDirectory(const std::filesystem::path& gameDirectory);
	std::filesystem::path ResolvePath();

	// Creates a missing INI from compiled defaults; never rewrites an existing file.
	// Returns true only when this call created it. I/O failures throw to Load's boundary.
	bool EnsureExists(const std::filesystem::path& path);
}
