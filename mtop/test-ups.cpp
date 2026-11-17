// [AI generated] Isolated validation of telemetry filtering and panel rendering.
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <string>
#include <tuple>
using std::string;
using std::to_string;
using std::max;
using std::min;
using std::clamp;
using std::round;
namespace Theme {
string c(const string&) { return ""; }
const std::array<string,101>& g(const string&) { static std::array<string,101> gradient{}; return gradient; }
}
namespace Draw { string createBox(int,int,int,int,string,bool,string,string,int) { return ""; } }
namespace Mv { string to(int,int) { return ""; } }
namespace Fx { const string reset = ""; }
namespace Symbols { const string meter = "#"; }
#include "../src/mtop_ups.hpp"
int main() {
    MtopUps::sampled = std::chrono::steady_clock::now();
    MtopUps::height = 15;
    MtopUps::data = {{"ups.status","OL"},{"battery.runtime","710"},{"battery.charge","100"},
        {"ups.load","49"},{"input.voltage","0"},{"output.voltage","0"},
        {"input.frequency","900"},{"output.frequency","900"},{"output.current","230"},
        {"ups.realpower","31"},{"battery.voltage","0"}};
    auto out=MtopUps::draw(1,1,56);
    assert(out.find("utility power") != string::npos);
    assert(out.find("11m 50s") != string::npos);
    assert(out.find("31 W reported") != string::npos);
    assert(out.find("input  ") == string::npos);
    assert(out.find("output  ") == string::npos);
    MtopUps::data["input.voltage"]="120";
    MtopUps::data["input.frequency"]="60";
    out=MtopUps::draw(1,1,56);
    assert(out.find("120 V") != string::npos && out.find("60 Hz") != string::npos);
    for (const string bad : {"NaN","inf","12junk","-1","101"}) {
        MtopUps::data["battery.charge"]=bad;
        assert(MtopUps::value("battery.charge",0,100)<0);
    }
    MtopUps::data.clear();
    assert(MtopUps::draw(1,1,56).find("UPS unavailable") != string::npos);
    MtopUps::height=0;
    assert(MtopUps::draw(1,1,56).empty());
}
