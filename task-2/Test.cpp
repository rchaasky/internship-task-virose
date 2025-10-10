#include <iostream>
#include <fstream>
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

namespace rj = rapidjson;

int main() {
    int M;
    std::cin >> M;

    std::stringstream ss_movie;
    ss_movie << "../../XL/motion_movie/" << M << ".json";
    std::string movie_path = ss_movie.str();

    std::ifstream movie_stream(movie_path);
    if (!movie_stream.is_open()) {
        std::cout << "error opening movie file" << std::endl;
        return 1;
    }

    rj::IStreamWrapper movie_is(movie_stream);
    rj::Document movie_doc;
    movie_doc.ParseStream(movie_is);
    if (movie_doc.HasParseError()) {
        std::cout << "JSON parse error in movie" << std::endl;
        return 1;
    }

    std::cout << "Motion Movie ID: " << movie_doc["id"].GetInt() << std::endl;

    const rj::Value& units = movie_doc["motion_unit"];
    for (const auto& unit_val : units.GetArray()) {
        int unit_id = unit_val["id"].GetInt();

        std::stringstream ss_unit;
        ss_unit << "../../XL/motion_unit/" << unit_id << ".json";
        std::string unit_path = ss_unit.str();

        std::ifstream unit_stream(unit_path);
        if (!unit_stream.is_open()) {
            std::cout << "error opening unit file" << std::endl;
            continue;
        }

        rj::IStreamWrapper unit_is(unit_stream);
        rj::Document unit_doc;
        unit_doc.ParseStream(unit_is);
        if (unit_doc.HasParseError()) {
            std::cout << "JSON parse error in unit" << std::endl;
            continue;
        }

        std::cout << "Motion Unit ID: " << unit_doc["id"].GetInt() << std::endl;

        if (unit_doc.HasMember("motion_frame") && unit_doc["motion_frame"].IsArray()) {
            const rj::Value& frames = unit_doc["motion_frame"];
            for (rj::SizeType j = 0; j < frames.Size(); j++) {
                const auto& f = frames[j];

                int shoulder = f.HasMember("shoulder") && f["shoulder"].IsInt() ? f["shoulder"].GetInt() : 0;
                int elbow    = f.HasMember("elbow")    && f["elbow"].IsInt()    ? f["elbow"].GetInt()    : 0;
                int wrist    = f.HasMember("wrist")    && f["wrist"].IsInt()    ? f["wrist"].GetInt()    : 0;
                int duration = f.HasMember("duration") && f["duration"].IsInt() ? f["duration"].GetInt() : 0;

                std::cout << "Frame " << j+1 << ": "
                          << "Shoulder=" << shoulder << ", "
                          << "Elbow=" << elbow << ", "
                          << "Wrist=" << wrist << ", "
                          << "Duration=" << duration << "ms"
                          << std::endl;
            }
        }
        std::cout << std::endl;
    }

    return 0;
}
