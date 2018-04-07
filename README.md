# gpsAnalyser
Automatically removes bad data from gps files and computes statistics.

This is hobby project for the cleanup of recorded gps data. A typical use case is:
~~~~{.txt}
./gpsAnalyser -o out -ride -s 2 ./*.gpx
~~~~
This command line will remove invalid points (r), interpolate wrong height values (i), split the tracks into up and down tracks (d), extract breaks (e) and compute statistics about the tracks (s).

The analyser checks for:
 - Invalid heights
 - To rapid accelerations
 - To rapid hight accelerations

The analyser can:
 - Extract breaks
 - Extract up and down tracks

Have fun!

## Command line options

~~~~{.txt}
USAGE:

   ./cmake-build-debug/gpsAnalyser  [--verbose <verbosity>] -o <dir> [--raiseDistance <raiseDistance>] [--pauseMaxRange
                                    <pauseMaxRange>] [--pauseMinTime <pauseMinTime>] [--climbTrendAdept
                                    <climbTrendAdapt>] [--climbMaxSpeed <climbMaxSpeed>] [--invalidSpeed <invalidSpeed>]
                                    [-s <stat_level>] [-e] [-d] [-i] [-r] [--] [--version] [-h] <files> ...


Where:

   --verbose <verbosity>
     Set the verbosity level. 0: No output, 1: Detected errors/regions 2: Detection values.

   -o <dir>,  --output <dir>
     (required)  Output directory.

   --raiseDistance <raiseDistance>
     The minimum change in elevation that is considered as a raise or fall change. Unit: meter

   --pauseMaxRange <pauseMaxRange>
     Maximum range for the movement in a pause. Unit: meter

   --pauseMinTime <pauseMinTime>
     Minimum time for a pause. Unit: seconds

   --climbTrendAdept <climbTrendAdapt>
     Adaption of the general trend detection. Range 0.0 to 1.0

   --climbMaxSpeed <climbMaxSpeed>
     Maximum allowed speed for the climbing or falling. Points with higher speed are interpolated according to the trend
     prior to the detected section. Unit: m/s

   --invalidSpeed <invalidSpeed>
     Maximum allowed speed for the track. Points with higher speed are removed. Unit: km/h

   -s <stat_level>,  --statistics <stat_level>
     Compute statistics of each track and produce a grand total. Verbosity: 0=Off, 1=Grand Total, 2=File Total, 3=All

   -e,  --extBreak
     Extract breaks from track.

   -d,  --divide
     Split tracks into up and down.

   -i,  --intHeight
     Remove wrong height values and interpolate the gap.

   -r,  --remInvalid
     Remove invalid values.

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.

   <files>  (accepted multiple times)
     (required)  Input files for analysis.

~~~~
