// © 2021 NVIDIA Corporation

#pragma once

namespace Core::RHI {

struct PipelineLayoutVal final : public ObjectVal {
    PipelineLayoutVal(DeviceVal& device, PipelineLayout* pipelineLayout, const PipelineLayoutDesc& pipelineLayoutDesc);

    inline PipelineLayout* GetImpl() const {
        return (PipelineLayout*)m_Impl;
    }

    inline const PipelineLayoutDesc& GetPipelineLayoutDesc() const {
        return m_PipelineLayoutDesc;
    }

private:
    PipelineLayoutDesc m_PipelineLayoutDesc = {}; // .natvis
    Vector<DescriptorSetDesc> m_DescriptorSetDescs;
    Vector<RootConstantDesc> m_RootConstantDescs;
    Vector<DescriptorRangeDesc> m_DescriptorRangeDescs;
};

} // namespace Core::RHI
