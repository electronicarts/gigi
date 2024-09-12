// Unnamed technique, shader Step
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint rulesSize;
	Rules.GetDimensions(rulesSize);

	uint destSymbolIndex = 0;
	for (uint srcSymbolIndex = 0; srcSymbolIndex < GlobalState[0].SymbolsCount; ++srcSymbolIndex)
	{
		uint symbol = (Symbols[srcSymbolIndex]) % /*$(Variable:RulesMax)*/;
		uint ruleIndex = RulesIndex[symbol] + 1;
		if (ruleIndex >= rulesSize)
		{
			if (/*$(Variable:EraseSymbolsWithoutRules)*/ == 0)
			{
				SymbolsNext[destSymbolIndex] = symbol;
				destSymbolIndex++;
			}
			continue;
		}

		int insertSymbol = Rules[ruleIndex];
		while(ruleIndex < rulesSize && insertSymbol >= 0)
		{
			SymbolsNext[destSymbolIndex] = insertSymbol;
			destSymbolIndex++;

			ruleIndex++;
			insertSymbol = Rules[ruleIndex];
		}
	}

	GlobalState[0].SymbolsCount = destSymbolIndex;
}

/*
Shader Resources:
	Buffer GlobalState (as UAV)
	Buffer Symbols (as SRV)
	Buffer Rules (as SRV)
	Buffer RulesIndex (as SRV)
	Buffer SymbolsNext (as UAV)
*/
