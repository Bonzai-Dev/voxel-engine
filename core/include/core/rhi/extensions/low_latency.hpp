// Goal: minimizing latency between input sampling and frame presentation

#pragma once

#define NRI_LOW_LATENCY_H 1

namespace Core::RHI {
  /*clang-format off*/
  class SwapChain;
  class Queue;

  // us = microseconds

  enum class LatencyMarker: uint8_t {     // Should be called:
    SimulationStart    = 0,             // at the start of the simulation execution each frame, but after the call to "LatencySleep"
    SimulationEnd      = 1,             // at the end of the simulation execution each frame
    RenderSubmitStart  = 2,             // at the beginning of the render submission execution each frame (must not span into asynchronous rendering)
    RenderSubmitEnd    = 3,             // at the end of the render submission execution each frame
    InputSample        = 6              // just before the application gathers input data, but between "SIMULATION_START" and "SIMULATION_END" (yes, 6!)
  };

  struct LatencySleepMode {
    uint32_t minIntervalUs;             // minimum allowed frame interval (0 - no frame rate limit)
    bool lowLatencyMode;                // low latency mode enablement
    bool lowLatencyBoost;               // hint to increase performance to provide additional latency savings at a cost of increased power consumption
  };

  struct LatencyReport {                  // The time stamp written:
    uint64_t inputSampleTimeUs;         // when "INPUT_SAMPLE" marker is set
    uint64_t simulationStartTimeUs;     // when "SIMULATION_START" marker is set
    uint64_t simulationEndTimeUs;       // when "SIMULATION_END" marker is set
    uint64_t renderSubmitStartTimeUs;   // when "RENDER_SUBMIT_START" marker is set
    uint64_t renderSubmitEndTimeUs;     // when "RENDER_SUBMIT_END" marker is set
    uint64_t presentStartTimeUs;        // right before "Present"
    uint64_t presentEndTimeUs;          // right after "Present"
    uint64_t driverStartTimeUs;         // when the first "QueueSubmitTrackable" is called
    uint64_t driverEndTimeUs;           // when the final "QueueSubmitTrackable" hands off from the driver
    uint64_t osRenderQueueStartTimeUs;
    uint64_t osRenderQueueEndTimeUs;
    uint64_t gpuRenderStartTimeUs;      // when the first submission reaches the GPU
    uint64_t gpuRenderEndTimeUs;        // when the final submission finishes on the GPU
  };

  // Multi-swapchain is supported only by VK
  // "QueueSubmitDesc::swapChain" must be used to associate work submission with a low latency swap chain
  // Threadsafe: no
  struct LowLatencyInterface {
    Result     (*setLatencySleepMode)   (SwapChain &swapChain, const LatencySleepMode &latencySleepMode);
    Result     (*setLatencyMarker)      (SwapChain &swapChain, LatencyMarker latencyMarker);
    Result     (*latencySleep)          (SwapChain &swapChain); // call once before "INPUT_SAMPLE"
    Result     (*getLatencyReport)      (const SwapChain &swapChain, LatencyReport &latencyReport);
  };
  /*clang-format on*/
}
