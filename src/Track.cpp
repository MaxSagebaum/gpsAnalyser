//
// Created by msagebaum on 3/31/18.
//

#include <util.h>
#include <iostream>
#include <cmath>
#include <settings.h>
#include <Statistics.h>
#include "../include/Track.h"

void Track::read(rapidxml::xml_node<>* trackSeg) {
  DataPoint point;
  for(rapidxml::xml_node<>* curPnt = trackSeg->first_node("trkpt"); curPnt; curPnt = curPnt->next_sibling("trkpt")) {
    point.lat = parseDouble(curPnt->first_attribute("lat"), -100.0);
    point.lon = parseDouble(curPnt->first_attribute("lon"), -100.0);
    point.height = parseDouble(curPnt->first_node("ele"), 0.0);
    point.time = parseDateTime(curPnt->first_node("time"), 0);

    points.push_back(point);
  }
}

void Track::write(rapidxml::xml_node<>* trackSeg, rapidxml::xml_document<>& doc) const {
  for(auto&& point : points) {

    rapidxml::xml_node<>* node = doc.allocate_node(rapidxml::node_element, "trkpt");
    rapidxml::xml_node<>* eleNode = doc.allocate_node(rapidxml::node_element, "ele", toString(doc, point.height));
    rapidxml::xml_node<>* timeNode = doc.allocate_node(rapidxml::node_element, "time", toString(doc, point.time));
    rapidxml::xml_attribute<>* latAttr = doc.allocate_attribute("lat", toString(doc, point.lat));
    rapidxml::xml_attribute<>* lonAttr = doc.allocate_attribute("lon", toString(doc, point.lon));

    node->append_node(eleNode);
    node->append_node(timeNode);
    node->append_attribute(lonAttr);
    node->append_attribute(latAttr);

    trackSeg->append_node(node);
  }
}

void Track::removeInvalid(const double maxSpeed_km_h) {

  double maxSpeed_m_ms = maxSpeed_km_h / 3600.0;
  // make a sanity check if the first node is valid
  while(points.size() > 0) {
    DataPoint& point = points[0];

    if(!isValidHeight(point)) {
      points.erase(points.begin());
      if(Settings::verbose >= 1) {std::cerr << "Removing initial point due to height: " << point.height << std::endl;}
    } else {
      break;
    }
  }

  for(int pos = 1; pos < points.size(); /* increased inside the loop */) {

    DataPoint& point = points[pos];
    bool remove = !isValidHeight(point);
    if(remove) {
      if(Settings::verbose >= 1) {std::cerr << "Removing point due to height: " << point.height << std::endl;}
    }
    if(!remove) {
      DataPoint& prev = points[pos - 1];

      double dist = computePlainDist(point, prev);
      long milliseconds = point.time - prev.time;

      if(dist > maxSpeed_m_ms * milliseconds) {
        remove = true;
        if(Settings::verbose >= 1) {std::cerr << "Removing point due to speed limit: " << dist << " " << maxSpeed_m_ms * milliseconds << " " << 3600.0 * dist / milliseconds << std::endl;}
      }
    }

    if(remove) {
      points.erase(points.begin() + pos);
    } else {
      pos += 1;
    }
  }
}

void Track::linearizeWrongHeight(const double maxClimbSpeed_m_s, const double trendAdapt) {
  double maxSpeed_m_ms = maxClimbSpeed_m_s / 1000.0;

  size_t invalidStart = 0;
  bool isInvalid = false;

  double trend = (points[1].height - points[0].height) / (points[1].time - points[0].time);

  for(size_t pos = 1; pos < points.size(); ++pos) {

    DataPoint& startPoint = points[invalidStart];
    DataPoint& prevPoint = points[pos - 1];
    DataPoint& point = points[pos];

    double trendStart = (point.height - startPoint.height) / (point.time - startPoint.time);
    double trendCur = (point.height - prevPoint.height) / (point.time - prevPoint.time);

    if(Settings::verbose >= 2) {std::cerr << "pos: " << pos << " trend:" << trend << " trendCur:" << trendCur << " invalid: " << isInvalid << "trendStartInv: " << trendStart << " maxSpeed_m_ms: " << maxSpeed_m_ms << std::endl;}

    if(isInvalid) {
      // invalid region try to find a position where the trend continues

      if(trend - 0.003 <= trendStart && trendStart <= trend + 0.003) {
        // found a continuing region

        if(Settings::verbose >= 1) {std::cerr << "Interpolating due to climb speed limit from " << invalidStart << " to " << pos << "." << std::endl;}
        // make a linear interpolation
        double heightDiff = point.height - startPoint.height;
        double timeDiff = point.time - startPoint.time;
        for(size_t interpolatePos = invalidStart + 1; interpolatePos < pos; ++interpolatePos) {
          DataPoint& curPoint = points[interpolatePos];

          curPoint.height = startPoint.height + heightDiff * (curPoint.time - startPoint.time) / timeDiff;
        }
        isInvalid = false;
      }
    } else {
      if(std::abs(trendCur) > maxSpeed_m_ms) {
        isInvalid = true;
        invalidStart = pos - 1;
      } else {
        trend = trend * (1.0 - trendAdapt) + trendAdapt * trendCur;
      }
    }
  }
}

