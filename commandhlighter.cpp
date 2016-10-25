#include "commandhlighter.h"

commandHlighter::commandHlighter(QTextDocument *t) : QSyntaxHighlighter(t)
{
    errorRules.format.setForeground(QColor(255, 0, 0));
    errorRules.exp = QRegExp("Error[^\n]*");
    hr.append(errorRules);

    comName.format.setForeground(QColor(0, 255, 190));
    comName.exp = QRegExp("COM\\d\\d?");
    hr.append(comName);

    command.setForeground(QColor(0, 0, 180));
    command.setFontWeight(QFont::Bold);
    commandList << "search com" << "\\bconnect\\b" << "disconnect" << "\\bexit\\b" << "\\bclear\\b" << "\\bhelp\\b"
                << "read" << "send" << "pattern\\s*=\\s*\\d+\\s*"<<"\\bupgrade\\b";
}


void commandHlighter::highlightBlock(const QString& text)
{
    foreach(const highligherRules &hrs, hr)
    {
        QRegExp expression(hrs.exp);
        int index = expression.indexIn(text);     //the first match

        while (index >= 0)
        {
            int length = expression.matchedLength();       //the last match
            setFormat(index, length, hrs.format);
            index = expression.indexIn(text, index + length);
        }
    }

    foreach(const QString &str, commandList)
    {
        QRegExp expression(str);
        int index = expression.indexIn(text);

        while (index >= 0)
        {
            int length = expression.matchedLength();       //the last match
            setFormat(index, length, command);
            index = expression.indexIn(text, index + length);
        }
    }
}
