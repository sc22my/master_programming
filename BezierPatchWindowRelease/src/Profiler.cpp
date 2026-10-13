#include "Profiler.h"

void Profile::Profiler::Reset() {
    m_Results.clear();
}

void Profile::Profiler::BeginProfile(std::string_view name) {
    // Create a result
    ProfileResult& result = m_Results[name];

    // Write current time into it
    result.StartTime = std::chrono::high_resolution_clock::now();
}

void Profile::Profiler::EndProfile(std::string_view name) {
    // Result doesnt exist
    if (m_Results.find(name) == m_Results.end())
        return;

    m_Results[name].EndTime = std::chrono::high_resolution_clock::now();
}