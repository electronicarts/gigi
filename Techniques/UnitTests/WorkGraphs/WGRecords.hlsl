// Unnamed technique, shader WG1
/*$(ShaderResources)*/

struct InputRecord
{
    int3 value;
};

[Shader("node")]
[NodeIsProgramEntry]
[NodeLaunch("thread")]
/*$(_workgraph:main)*/(ThreadNodeInputRecord<InputRecord> record)
{
    InterlockedAdd(Output[OUTPUT_INDEX].x, record.Get().value.x);
    InterlockedAdd(Output[OUTPUT_INDEX].y, record.Get().value.y);
    InterlockedAdd(Output[OUTPUT_INDEX].z, record.Get().value.z);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
