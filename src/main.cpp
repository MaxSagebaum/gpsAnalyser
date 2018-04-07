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
#include <settings.h>

#include <tclap/CmdLine.h>
#include <Statistics.h>
#include <sys/stat.h>
#include <cstring>


/** from https://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux */
static int do_mkdir(const char *path, mode_t mode)
{
  struct stat            st;
  int             status = 0;

  if (stat(path, &st) != 0)
  {
    /* Directory does not exist. EEXIST for race condition */
    if (mkdir(path, mode) != 0 && errno != EEXIST)
      status = -1;
  }
  else if (!S_ISDIR(st.st_mode))
  {
    errno = ENOTDIR;
    status = -1;
  }

  return(status);
}

/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
*/
int mkpath(const char *path, mode_t mode)
{
  char           *pp;
  char           *sp;
  int             status;
  char           *copypath = strdup(path);

  status = 0;
  pp = copypath;
  while (status == 0 && (sp = strchr(pp, '/')) != 0)
  {
    if (sp != pp)
    {
      /* Neither root nor double slash in path */
      *sp = '\0';
      status = do_mkdir(copypath, mode);
      *sp = '/';
    }
    pp = sp + 1;
  }
  if (status == 0) {
    status = do_mkdir(path, mode);
  }
  free(copypath);
  return (status);
}

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

Statistics outputTracksStatistics(const std::string& name, const std::vector<Track>& tracks, std::ostream& out, const Settings& settings) {
  Statistics total;
  total.init();

  for(size_t pos = 0; pos < tracks.size(); pos += 1) {
    Statistics cur = tracks[pos].computeStatistics(settings.splitUpDown);

    if(settings.outputStatistics >= 3) {
      cur.printRow(out, name + ":" + formatInt((int) pos, 0));
    }
    total.add(cur);
  }

  total.finalize(false);
  if(tracks.size() > 1) {
    if(settings.outputStatistics >= 2) {
      total.printRow(out, name + ":total");
    }
  }

  return total;
}

void outputFileStatistics(const std::string& name, const std::vector<Track>& tracks, const std::vector<Track>& breaks,
              const std::vector<Track>& up, const std::vector<Track>& down, Statistics& total, Statistics& totalUp,
              Statistics& totalDown, Statistics& totalBreak, std::ostream& out, const Settings& settings) {
  if(settings.splitUpDown) {
    Statistics combinedUp = outputTracksStatistics(name + ":up", up, out, settings);
    Statistics combinedDown = outputTracksStatistics(name + ":down", down, out, settings);

    totalUp.add(combinedUp);
    totalDown.add(combinedDown);
  } else {
    Statistics combined = outputTracksStatistics(name + ":normal", tracks, out, settings);
    total.add(combined);
  }
  if(settings.extractPause) {
    Statistics combinedBreak = outputTracksStatistics(name + ":break", breaks, out, settings);
    totalBreak.add(combinedBreak);
  }
}


void writeGps(const std::string& filename, rapidxml::xml_document<>& doc, const std::vector<Track>& tracks, const std::vector<Track>& breaks,
              const std::vector<Track>& up, const std::vector<Track>& down, const Settings& settings) {

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

  if(settings.splitUpDown) {
    addAllTracks(outRoot, "raise", up, docOut);
    addAllTracks(outRoot, "fall", down, docOut);
  } else {
    addAllTracks(outRoot, "normal", tracks, docOut);
  }

  if(settings.extractPause) {
    addAllTracks(outRoot, "break", breaks, docOut);
  }

  std::ofstream out(filename);
  out.imbue(std::locale("en_US.utf8"));
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
  out << docOut;
  out.close();
}

void analyzeFile(const std::string& inFile, const std::string& name, const std::string& outFile, Statistics& total, Statistics& totalUp,
                 Statistics& totalDown, Statistics& totalBreak, std::ostream& out, const Settings& settings) {
  rapidxml::xml_document doc;
  readDocument(inFile, doc);

  std::vector<Track> tracks;
  readTracks(doc, tracks);

  std::vector<Track> breaks;
  std::vector<Track> up;
  std::vector<Track> down;
  for(auto&& track : tracks) {
    if(settings.removeInvalid) {
      track.removeInvalid(settings.invalidSpeed_km_h);
    }
    if(settings.interpolateHeight) {
      track.linearizeWrongHeight(settings.climbMaxSpeed_m_s, settings.climbTrendAdapt);
    }
    if(settings.extractPause) {
      track.extractBreaks(settings.pauseMinTime_s, settings.pauseMaxRange_m, breaks);
    }
    if(settings.splitUpDown) {
      track.splitUpDown(settings.raiseDistance_m, up, down);
    }
  }

  if(0 != settings.outputStatistics) {
    outputFileStatistics(name, tracks, breaks, up, down, total, totalUp, totalDown, totalBreak, out, settings);
  }

  writeGps(outFile, doc, tracks, breaks, up, down, settings);
}

