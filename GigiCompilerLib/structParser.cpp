///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Schemas/Types.h"
#include "structParser.h"

// @param type is assumed to be without white spaces and just the type e.g. int3
// @return DataFieldType::Count if not recognized
DataFieldType getDataFieldType(const std::string& value)
{
	const Char* p = (const Char*)value.c_str();

	// slightly inefficient but more readable code

	if (value == "int") return DataFieldType::Int;
	if (value == "int2") return DataFieldType::Int2;
	if (value == "int3") return DataFieldType::Int3;
	if (value == "int4") return DataFieldType::Int4;

	if (value == "uint") return DataFieldType::Uint;
	if (value == "uint2") return DataFieldType::Uint2;
	if (value == "uint3") return DataFieldType::Uint3;
	if (value == "uint4") return DataFieldType::Uint4;

	if (value == "float") return DataFieldType::Float;
	if (value == "float2") return DataFieldType::Float2;
	if (value == "float3") return DataFieldType::Float3;
	if (value == "float4") return DataFieldType::Float4;

	if (value == "bool") return DataFieldType::Bool;
	if (value == "float4x4") return DataFieldType::Float4x4;
	if (value == "uint16") return DataFieldType::Uint_16;
	if (value == "int64_t") return DataFieldType::Int_64;
	if (value == "uint64_t") return DataFieldType::Uint_64;
	if (value == "half") return DataFieldType::Float_16;

	// not recognized
	return DataFieldType::Count;
}



// @param type is assumed to be without white spaces and just the type e.g. int3
// @return DataFieldType::Count if not recognized
void setDataFieldType(std::vector<Enum>& enums, StructField& sf, const std::string& value)
{
	sf.type = getDataFieldType(value);

	// no standard data type, it might be an enum
	if (sf.type == DataFieldType::Count)
	{
		// or it's an enum name, O(n)
		for (auto& e : enums)
		{
			if (value == e.name)
			{
				sf.type = DataFieldType::Int;
				sf.Enum = e.name;
				return;
			}
		}
	}
}

// @param type is assumed to be without white spaces and just the type e.g. float3
// @return StructFieldSemantic::Count if not recognized
StructFieldSemantic getStructFieldSemantic(const std::string& value)
{
	const Char* p = (const Char*)value.c_str();

	// slightly inefficient but more readable code

	if (value == "Position") return StructFieldSemantic::Position;
	if (value == "Color") return StructFieldSemantic::Color;
	if (value == "Normal") return StructFieldSemantic::Normal;
	if (value == "Tangent") return StructFieldSemantic::Tangent;

	if (value == "UV") return StructFieldSemantic::UV;
	if (value == "MaterialID") return StructFieldSemantic::MaterialID;
	if (value == "ShapeID") return StructFieldSemantic::ShapeID;

	// not recognized
	return StructFieldSemantic::Count;
}

// this is very specialized function that might need adjustments over time, best we replace with a proper parseDouble(p, double &Out)
// assumes float numbers without , (watch out for German local setting)
// todo: use proper float function, this also fails with scientific notation e.g. 1.0e-10
// e.g. "12", "-9", ".3f", "12.3"
// need to start with non white space 
bool parseFloat(const Char*& inoutP)
{
	if (isWhiteSpaceOrLF(*inoutP))
		return false;

	const Char* p = inoutP;

	// slightly inefficient but simple
	int64_t value = 0;
	// this also supports -
	if (parseInt64(p, value))
	{
		if (parseStartsWith(p, "."))
		{
			while (isDigitCharacter(*p))
				++p;
		}
		parseStartsWith(p, "f"); // e.g. "123.32f"

		if (*p == 0 || *p == ',' || *p == ';' || *p == ']' || *p == '}' || *p == ')')
		{
			inoutP = p;
			return true;
		}

		return false;
	}

	return false;
}

// use this as replacement for parseWhiteSpaceOrLF() to jump over C++ comment as well
void parseNonCode(const Char*& p)
{
	// we support C++ comments but not C comments

	while (*p)
	{
		parseWhiteSpaceOrLF(p);
		if (parseStartsWith(p, "//"))
		{
			parseToEndOfLine(p);
		}
		else break;
	}
}

