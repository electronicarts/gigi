bool WriteBinaryFile(const char* fileName, TDYNAMICARRAY<char>& data)
{
    FILE* file = nullptr;
    fopen_s(&file, fileName, "w+b");
    if (!file)
        return false;
    fwrite(&data[0], TDYNAMICARRAY_SIZE(data), 1, file);
    fclose(file);
    return true;
}

template<typename TROOT>
void WriteToBinaryBuffer(TROOT& root, TDYNAMICARRAY<char>& output)
{
    BinaryWrite(root, output);
}

// Write a structure to a binary file
template<typename TROOT>
bool WriteToBinaryFile(TROOT& root, const char* fileName)
{
    TDYNAMICARRAY<char> out;
    WriteToBinaryBuffer(root, out);

    if (!WriteBinaryFile(fileName, out))
    {
        DFS_LOG("Could not write file %s", fileName);
        return false;
    }

    return true;
}
