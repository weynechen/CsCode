#ifndef MSGEDIT_H
#define MSGEDIT_H

#include <QPlainTextEdit>
#include "msghlighter.h"

class msgEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    msgEdit(QWidget *parent = 0);
protected:
    virtual void keyPressEvent(QKeyEvent *e);
private:
    msgHlighter *msgLighter;
};

#endif // MSGEDIT_H
