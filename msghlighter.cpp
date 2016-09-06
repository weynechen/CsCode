#include "msghlighter.h"

msgHlighter::msgHlighter(QTextDocument *t) : QSyntaxHighlighter(t)
{
    errorRules.format.setForeground(QColor(255, 0, 0));
    errorRules.exp = QRegExp("Error[^\n]*");
    hr.append(errorRules);

    comName.format.setForeground(QColor(0, 255, 190));
    comName.exp = QRegExp("COM\\d\\d?");
    hr.append(comName);

    okMsg.format.setForeground(QColor(0, 0, 190));
    okMsg.exp = QRegExp("OK[^\n]*");
    hr.append(okMsg);

    command.setForeground(QColor(0, 180, 0));
    command.setFontWeight(QFont::Bold);
    commandList << "Info[^\n]*";
}


void msgHlighter::highlightBlock(const QString& text)
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
