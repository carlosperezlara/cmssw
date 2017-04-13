#ifndef FastTimerService_h
#define FastTimerService_h

// C++ headers
#include <cmath>
#include <string>
#include <map>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <unistd.h>

// boost headers
#include <boost/chrono.hpp>

// tbb headers
#include <tbb/concurrent_unordered_set.h>

// CMSSW headers
#include "FWCore/ServiceRegistry/interface/ActivityRegistry.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/ServiceRegistry/interface/SystemBounds.h"
#include "FWCore/ServiceRegistry/interface/ModuleCallingContext.h"
#include "FWCore/ServiceRegistry/interface/PathContext.h"
#include "FWCore/ServiceRegistry/interface/StreamContext.h"
#include "FWCore/ServiceRegistry/interface/ProcessContext.h"
#include "FWCore/ServiceRegistry/interface/GlobalContext.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/TriggerNamesService.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/Common/interface/HLTPathStatus.h"
#include "DataFormats/Provenance/interface/EventID.h"
#include "DataFormats/Provenance/interface/Timestamp.h"
#include "DataFormats/Provenance/interface/ModuleDescription.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "HLTrigger/Timer/interface/ProcessCallGraph.h"


/*
procesing time is diveded into
 - source
 - pre-event processing overhead
 - event processing
 - post-event processing overhead

until lumi-processing and run-processing are taken into account, they will count as inter-event overhead

event processing time is diveded into
 - trigger processing (from the begin of the first path to the end of the last path)
 - trigger overhead
 - endpath processing (from the begin of the first endpath to the end of the last endpath)
 - endpath overhead
*/

/*
Assuming an HLT process with ~2500 modules and ~500 paths, tracking each step (with two calls per step, to start and stop the timer)
with std::chrono::high_resolution_clock gives a per-event overhead of 1 ms

Detailed informations on different timers can be extracted running $CMSSW_RELEASE_BASE/test/$SCRAM_ARCH/testChrono .


Timer per-call overhead on SLC5:

Linux 2.6.18-371.1.2.el5 x86_64
glibc version: 2.5
clock source: unknown
For each timer the resolution reported is the MINIMUM (MEDIAN) (MEAN +/- its STDDEV) of the increments measured during the test.

Performance of std::chrono::high_resolution_clock
        Average time per call:      317.0 ns
        Clock tick period:            1.0 ns
        Measured resolution:       1000.0 ns (median: 1000.0 ns) (sigma: 199.4 ns) (average: 1007.6 +/- 0.4 ns)


Timer per-call overhead on SLC6 (virtualized):

Linux 2.6.32-358.23.2.el6.x86_64 x86_64
glibc version: 2.12
clock source: kvm-clock
For each timer the resolution reported is the MINIMUM (MEDIAN) (MEAN +/- its STDDEV) of the increments measured during the test.

Performance of std::chrono::high_resolution_clock
        Average time per call:      351.2 ns
        Clock tick period:            1.0 ns
        Measured resolution:          1.0 ns (median: 358.0 ns) (sigma: 30360.8 ns) (average: 685.7 +/- 42.4 ns)
*/


class FastTimerService {
public:
  FastTimerService(const edm::ParameterSet &, edm::ActivityRegistry & );
  ~FastTimerService();

private:
  void ignoredSignal(std::string signal) const;
  void unsupportedSignal(std::string signal) const;

  // these signal pairs are not guaranteed to happen in the same thread

  void preallocate(edm::service::SystemBounds const&);

  void preBeginJob(edm::PathsAndConsumesOfModulesBase const&, edm::ProcessContext const&);
  void postBeginJob();

  void postEndJob();

  void preGlobalBeginRun(edm::GlobalContext const&);
  void postGlobalBeginRun(edm::GlobalContext const&);

  void preGlobalEndRun(edm::GlobalContext const&);
  void postGlobalEndRun(edm::GlobalContext const&);

  void preStreamBeginRun(edm::StreamContext const&);
  void postStreamBeginRun(edm::StreamContext const&);

  void preStreamEndRun(edm::StreamContext const&);
  void postStreamEndRun(edm::StreamContext const&);

