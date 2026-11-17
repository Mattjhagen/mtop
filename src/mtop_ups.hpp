// [AI generated] Native read-only NUT panel for the mtop fork.
// Licensed under Apache-2.0, like the surrounding btop source.
#include <chrono>
#include <cstdio>
#include <map>

namespace MtopUps {
    inline int height = 0;
    inline std::map<string, string> data;
    inline std::chrono::steady_clock::time_point sampled{};

    inline void poll() {
        const auto now = std::chrono::steady_clock::now();
        if (now - sampled < std::chrono::seconds(5)) return;
        sampled = now;
        data.clear();
        // One bounded, read-only query. No device commands or shutdown actions.
        FILE* pipe = popen("/usr/bin/timeout 2 /usr/bin/upsc minuteman@localhost 2>/dev/null", "r");
        if (!pipe) return;
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            string line(buffer);
            const auto pos = line.find(": ");
            if (pos == string::npos) continue;
            while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();
            data[line.substr(0, pos)] = line.substr(pos + 2);
        }
        if (pclose(pipe) != 0) data.clear();
    }
    inline double value(const string& key, double low, double high) {
        try {
            size_t used = 0;
            const auto& raw = data.at(key);
            const double n = std::stod(raw, &used);
            return used == raw.size() && std::isfinite(n) && n >= low && n <= high ? n : -1;
        } catch (...) { return -1; }
    }
    inline string draw(int x, int y, int width) {
        if (!height) return "";
        poll();
        string out = Draw::createBox(x, y, width, height, Theme::c("proc_box"), true, "ups", "", 0);
        int row = y + 1;
        auto line = [&](const string& label, const string& text, const string& color = "main_fg") {
            if (row >= y + height - 1) return;
            string plain = label + text;
            // All telemetry rendered here is ASCII, so byte truncation is safe.
            if ((int)plain.size() > width - 4) plain.resize(width - 4);
            out += Mv::to(row++, x + 2) + Theme::c(color) + plain;
        };
        if (data.empty()) { line("", "UPS unavailable", "inactive_fg"); return out + Fx::reset; }
        const string status = data.contains("ups.status") ? data.at("ups.status") : "";
        const string tokens = " " + status + " ";
        if (tokens.find(" OB ") != string::npos) line("", "on battery", "hi_fg");
        else if (tokens.find(" OL ") != string::npos) line("", "utility power", "title");
        const double seconds = value("battery.runtime", 1, 604800);
        if (seconds >= 0) {
            const int s = (int)seconds;
            line("time left  ", to_string(s / 3600) + "h " + to_string(s / 60 % 60) + "m " + to_string(s % 60) + "s est.", "title");
        }
        auto meter = [&](const string& label, const string& key, const string& gradient) {
            const double n = value(key, 0, 100);
            if (n < 0 || row >= y + height - 1) return;
            const int cells = max(3, min(22, width - 23));
            const int filled = (int)round(n * cells / 100);
            out += Mv::to(row++, x + 2) + Theme::c("main_fg") + label;
            for (int i = 0; i < cells; ++i)
                out += (i < filled ? Theme::g(gradient).at(clamp(i * 100 / cells, 0, 100)) : Theme::c("meter_bg")) + Symbols::meter;
            out += Theme::c("main_fg") + " " + to_string((int)round(n)) + '%';
        };
        meter("battery    ", "battery.charge", "cpu");
        meter("load       ", "ups.load", "used");
        for (const string side : {"input", "output"}) {
            string readings;
            for (const auto& item : {std::tuple{"voltage", " V", 50., 300.}, std::tuple{"frequency", " Hz", 40., 70.}, std::tuple{"current", " A", 0., 20.}}) {
                const auto& [key, unit, low, high] = item;
                if (value(side + "." + key, low, high) >= 0) readings += data.at(side + "." + key) + unit + "  ";
            }
            if (!readings.empty()) line(side + "  ", readings);
        }
        if (value("ups.realpower", 0, 2000) >= 0) line("power      ", data.at("ups.realpower") + " W reported");
        if (value("battery.voltage", 10, 60) >= 0) line("battery    ", data.at("battery.voltage") + " V");
        if (data.contains("ups.beeper.status") && (data.at("ups.beeper.status") == "enabled" || data.at("ups.beeper.status") == "disabled"))
            line("beeper     ", data.at("ups.beeper.status"), "inactive_fg");
        return out + Fx::reset;
    }
}
