#ifndef MSGHLIGHTER_H
#define MSGHLIGHTER_H
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QVector>
#include <QStringList>
class msgHlighter :QSyntaxHighlighter
{
    Q_OBJECT

public:
    msgHlighter(QTextDocument *t);
private:

    typedef struct
    {
        QTextCharFormat format;
        QRegExp exp;
    }highligherRules;

    QVector<highligherRules> hr;

    QStringList commandList;
    QTextCharFormat command;

    highligherRules errorRules;
    highligherRules comName;
    highligherRules okMsg;

protected:
    void highlightBlock(const QString &text);
};

#endif // MSGHLIGHTER_H