  void preGlobalBeginLumi(edm::GlobalContext const&);
  void postGlobalBeginLumi(edm::GlobalContext const&);

  void preGlobalEndLumi(edm::GlobalContext const&);
  void postGlobalEndLumi(edm::GlobalContext const&);

  void preStreamBeginLumi(edm::StreamContext const&);
  void postStreamBeginLumi(edm::StreamContext const&);

  void preStreamEndLumi(edm::StreamContext const&);
  void postStreamEndLumi(edm::StreamContext const&);

  void preEvent(edm::StreamContext const&);
  void postEvent(edm::StreamContext const&);

  void prePathEvent(edm::StreamContext const&, edm::PathContext const&);
  void postPathEvent(edm::StreamContext const&, edm::PathContext const&, edm::HLTPathStatus const&);

  // these signal pairs are guaranteed to be called within the same thread

  //void preOpenFile(std::string const&, bool);
  //void postOpenFile(std::string const&, bool);

  //void preCloseFile(std::string const&, bool);
  //void postCloseFile(std::string const&, bool);

  void preSourceConstruction(edm::ModuleDescription const&);
  //void postSourceConstruction(edm::ModuleDescription const&);

  void preSourceRun();
  void postSourceRun();

  void preSourceLumi();
  void postSourceLumi();

  void preSourceEvent(edm::StreamID);
  void postSourceEvent(edm::StreamID);

  //void preModuleConstruction(edm::ModuleDescription const&);
  //void postModuleConstruction(edm::ModuleDescription const&);

  //void preModuleBeginJob(edm::ModuleDescription const&);
  //void postModuleBeginJob(edm::ModuleDescription const&);

  //void preModuleEndJob(edm::ModuleDescription const&);
  //void postModuleEndJob(edm::ModuleDescription const&);

  //void preModuleBeginStream(edm::StreamContext const&, edm::ModuleCallingContext const&);
  //void postModuleBeginStream(edm::StreamContext const&, edm::ModuleCallingContext const&);

  //void preModuleEndStream(edm::StreamContext const&, edm::ModuleCallingContext const&);
  //void postModuleEndStream(edm::StreamContext const&, edm::ModuleCallingContext const&);

  void preModuleGlobalBeginRun(edm::GlobalContext const&, edm::ModuleCallingContext const&);
  void postModuleGlobalBeginRun(edm::GlobalContext const&, edm::ModuleCallingContext const&);

  void preModuleGlobalEndRun(edm::GlobalContext const&, edm::ModuleCallingContext const&);
  void postModuleGlobalEndRun(edm::GlobalContext const&, edm::ModuleCallingContext const&);

  void preModuleGlobalBeginLumi(edm::GlobalContext const&, edm::ModuleCallingContext const&);
  void postModuleGlobalBeginLumi(edm::GlobalContext const&, edm::ModuleCallingContext const&);

  void preModuleGlobalEndLumi(edm::GlobalContext const&, edm::ModuleCallingContext const&);
  void postModuleGlobalEndLumi(edm::GlobalContext const&, edm::ModuleCallingContext const&);

  void preModuleStreamBeginRun(edm::StreamContext const&, edm::ModuleCallingContext const&);
  void postModuleStreamBeginRun(edm::StreamContext const&, edm::ModuleCallingContext const&);

  void preModuleStreamEndRun(edm::StreamContext const&, edm::ModuleCallingContext const&);
  void postModuleStreamEndRun(edm::StreamContext const&, edm::ModuleCallingContext const&);

  void preModuleStreamBeginLumi(edm::StreamContext const&, edm::ModuleCallingContext const&);
  void postModuleStreamBeginLumi(edm::StreamContext const&, edm::ModuleCallingContext const&);

  void preModuleStreamEndLumi(edm::StreamContext const&, edm::ModuleCallingContext const&);
  void postModuleStreamEndLumi(edm::StreamContext const&, edm::ModuleCallingContext const&);

  void preModuleEventPrefetching(edm::StreamContext const&, edm::ModuleCallingContext const&);
  void postModuleEventPrefetching(edm::StreamContext const&, edm::ModuleCallingContext const&);

