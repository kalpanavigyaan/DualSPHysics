// HEAD_DSPH
/// \file JSphGpuLib.cpp \brief Implements JSphGpuLib.

#include "JSphGpuLib.h"

#include <cuda_runtime.h>
#include <sys/stat.h>

#include <cstring>
#include <stdexcept>

#include "JAppInfo.h"
#include "JLog2.h"
#include "JSphCfgRun.h"
#include "JSphGpuSingle.h"

static void MkDir(const std::string& path) { ::mkdir(path.c_str(), 0755); }

// Provide the global AppInfo instance that DSPH headers reference (was in
// main.cpp).
JAppInfo AppInfo("DualSPHysics5-Lib", "v5.4.355", "08-04-2025");

//==============================================================================
JSphGpuLib::JSphGpuLib() = default;

//==============================================================================
JSphGpuLib::~JSphGpuLib() { Finish(); }

//==============================================================================
void JSphGpuLib::Init(const std::string& case_path, const std::string& case_dir,
                      double tmax, int gpu_id) {
  if (initialized_)
    throw std::runtime_error("JSphGpuLib::Init() already called");

  // Ensure output directories exist
  MkDir(case_dir);
  MkDir(case_dir + "/data");

  // ── Build a minimal JSphCfgRun ──────────────────────────────────────
  cfg_ = new JSphCfgRun();
  cfg_->Reset();
  cfg_->Cpu = false;
  cfg_->Gpu = true;
  cfg_->GpuId = gpu_id;  // -1 means "reuse current context"
  cfg_->CaseName = case_path;
  cfg_->DirOut = case_dir;
  cfg_->DirDataOut = "data";  // relative to DirOut — JSph prepends DirOut
  cfg_->TimeMax = tmax;
  cfg_->Sv_Binx = true;
  cfg_->Sv_Info = true;
  cfg_->Sv_Csv = false;
  cfg_->Sv_Vtk = false;
  cfg_->SvRes = true;
  cfg_->SvTimers = false;

  // ── Create log ──────────────────────────────────────────────────────
  // Initialise the *global* AppInfo (used by JCellDivGpu and other DSPH
  // classes that call AppInfo.LogPtr() in their constructors).
  AppInfo.ConfigRunPaths("");
  AppInfo.ConfigOutput(true, false, case_dir, "data");  // relative to DirOut
  AppInfo.LogInit(case_dir + "/Run.out");
  log_ = AppInfo.LogPtr();

  // ── Construct and initialise the solver ─────────────────────────────
  sph_ = new JSphGpuSingle();
  sph_->RunInit("DualSPHysics5-Lib v5.4.355", cfg_, log_);

  initialized_ = true;
  finished_ = false;
}

//==============================================================================
double JSphGpuLib::StepOnce() {
  if (!initialized_ || !sph_)
    throw std::runtime_error("JSphGpuLib::StepOnce() called before Init()");
  if (finished_) return 0.0;

  double dt = sph_->RunStep();
  if (sph_->GetTimeStep() >= sph_->GetTimeMax()) finished_ = true;
  return dt;
}

//==============================================================================
double JSphGpuLib::GetTime() const { return sph_ ? sph_->GetTimeStep() : 0.0; }

//==============================================================================
bool JSphGpuLib::IsFinished() const { return finished_ || !sph_; }

//==============================================================================
void JSphGpuLib::Finish() {
  if (sph_ && initialized_) {
    sph_->RunFinish();
    delete sph_;
    sph_ = nullptr;
  }
  delete cfg_;
  cfg_ = nullptr;
  initialized_ = false;
  finished_ = true;
}

// ── Device pointer access ────────────────────────────────────────────────────

const double2* JSphGpuLib::GetPosxyDevice() {
  return sph_ ? sph_->GetPosxyPtr() : nullptr;
}

const double* JSphGpuLib::GetPoszDevice() {
  return sph_ ? sph_->GetPoszPtr() : nullptr;
}

const float4* JSphGpuLib::GetVelrhoDevice() {
  return sph_ ? sph_->GetVelrhoPtr() : nullptr;
}

const unsigned* JSphGpuLib::GetIdpDevice() {
  return sph_ ? sph_->GetIdpPtr() : nullptr;
}

const typecode* JSphGpuLib::GetCodeDevice() {
  return sph_ ? sph_->GetCodePtr() : nullptr;
}

unsigned JSphGpuLib::GetNp() const { return sph_ ? sph_->GetNp() : 0; }

unsigned JSphGpuLib::GetNpb() const { return sph_ ? sph_->GetNpb() : 0; }
