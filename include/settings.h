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
    double invalidSpeed_km_h = 100.0;

    bool interpolateHeight;
    double climbMaxSpeed_m_s = 15.0;
    double climbTrendAdapt = 0.25;

    bool extractPause;
    double pauseMinTime_s = 30.0;
    double pauseMaxRange_m = 10.0;

    bool splitUpDown;
    double raiseDistance = 100.0;



};

#endif //GPSANALYSER_SETTINGS_H
