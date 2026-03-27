#==================================================================
# DualSPHysics GPU v5.4 – Static library build for sph-dem coupling
#==================================================================
# This file is included via add_subdirectory() from the top-level
# sph-dem project.  It relies on the parent project having already
# done  project(... LANGUAGES CXX CUDA)  and set CMAKE_CUDA_ARCHITECTURES.
#==================================================================

cmake_minimum_required(VERSION 3.18)

# Guard: disable optional modules we don't need for coupling
set(ENABLE_CHRONO    OFF CACHE BOOL "" FORCE)
set(ENABLE_WAVEGEN   OFF CACHE BOOL "" FORCE)
set(ENABLE_MOORDYNPLUS OFF CACHE BOOL "" FORCE)

#------------------------------------------------------------------
# Source file lists  (mirrors the original exactly)
#------------------------------------------------------------------
set(OBJXML        JXml.cpp tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp)
set(OBJSPHMOTION  JMotion.cpp JMotionData.cpp JMotionList.cpp JMotionMov.cpp JMotionObj.cpp JMotionPos.cpp JDsMotion.cpp)
set(OBCOMMON      Functions.cpp FunGeo3d.cpp FunSphKernelsCfg.cpp JAppInfo.cpp JBinaryData.cpp JCfgRunBase.cpp JDataArrays.cpp JException.cpp JLinearValue.cpp JLog2.cpp JObject.cpp JOutputCsv.cpp JRadixSort.cpp JRangeFilter.cpp JReadDatafile.cpp JSaveCsv2.cpp JSpVtkData.cpp JSpVtkShape.cpp JTimeControl.cpp JTrianglesMesh.cpp randomc.cpp)
set(OBCOMMONDSPH  JDsphConfig.cpp JDsPips.cpp JPartDataBi4.cpp JPartDataHead.cpp JDsPartFloatSave.cpp JDsPartMotionSave.cpp JPartFloatInfoBi4.cpp JPartMotRefBi4Save.cpp JPartOutBi4Save.cpp JCaseCtes.cpp JCaseEParms.cpp JCaseParts.cpp JCaseProperties.cpp)
# OBSPH: same as original but without main.cpp + add JSphGpuLib.cpp
set(OBSPH         JArraysCpu.cpp JCellDivCpu.cpp JDebugSphCpu.cpp JSphCfgRun.cpp JComputeMotionRef.cpp JDsDcell.cpp JDsDamping.cpp JDsExtraData.cpp JDsGaugeItem.cpp JDsGaugeSystem.cpp JDsPartsOut.cpp JDsSaveDt.cpp JSphShifting.cpp JSphShiftingAdv.cpp JSph.cpp JDsAccInput.cpp JSphCpu.cpp JSphCpu_mdbc.cpp JSphCpu_preloop.cpp JDsInitialize.cpp JSphMk.cpp JDsOutputParts.cpp JDsPartsInit.cpp JDsFixedDt.cpp JDsViscoInput.cpp JDsOutputTime.cpp JDsTimers.cpp JWaveSpectrumGpu.cpp)
set(OBSPHSINGLE   JCellDivCpuSingle.cpp JPartsLoad4.cpp JSphCpuSingle.cpp)

# GPU-specific sources
set(OBCOMMONGPU     FunctionsCuda.cpp JObjectGpu.cpp)
set(OBSPHGPU        JArraysGpu.cpp JDebugSphGpu.cpp JCellDivGpu.cpp JSphGpu.cpp JDsGpuInfo.cpp)
set(OBSPHSINGLEGPU  JCellDivGpuSingle.cpp JSphGpuSingle.cpp)
set(OBCUDA          JCellDivGpu_ker.cu JCellDivGpuSingle_ker.cu JDsPips_ker.cu JDsGauge_ker.cu JReduSum_ker.cu JSphShifting_ker.cu JDsAccInput_ker.cu JSphGpu_ker.cu JSphGpuSimple_ker.cu JWaveOrder2_ker.cu)

# Wave relaxation zones
set(OBWAVERZGPU     JMLPistonsGpu.cpp JRelaxZonesGpu.cpp)
set(OBWAVERZCUDA    JRelaxZone_ker.cu)

