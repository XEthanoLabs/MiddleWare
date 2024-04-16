#include "MiscFuncs.h"

void SplitString(string sz_line, char chSplit, list<string>& outStringList)
{
    int len = (int)sz_line.length();
    string szSub;
    bool bInQuotes = false;
    for (int i = 0; i < len; i++)
    {
        char ch = sz_line[i];
        if (ch == '\"')
        {
            if (!bInQuotes)
            {
                bInQuotes = true;
            }
            else
            {
                bInQuotes = false;
            }
        }

        if (ch == '\n' || ch == '\r')
        {
            // ignore it
            continue;
        }

        if (ch == chSplit)
        {
            if (bInQuotes)
            {
                szSub += ch;
            }
            else
            {
                // save off our sub string. what if it starts with quotes?
                if (szSub.empty())
                {
                    szSub = "NA";
                }
                outStringList.push_back(std::move(szSub));
                szSub.clear();
            }
        }
        else
        {
            szSub += ch;
        }
    }

    if (!szSub.empty())
    {
        outStringList.push_back(std::move(szSub));
    }
}

void PullOutStringUntilDelimiter(string& szLine, char cDelimiter, string& szFirst)
{
    for (int i = 0; i < szLine.size(); i++)
    {
        if (szLine[i] == cDelimiter)
        {
            szFirst = szLine.substr(0, i);
            szLine = szLine.substr(i + 1);
            return;
        }
    }
    
    szFirst = string();
}

