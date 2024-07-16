    fprintf(file,
        "<html>\n"
        "<style>\n"
        "table, th, td, tr\n"
        "{\n"
        "    border: 1px solid black;\n"
        "    border-collapse: collapse;\n"
        "}\n"
        "</style>\n"
        "\n"
        "<h1>Enums</h1>\n"
        "%s"
        "<h1>Structs</h1>\n"
        "Italicized fields are not serialized, and are just used for runtime storage.<br/>\n"
        "<br/>\n"
        "%s\n"
        "<h1>Variants</h1>\n"
        "%s\n"
        "</html>\n",
        &enums[0],
        &structs[0],
        &variants[0]
    );
    fclose(file);
    return true;
}
