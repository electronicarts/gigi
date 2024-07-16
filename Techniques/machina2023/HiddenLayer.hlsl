/*$(ShaderResources)*/

/*$(_compute:HiddenLayer)*/(uint3 DTid : SV_DispatchThreadID)
{
    int hiddenNeuronIndex = DTid.x;

    // Calculate where the weights begin and end for this neuron.
    // There is an extra weight for the bias
    int weightsBeginIndex = hiddenNeuronIndex * (/*$(Variable:c_numInputNeurons)*/ + 1);

    const uint2 inputResolution = uint2/*$(Variable:c_NNInputImageSize)*/;

    float output = NNWeights[weightsBeginIndex + int/*$(Variable:c_numInputNeurons)*/]; // bias
    for (int inputNeuronIndex = 0; inputNeuronIndex < int/*$(Variable:c_numInputNeurons)*/; ++inputNeuronIndex)
    {
        uint2 inputPixelPos = uint2(inputNeuronIndex % inputResolution.x, inputNeuronIndex / inputResolution.x );
        output += NNInput[inputPixelPos] * NNWeights[weightsBeginIndex + inputNeuronIndex];
    }

    // activation function
    HiddenLayerActivations[hiddenNeuronIndex] = 1.0f / (1.0f + exp(-output));
}