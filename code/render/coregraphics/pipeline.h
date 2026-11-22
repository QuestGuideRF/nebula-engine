#pragma once
#include "shader.h"
#include "pass.h"
namespace CoreGraphics
{
ID_24_8_TYPE(PipelineId);
struct PipelineCreateInfo
{
    CoreGraphics::ShaderProgramId shader;
    CoreGraphics::PassId pass;
    uint subpass;
    CoreGraphics::InputAssemblyKey inputAssembly;
};
PipelineId CreateGraphicsPipeline(const PipelineCreateInfo& info);
void DestroyGraphicsPipeline(const PipelineId pipeline);
struct PipelineRayTracingTable
{
    CoreGraphics::PipelineId pipeline;
    CoreGraphics::BufferId raygenBindingBuffer, missBindingBuffer, hitBindingBuffer, callableBindingBuffer;
    CoreGraphics::RayDispatchTable table;
};
const PipelineRayTracingTable CreateRaytracingPipeline(const Util::Array<CoreGraphics::ShaderProgramId> programs, const CoreGraphics::QueueType queueType = CoreGraphics::QueueType::GraphicsQueueType);
void DestroyRaytracingPipeline(const PipelineRayTracingTable& table);
}