bool Track::isValidHeight(const DataPoint& point) const {
  return !(point.height <= 0.0 || point.height >= 3000.0);
}

double Track::computeDist(const DataPoint& p1, const DataPoint& p2) const {
  double distPlain = computePlainDist(p1, p2);

  double h = p2.height - p1.height;
  return std::sqrt(h * h + distPlain * distPlain);
}

double Track::computePlainDist(const DataPoint& p1, const DataPoint& p2) const {
  // very simple distance
  // see: https://www.movable-type.co.uk/scripts/latlong.html
  double x = deg2Rad(p2.lon - p1.lon) * std::cos(deg2Rad(p2.lat + p1.lat) * 0.5);
  double y = deg2Rad(p2.lat - p1.lat);
  return std::sqrt(x * x + y * y) * R;
}

void Track::extractBreaks(double minSeconds, double maxDistance, std::vector<Track>& breaks) {
  size_t breakStart = 0;
  long breakTime = 0;

  for(size_t pos = 1; pos < points.size(); ++pos) {
    DataPoint& startPoint = points[breakStart];
    DataPoint& point = points[pos];

    double dist = computeDist(point, startPoint);
    long milliseconds = point.time - startPoint.time;

    if(dist > maxDistance) {
      // leaving the radius of the break point
      if(breakTime > minSeconds * 1000.0) {
        // break was long enough, extract it

        if(Settings::verbose >= 1) {std::cerr << "Extracting break from " << breakStart << " to " << pos << "." << std::endl;}

        extractTrack(breaks, breakStart, pos);
        points.erase(points.begin() + breakStart + 1, points.begin() + pos); // leave the start of the break in the track

        pos = breakStart; // reset the iteration back to the point where the break started
      }

      // reset the values for the break start
      breakStart = pos;
      breakTime = 0;
    } else {
      // break is still valid
      breakTime = milliseconds; // store the duration of the break;
    }
  }
}

void Track::splitUpDown(double changeTolerance, std::vector<Track>& up, std::vector<Track>& down) {

  if(points.size() == 0) {
    return;
  }

  size_t sectionStart = 0;

  size_t sectionMinPos = 0;
  double sectionMin = points[0].height;
  size_t sectionMaxPos = 0;
  double sectionMax = points[0].height;

  for(size_t pos = 1; pos < points.size(); ++pos) {
    DataPoint& point = points[pos];

    if(point.height < sectionMin) {
      // new minimum
      sectionMin = point.height;
      sectionMinPos = pos;
    } else if(point.height > sectionMax) {
      // new maximum
      sectionMax = point.height;
      sectionMaxPos = pos;
    } else {
      // no update check if we have a change in direction

      if(sectionMinPos < sectionMaxPos) {
        // maximum is last so check for a decrease
        double diff = sectionMax - point.height;

        if(diff > changeTolerance) {
          // we have a change of direction add the old section and update the values
          extractTrack(up, sectionStart, sectionMaxPos + 1);

          if(Settings::verbose >= 1) {std::cerr << "Extracting raise from " << sectionStart << " to " << sectionMaxPos << "." << std::endl;}

          sectionStart = sectionMaxPos;
          sectionMinPos = pos;
          sectionMin = point.height;
        }
      } else {
        // minimum is last so check for a increase
        double diff = point.height - sectionMin;

        if(diff > changeTolerance) {
          // we have a change of direction add the old section and update the values
          extractTrack(down, sectionStart, sectionMinPos + 1);

          if(Settings::verbose >= 1) {std::cerr << "Extracting fall from " << sectionStart << " to " << sectionMinPos << "." << std::endl;}

          sectionStart = sectionMinPos;
          sectionMaxPos = pos;
          sectionMax = point.height;
        }
      }
    }
  }

  // add the last section
  DataPoint& startPoint = points[sectionStart];
  if(sectionMax - startPoint.height > startPoint.height - sectionMin) {
    // maximum has a bigger raise
    extractTrack(up, sectionStart, points.size());
  } else {
    // minimum has a bigger fall
    extractTrack(down, sectionStart, points.size());
  }
}

Statistics Track::computeStatistics(bool combineHeight) const {
  Statistics s;
  s.init();

  for(size_t pos = 1; pos < points.size(); pos += 1) {
    const DataPoint& prevPoint = points[pos - 1];
    const DataPoint& curPoint = points[pos];

    long time = curPoint.time - prevPoint.time;
    double height = curPoint.height - prevPoint.height;
    double distancePlain = computePlainDist(curPoint, prevPoint);
    s.update(time, distancePlain, height);
  }

  s.finalize(combineHeight);

  return s;
}

void Track::extractTrack(std::vector<Track>& tracks, size_t start, size_t end) const {
  tracks.resize(tracks.size() + 1);
  Track& track = tracks.back();

  track.points.insert(track.points.end(), points.begin() + start, points.begin() + end);
}