  void preModuleEvent(edm::StreamContext const&, edm::ModuleCallingContext const&);
  void postModuleEvent(edm::StreamContext const&, edm::ModuleCallingContext const&);

  void preModuleEventDelayedGet(edm::StreamContext const&, edm::ModuleCallingContext const&);
  void postModuleEventDelayedGet(edm::StreamContext const&, edm::ModuleCallingContext const&);

  void preEventReadFromSource(edm::StreamContext const&, edm::ModuleCallingContext const&);
  void postEventReadFromSource(edm::StreamContext const&, edm::ModuleCallingContext const&);

public:
  static void fillDescriptions(edm::ConfigurationDescriptions & descriptions);

private:
  // resources being monitored by the service
  struct Resources {
    boost::chrono::nanoseconds time_thread;
    boost::chrono::nanoseconds time_real;

    Resources() :
      time_thread(boost::chrono::nanoseconds::zero()),
      time_real(boost::chrono::nanoseconds::zero())
    { }

    void reset() {
      time_thread = boost::chrono::nanoseconds::zero();
      time_real   = boost::chrono::nanoseconds::zero();
    }

    Resources & operator+=(Resources const& other) {
      time_thread += other.time_thread;
      time_real   += other.time_real;
      return *this;
    }

    Resources operator+(Resources const& other) const {
      Resources result;
      result.time_thread = time_thread + other.time_thread;
      result.time_real   = time_real   + other.time_real;
      return result;
    }
  };

  struct ResourcesPerPath {
    Resources active;       // resources used by all modules on this path
    Resources total;        // resources used by all modules on this path, and their dependencies
    unsigned  last;         // one-past-the last module that ran on this path
    bool      status;       // whether the path accepted or rejected the event

    void reset() {
      active.reset();
      total.reset();
      last = 0;
      status = false;
    }
  };

  struct ResourcesPerProcess {
    Resources                     total;
    std::vector<ResourcesPerPath> paths;
    std::vector<ResourcesPerPath> endpaths;

    void reset() {
      total.reset();
      for (auto & path: paths)
        path.reset();
      for (auto & path: endpaths)
        path.reset();
    }
  };

  struct ResourcesPerJob {
    Resources                        total;
    std::vector<Resources>           modules;
    std::vector<ResourcesPerProcess> processes;

    void reset() {
      total.reset();
      for (auto & module: modules)
        module.reset();
      for (auto & process: processes)
        process.reset();
    }
  };


  // per-thread measurements
  struct Measurement {
    boost::chrono::thread_clock::time_point          time_thread;
    boost::chrono::high_resolution_clock::time_point time_real;

    void measure() {
      time_thread = boost::chrono::thread_clock::now();
      time_real   = boost::chrono::high_resolution_clock::now();
    }

    void measure_and_store(Resources & store) {
      auto new_time_thread = boost::chrono::thread_clock::now();
      auto new_time_real   = boost::chrono::high_resolution_clock::now();
      store.time_thread = new_time_thread - time_thread;
      store.time_real   = new_time_real   - time_real;
      time_thread = new_time_thread;
      time_real   = new_time_real;
    }
  };


  // plots associated to each module or other element (path, process, etc)
  class PlotsPerElement {
  public:
    PlotsPerElement();
    void reset();
    void book(DQMStore::IBooker &, std::string const& name, std::string const& title, double range, double resolution, unsigned int lumisections);
    void fill(Resources const&, unsigned int lumisection);

  private:
    // resources spent in the module
    TH1F *      time_thread_;
    TProfile *  time_thread_byls_;
    TH1F *      time_real_;
    TProfile *  time_real_byls_;
  };

  // plots associated to each path or endpath
  class PlotsPerPath {
  public:
    PlotsPerPath();
    void reset();
    void book(DQMStore::IBooker &, ProcessCallGraph const&, ProcessCallGraph::PathType const&, double range, double resolution, unsigned int lumisections);
    void fill(ProcessCallGraph::PathType const&, ResourcesPerJob const&, ResourcesPerPath const&, unsigned int lumisection);

