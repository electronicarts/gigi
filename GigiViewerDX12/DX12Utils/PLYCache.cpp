///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "PLYCache.h"
#include <filesystem>
#include "intrin.h"
#include <bit>

#include "GigiCompilerLib/ParseText.h"

// Info on format at:
// https://en.wikipedia.org/wiki/PLY_(file_format)

using namespace ParseText;

namespace
{
	enum class Format
	{
		unknown,
		ascii,
		binary_LE,
		binary_BE,
	};

	size_t FieldTypeSizeBytes(PLYCache::FieldType fieldType)
	{
		switch (fieldType)
		{
			case PLYCache::FieldType::i8:
			case PLYCache::FieldType::u8: return 1;
			case PLYCache::FieldType::i16:
			case PLYCache::FieldType::u16: return 2;
			case PLYCache::FieldType::i32:
			case PLYCache::FieldType::u32:
			case PLYCache::FieldType::f32: return 4;
			case PLYCache::FieldType::f64: return 8;
			default: return 0;
		}
	}

	PLYCache::FieldType StringToFieldType(const std::string_view& s)
	{
		if (s == "char" || s == "int8")
			return PLYCache::FieldType::i8;
		else if (s == "uchar" || s == "uint8")
			return PLYCache::FieldType::u8;
		else if (s == "short" || s == "int16")
			return PLYCache::FieldType::i16;
		else if (s == "ushort" || s == "uint16")
			return PLYCache::FieldType::u16;
		else if (s == "int" || s == "int32")
			return PLYCache::FieldType::i32;
		else if (s == "uint" || s == "uint32")
			return PLYCache::FieldType::u32;
		else if (s == "float" || s == "float32")
			return PLYCache::FieldType::f32;
		else if (s == "double" || s == "float64")
			return PLYCache::FieldType::f64;
		else
			return PLYCache::FieldType::unknown;
	}

	template <typename T>
	void PushValueIntoBuffer(std::vector<unsigned char>& buffer, T value)
	{
		size_t start = buffer.size();
		buffer.resize(start + sizeof(T));
		memcpy(&buffer[start], &value, sizeof(T));
	}

	bool ReadValueIntoBuffer(std::string_view& fileView, const char*& binaryData, Format format, PLYCache::FieldType type, std::vector<unsigned char>& buffer, uint64_t& valueOut)
	{
		if (format == Format::ascii)
		{
			std::string_view word;
			if (!ReadWord(fileView, word))
				return false;

			switch (type)
			{
				case PLYCache::FieldType::i8:
				{
					int32_t val;
					sscanf_s(&word[0], "%i", &val);
					PushValueIntoBuffer(buffer, (int8_t)val);
					valueOut = (uint64_t)val;
					break;
				}
				case PLYCache::FieldType::u8:
				{
					uint32_t val;
					sscanf_s(&word[0], "%u", &val);
					PushValueIntoBuffer(buffer, (uint8_t)val);
					valueOut = (uint64_t)val;
					break;
				}
				case PLYCache::FieldType::i16:
				{
					int32_t val;
					sscanf_s(&word[0], "%i", &val);
					PushValueIntoBuffer(buffer, (int16_t)val);
					valueOut = (uint64_t)val;
					break;
				}
				case PLYCache::FieldType::u16:
				{
					uint32_t val;
					sscanf_s(&word[0], "%u", &val);
					PushValueIntoBuffer(buffer, (uint16_t)val);
					valueOut = (uint64_t)val;
					break;
				}
				case PLYCache::FieldType::i32:
				{
					int32_t val;
					sscanf_s(&word[0], "%i", &val);
					PushValueIntoBuffer(buffer, (int32_t)val);
					valueOut = (uint64_t)val;
					break;
				}
				case PLYCache::FieldType::u32:
				{
					uint32_t val;
					sscanf_s(&word[0], "%u", &val);
					PushValueIntoBuffer(buffer, (uint32_t)val);
					valueOut = (uint64_t)val;
					break;
				}
				case PLYCache::FieldType::f32:
				{
					float val;
					sscanf_s(&word[0], "%f", &val);
					PushValueIntoBuffer(buffer, val);
					valueOut = (uint64_t)val;
					break;
				}
				case PLYCache::FieldType::f64:
				{
					double val;
					sscanf_s(&word[0], "%lf", &val);
					PushValueIntoBuffer(buffer, val);
					valueOut = (uint64_t)val;
					break;
				}
				default:
				{
					return false;
				}
			}
		}
		// else binary. big endian or little endian
		else
		{
			bool doByteSwap = (format == Format::binary_LE) != (std::endian::native == std::endian::little);

			switch (type)
			{
				case PLYCache::FieldType::u8:
				case PLYCache::FieldType::i8:
				{
					uint8_t val;
					memcpy(&val, binaryData, sizeof(val));
					PushValueIntoBuffer(buffer,val);
					valueOut = (uint64_t)val;
					binaryData += sizeof(val);
					break;
				}
				case PLYCache::FieldType::u16:
				case PLYCache::FieldType::i16:
				{
					uint16_t val;
					memcpy(&val, binaryData, sizeof(val));

					if (doByteSwap)
						val = _byteswap_ushort(val);

					PushValueIntoBuffer(buffer, val);
					valueOut = (uint64_t)val;
					binaryData += sizeof(val);
					break;
				}
				case PLYCache::FieldType::u32:
				case PLYCache::FieldType::i32:
				case PLYCache::FieldType::f32:
				{
					uint32_t val;
					memcpy(&val, binaryData, sizeof(val));

					if (doByteSwap)
						val = _byteswap_ulong(val);

					PushValueIntoBuffer(buffer, val);
					if (type == PLYCache::FieldType::f32)
						valueOut = (uint64_t)*((float*)&val);
					else
						valueOut = (uint64_t)val;
					binaryData += sizeof(val);
					break;
				}
				case PLYCache::FieldType::f64:
				{
					uint64_t val;
					memcpy(&val, binaryData, sizeof(val));

					if (doByteSwap)
						val = _byteswap_uint64(val);

					PushValueIntoBuffer(buffer, val);
					valueOut = (uint64_t)*((double*)&val);
					binaryData += sizeof(val);
					break;
				}
				default:
				{
					return false;
				}
			}
		}

		return true;
	}
}

