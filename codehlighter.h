#ifndef TEXTHLIGHTER_H
#define TEXTHLIGHTER_H
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QVector>

class CodeHlighter :QSyntaxHighlighter
{
    Q_OBJECT

public:
    CodeHlighter(QTextDocument *t);
private:

    typedef struct
    {
        QTextCharFormat format;
        QRegExp exp;
    }highligherRules;

    QVector<highligherRules> hr;

    QTextCharFormat sectorTitle;
    QRegExp TitleStartExpression;
    QRegExp TitleEndExpression;

    highligherRules hexNum;
    highligherRules decNum;
    highligherRules lineComment;

protected:
    void highlightBlock(const QString &text);
};

#endif // TEXTHLIGHTER_H
