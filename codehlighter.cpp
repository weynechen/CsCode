#include "codehlighter.h"

CodeHlighter::CodeHlighter(QTextDocument *t):QSyntaxHighlighter(t)
{
    sectorTitle.setForeground(Qt::darkBlue);
    sectorTitle.setFontWeight(QFont::Bold);
    TitleStartExpression = QRegExp("\\[");
    TitleEndExpression = QRegExp("\\]");

    blockComments.setForeground(QColor(0,127,0));
    blockCommentsStartExpression = QRegExp("/\\*");
    blockCommentsEndExpression = QRegExp("\\*/");

    hexNum.format.setForeground(QColor(0,127,127));
    hexNum.exp= QRegExp("\\b0x[0-9A-Fa-f]+");
    hr.append(hexNum);

    decNum.format.setForeground(QColor(0,127,127));
    decNum.exp= QRegExp("\\b\\d+");
    hr.append(decNum);

    lineComment.format.setForeground(QColor(0,127,0));
    lineComment.exp=  QRegExp("//[^\n]*");
    hr.append(lineComment);

    title.format.setForeground(Qt::darkBlue);
    title.format.setFontWeight(QFont::Bold);
    title.exp=  QRegExp("\\[[\\s\\S]*\\]");
    hr.append(title);

    atSymbol.format.setForeground(QColor(127,127,0));
    atSymbol.exp=  QRegExp("@[\\S]+");
    hr.append(atSymbol);

}

void CodeHlighter::highlightBlock(const QString &text)
{

    foreach(const highligherRules &hrs,hr)
    {
        QRegExp expression(hrs.exp);
        int index = expression.indexIn(text); //the first match
        while(index>=0){
            int length = expression.matchedLength(); //the last match
            setFormat(index,length,hrs.format);
            index = expression.indexIn(text,index + length);
        }
    }

    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = blockCommentsStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = blockCommentsEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                    + blockCommentsEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, blockComments);
        startIndex = blockCommentsStartExpression.indexIn(text, startIndex + commentLength);
    }

    QRegExp expression(atSymbol.exp);
    int index = expression.indexIn(text); //the first match
    while(index>=0){
        int length = expression.matchedLength(); //the last match
        setFormat(index,length,atSymbol.format);
        index = expression.indexIn(text,index + length);
    }

}
