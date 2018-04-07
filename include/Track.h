//
// Created by msagebaum on 3/31/18.
//

#ifndef GPSANALYSER_TRACK_H
#define GPSANALYSER_TRACK_H


#include <vector>
#include <rapidxml.hpp>

#include "Statistics.h"

struct DataPoint {
    double lat;
    double lon;
    double height;
    long time;
};

class Track {
private:

    std::vector<DataPoint> points;

public:

    static constexpr double R = 6371000; //Meter
    static constexpr double TAN_5 = 0.0875; // tangent of 5 degree

    void read(rapidxml::xml_node<>* trackSeg);

    void removeInvalid(const double maxSpeed_km_h);

    bool isValidHeight(const DataPoint& point) const;

    double computePlainDist(const DataPoint& p1, const DataPoint& p2) const;

    void extractBreaks(double minSeconds, double maxDistance, std::vector<Track>& breaks);

    double computeDist(const DataPoint& p1, const DataPoint& p2) const;

    void splitUpDown(double changeTolerance, std::vector<Track>& up, std::vector<Track>& down);

    void extractTrack(std::vector<Track>& tracks, size_t start, size_t end) const;

    void write(rapidxml::xml_node<>* trackSeg, rapidxml::xml_document<>& doc) const;

    void linearizeWrongHeight(const double maxClimbSpeed_m_s, const double trendAdapt);

    Statistics computeStatistics(bool combineHeight) const;
};


#endif //GPSANALYSER_TRACK_H
