#pragma once

#include "PCH.h"

#include <regex>
#include <string>
#include <vector>

bool InitializeFormResolver();
bool IsPluginInstalled(const char* name);
RE::TESForm* GetFormFromIdentifier(const std::string& identifier);
bool FormMatchesModNames(const RE::TESForm* form, const std::vector<std::string>& modNames);

std::int32_t getPropertyFromString(
	std::string text,
	RE::ENUM_FORM_ID targetFormType = RE::ENUM_FORM_ID::kWEAP);
int findPositionInArray(const RE::BSTArray<RE::TESForm*>& array, RE::TESForm* form);

bool changeAVIF_NPC(RE::TESNPC* npc, RE::ActorValueInfo* actorValue, float value);
bool changeAVIF_Race(RE::TESRace* race, RE::ActorValueInfo* actorValue, float value);
bool changeDamageType_Weapon(RE::TESObjectWEAP* object, RE::BGSDamageType* type, float value);
bool changeDamageType_Armor(RE::TESObjectARMO* object, RE::BGSDamageType* type, float value);
bool changeDamageTypeMult_Armor(RE::TESObjectARMO* object, RE::BGSDamageType* type, float value);
bool changeKeyword_TESLevItem(RE::TESLevItem* list, RE::BGSKeyword* keyword, float value);
bool eraseDamageType_Weapon(RE::TESObjectWEAP* object, RE::BGSDamageType* type);

std::vector<std::string> splitRelationNumber(const std::string& input);
std::string trim(const std::string& value);
std::string toLowerCase(std::string value);
std::string to_string(RE::BipedObjectSlot slot);
RE::BipedObjectSlot getBipedObjectSlot(int slot);

int getRandomNumberCustom(int min, int max);
int getRandomNumber();
float getRandomFloat(float min, float max);

bool HasIniExtension(const std::string& fileName);
bool regexSearchParameter(const std::string& line, std::smatch& match, const std::regex& pattern);
void extractForms(const std::string& line, const std::string& pattern, std::vector<std::string>& destination);
void extractStrings(const std::string& line, const std::string& pattern, std::vector<std::string>& destination);
void extractMultiFormsInt(
	const std::string& line,
	const std::string& pattern,
	std::vector<std::string>& flags,
	std::vector<int>& minValues,
	std::vector<int>& maxValues);
void extractMultiFormsFloat(
	const std::string& line,
	const std::string& pattern,
	std::vector<std::string>& flags,
	std::vector<float>& minValues,
	std::vector<float>& maxValues);
void extractMultiFormsString(
	const std::string& line,
	const std::string& pattern,
	std::vector<std::string>& flags,
	std::vector<std::string>& minValues,
	std::vector<std::string>& maxValues);
void extractValueString(const std::string& line, const std::string& pattern, std::string& value);
void extractDataStrings(const std::string& line, const std::string& pattern, std::vector<std::string>& destination);
void extractToArr2D(const std::string& line, const std::string& pattern, std::vector<std::vector<std::string>>& destination);
void extractMultiDataFormsFloat(
	const std::string& line,
	const std::string& pattern,
	std::vector<std::string>& flags,
	std::vector<float>& minValues,
	std::vector<float>& maxValues);

std::string FormatFormID(RE::TESForm* form);
bool ShouldSkipPatch(const std::string& category, RE::TESForm* form);