# VRes (multi-resolution)
set(OBVRESGPU       JSphGpuSingle_VRes.cpp)
set(OBVRES          JSphVRes.cpp JSphVResZone.cpp JSphCpuSingle_VRes.cpp JSphCpu_VRes.cpp JCaseVRes.cpp JVResDataLimits.cpp JDsVresData.cpp JBoxDef.cpp)

# InOut
set(OBINOUT         JSphCpu_InOut.cpp JSphCpuSingle_InOut.cpp JSphInOut.cpp JSphInOutZone.cpp JSphInOutGridData.cpp JSphInOutPoints.cpp JSphInOutVel.cpp JSphInOutVelAwas.cpp JSphInOutZsurf.cpp JSimpleNeigs.cpp)
set(OBINOUTGPU      JSphGpuSingle_InOut.cpp)

# Mesh / FlexStruc / Chrono / MoorDyn stubs (compiled unconditionally; disabled via defines)
set(OBMESH          JMeshData.cpp JMeshTDatas.cpp JMeshTDatasLoad.cpp JMeshTDatasSave.cpp JMeshTDatasDsVel.cpp JMeshTDatasXml.cpp)
set(OBFLEXSTRUC     JSphFlexStruc.cpp)
set(OBCHRONO        JChronoGeoObj.cpp JChronoObjects.cpp)
set(OBMOORDYNPLUS   JDsMooredFloatings.cpp JDsFtForcePoints.cpp)

# Library wrapper
set(OBLIB           JSphGpuLib.cpp)

#------------------------------------------------------------------
# Assemble the complete source list for the GPU library
#------------------------------------------------------------------
set(DSPH_GPU_LIB_SOURCES
  ${OBJXML}
  ${OBJSPHMOTION}
  ${OBCOMMON}
  ${OBCOMMONDSPH}
  ${OBSPH}
  ${OBSPHSINGLE}
  ${OBCOMMONGPU}
  ${OBSPHGPU}
  ${OBSPHSINGLEGPU}
  ${OBCUDA}
  ${OBWAVERZGPU}
  ${OBWAVERZCUDA}
  ${OBCHRONO}
  ${OBMOORDYNPLUS}
  ${OBINOUT}
  ${OBINOUTGPU}
  ${OBVRESGPU}
  ${OBMESH}
  ${OBVRES}
  ${OBFLEXSTRUC}
  ${OBLIB}
)

#------------------------------------------------------------------
# Static library target
#------------------------------------------------------------------
add_library(DualSPHGpu STATIC ${DSPH_GPU_LIB_SOURCES})

# DualSPHysics was written for C++11/14; C++17 introduces std::byte which
# conflicts with DSPH's own "typedef unsigned char byte" in TypesDef.h.
set_target_properties(DualSPHGpu PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  CUDA_STANDARD 14
  CUDA_STANDARD_REQUIRED ON
)

target_include_directories(DualSPHGpu PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compile definitions
target_compile_definitions(DualSPHGpu PUBLIC
  _WITHGPU
  _WITHMR
  DISABLE_CHRONO
  DISABLE_WAVEGEN
  DISABLE_MOORDYNPLUS
)

# C++ flags for the .cpp files
target_compile_options(DualSPHGpu PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-O3 -fPIC>
  $<$<COMPILE_LANGUAGE:CUDA>:--use_fast_math -O3>
)

# Suppress noisy deprecation warnings from CUB/Thrust headers in CUDA 12+
target_compile_definitions(DualSPHGpu PRIVATE
  $<$<COMPILE_LANGUAGE:CUDA>:CUB_IGNORE_DEPRECATED_CPP_DIALECT THRUST_IGNORE_DEPRECATED_CPP_DIALECT>
)

#------------------------------------------------------------------
# OpenMP
#------------------------------------------------------------------
find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
  target_link_libraries(DualSPHGpu PUBLIC OpenMP::OpenMP_CXX)
endif()

#------------------------------------------------------------------
# Vendor static libraries (wavegen etc. are disabled,
# Chrono/MoorDyn disabled too – nothing to link here)
#------------------------------------------------------------------
set(DSPH_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib/linux_gcc)
target_link_directories(DualSPHGpu PUBLIC ${DSPH_LIB_DIR})

# Ensure CUDA runtime is available for consumers
target_link_libraries(DualSPHGpu PUBLIC CUDA::cudart_static)
