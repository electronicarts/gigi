///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

inline std::string PrettyLabel(const char* label, bool makePretty)
{
    if (!makePretty)
        return label;

	// Handle null and empty string
	std::string ret;
	if (!label || !label[0])
		return ret;

	// capitolize first letter
	ret = label;
	if (ret[0] >= 'a' && ret[0] <= 'z')
		ret[0] = ret[0] - 'a' + 'A';

	// put a space between each lower case letter and upper case letter
	int i = 1;
	bool wasLowerCase = false;
	int upperCaseCount = 1;
	while (i < ret.length())
	{
		bool isLowerCase = (ret[i] >= 'a' && ret[i] <= 'z');
		if (!isLowerCase)
			upperCaseCount++;

		// Put a space between each lower case letter that is followed by an upper case letter
		if (wasLowerCase && !isLowerCase)
		{
			ret.insert(i, " ");
			i++;
		}
		// When we reach a lower case letter after multiple upper case letters, we need to split up the word differently
		// Something like "SRVDescriptors" should become "SRV Descriptors"
		else if (!wasLowerCase && isLowerCase)
		{
			if (upperCaseCount > 1)
			{
				ret.insert(i - 1, " ");
				i++;
			}
		}

		// prepare for next loop
		if (isLowerCase)
			upperCaseCount = 0;
		wasLowerCase = isLowerCase;
		i++;
	}

	return ret;
}
