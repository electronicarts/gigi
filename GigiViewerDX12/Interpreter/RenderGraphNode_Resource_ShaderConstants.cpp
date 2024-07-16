///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "NodesShared.h"

bool GigiInterpreterPreviewWindowDX12::OnNodeAction(const RenderGraphNode_Resource_ShaderConstants& node, RuntimeTypes::RenderGraphNode_Resource_ShaderConstants& runtimeData, NodeAction nodeAction)
{
	ScopeProfiler _p(m_profiler, (node.c_shorterTypeName + ": " + node.name).c_str(), nullptr, nodeAction == NodeAction::Execute, true);

	// Get the definition of this constant buffer
	const Struct& structDef = m_renderGraph.structs[node.structure.structIndex];

	// On init, allocate cpu side storage for the constant buffer data
	if (nodeAction == NodeAction::Init)
	{
		if (runtimeData.m_cpuData.size() != structDef.sizeInBytes)
			runtimeData.m_cpuData.resize(structDef.sizeInBytes, 0);

		return true;
	}

	// On execute, copy the variables into the cpu side data
	// Also copy them to the GPU
	if (nodeAction == NodeAction::Execute)
	{
		// Copy variables into the cpu memory
		for (const SetCBFromVar& setFromvar : node.setFromVar)
		{
			const RuntimeVariable& srcVariable = m_runtimeVariables[setFromvar.variable.variableIndex];

			int fieldIndex = 0;
			size_t offset = 0;
			while (fieldIndex < structDef.fields.size() && _stricmp(structDef.fields[fieldIndex].name.c_str(), setFromvar.field.c_str()))
			{
				offset += structDef.fields[fieldIndex].sizeInBytes;
				fieldIndex++;
			}

			if (fieldIndex >= structDef.fields.size())
				return false;

			memcpy(&runtimeData.m_cpuData[offset], srcVariable.storage.value, srcVariable.storage.size);
		}

		// Get an upload gpu buffer
		runtimeData.m_buffer = m_uploadBufferTracker.GetBuffer(m_device, structDef.sizeInBytes, true);

		if (runtimeData.m_buffer == nullptr)
		{
			m_logFn(LogLevel::Error, "Could not allocate constant buffer \"%s\"", node.name.c_str());
			return false;
		}

		// write into it!
		{
			void* CBStart = nullptr;
			HRESULT hr = runtimeData.m_buffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&CBStart));
			if (hr)
				return false;

			// fill with zeros, to make padding values be deterministic
			memset(CBStart, 0, ALIGN(256, runtimeData.m_cpuData.size()));

			// write the data
			memcpy(CBStart, runtimeData.m_cpuData.data(), runtimeData.m_cpuData.size());

			runtimeData.m_buffer->buffer->Unmap(0, nullptr);
		}

		runtimeData.HandleViewableConstantBuffer(*this, (node.name + ".resource").c_str(), runtimeData.m_buffer->buffer, (int)runtimeData.m_buffer->size, node.structure.structIndex, false, true);

		return true;
	}

	return true;
}
