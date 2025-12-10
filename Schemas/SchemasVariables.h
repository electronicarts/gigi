///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

STRUCT_BEGIN(VariableReference, "A reference to a variable")
    STRUCT_FIELD(std::string, name, "", "The name of the variable.", 0)

    STRUCT_FIELD(int, variableIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(DataFieldType, UIType, DataFieldType::Count, "If not count, will limit to variables of the specific type.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(VariableReferenceNoConst, "A reference to a variable. No const variables allowed.")
    STRUCT_FIELD(std::string, name, "", "The name of the variable.", 0)

    STRUCT_FIELD(int, variableIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(VariableReferenceConstOnly, "A reference to a variable. Only const variables allowed.")
    STRUCT_FIELD(std::string, name, "", "The name of the variable.", 0)

    STRUCT_FIELD(int, variableIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

ENUM_BEGIN(VariableVisibility, "The visibility of the variable")
    ENUM_ITEM(Internal, "Internal to the technique.")
    ENUM_ITEM(Host, "The host app has access.")
    ENUM_ITEM(User, "The host app has access and it should also be exposed to the user through UI and Script, where possible.")
    ENUM_ITEM(Count, "")
ENUM_END()

ENUM_BEGIN(VariableUIHint, "Hints about how the UI for a variable should look or act, if it is made into UI")
    ENUM_ITEM(Button, "There should be a button that when pressed makes the value true for a single frame, else is false.")
    ENUM_ITEM(Color, "This represents a color, so a color selector would be appropriate.")
    ENUM_ITEM(Drag, "Use the DragX functions in imgui.")
    ENUM_ITEM(Slider, "Use the SliderX functions in imgui.")
    ENUM_ITEM(Angle, "This value is an angle.")
    ENUM_ITEM(Count, "")
ENUM_END()

STRUCT_BEGIN(VariableUISettings, "UI information for variables")
    STRUCT_FIELD(VariableUIHint, UIHint, VariableUIHint::Count, "Any hints for UI", 0)
    STRUCT_FIELD(std::string, min, "", "The minimum value of the variable. Leave blank for no minimum.", 0)
    STRUCT_FIELD(std::string, max, "", "The maximum value of the variable. Leave blank for no maximum.", 0)
    STRUCT_FIELD(std::string, step, "", "The step size of the variable. Leave blank for default step size.", 0)
STRUCT_END()

STRUCT_BEGIN(Variable, "A variable definition")
    STRUCT_FIELD(std::string, name, "", "The name of the variable.", 0)
    STRUCT_FIELD(std::string, comment, "", "A comment for the variable.", 0)
    STRUCT_FIELD(DataFieldType, type, DataFieldType::Count, "The type of the variable", 0)
    STRUCT_FIELD(bool, Const, false, "If true, the variable is declared const and cannot change at runtime", 0)
    STRUCT_FIELD(bool, Static, false, "If true, the variable has the same value for all instances of the technique", 0)
    STRUCT_FIELD(std::string, dflt, "", "The default value of the variable. The default memory is zero initialized before this is parsed, so if you don't give it enough initializers, it will use zero for the unlisted fields.", 0)
    STRUCT_FIELD(VariableVisibility, visibility, VariableVisibility::Internal, "Who can see and interact with this variable", 0)
    STRUCT_FIELD(std::string, Enum, "", "Integer types can specify an enum, which will then make symbols in both C++ and shader code.", 0)
    STRUCT_FIELD(VariableReference, onUserChange, {}, "A boolean variable that gets set to true when the user changes this variable.", 0)
    STRUCT_FIELD(BackendRestriction, backends, {}, "This variable can be limited to specific backends", SCHEMA_FLAG_UI_COLLAPSABLE)
    STRUCT_FIELD(bool, transient, false, "If true, the variable should not be saved between runs of this technique. The Gigi viewer uses this to decide if it should save it in the gguser file or not, for example.", 0)
    STRUCT_FIELD(VariableUISettings, UISettings, {}, "UI Settings.", 0)
    STRUCT_FIELD(std::string, UIGroup, "", "Used to organize variables into folders in the viewer. separate folders with dots.  For instance: settings.controls", 0)

    STRUCT_FIELD(int, enumIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(std::string, originalName, "", "The name before renames and sanitization", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, scope, "", "The scope that the node lives in. A possibly nested list of subgraph node names, seperated by a dot.", SCHEMA_FLAG_NO_SERIALIZE)

	STRUCT_FIELD(bool, system, false, "Is set if the runtime overrides the value", SCHEMA_FLAG_NO_SERIALIZE | SCHEMA_FLAG_NO_UI)

    // deprecated in 0.94b
    // replaced by UISettings.UIHint
    STRUCT_FIELD(VariableUIHint, UIHint, VariableUIHint::Count, "Any hints for UI", SCHEMA_FLAG_NO_UI)
STRUCT_END()

ENUM_BEGIN(ConditionComparison, "The comparison to make between the two condition values")
    ENUM_ITEM(IsFalse, "Checks if value1 is false")
    ENUM_ITEM(IsTrue, "Checks if value1 is true")
    ENUM_ITEM(Equals, "Checks if the two values are the same")
    ENUM_ITEM(NotEquals, "Checks if the two values are notthe same")
    ENUM_ITEM(LT, "value1 < value2")
    ENUM_ITEM(LTE, "value1 <= value2")
    ENUM_ITEM(GT, "value1 > value2")
    ENUM_ITEM(GTE, "value1 >= value2")
    ENUM_ITEM(Count, "")
ENUM_END()

STRUCT_BEGIN(Condition, "Specifiy a condition to make something conditional")
    STRUCT_FIELD(std::string, variable1, {}, "Value 1", 0)
    STRUCT_FIELD(ConditionComparison, comparison, ConditionComparison::Count, "The comparison operator", 0)
    STRUCT_FIELD(std::string, value2, {}, "Value 2", 0)
    STRUCT_FIELD(std::string, variable2, {}, "Value 2", 0)
    STRUCT_FIELD(int, variable1Index, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(int, variable2Index, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(bool, alwaysFalse, false, "If checked, condition always evaluates to false", 0)
    STRUCT_FIELD(bool, hideUI, false, "Used by editor to hide this UI when it isn't supported", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(EnumItem, "Specifiy an enum")
    STRUCT_FIELD(std::string, label, "", "The text label of an enum", 0)
    STRUCT_FIELD(std::string, displayLabel, "", "name before sanitization", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, comment, "", "", 0)
STRUCT_END()

STRUCT_BEGIN(Enum, "Specifiy an enum")
    STRUCT_FIELD(std::string, name, "", "The name of the enum", 0)
    STRUCT_DYNAMIC_ARRAY(EnumItem, items, "The items in the enum. Values start at 0 and count up from there.", SCHEMA_FLAG_UI_ARRAY_FATITEMS)
    STRUCT_FIELD(std::string, comment, "", "", 0)

    STRUCT_FIELD(std::string, originalName, "", "The name before renames and sanitization", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, scope, "", "The scope that the node lives in. A possibly nested list of subgraph node names, seperated by a dot.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

//========================================================
// ValueOrVariable Structs
//========================================================

STRUCT_BEGIN(ValueOrVariable_Bool, "ValueOrVariable<bool>")
    STRUCT_FIELD(bool, value, false, "The value to use if no variable given", SCHEMA_FLAG_SERIALIZE_DFLT)
    STRUCT_FIELD(VariableReference, variable, {}, "The variable to use", 0)
STRUCT_END()

STRUCT_BEGIN(ValueOrVariable_Float, "ValueOrVariable<float>")
    STRUCT_FIELD(float, value, 0.0f, "The value to use if no variable given", SCHEMA_FLAG_SERIALIZE_DFLT)
    STRUCT_FIELD(VariableReference, variable, {}, "The variable to use", 0)
STRUCT_END()

STRUCT_BEGIN(ValueOrVariable_Float2, "ValueOrVariable<float2>")
    STRUCT_STATIC_ARRAY(float, value, 2, { 0 COMMA 0 }, "The value to use if no variable given", SCHEMA_FLAG_SERIALIZE_DFLT | SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_FIELD(VariableReference, variable, {}, "The variable to use", 0)
STRUCT_END()

STRUCT_BEGIN(ValueOrVariable_Float3, "ValueOrVariable<float3>")
    STRUCT_STATIC_ARRAY(float, value, 3, { 0 COMMA 0 COMMA 0 }, "The value to use if no variable given", SCHEMA_FLAG_SERIALIZE_DFLT | SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_FIELD(VariableReference, variable, {}, "The variable to use", 0)
STRUCT_END()

STRUCT_BEGIN(ValueOrVariable_Int4, "ValueOrVariable<int4>")
    STRUCT_STATIC_ARRAY(int, value, 4, { 0 COMMA 0 COMMA 0 COMMA 0 }, "The value to use if no variable given", SCHEMA_FLAG_SERIALIZE_DFLT | SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_FIELD(VariableReference, variable, {}, "The variable to use", 0)
STRUCT_END()
