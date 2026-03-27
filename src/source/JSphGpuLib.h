#pragma once
//HEAD_DSPH
/// \file JSphGpuLib.h \brief Thin library wrapper around JSphGpuSingle.
///
/// Decomposes JSphGpuSingle::Run() into discrete Init / StepOnce phases
/// so DualSPHysics can be driven as an in-process library sharing a CUDA
/// context with another solver (e.g. DEME).

#include "DualSphDef.h"
#include <cuda_runtime.h>
#include <string>

class JSphGpuSingle;
class JSphCfgRun;
class JLog2;

/// Step-by-step library API for DualSPHysics GPU single-device mode.
///
/// Usage:
///   JSphGpuLib lib;
///   lib.Init(case_path, out_dir, tmax, gpu_id);
///   while (!lib.IsFinished()) { lib.StepOnce(); }
///   lib.Finish();
class JSphGpuLib {
public:
    JSphGpuLib();
    ~JSphGpuLib();

    /// Initialise the simulation.
    /// @param case_path  Base output path (e.g. "output/case/case_name")
    /// @param case_dir   Working / output directory
    /// @param tmax       Maximum simulation time [s]
    /// @param gpu_id     GPU to use.  -1 = reuse the current CUDA context.
    void Init(const std::string& case_path,
              const std::string& case_dir,
              double tmax,
              int gpu_id = -1);

    /// Advance one timestep.  Returns the step dt.
    double StepOnce();

    /// Current simulation time [s].
    double GetTime() const;

    /// True when TimeStep >= TimeMax.
    bool IsFinished() const;

    /// Clean up (called automatically by destructor if not called explicitly).
    void Finish();

    // ── Device pointer access ────────────────────────────────────────────
    const double2*  GetPosxyDevice();
    const double*   GetPoszDevice();
    const float4*   GetVelrhoDevice();
    const unsigned* GetIdpDevice();
    const typecode* GetCodeDevice();

    unsigned GetNp()  const;  ///< Total particles (including periodic)
    unsigned GetNpb() const;  ///< Boundary particles

private:
    JSphGpuSingle* sph_  = nullptr;
    JSphCfgRun*    cfg_  = nullptr;
    JLog2*         log_  = nullptr;
    bool           finished_ = false;
    bool           initialized_ = false;
};