PLYCache::PLYData& PLYCache::Get(FileCache& fileCache, const char* fileName_)
{
	// normalize the string by making it canonical and making it lower case
	std::string s = std::filesystem::weakly_canonical(fileName_).string();
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	const char* fileName = s.c_str();

	// Return the data if we already have it in the cache
	if (m_cache.count(fileName) != 0)
		return m_cache[fileName];

	// load the file
	FileCache::File fileData = fileCache.Get(fileName);

	// Parse the file
	const char* binaryData = nullptr;
	PLYData plyData;
	if (fileData.Valid())
	{
		plyData.valid = true;

		std::string_view fileView = fileData.GetBytes();
		Format format = Format::unknown;
		ElementGroup* elementGroup = nullptr;

		// read the header
		{
			bool headerEnded = false;
			bool gotPly = false;
			while (true)
			{
				// read a line
				std::string_view line;
				if (!ReadLine(fileView, line))
				{
					plyData.error += "Could not read header\n";
					fileView = "";
					plyData.valid = false;
					break;
				}

				// verify the file starts with ply
				if (!gotPly)
				{
					if (line != "ply")
					{
						plyData.error += "ply not found in header\n";
						fileView = "";
						plyData.valid = false;
						break;
					}
					gotPly = true;
					continue;
				}

				// Else get the command
				std::string_view word;
				if (!ReadWord(line, word))
				{
					plyData.error += "Could not read header\n";
					fileView = "";
					plyData.valid = false;
					break;
				}

				// format
				if (word == "format")
				{
					if (!ReadWord(line, word))
					{
						plyData.error += "Could not read header\n";
						fileView = "";
						plyData.valid = false;
						break;
					}

					if (word == "ascii")
						format = Format::ascii;
					else if (word == "binary_little_endian")
						format = Format::binary_LE;
					else if (word == "binary_big_endian")
						format = Format::binary_BE;
					else
					{
						plyData.error += "Unknown format encountered\n";
						fileView = "";
						plyData.valid = false;
						break;
					}
				}

				// element
				else if (word == "element")
				{
					if (!ReadWord(line, word))
					{
						plyData.error += "Could not read element type\n";
						fileView = "";
						plyData.valid = false;
						break;
					}

					ElementGroup newElementGroup;
					newElementGroup.name = word;

					if (!ReadWord(line, word) || !sscanf_s(&word[0], "%u", &newElementGroup.count))
					{
						plyData.error += "Could not read vertex count\n";
						fileView = "";
						plyData.valid = false;
						break;
					}
					plyData.elementGroups.push_back(newElementGroup);
					elementGroup = &(*plyData.elementGroups.rbegin());
				}

				// comment
				else if (word == "comment")
				{
					// no-op!
				}

				// end_header
				else if (word == "end_header")
				{
					binaryData = &word.data()[word.size()];
					while (IsNewLine(*binaryData))
						binaryData++;
					headerEnded = true;
					break;
				}

				// property
				else if (word == "property")
				{
					if (elementGroup == nullptr)
					{
						plyData.error += "Properties not associated with an element group\n";
						fileView = "";
						plyData.valid = false;
						break;
					}

					std::vector<Property>& properties = elementGroup->properties;

					if (!ReadWord(line, word))
					{
						plyData.error += "Could not read property type\n";
						fileView = "";
						plyData.valid = false;
						break;
					}

					// handle lists
					Property newProperty;
					if (word == "list")
					{
						newProperty.isList = true;

						if (!ReadWord(line, word))
						{
							plyData.error += "Could not read list index type\n";
							fileView = "";
							plyData.valid = false;
							break;
						}

						newProperty.listSizeType = StringToFieldType(word);
						if (newProperty.listSizeType == FieldType::unknown)
						{
							plyData.error += "unknown list size type\n";
							fileView = "";
							plyData.valid = false;
							break;
						}

						if (!ReadWord(line, word))
						{
							plyData.error += "Could not read list value type\n";
							fileView = "";
							plyData.valid = false;
							break;
						}

						newProperty.type = StringToFieldType(word);
						if (newProperty.type == FieldType::unknown)
						{
							plyData.error += "unknown list value type\n";
							fileView = "";
							plyData.valid = false;
							break;
						}
					}
					// handle non lists
					else
					{
						newProperty.type = StringToFieldType(word);
						if (newProperty.type == FieldType::unknown)
						{
							plyData.error += "unknown field property type\n";
							fileView = "";
							plyData.valid = false;
							break;
						}
						elementGroup->propertiesSizeBytes += FieldTypeSizeBytes(newProperty.type);
					}

					// read the name
					if (!ReadWord(line, word))
					{
						plyData.error += "Could not read property name\n";
						fileView = "";
						plyData.valid = false;
						break;
					}
					newProperty.name = word;

					properties.push_back(newProperty);
				}

				// unknown command
				else
				{
					plyData.error += "Unknown command encountered\n";
					fileView = "";
					plyData.valid = false;
					break;
				}
			}
		}

		// verify the info from the header
		{
			if (format == Format::unknown)
			{
				plyData.error += "No format specified\n";
				fileView = "";
				plyData.valid = false;
			}
		}

		// read each element group
		for (ElementGroup& elementGroup : plyData.elementGroups)
		{
			for (unsigned int index = 0; index < elementGroup.count; ++index)
			{
				for (const Property& property : elementGroup.properties)
				{
					if (property.isList)
					{
						uint64_t listSize;
						if (!ReadValueIntoBuffer(fileView, binaryData, format, property.listSizeType, elementGroup.data, listSize))
						{
							plyData.error += "Couldn't read list count\n";
							fileView = "";
							binaryData = nullptr;
							plyData.valid = false;
							break;
						}

						for (uint64_t listIndex = 0; listIndex < listSize; ++listIndex)
						{
							uint64_t dummy = 0;
							if (!ReadValueIntoBuffer(fileView, binaryData, format, property.type, elementGroup.data, dummy))
							{
								plyData.error += "Couldn't read list item\n";
								fileView = "";
								binaryData = nullptr;
								plyData.valid = false;
								break;
							}
						}
					}
					else
					{
						uint64_t dummy = 0;
						if (!ReadValueIntoBuffer(fileView, binaryData, format, property.type, elementGroup.data, dummy))
						{
							plyData.error += "Not enough elements found\n";
							fileView = "";
							binaryData = nullptr;
							plyData.valid = false;
							break;
						}
					}
				}
				if (!plyData.valid)
					break;
			}
			if (!plyData.valid)
				break;
		}
	}

	// Store the result in the cache
	m_cache[fileName] = plyData;
	return m_cache[fileName];
}

