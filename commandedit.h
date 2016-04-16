#ifndef COMMANDEDIT_H
#define COMMANDEDIT_H

#include <QPlainTextEdit>
#include "commandhlighter.h"

class commandEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    commandEdit(QWidget *parent = 0);
protected:
    virtual void keyPressEvent(QKeyEvent *e);
private:
    commandHlighter *cmdLighter;
    QStringList commandStr;
    int commandCounter;

signals:
   void command(QString str);

};

#endif // COMMANDEDIT_H
