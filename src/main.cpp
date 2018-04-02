#include <iostream>

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <util.h>
#include <Track.h>

void addAllTracks(rapidxml::xml_node<>* base, const std::string& name, const std::vector<Track>& tracks, rapidxml::xml_document<>& doc) {
  rapidxml::xml_node<>* trackRoot = doc.allocate_node(rapidxml::node_element, "trk");
  rapidxml::xml_node<>* trackName = doc.allocate_node(rapidxml::node_element, "name", toString(doc, name));
  base->append_node(trackRoot);
  trackRoot->append_node(trackName);

  for(auto&& track : tracks) {
    rapidxml::xml_node<>* segRoot = doc.allocate_node(rapidxml::node_element, "trkseg");
    trackRoot->append_node(segRoot);

    track.write(segRoot, doc);
  }
}

void readTracks(const rapidxml::xml_document<>& doc, std::vector<Track>& tracks) {
  rapidxml::xml_node<>* root = doc.first_node();
  for(rapidxml::xml_node<>* curTrk = root->first_node("trk"); curTrk; curTrk = curTrk->next_sibling("trk")) {
    for(rapidxml::xml_node<>* curSeg = curTrk->first_node("trkseg"); curSeg; curSeg = curSeg->next_sibling("trkseg")) {

      tracks.resize(tracks.size() + 1);
      Track& curTrack = tracks[tracks.size() - 1];

      curTrack.read(curSeg);
    }
  }
}

void readDocument(const std::string& file, rapidxml::xml_document<>& doc) {
  std::ifstream in(file);
  std::vector<char> xmlText;
  char buffer[4096];

  long read = 0;
  while(0 != (read = in.readsome(buffer, 4096))) {
    xmlText.insert(xmlText.end(), buffer, &buffer[read]);
  }
  xmlText.push_back(0);

  char* text = doc.allocate_string(xmlText.data(), xmlText.size());
  doc.parse<0>(text);
}

void writeGps(const std::string& filename, rapidxml::xml_document<>& doc, const std::vector<Track>& tracks, const std::vector<Track>& breaks,
              const std::vector<Track>& up, const std::vector<Track>& down) {

  rapidxml::xml_node<>* root = doc.first_node();
  rapidxml::xml_document docOut;
  rapidxml::xml_node<>* outRoot = docOut.allocate_node(rapidxml::node_element, "gpx");
  docOut.append_node(outRoot);
  for(rapidxml::xml_attribute<>* curAttr = root->first_attribute(); curAttr; curAttr = curAttr->next_attribute()) {
    rapidxml::xml_attribute<>* attrClone = docOut.allocate_attribute(curAttr->name(), curAttr->value(),
                                                                     curAttr->name_size(), curAttr->value_size());
    outRoot->append_attribute(attrClone);
  }

  rapidxml::xml_node<>* timeRoot = root->first_node("time");
  if(nullptr != timeRoot) {
    rapidxml::xml_node<>* timeClone = docOut.allocate_node(rapidxml::node_element, timeRoot->name(), timeRoot->value(),
                                                           timeRoot->name_size(), timeRoot->value_size());
    outRoot->append_node(timeClone);
  }

  addAllTracks(outRoot, "normal", tracks, docOut);
  addAllTracks(outRoot, "raise", up, docOut);
  addAllTracks(outRoot, "fall", down, docOut);
  addAllTracks(outRoot, "break", breaks, docOut);

  std::ofstream out(filename);
  out.imbue(std::locale("en_US.utf8"));
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
  out << docOut;
  out.close();
}

void analyzeFile(const std::string& inFile, const std::string& outFile) {
  rapidxml::xml_document doc;
  readDocument(inFile, doc);

  std::vector<Track> tracks;
  rapidxml::xml_node<>* root;
  readTracks(doc, tracks);

  std::vector<Track> breaks;
  std::vector<Track> up;
  std::vector<Track> down;
  for(auto&& track : tracks) {
    track.removeInvalid(100.0);
    track.linearizeWrongHeight(15.0, 0.25);
    track.extractBreaks(30.0, 10.0, breaks);

    track.splitUpDown(100.0, up, down);
  }

  writeGps(outFile, doc, tracks, breaks, up, down);
}

int main() {

  //std::ifstream in("/home/msagebaum/Documents/Programming/gpsAnalyser/test/BlackSlope.gpx");
  //std::ifstream in("/home/msagebaum/Documents/Programming/gpsAnalyser/test/ShortError.gpx");

  analyzeFile("/home/msagebaum/Documents/Programming/gpsAnalyser/test/20180318085841.gpx", "test.gpx");
//  analyzeFile("/home/msagebaum/Documents/Programming/gpsAnalyser/test/ShortError.gpx", "test1.gpx");
//  analyzeFile("/home/msagebaum/Documents/Programming/gpsAnalyser/test/ShortError2.gpx", "test2.gpx");
//  analyzeFile("/home/msagebaum/Documents/Programming/gpsAnalyser/test/BlackSlope.gpx", "test3.gpx");

  
  return 0;
}