PLYCache::PLYData PLYCache::GetFlattened(FileCache& fileCache, const char* fileName)
{
	PLYData invalid;
	invalid.valid = false;

	// Get the data from the cache
	PLYCache::PLYData& plyData = Get(fileCache, fileName);
	if (!plyData.valid)
		return invalid;

	// If the plyData doesn't have indices, it's already flattened, so we are done
	size_t indexElementGroupIndex = ~0;
	{
		bool hasIndex = false;
		for (size_t index = 0; index < plyData.elementGroups.size(); ++index)
		{
			PLYCache::ElementGroup& elementGroup = plyData.elementGroups[index];
			if (elementGroup.name == "face")
			{
				indexElementGroupIndex = index;
				hasIndex = true;
				break;
			}
		}
		if (!hasIndex)
			return plyData;
	}
	const ElementGroup& indexElementGroup = plyData.elementGroups[indexElementGroupIndex];

	// If the indices are in a format we don't understand, we don't know what to do, so return invalid.
	if (indexElementGroup.properties.size() != 1 || indexElementGroup.properties[0].isList == false)
		return invalid;

	// If we can't find the vertex group, we don't know what to flatten, so return invalid
	size_t vertexElementGroupIndex = ~0;
	{
		bool hasVertex = false;
		for (size_t index = 0; index < plyData.elementGroups.size(); ++index)
		{
			PLYCache::ElementGroup& elementGroup = plyData.elementGroups[index];
			if (elementGroup.name == "vertex")
			{
				vertexElementGroupIndex = index;
				hasVertex = true;
				break;
			}
		}
		if (!hasVertex)
			return invalid;
	}
	const ElementGroup& vertexElementGroup = plyData.elementGroups[vertexElementGroupIndex];

	// Flatten!
	PLYCache::PLYData ret;
	{
		//prepare data
		ret.valid = true;
		ret.elementGroups.resize(1);
		ElementGroup& newVertexElementGroup = ret.elementGroups[0];
		newVertexElementGroup.name = vertexElementGroup.name;
		newVertexElementGroup.properties = vertexElementGroup.properties;
		newVertexElementGroup.propertiesSizeBytes = vertexElementGroup.propertiesSizeBytes;

		// flatten
		const unsigned char* srcIndexPointer = indexElementGroup.data.data();
		const unsigned char* srcVertexPointer = vertexElementGroup.data.data();
		size_t vertexSize = vertexElementGroup.propertiesSizeBytes;
		std::vector<size_t> indices;
		for (size_t dataRow = 0; dataRow < indexElementGroup.count; dataRow++)
		{
			// get the number of indices
			size_t numIndices;
			srcIndexPointer = ReadFromBinaryAndCastTo(srcIndexPointer, indexElementGroup.properties[0].listSizeType, numIndices);

			// get the indices
			indices.resize(numIndices);
			for (size_t& index : indices)
				srcIndexPointer = ReadFromBinaryAndCastTo(srcIndexPointer, indexElementGroup.properties[0].type, index);

			// copy up to 3 vertices over
			// indices[0], indices[1], indices[2]
			for (size_t i = 0; i < std::min<size_t>(numIndices, 3); ++i)
			{
				size_t index = indices[i];

				// copy data over
				size_t destOffset = newVertexElementGroup.data.size();
				newVertexElementGroup.data.resize(destOffset + vertexSize);
				memcpy(&newVertexElementGroup.data[destOffset], &srcVertexPointer[index * vertexSize], vertexSize);

				// increment vertex count
				newVertexElementGroup.count++;
			}

			// if > 3 vertices, treat it as a triangle fan
			// for each index i from 3 upwards...
			// copy indices[0], indices[i-1], indices[i]
			for (size_t i = 3; i < numIndices; ++i)
			{
				// Copy indices[0]
				{
					size_t index = indices[0];

					// copy data over
					size_t destOffset = newVertexElementGroup.data.size();
					newVertexElementGroup.data.resize(destOffset + vertexSize);
					memcpy(&newVertexElementGroup.data[destOffset], &srcVertexPointer[index * vertexSize], vertexSize);

					// increment vertex count
					newVertexElementGroup.count++;
				}

				// copy indics[i-1]
				{
					size_t index = indices[i-1];

					// copy data over
					size_t destOffset = newVertexElementGroup.data.size();
					newVertexElementGroup.data.resize(destOffset + vertexSize);
					memcpy(&newVertexElementGroup.data[destOffset], &srcVertexPointer[index * vertexSize], vertexSize);

					// increment vertex count
					newVertexElementGroup.count++;
				}

				// copy indics[i]
				{
					size_t index = indices[i];

					// copy data over
					size_t destOffset = newVertexElementGroup.data.size();
					newVertexElementGroup.data.resize(destOffset + vertexSize);
					memcpy(&newVertexElementGroup.data[destOffset], &srcVertexPointer[index * vertexSize], vertexSize);

					// increment vertex count
					newVertexElementGroup.count++;
				}
			}
		}
	}

	return ret;
}