// only used by parseComplexLiteral(), call parseComplexLiteral instead
// @param closingCharacter 0 if not used, e.g. }, ), ]
bool _parseComplexLiteralRecursion(const Char*& inoutP, std::string& Out, char closingCharacter = 0)
{
	const Char* p = inoutP;

	for (;;)
	{
		parseNonCode(p);

		const Char* start = p;

		if (parseFloat(p))
		{
			Out += std::string((const char*)start, p - start);
		}
		else if (parseStartsWith(p, "{"))
		{
			Out += '{';
			if (!_parseComplexLiteralRecursion(p, Out, '}'))
				return false;
		}
		else if (parseStartsWith(p, "("))
		{
			Out += '(';
			if (!_parseComplexLiteralRecursion(p, Out, ')'))
				return false;
		}
		else if (parseStartsWith(p, "["))
		{
			Out += '[';
			if (!_parseComplexLiteralRecursion(p, Out, ']'))
				return false;
		}
		else if (closingCharacter && *p == closingCharacter)
		{
			Out += closingCharacter;
			++p;
			inoutP = p;
			return true;
		}
		else if (closingCharacter && parseStartsWith(p, ","))
		{
			Out += ',';
		}
		else if (*p == ',' || *p == ';')	// depending on the use of this function we might want to extend this e.g. ], ), }
		{
			if (!Out.empty())
			{
				inoutP = p;
				return true;
			}

			return false;
		}
		else
			return false;
	}

	return false;
}

// e.g. "12", "12.3f", "{}", "{12,3,2.f}"
bool parseComplexLiteral(const Char*& p, std::string& Out)
{
	Out.clear();

	// first try as number or complex literal e.g. { (2, 3), [12,2,2] }
	if (_parseComplexLiteralRecursion(p, Out))
		return true;

	return parseName(p, Out);	// e.g. enum name
}

// parser is similar to struct, no assignment support, trailing comments are ignored
// e.g.
//  enum MyEnum
//  {
//    ME_One,
//    ME_Two, // comments are ignored
//    ME_Last,// last , is optional like in C/C++
//  };
// @param outName e.g. "MyEnum"
// @param outItems e.g. "ME_One", "ME_Two", "ME_Last"
bool enumParser(const Char*& p, Enum& newEnum, std::string& outError)
{
	newEnum.items.clear();

	parseNonCode(p);

	if (parseStartsWith(p, "enum") && !isNameCharacter(*p))
	{
		parseNonCode(p);
		if (!parseName(p, newEnum.name))
		{
			outError = "missing enum name";
			return false;
		}
		parseNonCode(p);
		if (parseStartsWith(p, "{"))
		{
			for (;;)
			{
				parseNonCode(p);

				EnumItem item;

				if (parseName(p, item.label))	// e.g. ME_One
				{
					// todo: should we do more here?
					item.displayLabel = item.label;

					parseWhiteSpaceNoLF(p);
				}

				// we capture trailing C++ comments
				if (parseStartsWith(p, "//"))
				{
					parseWhiteSpaceNoLF(p);
					parseLine(p, item.comment);
				}

				parseNonCode(p);

				if(!item.label.empty())
				{
					// todo: check if name already exists
					newEnum.items.push_back(item);
				}

				if (parseStartsWith(p, ","))	// another enum entry
				{
					continue;
				} 
				else if (parseStartsWith(p, "}"))	// end of enum
				{
					parseNonCode(p);
					if (parseStartsWith(p, ";"))
					{
						// we got the enum, after this there might be more things to parse
						return true;
					}
					else
					{
						outError = "missing ';' after closing enum";
						return false;
					}
				}

				{
					if (*p)
						outError = "enum parse error, unexpected character"; //todo '" + *p + "'";
					else
						outError = "missing '}' to close enum";

					return false;
				}
			}
		}
		else
		{
			outError = "missing '{' after enum";
			return false;
		}
	}

	// outError is empty as this might be the start of a struct
	return false;
}

