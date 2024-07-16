inline bool LoadBinaryFile(const char* fileName, TDYNAMICARRAY<char>& data)
{
    // open the file if we can
    FILE* file = nullptr;
    fopen_s(&file, fileName, "rb");
    if (!file)
        return false;

    // get the file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read the file into memory and return success.
    TDYNAMICARRAY_RESIZE(data,size);
    fread(&data[0], 1, size, file);
    fclose(file);
    return true;
}

// Read a structure from a binary string
template<typename TROOT>
bool ReadFromBinaryBuffer(TROOT& root, TDYNAMICARRAY<char>& data)
{
    size_t offset = 0;
    return BinaryRead(root, data, offset);
}

// Read a structure from a binary file
template<typename TROOT>
bool ReadFromBinaryFile(TROOT& root, const char* fileName)
{
    TDYNAMICARRAY<char> fileData;
    if (!LoadBinaryFile(fileName, fileData))
    {
        DFS_LOG("Could not read file %s", fileName);
        return false;
    }

    return ReadFromBinaryBuffer(root, fileData);
}
