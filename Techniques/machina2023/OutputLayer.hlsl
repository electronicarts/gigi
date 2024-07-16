/*$(ShaderResources)*/

/*$(_compute:OutputLayer)*/(uint3 DTid : SV_DispatchThreadID)
{
    int outputNeuronIndex = DTid.x;

    // Calculate where the weights begin and end for this neuron.
    // There is an extra weight for the bias
    int weightsBeginIndex = /*$(Variable:c_numHiddenWeights)*/ + outputNeuronIndex * (/*$(Variable:c_numHiddenNeurons)*/ + 1);

    float output = NNWeights[weightsBeginIndex + int/*$(Variable:c_numHiddenNeurons)*/]; // bias
    for (int hiddenNeuronIndex = 0; hiddenNeuronIndex < int/*$(Variable:c_numHiddenNeurons)*/; ++hiddenNeuronIndex)
        output += HiddenLayerActivations[hiddenNeuronIndex] * NNWeights[weightsBeginIndex + hiddenNeuronIndex];

    // activation function
    OutputLayerActivations[outputNeuronIndex] = 1.0f / (1.0f + exp(-output));
}
