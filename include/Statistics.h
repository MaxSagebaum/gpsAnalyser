//
// Created by msagebaum on 4/2/18.
//

#ifndef GPSANALYSER_STATISTICS_H
#define GPSANALYSER_STATISTICS_H

#include <cmath>
#include <ostream>
#include "util.h"

struct Statistics {
    long totalTime;
    double totalDistance;
    double totalRaiseHeight;
    double totalFallHeight;
    double totalRaiseDistance;
    double totalFallDistance;

    double averageSpeed;
    double maxSpeed;

    Statistics() {
      init();
    }

    void init() {
      totalTime = 0;
      totalDistance = 0.0;
      totalRaiseHeight = 0.0;
      totalFallHeight = 0.0;
      totalRaiseDistance = 0.0;
      totalFallDistance = 0.0;

      averageSpeed = 0.0;
      maxSpeed = 0.0;
    }

    void update(long time_ms, double distancePlain, double height) {
      totalTime += time_ms;

      double distance = std::sqrt(distancePlain * distancePlain + height * height);

      totalDistance += distance;
      if(height >= 0.0) {
        totalRaiseHeight += height;
        totalRaiseDistance += distance;
      } else {
        totalFallHeight += -height;
        totalFallDistance += distance;
      }


      double time_s = time_ms / 1000.0;
      double speed = distance / time_s;
      maxSpeed = std::max(maxSpeed, speed);

      // computed in finalize: averageSpeed
    }

    void finalize(bool combineHeight) {
      if(0 != totalTime) {
        double totalTime_s = (double) totalTime / 1000.0;
        averageSpeed = totalDistance / totalTime_s;
      } else {
        averageSpeed = 0.0;
      }

      if(combineHeight) {
        double height = totalRaiseHeight - totalFallHeight;
        totalRaiseHeight = 0.0;
        totalFallHeight = 0.0;
        if(height >= 0) {
          totalRaiseHeight = height;
        } else {
          totalFallHeight = -height;
        }
      }
    }

    void add(const Statistics& other) {
      totalTime += other.totalTime;
      totalDistance += other.totalDistance;
      totalRaiseHeight += other.totalRaiseHeight;
      totalFallHeight += other.totalFallHeight;
      totalRaiseDistance += other.totalRaiseDistance;
      totalFallDistance += other.totalFallDistance;

      // no add required averageSpeed += other.averageSpeed;
      maxSpeed = std::max(maxSpeed, other.maxSpeed);
    }

    void printHeader(std::ostream& out) {
      out << "name; total time(hh::mm::ss); total distance(km); total raise height(km); "
             "total fall height(km); total raise distance(km); total fall distance(km); average speed(km/h); max speed(km/h);\n";
    }

    void printRow(std::ostream& out, const std::string& name) {
      out << std::setw(40) << name << "; "
          << formatDateTime(totalTime, false, true, true) << "; "
          << formatDouble(totalDistance / 1000, 3, 7) << "; "
          << formatDouble(totalRaiseHeight / 1000, 3, 7) << "; "
          << formatDouble(totalFallHeight / 1000, 3, 7) << "; "
          << formatDouble(totalRaiseDistance / 1000, 3, 7) << "; "
          << formatDouble(totalFallDistance/ 1000, 3, 7) << "; "
          << formatDouble(averageSpeed * 3.6, 3, 7) << "; "
          << formatDouble(maxSpeed * 3.6, 3, 7) << ";\n";
    }
};

#endif //GPSANALYSER_STATISTICS_H
