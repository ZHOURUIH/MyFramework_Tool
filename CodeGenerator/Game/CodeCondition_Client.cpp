#include "CodeCondition_Client.h"

void CodeCondition_Client::generateCSharpConditionRegister(const myVector<string>& conditionList, const string& filePath)
{
	string content;
	line(content, "// auto generate start");
	line(content, "using static GBR;");
	line(content, "");
	line(content, "public class ConditionRegister");
	line(content, "{");
	line(content, "\tpublic static void registeAll()");
	line(content, "\t{");
	FOR_VECTOR(conditionList)
	{
		line(content, "\t\tregiste<Condition" + conditionList[i] + ", Condition" + conditionList[i] + "Param>(EDCondition." + conditionList[i] + "_ID);");
	}
	line(content, "\t}");
	line(content, "\t//------------------------------------------------------------------------------------------------------------------------------");
	line(content, "\tprotected static void registe<T, Param>(int type) where T : Condition where Param : ConditionParam");
	line(content, "\t{");
	line(content, "\t\tmConditionManager.registe(typeof(T), typeof(Param), type);");
	line(content, "\t}");
	line(content, "};");
	line(content, "// auto generate end", false);
	writeFile(filePath + "ConditionRegister.cs", content);
}