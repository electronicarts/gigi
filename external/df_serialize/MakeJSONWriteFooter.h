inline bool WriteTextFile(const char* fileName, const TSTRING& data)
{
    FILE* file = nullptr;
    fopen_s(&file, fileName, "w+b");
    if (!file)
        return false;
    fwrite(&data[0], strlen(&data[0]), 1, file);
    fclose(file);
    return true;
}

// Write a structure to a JSON string
template<typename TROOT>
void WriteToJSONBuffer(TROOT& root, TSTRING& data, bool minimize = false)
{
    // make the document
    rapidjson::Document document;
    rapidjson::Value rootValue = MakeJSONValue(root, document.GetAllocator());

    // make the string
    rapidjson::StringBuffer buffer;
    if (minimize)
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        rootValue.Accept(writer);
    }
    else
    {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        rootValue.Accept(writer);
    }
    data = buffer.GetString();
}

// Write a structure to a JSON file
template<typename TROOT>
bool WriteToJSONFile(TROOT& root, const char* fileName, bool minimize = false)
{
    TSTRING data;
    WriteToJSONBuffer(root, data, minimize);
    if (!WriteTextFile(fileName, data))
    {
        DFS_LOG("Could not write file %s", fileName);
        return false;
    }

    return true;
}
