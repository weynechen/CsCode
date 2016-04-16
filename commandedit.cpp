#include "commandedit.h"
#include "QTextStream"
#include "QDebug"
#include "QTextCursor"


commandEdit::commandEdit(QWidget *parent):QPlainTextEdit(parent),commandCounter(0)
{
    QFont f("Courier",12);
    this->setFont(f);
    this->setReadOnly(false);
    this->setCursorWidth(1);
//    QPalette p;
//    p.setColor(QPalette::Base,QColor(199,237,204));
//    this->setPalette(p);

    this->setPlainText("-> ");
    cmdLighter= new commandHlighter(this->document());
}

void commandEdit::keyPressEvent(QKeyEvent *e)
{
    //ensure that cursor is always at last postion
    QString text=this->toPlainText();
    QTextCursor textCursor(this->document());
    textCursor.movePosition(QTextCursor::End);
    this->setTextCursor(textCursor);
    QTextStream ts(&text);
    QString strLine;

    switch (e->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        //read last line as command
        ts.seek((text.lastIndexOf("-> "))+3);
                strLine=ts.readLine();
        if(!strLine.isEmpty())
        {
             if(commandStr.isEmpty())
             {
                 commandStr<<strLine;
                 commandCounter=commandStr.size();
             }
            else
             {
                 if(strLine!=commandStr.last())
                 {
                     commandStr<<strLine;
                 }
                     commandCounter=commandStr.size();
             }
        }

        emit command(strLine);
        this->appendPlainText("-> ");
        break;

    case Qt::Key_Backspace:
        if(!(this->toPlainText().endsWith("-> ")))
        {
         QPlainTextEdit::keyPressEvent(e);
        }
        break;

    case Qt::Key_Up:
     //   qDebug()<<commandCounter;
        if(commandStr.isEmpty()==false)
          if(commandCounter>0)
          {
            //选中一行，再覆盖
            QTextCursor cursor = this->textCursor();
            cursor.select(QTextCursor::LineUnderCursor);
            setTextCursor(cursor);
            insertPlainText("-> ");

            this->insertPlainText(commandStr[--commandCounter]);
          }
        break;

    case Qt::Key_Down:
          //      qDebug()<<commandCounter;
        if(commandStr.isEmpty()==false)
          if(commandCounter<commandStr.size())
          {
            //选中一行，再覆盖
            QTextCursor cursor = this->textCursor();
            cursor.select(QTextCursor::LineUnderCursor);
            setTextCursor(cursor);
            insertPlainText("-> ");

            this->insertPlainText(commandStr[commandCounter++]);
          }
        break;


    default:
            QPlainTextEdit::keyPressEvent(e);
        break;
    }
}

