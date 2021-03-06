//
// Created by msagebaum on 3/31/18.
//

#ifndef GPSANALYSER_UTIL_H
#define GPSANALYSER_UTIL_H


#include <ctime>
#include <iomanip>
#include <rapidxml.hpp>

inline std::string formatDateTime(long value, bool dateOut, bool timeOut, bool milliOut) {
  long milli = value % 1000;
  std::time_t time = value / 1000;

  std::tm* r;
  r = gmtime(&time);
  std::stringstream s;
  if(dateOut) {s << std::put_time(r, "%Y-%m-%d");}
  if(dateOut && timeOut) {s << "T";}
  if(timeOut) {s << std::put_time(r, "%H:%M:%S");}
  if(milliOut) {s << "." << std::setfill('0') << std::setw(3) << milli;}

  return s.str();
}

inline std::string formatDouble(double value, int precission, int width) {
  std::stringstream s;
  s.setf(std::stringstream::fixed);
  s.precision(precission);
  s << std::setw(width) << value;

  return s.str();
}

inline std::string formatInt(int value, int width) {
  std::stringstream s;
  s.width(width);
  s << value;

  return s.str();
}

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
  std::string s = formatDouble(value, 14, 0);

  return doc.allocate_string(s.c_str(), s.size() + 1);
}

inline const char* toString(rapidxml::xml_document<>& doc, long value) {
  std::string s = formatDateTime(value, true, true, true) + "Z";
  return doc.allocate_string(s.c_str(), s.size() + 1);
}

inline const char* toString(rapidxml::xml_document<>& doc, const std::string& value) {
  return doc.allocate_string(value.c_str(), value.size() + 1);
}

inline std::string extractFileName(const std::string& input) {// TODO: Improved splitting of file name
  std::string::size_type pos = input.find_last_of('/');
  if(pos == std::string::npos) {
    // local file name
    pos = 0;
  } else {
    pos += 1;
  }

  return input.substr(pos);
}

inline double deg2Rad(const double deg) {
  return deg * 3.14159265359 / 180.0;
}

#endif //GPSANALYSER_UTIL_H
