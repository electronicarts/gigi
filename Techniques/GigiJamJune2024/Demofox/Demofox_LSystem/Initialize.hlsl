// Unnamed technique, shader Initialize
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	// Initialize the symbols buffer
	GlobalState[0].SymbolsCount = 1;
	Symbols[0] = /*$(Variable:StartingSymbol)*/;

	// Clear out the vertex buffer
	GlobalState[0].LineListCount = 0;

	// Clear out the rules indices
	{
		uint rulesIndexSize;
		RulesIndex.GetDimensions(rulesIndexSize);
		for (uint i = 0; i < rulesIndexSize; ++i)
			RulesIndex[i] = ~0;
	}

	// Load the rules
	{
		uint rulesSize;
		Rules.GetDimensions(rulesSize);
		bool firstSymbol = true;
		for (uint i = 0; i < rulesSize; ++i)
		{
			int ruleSymbol = Rules[i];

			if (firstSymbol)
			{
				RulesIndex[ruleSymbol] = i + 1;
				firstSymbol = false;
			}
			if (ruleSymbol < 0)
				firstSymbol = true;
		}
	}
}

/*
Shader Resources:
	Buffer GlobalState (as UAV)
	Buffer Symbols (as UAV)
	Buffer Rules (as UAV)
	Buffer RulesIndex (as UAV)
*/
