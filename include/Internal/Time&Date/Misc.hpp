#pragma once
// MFWork/Internal/Time&Date/Timer.hpp
// advanced timer + duration parser/formatter (header-only)

#include <chrono>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <climits>

namespace Time {

using ns_t = std::chrono::nanoseconds;
using steady_clock = std::chrono::steady_clock;

struct FormatOptions {
    int maxUnits = 3;         // how many units to show (precision). <=0 => show all
    bool showZeros = false;   // include zero-value units (rare)
    // unit order preference could be added later
};

static inline std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
    { 
        return static_cast<char>(std::tolower(c)); 
    });
    return s;
}

// multipliers in nanoseconds (using long double for intermediate arithmetic)
static inline const std::vector<std::pair<std::string, long double>> unit_table_desc = {
    {"y",  (long double)365.0L * 24.0L * 60.0L * 60.0L * 1000000000.0L}, // years
    {"mo", (long double)30.0L  * 24.0L * 60.0L * 60.0L * 1000000000.0L}, // months (approx 30d)
    {"w",  (long double)7.0L   * 24.0L * 60.0L * 60.0L * 1000000000.0L},  // weeks
    {"d",  (long double)24.0L  * 60.0L * 60.0L * 1000000000.0L},         // days
    {"h",  (long double)60.0L  * 60.0L * 1000000000.0L},                 // hours
    {"m",  (long double)60.0L  * 1000000000.0L},                         // minutes (m)
    {"s",  (long double)1000000000.0L},                                  // seconds
    {"ms", (long double)1000000.0L}                                      // milliseconds
};

// canonical aliases -> normalized key in unit_table_desc
static inline const std::unordered_map<std::string, std::string> unit_aliases = {
    // years
    {"y","y"}, {"yr","y"}, {"year","y"}, {"years","y"},
    // months
    {"mo","mo"}, {"mon","mo"}, {"month","mo"}, {"months","mo"},
    // weeks
    {"w","w"}, {"week","w"}, {"weeks","w"},
    // days
    {"d","d"}, {"day","d"}, {"days","d"},
    // hours
    {"h","h"}, {"hr","h"}, {"hour","h"}, {"hours","h"},
    // minutes (m)
    {"m","m"}, {"min","m"}, {"mins","m"}, {"minute","m"}, {"minutes","m"},
    // seconds
    {"s","s"}, {"sec","s"}, {"secs","s"}, {"second","s"}, {"seconds","s"},
    // milliseconds
    {"ms","ms"}, {"millisecond","ms"}, {"milliseconds","ms"}
};

// rank map descending (higher => bigger unit)
static inline std::unordered_map<std::string,int> unit_rank() {
    std::unordered_map<std::string,int> r;
    for (int i = 0; i < (int)unit_table_desc.size(); ++i) {
        r[ unit_table_desc[i].first ] = (int)unit_table_desc.size() - i; // bigger units have larger rank
    }
    return r;
}

