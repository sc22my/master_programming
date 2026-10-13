// Profiler

#ifndef _PROFILER_H
#define _PROFILER_H

#include <unordered_map>
#include <string_view>
#include <chrono>

// Begin and end profiling section macros
// Should be called from the main thread only
#define BEGIN_PROFILE(name) Profile::Profiler::GetInstance().BeginProfile(name)
#define END_PROFILE(name) Profile::Profiler::GetInstance().EndProfile(name)

namespace Profile {
    using TimePoint = std::chrono::high_resolution_clock::time_point;

    struct ProfileResult {
        TimePoint StartTime;
        TimePoint EndTime;

        // Gets duration of profile in seconds
        double GetDuration() const {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(EndTime - StartTime).count() * 0.001 * 0.001 * 0.001;
        }
    };

    // Profiler singleton. NOT thread safe :p
    class Profiler {
    public:
        // This is thread safe
        static Profiler& GetInstance() {
            static Profiler s_Instance;
            return s_Instance;
        }

        // Resets all results for the start of a frame
        void Reset();

        // Get profile results
        const std::unordered_map<std::string_view, ProfileResult>& GetResults() const { return m_Results; }

        void BeginProfile(std::string_view name);
        void EndProfile(std::string_view name);
    private:
        std::unordered_map<std::string_view, ProfileResult> m_Results;
    };
}

#endif