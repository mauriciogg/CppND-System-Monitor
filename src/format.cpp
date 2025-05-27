#include "format.h"

#include <string>
#include <sstream>
#include <iomanip>

using std::string;

constexpr int kSecondsPerMinute = 60;
constexpr int kMinutesPerHour = 60;
constexpr int kSecondsPerHour = kMinutesPerHour * kSecondsPerMinute;

// TODO(mgg): DONE 
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds[[maybe_unused]]) {
	int hours_in_ts = seconds / kSecondsPerHour;
	int tmp_seconds = (seconds % kSecondsPerHour);
	int minutes_in_ts = tmp_seconds / 60;
	int seconds_in_ts =tmp_seconds % kSecondsPerMinute;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours_in_ts << ":"
       << std::setw(2) << std::setfill('0') << minutes_in_ts << ":"
       << std::setw(2) << std::setfill('0') << seconds_in_ts;
    return ss.str();
}
