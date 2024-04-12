#pragma once
#include <list>
#include <string>
using namespace std;

void SplitString(string sz_line, char chSplit, list<string>& outStringList);
void PullOutStringUntilDelimiter(string& szLine, char cDelimiter, string& szFirst);