bool structParser(const Char*& p, Struct& cb, std::vector<Enum>& enums, std::string& outError)
{
	std::string tempName;

	if (parseStartsWith(p, "struct") && !isNameCharacter(*p))
	{
		parseNonCode(p);
		if (!parseName(p, cb.name))
		{
			outError = "missing struct name";
			return false;
		}
		parseNonCode(p);
		if (parseStartsWith(p, "{"))
		{
			for (;;)
			{
				parseNonCode(p);

				if (parseStartsWith(p, "}"))	// end of struct
				{
					parseNonCode(p);
					if (parseStartsWith(p, ";"))
					{
						parseNonCode(p);
						return true;
					}
					else
					{
						outError = "missing ';' after struct definition";
						return false;
					}
				}
				else if (parseName(p, tempName))	// e.g. int
				{
					StructField sf;

					setDataFieldType(enums, sf, tempName);

					if (sf.type == DataFieldType::Count)
					{
						outError = "cannot recognize data type '" + tempName + "'";
						return false;
					}

					// for all names after the type
					bool variablesWithSameType = true;
					while(variablesWithSameType)
					{
						parseNonCode(p);
						if (parseName(p, sf.name))	// e.g. myVar
						{
							parseNonCode(p);
							if (parseStartsWith(p, ":"))
							{
								parseNonCode(p);
								if (!parseName(p, tempName))	// e.g. 123
								{
									outError = "struct parse error, missing semantic name";
									return false;
								}
								sf.semantic = getStructFieldSemantic(tempName);

								if (sf.semantic == StructFieldSemantic::Count)
								{
									outError = "cannot recognize semantic '" + tempName + "'";
									return false;
								}
								parseNonCode(p);
							}
							if (parseStartsWith(p, "="))
							{
								parseNonCode(p);
								if (!parseComplexLiteral(p, sf.dflt))	// e.g. 123.2
								{
									outError = "struct parse error, cannot read constant";
									return false;
								}
								parseNonCode(p);
							}
							if (parseStartsWith(p, ";"))
							{
								// end iteration of variables with same type
								parseWhiteSpaceNoLF(p);
								// we capture trailing C++ comments
								if (parseStartsWith(p, "//"))
								{
									parseWhiteSpaceNoLF(p);
									parseLine(p, sf.comment);
								}
								variablesWithSameType = false;
							}
							else if (parseStartsWith(p, ","))
							{
								// continue looping
							}
							else
							{
								outError = "struct parse error, unexpected character"; // todo + *p + "'";
								return false;
							}

							// todo: check if name already exists
							cb.fields.push_back(sf);

							// clear all data except type
							DataFieldType backup = sf.type;
							sf = {};
							sf.type = backup;
						}
						else
						{
							outError = "struct parse error, unexpected character"; // todo + *p + "'";
							return false;
						}
					}
				}
				else 
				{
					if (*p)
						outError = "parse error, unexpected character"; //todo '" + *p + "'";
					else
						outError = "missing '}' to close struct";

					return false;
				}
			}
		}
		else
		{
			outError = "missing '{' after struct";
			return false;
		}
	}

	// no error, it might be an enum
	return false;
}


bool reparseStructDefinition(const char *definition, std::vector<Enum>& enums, std::vector<Struct>& structs, ParseErrorObject& parseErrorObject)
{
	assert(definition);

	parseErrorObject = {};

	const Char* p = (const Char*)definition;

	StructField sf;
	std::string tempName;

	for (;;)
	{
		parseNonCode(p);

		{
			Enum newEnum;
			if (enumParser(p, newEnum, parseErrorObject.errorMessage))
			{
				assert(parseErrorObject.errorMessage.empty());
				assert(!newEnum.name.empty());
				// todo: name collision check
				enums.push_back(newEnum);
				continue;
			}
			if (!parseErrorObject.errorMessage.empty())
				break;
		}

		{
			Struct newStruct;
			if (structParser(p, newStruct, enums, parseErrorObject.errorMessage))
			{
				assert(parseErrorObject.errorMessage.empty());
				assert(!newStruct.name.empty());
				// todo: name collision check
				structs.push_back(newStruct);
				continue;
			}
			if (!parseErrorObject.errorMessage.empty())
				break;
		}

		if (*p == 0)
			break;

		parseErrorObject.errorMessage = "struct or enum expected";
		break;
	}

	if (parseErrorObject.errorMessage.empty())
		return true;

	// generate error object

	computeLocationInFile((const Char*)definition, p, parseErrorObject.line, parseErrorObject.column);

	const Char* lineStart = p - (parseErrorObject.column - 1);
	parseLine(lineStart, parseErrorObject.errorLine);

	return false;
}
