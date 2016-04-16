#ifndef CMDHLIGHTER_H
#define CMDHLIGHTER_H
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QVector>
#include <QStringList>
class commandHlighter :QSyntaxHighlighter
{
    Q_OBJECT

public:
    commandHlighter(QTextDocument *t);
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

protected:
    void highlightBlock(const QString &text);
};

#endif // CMDHLIGHTER_H
