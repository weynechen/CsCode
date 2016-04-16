#include "msgedit.h"


msgEdit::msgEdit(QWidget *parent):QPlainTextEdit(parent)
{
    QFont f("Courier",10);
    this->setFont(f);
//    QPalette p;
//    p.setColor(QPalette::Base,QColor(199+10,237+10,204+10));
//    this->setPalette(p);

    this->setReadOnly(true);
    msgLighter = new msgHlighter(this->document());
}

void msgEdit::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        break;

    default:
            //QPlainTextEdit::keyPressEvent(e);
        break;
    }
}
