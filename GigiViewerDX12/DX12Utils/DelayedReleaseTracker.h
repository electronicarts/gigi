///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d12.h>
#include <vector>

class DelayedReleaseTracker
{
public:
	void OnNewFrame(int maxFramesInFlight)
	{
        // advance the age of each object. Free them when it's safe to do so.
        m_objects.erase(
            std::remove_if(m_objects.begin(), m_objects.end(),
                [maxFramesInFlight](Object& object)
                {
                    object.age++;
                    if (object.age >= maxFramesInFlight)
                    {
                        object.object->Release();
                        return true;
                    }
                    return false;
                }
            ),
            m_objects.end()
        );
    }

	void Add(ID3D12DeviceChild* object)
	{
        if (!object)
            return;

        m_objects.push_back({ object, 0 });
	}

    void Release()
    {
        // This assumes there are no more frames in flight and that it's safe to release everything
        for (Object& object: m_objects)
            object.object->Release();
        m_objects.clear();
    }

    size_t getObjectSize()
    {
        return m_objects.size();
    }

private:
	struct Object
	{
        ID3D12DeviceChild* object = nullptr;
		int age = 0;
	};

	std::vector<Object> m_objects;
};