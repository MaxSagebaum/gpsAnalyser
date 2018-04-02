//
// Created by msagebaum on 4/2/18.
//

#ifndef GPSANALYSER_SETTINGS_H
#define GPSANALYSER_SETTINGS_H

#include <string>
#include <vector>

struct Settings {
    std::vector<std::string> inputFiles;
    std::vector<std::string> outputFiles;

    bool isDirectory;

    bool removeInvalid;
    double invalidSpeed_km_h;

    bool interpolateHeight;
    double climbMaxSpeed_m_s;
    double climbTrendAdapt;

    bool extractPause;
    double pauseMinTime_s;
    double pauseMaxRange_m;

    bool splitUpDown;
    double raiseDistance_m;

    static int verbose;
};

#endif //GPSANALYSER_SETTINGS_H
