if(NOT DEFINED VALIDATOR)
  message(FATAL_ERROR "VALIDATOR is required")
endif()

set(test_dir "${CMAKE_CURRENT_BINARY_DIR}/loader-validator-fixtures")
file(MAKE_DIRECTORY "${test_dir}")

set(good_source "${test_dir}/good.cpp")
file(WRITE "${good_source}" [=[
bool F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo*) {
  a_f4se->IsEditor();
  REL::Module::IsVR();
  const auto requiredRuntime = F4SE::RUNTIME_1_10_138;
  a_f4se->RuntimeVersion() < requiredRuntime;
  const auto executableVersion = REL::Module::get().version();
  executableVersion != F4SE::RUNTIME_VR_1_2_72;
}
bool F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se) {
  F4SE::Init(a_f4se, false);
  EngineAdapters::IsSupportedRuntime();
  RegisterListener(MessageHandler);
  InitializeFormResolver();
  F4SE::MessagingInterface::kGameDataReady;
}
]=])

execute_process(
  COMMAND "${CMAKE_COMMAND}" -DLOADER_SOURCE=${good_source} -P "${VALIDATOR}"
  RESULT_VARIABLE good_result
  OUTPUT_VARIABLE good_output
  ERROR_VARIABLE good_error)
if(NOT good_result EQUAL 0)
  message(FATAL_ERROR "Valid loader fixture was rejected:\n${good_output}\n${good_error}")
endif()

set(bad_source "${test_dir}/bad.cpp")
file(READ "${good_source}" bad_text)
string(REPLACE
  "a_f4se->RuntimeVersion() < requiredRuntime;"
  "a_f4se->RuntimeVersion() != F4SE::RUNTIME_VR_1_2_72;"
  bad_text "${bad_text}")
file(WRITE "${bad_source}" "${bad_text}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -DLOADER_SOURCE=${bad_source} -P "${VALIDATOR}"
  RESULT_VARIABLE bad_result
  OUTPUT_VARIABLE bad_output
  ERROR_VARIABLE bad_error)
if(bad_result EQUAL 0)
  message(FATAL_ERROR "Invalid cross-domain loader fixture was accepted")
endif()
