/*$(ShaderResources)*/

/*$(_compute:FillIndirectDispatch)*/(uint3 DTid : SV_DispatchThreadID)
{
    IndirectArgsAndCounterBuffer[1] = 2;
    
    IndirectArgsAndCounterBuffer[4] = 1;
    IndirectArgsAndCounterBuffer[5] = 1;
    IndirectArgsAndCounterBuffer[6] = 2;
    IndirectArgsAndCounterBuffer[7] = 100;

    IndirectArgsAndCounterBuffer[8] = 3;
    IndirectArgsAndCounterBuffer[9] = 3;
    IndirectArgsAndCounterBuffer[10] = 2;
    IndirectArgsAndCounterBuffer[11] = 100;

    IndirectArgsAndCounterBuffer[12] = 2;
    IndirectArgsAndCounterBuffer[13] = 2;
    IndirectArgsAndCounterBuffer[14] = 2;
    IndirectArgsAndCounterBuffer[15] = 100;
}