// parse duration string -> optional nanoseconds
// returns std::nullopt on invalid format or rule violation
static inline std::optional<ns_t> parseDuration(const std::string& raw) {
    if (raw.empty()) return std::nullopt;
    // normalize separators: replace commas with spaces
    std::string s;
    s.reserve(raw.size());
    for (char c : raw) {
        if (c == ',') s.push_back(' ');
        else s.push_back(c);
    }

    // tokenization: extract sequences of [number][unit]
    // allow formats like: "1h 2m", "1.5h", "237 ms", "1h2m3s237ms"
    size_t i = 0;
    const size_t n = s.size();

    // prepare unit lookup
    std::unordered_map<std::string,long double> unitToNs;
    for (auto &p : unit_table_desc) unitToNs[p.first] = p.second;
    auto aliases = unit_aliases; // copy
    auto ranks = unit_rank();

    // collected tokens in order
    struct Token { std::string unit; long double value; bool explicitProvided; };
    std::vector<Token> tokens;

    // parsing loop: read number, then unit letters
    while (i < n) {
        // skip spaces
        while (i < n && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        if (i >= n) break;

        // read number (signed? we don't accept negative)
        size_t startNum = i;
        bool hasDigits = false;
        bool sawDot = false;
        // optional sign not allowed (no negative)
        while (i < n && (std::isdigit(static_cast<unsigned char>(s[i])) || s[i] == '.')) {
            if (s[i] == '.') {
                if (sawDot) break; // second dot -> stop number
                sawDot = true;
            } else {
                hasDigits = true;
            }
            ++i;
        }
        if (!hasDigits) return std::nullopt;
        std::string numStr = s.substr(startNum, i - startNum);
        // parse double
        long double val = 0.0L;
        try {
            val = std::stold(numStr);
        } catch (...) { return std::nullopt; }

        // skip spaces between number and unit (allow "237 ms")
        while (i < n && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        if (i >= n) {
            // number with no unit -> interpret as milliseconds? no, invalid
            return std::nullopt;
        }

        // read unit letters (alpha)
        size_t startUnit = i;
        while (i < n && std::isalpha(static_cast<unsigned char>(s[i]))) ++i;
        if (startUnit == i) return std::nullopt; // no unit letters
        std::string u = s.substr(startUnit, i - startUnit);
        u = toLower(u);

        // normalize alias
        auto itAlias = aliases.find(u);
        if (itAlias == aliases.end()) {
            // acceptance: treat single 'm' as minute; 'mo' is month; ensure user uses 'mo' for months
            return std::nullopt;
        }
        std::string canon = itAlias->second;

        // push token
        tokens.push_back({canon, val, true});

        // continue
    } // end parse loop

    if (tokens.empty()) return std::nullopt;

    // check ordering: tokens must be in descending order of unit rank
    auto ranksMap = unit_rank();
    int prevRank = INT_MAX;
    for (const auto &t : tokens) {
        int r = ranksMap.count(t.unit) ? ranksMap[t.unit] : -1;
        if (r == -1) return std::nullopt;
        if (r > prevRank) {
            // this token is a larger unit than previous (out of order)
            return std::nullopt;
        }
        prevRank = r;
    }

    // rule: reject if any explicitly provided higher-order unit has value == 0 while
    // a smaller (later) unit was provided with value > 0.
    // find, for each token with value==0, whether any later token has >0
    for (size_t idx = 0; idx < tokens.size(); ++idx) {
        if (tokens[idx].value == 0.0L) {
            for (size_t j = idx + 1; j < tokens.size(); ++j) {
                if (tokens[j].value > 0.0L) {
                    // invalid per strict rule
                    return std::nullopt;
                }
            }
        }
    }

    // accumulate total ns
    long double total_ns_ld = 0.0L;
    for (const auto &t : tokens) {
        auto it = unitToNs.find(t.unit);
        if (it == unitToNs.end()) return std::nullopt;
        total_ns_ld += t.value * it->second;
    }

    if (!std::isfinite(total_ns_ld)) return std::nullopt;
    if (total_ns_ld < 0.0L) return std::nullopt;

    // clamp into 64-bit nanoseconds safely
    const long double max_ns_ld = (long double)std::numeric_limits<long long>::max();
    if (total_ns_ld > max_ns_ld) return std::nullopt;

    long long total_ns = (long long)std::llround(total_ns_ld);
    return ns_t(total_ns);
}

// format duration (nanoseconds) into readable string
// respects descending units; drops zero units unless showZeros==true
static inline std::string formatDuration(ns_t ns, const FormatOptions& opts = {}) {
    long long total_ns = ns.count();
    if (total_ns < 0) total_ns = 0;

    struct UnitOut { std::string name; long long qty; };

    std::vector<UnitOut> parts;
    long long rest_ns = total_ns;

    for (const auto &p : unit_table_desc) {
        const std::string &unit = p.first;
        long double unit_ns_ld = p.second;
        long long unit_ns = (long long)std::llround(unit_ns_ld);
        if (unit_ns <= 0) continue;
        long long qty = (long long)(rest_ns / unit_ns);
        if (qty > 0) {
            parts.push_back({unit, qty});
            rest_ns -= qty * unit_ns;
        } else {
            if (opts.showZeros) parts.push_back({unit, 0});
        }
    }

    // If nothing and showZeros false, return "0ms"
    if (parts.empty()) {
        if (opts.showZeros) {
            std::ostringstream os; os << "0ms"; return os.str();
        } else {
            return std::string("0ms");
        }
    }

    // build output respecting maxUnits
    std::ostringstream os;
    int shown = 0;
    int maxu = opts.maxUnits <= 0 ? (int)parts.size() : std::min((int)parts.size(), opts.maxUnits);

    for (size_t i = 0; i < parts.size() && shown < maxu; ++i) {
        if (!opts.showZeros && parts[i].qty == 0) continue;
        if (shown) os << " ";
        os << parts[i].qty << parts[i].name;
        ++shown;
    }

    // if we limited to 1 unit and there is remainder, show decimal fraction for that unit
    if (opts.maxUnits == 1 && !parts.empty()) {
        // compute largest unit (first non-zero)
        const auto &p = parts.front();
        long double unit_ns_ld = 0.0L;
        for (auto &ut : unit_table_desc) if (ut.first == p.name) unit_ns_ld = ut.second;
        if (unit_ns_ld > 0.0L) {
            long double fullVal = (long double)total_ns / unit_ns_ld;
            // show with 3 decimal places if fractional
            std::ostringstream os2;
            os2 << std::fixed << std::setprecision(3);
            os2 << fullVal << p.name;
            return os2.str();
        }
    }

    return os.str();
}

// Timer class: stopwatch style
    class Timer {
    public:
        enum class Precision {
            Milliseconds,
            Seconds,
            Minutes,
            Hours,
            Days,
            Weeks,
            Months,
            Years
        };

        // ctor: direct precision
        explicit Timer(Precision p = Precision::Milliseconds)
            : precision_(p), running_(false), threshold_(ns_t::zero()) {}

        // ctor: token like "ms", "s", "m", "h", "d", "w", "mo", "y"
        explicit Timer(const std::string& unitToken)
            : precision_(parsePrecisionToken(unitToken)), running_(false), threshold_(ns_t::zero()) {}

        // start / stop / reset
        void start() {
            start_ = clock::now();
            last_lap_ = start_;
            running_ = true;
        }

        void stop() {
            if (running_) {
                end_ = clock::now();
                running_ = false;
            }
        }

        void reset() {
            running_ = false;
            start_ = time_point();
            end_ = time_point();
            last_lap_ = time_point();
        }

        void restart() { reset(); start(); }

        // lap: duration since last lap or start
        std::chrono::nanoseconds lap() {
            auto now = running_ ? clock::now() : end_;
            if (last_lap_.time_since_epoch().count() == 0) last_lap_ = running_ ? start_ : end_;
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_lap_);
            last_lap_ = now;
            return ns;
        }

        // threshold helpers
        void setThreshold(std::chrono::nanoseconds ns) { threshold_ = ns; }
        void setThreshold(const std::string& spec) {
            auto o = ::Time::parseDuration(spec);
            threshold_ = o ? *o : ns_t::zero();
        }
        bool reachedThreshold() const { return elapsedNs() >= threshold_; }

        // precision control
        void setPrecision(Precision p) { precision_ = p; }
        Precision precision() const { return precision_; }

        // raw ns
        std::chrono::nanoseconds elapsedNs() const {
            if (start_.time_since_epoch().count() == 0) return std::chrono::nanoseconds::zero();
            auto endp = running_ ? clock::now() : end_;
            return std::chrono::duration_cast<std::chrono::nanoseconds>(endp - start_);
        }

        // numeric elapsed using current precision (may be fractional)
        double elapsed() const { return toUnits(elapsedNs(), precision_); }
        double elapsed(Precision p) const { return toUnits(elapsedNs(), p); }

        // string: if timer was constructed with "ms" precision, force fractional ms output (e.g. "0.341 ms")
        // otherwise fallback to the global formatDuration (multi-unit) using maxParts
        std::string elapsedString(int maxParts = 3) const {
            if (precision_ == Precision::Milliseconds) {
                long double ms = (long double)elapsedNs().count() / NS_IN_MS;
                std::ostringstream os;
                // show up to 3 decimal places, but trim trailing zeros
                os << std::fixed << std::setprecision(3) << (double)ms;
                std::string s = os.str();
                // trim trailing zeros and dot
                while (!s.empty() && s.back() == '0') s.pop_back();
                if (!s.empty() && s.back() == '.') s.pop_back();
                return s + " ms";
            } else {
                // use global formatter you already have (formatDuration)
                FormatOptions fo;
                fo.maxUnits = maxParts;
                fo.showZeros = false;
                return formatDuration(elapsedNs(), fo);
            }
        }

    private:
        // clock typedefs
        using clock = std::chrono::steady_clock;
        using time_point = std::chrono::time_point<clock>;

        // constants
        static constexpr long double NS_IN_S   = 1'000'000'000.0L;
        static constexpr long double NS_IN_MS  = 1'000'000.0L;
        static constexpr long double NS_IN_US  = 1'000.0L;
        static constexpr long double SEC_NS    = NS_IN_S;
        static constexpr long double MIN_NS    = 60.0L * SEC_NS;
        static constexpr long double HOUR_NS   = 60.0L * MIN_NS;
        static constexpr long double DAY_NS    = 24.0L * HOUR_NS;
        static constexpr long double WEEK_NS   = 7.0L  * DAY_NS;
        static constexpr long double MONTH_NS  = 30.44L * DAY_NS;
        static constexpr long double YEAR_NS   = 365.25L * DAY_NS;
        static constexpr long double MS_NS     = NS_IN_MS;
        static constexpr long double US_NS     = NS_IN_US;

        // parse precision token to enum
        static Precision parsePrecisionToken(const std::string& tok) {
            std::string t; t.reserve(tok.size());
            for (char c : tok) t.push_back((char)std::tolower((unsigned char)c));
            if (t == "ms" || t == "millisecond" || t == "milliseconds") return Precision::Milliseconds;
            if (t == "s"  || t == "sec" || t == "secs" || t == "second" || t == "seconds") return Precision::Seconds;
            if (t == "m"  || t == "min" || t == "mins"  || t == "minute" || t == "minutes") return Precision::Minutes;
            if (t == "h"  || t == "hr"  || t == "hour"  || t == "hours") return Precision::Hours;
            if (t == "d"  || t == "day" || t == "days") return Precision::Days;
            if (t == "w"  || t == "wk"  || t == "week" || t == "weeks") return Precision::Weeks;
            if (t == "mo" || t == "month" || t == "months") return Precision::Months;
            if (t == "y"  || t == "yr"  || t == "year"  || t == "years") return Precision::Years;
            return Precision::Milliseconds;
        }

        // convert ns -> chosen unit (double, fractional)
        static double toUnits(std::chrono::nanoseconds ns, Precision p) {
            const long double n = (long double)ns.count();
            switch (p) {
                case Precision::Milliseconds: return (double)(n / NS_IN_MS);
                case Precision::Seconds:      return (double)(n / NS_IN_S);
                case Precision::Minutes:      return (double)(n / MIN_NS);
                case Precision::Hours:        return (double)(n / HOUR_NS);
                case Precision::Days:         return (double)(n / DAY_NS);
                case Precision::Weeks:        return (double)(n / WEEK_NS);
                case Precision::Months:       return (double)(n / MONTH_NS);
                case Precision::Years:        return (double)(n / YEAR_NS);
            }
            return 0.0;
        }

    private:
        Precision precision_;
        bool running_;
        time_point start_{};
        time_point end_{};
        time_point last_lap_{};
        std::chrono::nanoseconds threshold_;
    };
}   // namespace Time