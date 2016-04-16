#include "codehlighter.h"

CodeHlighter::CodeHlighter(QTextDocument *t):QSyntaxHighlighter(t)
{
    sectorTitle.setForeground(Qt::darkBlue);
    sectorTitle.setFontWeight(QFont::Bold);
    TitleStartExpression = QRegExp("\\[");
    TitleEndExpression = QRegExp("\\]");

    hexNum.format.setForeground(QColor(0,127,127));
    hexNum.exp= QRegExp("\\b0x[0-9A-Fa-f]+");
    hr.append(hexNum);

    decNum.format.setForeground(QColor(0,127,127));
    decNum.exp= QRegExp("\\b\\d+");
    hr.append(decNum);

    lineComment.format.setForeground(QColor(0,127,0));
    lineComment.exp=  QRegExp("//[^\n]*");
    hr.append(lineComment);

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
            startIndex = TitleStartExpression.indexIn(text);

        while (startIndex >= 0) {
            int endIndex = TitleEndExpression.indexIn(text, startIndex);
            int commentLength;
            if (endIndex == -1) {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex
                                + TitleEndExpression.matchedLength();
            }
            setFormat(startIndex, commentLength, sectorTitle);
            startIndex = TitleStartExpression.indexIn(text, startIndex + commentLength);
        }


}
