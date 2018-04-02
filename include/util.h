//
// Created by msagebaum on 3/31/18.
//

#ifndef GPSANALYSER_UTIL_H
#define GPSANALYSER_UTIL_H


#include <ctime>
#include <iomanip>
#include <rapidxml.hpp>

inline double parseDouble(const char* value) {
  const std::string s(value);

  return std::stod(s, nullptr);
}

inline double parseDouble(rapidxml::xml_node<>* node, const double def) {
  if(nullptr == node) {
    return def;
  } else {
    return parseDouble(node->value());
  }
}

inline double parseDouble(rapidxml::xml_attribute<>* node, const double def) {
  if(nullptr == node) {
    return def;
  } else {
    return parseDouble(node->value());
  }
}

inline long parseDateTime(rapidxml::xml_node<>* node, const long def) {
  if(nullptr == node) {
    return def;
  } else {
    const char* value = node->value();
    std::istringstream s(value);

    std::tm r = {};
    long ms;
    s >> std::get_time(&r, "%Y-%m-%dT%H:%M:%S.");
    s >> ms;

    ms += mktime(&r) * 1000;

    return ms;
  }
}

inline const char* toString(rapidxml::xml_document<>& doc, double value) {
  std::stringstream s;
  s.precision(14);
  s << value;

  return doc.allocate_string(s.str().c_str(), s.str().size() + 1);
}

inline const char* toString(rapidxml::xml_document<>& doc, long value) {

  long milli = value % 1000;
  std::time_t time = value / 1000;

  std::tm* r;
  r = gmtime(&time);
  std::stringstream s;
  s << std::put_time(r, "%Y-%m-%dT%H:%M:%S.") << std::setfill('0') << std::setw(3) << milli << "Z";

  return doc.allocate_string(s.str().c_str(), s.str().size() + 1);
}

inline const char* toString(rapidxml::xml_document<>& doc, const std::string& value) {
  return doc.allocate_string(value.c_str(), value.size() + 1);
}

inline double deg2Rad(const double deg) {
  return deg * 3.14159265359 / 180.0;
}

#endif //GPSANALYSER_UTIL_H