void performAnalysis(const Settings& settings) {
  Statistics total;
  Statistics up;
  Statistics down;
  Statistics breaks;

  total.init();
  up.init();
  down.init();
  breaks.init();

  std::ostream& out = std::cout;
  if(0 != settings.outputStatistics) {
    total.printHeader(out);
  }

  for(int i = 0; i < settings.inputFiles.size(); i += 1) {
    const std::string& input = settings.inputFiles[i];
    std::string output = settings.outputDir;

    std::string name = extractFileName(input);

    if(output.size() - 1 != output.find_last_of('/')) {
      output += "/";
    }

    output += name;

    analyzeFile(input, name, output, total, up, down, breaks, out, settings);
  }

  if(settings.outputStatistics >= 1) {
    total.finalize(false);
    up.finalize(false);
    down.finalize(false);
    breaks.finalize(false);

    if(settings.splitUpDown) {
      up.printRow(out, "total:up");
      down.printRow(out, "total:down");
    } else {
      total.printRow(out, "total");
    }
    if(settings.extractPause) {
      breaks.printRow(out, "total:breaks");
    }
  }
}

int main(int nargs, const char* const* args) {

  try {
    Settings settings;

    TCLAP::CmdLine cmd("gps analyzer", ' ', "beta");

    TCLAP::SwitchArg removeInvalid("r", "remInvalid", "Remove invalid values.", cmd);
    TCLAP::SwitchArg interploateHeight("i", "intHeight", "Remove wrong height values and interpolate the gap.", cmd);
    TCLAP::SwitchArg splitUpDown("d", "divide", "Split tracks into up and down.", cmd);
    TCLAP::SwitchArg extractBreak("e", "extBreak", "Extract breaks from track.", cmd);

    TCLAP::ValueArg<int> outputStatistics("s", "statistics", "Compute statistics of each track and produce a grand total. Verbosity: 0=Off, 1=Grand Total, 2=File Total, 3=All", false, 0, "stat_level", cmd);

    TCLAP::ValueArg<double> invalidSpeed("", "invalidSpeed", "Maximum allowed speed for the track. Points with higher speed are removed. Unit: km/h", false, 100.0, "invalidSpeed", cmd);
    TCLAP::ValueArg<double> climbMaxSpeed("", "climbMaxSpeed", "Maximum allowed speed for the climbing or falling. Points with higher speed are interpolated according to the trend prior to the detected section. Unit: m/s", false, 15.0, "climbMaxSpeed", cmd);
    TCLAP::ValueArg<double> climbTrendAdapt("", "climbTrendAdept", "Adaption of the general trend detection. Range 0.0 to 1.0", false, 0.1, "climbTrendAdapt", cmd);
    TCLAP::ValueArg<double> pauseMinTime("", "pauseMinTime", "Minimum time for a pause. Unit: seconds", false, 30.0, "pauseMinTime", cmd);
    TCLAP::ValueArg<double> pauseMaxRange("", "pauseMaxRange", "Maximum range for the movement in a pause. Unit: meter", false, 10.0, "pauseMaxRange", cmd);
    TCLAP::ValueArg<double> raiseDistance("", "raiseDistance", "The minimum change in elevation that is considered as a raise or fall change. Unit: meter", false, 100.0, "raiseDistance", cmd);
    TCLAP::ValueArg<std::string> output("o", "output", "Output directory.", true, "out", "dir", cmd);

    TCLAP::UnlabeledMultiArg<std::string> input("input", "Input files for analysis.", true, "files", cmd);

    TCLAP::ValueArg<int> verbosity("", "verbose", "Set the verbosity level. 0: No output, 1: Detected errors/regions 2: Detection values.", false, 0, "verbosity", cmd);
    cmd.parse(nargs, args);

    settings.inputFiles = input.getValue();
    settings.outputDir = output.getValue();
    settings.extractPause = extractBreak.isSet();
    settings.splitUpDown = splitUpDown.isSet();
    settings.interpolateHeight = interploateHeight.isSet();
    settings.removeInvalid = removeInvalid.isSet();
    settings.outputStatistics = outputStatistics.getValue();

    settings.invalidSpeed_km_h = invalidSpeed.getValue();
    settings.climbMaxSpeed_m_s = climbMaxSpeed.getValue();
    settings.climbTrendAdapt = climbTrendAdapt.getValue();
    settings.pauseMinTime_s = pauseMinTime.getValue();
    settings.pauseMaxRange_m = pauseMaxRange.getValue();
    settings.raiseDistance_m = raiseDistance.getValue();

    Settings::verbose = verbosity.getValue();

    // create all directories
    if(0 != mkpath(settings.outputDir.c_str(), 0777)) {
      std::cerr << "Could not create directory: " << settings.outputDir << std::endl;
      return -1;
    }

    performAnalysis(settings);
  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
  }
  return 0;
}

int Settings::verbose;