  private:
    // resources spent in all the modules in the path, including their dependencies
    PlotsPerElement total_;
    // how many times each module and their dependencies has run
    // Note: a TH1I might be better suited, but is not supported by the DQM
    TH1F * module_counter_;
    // resources spent in each module and their dependencies
    TH1F * module_time_thread_total_;
    TH1F * module_time_real_total_;
  };

  class PlotsPerProcess {
  public:
    PlotsPerProcess(ProcessCallGraph::ProcessType const&);
    void reset();
    void book(DQMStore::IBooker &, ProcessCallGraph const&, ProcessCallGraph::ProcessType const&, 
        double event_range, double event_resolution, double path_range, double path_resolution, unsigned int lumisections);
    void fill(ProcessCallGraph::ProcessType const&, ResourcesPerJob const&, ResourcesPerProcess const&, unsigned int ls);
  
  private:
    // resources spent in all the modules of the (sub)process
    PlotsPerElement           event_;
    // resources spent in each path and endpath
    std::vector<PlotsPerPath> paths_;
    std::vector<PlotsPerPath> endpaths_;
  };

  class PlotsPerJob {
  public:
    PlotsPerJob();
    PlotsPerJob(ProcessCallGraph const&);
    void reset();
    void book(DQMStore::IBooker &, ProcessCallGraph const&, double event_range, double event_resolution,
        double path_range, double path_resolution, double module_range, double module_resolution, unsigned int lumisections);
    void fill(ProcessCallGraph const&, ResourcesPerJob const&, unsigned int ls);

  private:
    // resources spent in all the modules of the job
    PlotsPerElement              event_;
    // resources spent in each module
    std::vector<PlotsPerElement> modules_;
    // resources spent in each (sub)process
    std::vector<PlotsPerProcess> processes_;
  };


  // keep track of the dependencies among modules
  ProcessCallGraph callgraph_;

  // per-stream information
  std::vector<ResourcesPerJob>  streams_;
  std::vector<PlotsPerJob>      stream_plots_;

  // per-thread quantities, indexed by a thread_local id
  std::vector<Measurement>      threads_;

  // define a unique id per thread
  static unsigned int threadId();

  // retrieve the current thread's per-thread quantities
  Measurement & thread();

  // job configuration
  unsigned int                                  concurrent_runs_;
  unsigned int                                  concurrent_streams_;
  unsigned int                                  concurrent_threads_;

  // dqm configuration
  unsigned int                                  module_id_;                     // pseudo module id for the FastTimerService, needed by the thread-safe DQMStore

  bool                                          enable_dqm_;                    // non const, depends on  the availability of the DQMStore
  const bool                                    enable_dqm_bymodule_;
  const bool                                    enable_dqm_byls_;
  const bool                                    enable_dqm_bynproc_;

  const double                                  dqm_eventtime_range_;
  const double                                  dqm_eventtime_resolution_;
  const double                                  dqm_pathtime_range_;
  const double                                  dqm_pathtime_resolution_;
  const double                                  dqm_moduletime_range_;
  const double                                  dqm_moduletime_resolution_;
  const uint32_t                                dqm_lumisections_range_;
  std::string                                   dqm_path_;

  /*
  // summary data
  std::vector<Timing>                           run_summary_;                   // whole event time accounting per-run
  Timing                                        job_summary_;                   // whole event time accounting per-run
  std::vector<std::vector<TimingPerProcess>>    run_summary_perprocess_;        // per-process time accounting per-job
  std::vector<TimingPerProcess>                 job_summary_perprocess_;        // per-process time accounting per-job
  std::mutex                                    summary_mutex_;                 // synchronise access to the summary objects across different threads
  */

  // log unsupported signals
  mutable tbb::concurrent_unordered_set<std::string> unsupported_signals_;      // keep track of unsupported signals received

  /*
  // print a timing summary for the run or job
  void printSummary(Timing const& summary, std::string const& label) const;
  void printProcessSummary(Timing const& total, TimingPerProcess const& summary, std::string const& label, std::string const& process) const;
  */
};

#endif // ! FastTimerService_h
