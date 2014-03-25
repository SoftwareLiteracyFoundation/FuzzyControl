#ifndef FCLL_Version_H
#define FCLL_Version_H

#include <ctime>

//-----------------------------------------------------------
// FCLL Version
//-----------------------------------------------------------
class FCLL_Version {
protected:
  string buildDate;
  string releaseDate;
  string currentDate;
  string versionString;
  string major;
  string minor;
  string mini;

public:
  time_t currentTime;
  tm*    gmt;

  // Encapsulation methods
  string  CurrentDate()   const { return currentDate; }

  string  BuildDate()     const { return buildDate; }
  string &BuildDate()           { return buildDate; }

  string  ReleaseDate()   const { return releaseDate; }
  string &ReleaseDate()         { return releaseDate; }

  string  VersionString() const { return versionString; }
  string &VersionString()       { return versionString; }

  string  Major() const { return major; }
  string &Major()       { return major; }

  string  Minor() const { return minor; }
  string &Minor()       { return minor; }

  string  Mini()  const { return mini; }
  string &Mini()        { return mini; }

  // Constructor
  FCLL_Version() { GetCurrentDate(); }

  // Version methods
  void GetCurrentDate() {
    time(&currentTime);
    gmt = gmtime(&currentTime);    
    currentDate = asctime(gmt);
  }

  void SetBuildDate(string build) {
    BuildDate() = build;
    BuildDate() += "\n";
  }

  void SetReleaseDate(string release) {
    ReleaseDate() = release;
    ReleaseDate() += "\n";
  }

  void SetVersionNumber(string majorVer, string minorVer, string miniVer) {
    major = majorVer;
    minor = minorVer;
    mini  = miniVer;
  }

  void SetVersionString() {
    versionString += "Fuzzy Control Language Library\n";
    versionString += "Version: " +  major + "." + minor + "." + mini + "\n";
    //versionString += "Build: " + buildDate + "Release: " + releaseDate;
    cout << versionString;
    #ifdef DEBUG
    cout << versionString;
    #endif
  }
};
#endif
