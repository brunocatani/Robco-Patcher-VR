function(robco_verify_fo4vr_loader source_path)
  if(NOT EXISTS "${source_path}")
    message(FATAL_ERROR "FO4VR loader source is missing: ${source_path}")
  endif()

  file(READ "${source_path}" loader_source)
  string(FIND "${loader_source}" "F4SEPlugin_Query" query_start)
  string(FIND "${loader_source}" "F4SEPlugin_Load" load_start)
  if(query_start EQUAL -1 OR load_start EQUAL -1 OR load_start LESS_EQUAL query_start)
    message(FATAL_ERROR "FO4VR loader contract regression: Query/Load boundaries are invalid")
  endif()

  math(EXPR query_length "${load_start} - ${query_start}")
  string(SUBSTRING "${loader_source}" "${query_start}" "${query_length}" query_source)

  foreach(required_text IN ITEMS
      "a_f4se->IsEditor()"
      "REL::Module::IsVR()"
      "const auto requiredRuntime = F4SE::RUNTIME_1_10_138"
      "a_f4se->RuntimeVersion() < requiredRuntime"
      "REL::Module::get().version()"
      "executableVersion != F4SE::RUNTIME_VR_1_2_72")
    string(FIND "${query_source}" "${required_text}" found_at)
    if(found_at EQUAL -1)
      message(FATAL_ERROR "FO4VR Query contract regression: '${required_text}' is absent")
    endif()
  endforeach()

  foreach(required_text IN ITEMS
      "F4SE::Init(a_f4se, false)"
      "EngineAdapters::IsSupportedRuntime()"
      "RegisterListener(MessageHandler)"
      "InitializeFormResolver()"
      "F4SE::MessagingInterface::kGameDataReady")
    string(FIND "${loader_source}" "${required_text}" found_at)
    if(found_at EQUAL -1)
      message(FATAL_ERROR "FO4VR Load contract regression: '${required_text}' is absent")
    endif()
  endforeach()

  if(query_source MATCHES "RuntimeVersion\\(\\)[ \t\r\n]*[=!<>]+[ \t\r\n]*F4SE::RUNTIME_(LATEST_)?VR")
    message(FATAL_ERROR "FO4VR Query contract regression: loader RuntimeVersion was compared with a VR executable constant")
  endif()
  if(query_source MATCHES "F4SE::RUNTIME_(LATEST_)?VR_[A-Za-z0-9_]*[ \t\r\n]*[=!<>]+[ \t\r\n]*a_f4se->RuntimeVersion\\(\\)")
    message(FATAL_ERROR "FO4VR Query contract regression: a VR executable constant was compared with loader RuntimeVersion")
  endif()
endfunction()

if(DEFINED LOADER_SOURCE)
  robco_verify_fo4vr_loader("${LOADER_SOURCE}")
endif()
