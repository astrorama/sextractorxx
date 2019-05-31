/*
 * ProgressBar.h
 *
 *  Created on: Mar 19, 2019
 *      Author: Alejandro Alvarez Ayllon
 */

#ifndef _SEMAIN_PROGRESSBAR_H
#define _SEMAIN_PROGRESSBAR_H

#include "SEMain/ProgressReporter.h"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/thread.hpp>
#include <vector>

namespace SExtractor {

/**
 * @class ProgressBar
 * This concrete implementation of ProgressReporter uses a progress bar
 * on the bottom of the terminal to report the progress. std::cerr is used, as std::cout could have been used
 * for the output catalog.
 * It uses a separate thread, and refreshes the content every second.
 * It relies on raw ANSI escape codes, instead of ncurses, to avoid introducing an extra dependency.
 * @see https://en.wikipedia.org/wiki/ANSI_escape_code
 */
class ProgressBar: public ProgressReporter {
public:
  virtual ~ProgressBar();

  ProgressBar();

  /**
   * This class intercepts the first call to update to decide on the size of the bottom
   * block, and start the thread.
   */
  void handleMessage(const std::map<std::string, std::pair<int, int>> & info) override;

  /**
   * Give the printing thread a chance to exit.
   */
  void handleMessage(const bool &done) override;

  /**
   * @return true if the attached terminal has the required capabilities to render the progress bar
   */
  static bool isTerminalCapable();

protected:
  boost::posix_time::ptime m_started;
  std::unique_ptr<boost::thread> m_progress_thread;

  /**
   * This method runs on a separate thread, handling UI drawing and events.
   */
  static void uiThread(void *);

  /**
   * Re-draw progress report.
   */
  void updateProgress(void);
};

} // end SExtractor

#endif // _SEMAIN_PROGRESSBAR_H
