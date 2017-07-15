#include "QMessageBox"
#include "commandedit.h"
#include "QTextStream"
#include "QDebug"
#include "QTextCursor"
#include <QMimeData>
#include "QUrl"

CommandEdit::CommandEdit(QWidget *parent) : QPlainTextEdit(parent), mCommandCounter(0)
{
    QFont f("Courier", 12);

    this->setFont(f);
    this->setReadOnly(false);
    this->setCursorWidth(1);
    //    QPalette p;
    //    p.setColor(QPalette::Base,QColor(199,237,204));
    //    this->setPalette(p);

    this->setPlainText("-> ");
    QTextCursor textCursor(this->document());
    textCursor.movePosition(QTextCursor::End);
    this->setTextCursor(textCursor);
    mCmdLighter = new commandHlighter(this->document());
}


void CommandEdit::keyPressEvent(QKeyEvent *e)
{
    //ensure that cursor is always at last postion
    QString text = this->toPlainText();
    QTextCursor textCursor(this->document());

    textCursor.movePosition(QTextCursor::End);
    this->setTextCursor(textCursor);
    QTextStream ts(&text);
    QString strLine;

    switch (e->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        //read last line as command
        ts.seek((text.lastIndexOf("-> ")) + 3);
        strLine = ts.readLine();
        if (!strLine.isEmpty())
        {
            if (mCommandStr.isEmpty())
            {
                mCommandStr << strLine;
                mCommandCounter = mCommandStr.size();
            }
            else
            {
                if (strLine != mCommandStr.last())
                {
                    mCommandStr << strLine;
                }
                mCommandCounter = mCommandStr.size();
            }
        }

        emit command(strLine);
        this->appendPlainText("-> ");
        break;

    case Qt::Key_Backspace:
        if (!(this->toPlainText().endsWith("-> ")))
        {
            QPlainTextEdit::keyPressEvent(e);
        }
        break;

    case Qt::Key_Up:
        //   qDebug()<<commandCounter;
        if (mCommandStr.isEmpty() == false)
        {
            if (mCommandCounter > 0)
            {
                //选中一行，再覆盖
                QTextCursor cursor = this->textCursor();
                cursor.select(QTextCursor::LineUnderCursor);
                setTextCursor(cursor);
                insertPlainText("-> ");

                this->insertPlainText(mCommandStr[--mCommandCounter]);
            }
        }
        break;

    case Qt::Key_Down:
        //      qDebug()<<commandCounter;
        if (mCommandStr.isEmpty() == false)
        {
            if (mCommandCounter < mCommandStr.size())
            {
                //选中一行，再覆盖
                QTextCursor cursor = this->textCursor();
                cursor.select(QTextCursor::LineUnderCursor);
                setTextCursor(cursor);
                insertPlainText("-> ");

                this->insertPlainText(mCommandStr[mCommandCounter++]);
            }
        }
        break;


    default:
        QPlainTextEdit::keyPressEvent(e);
        break;
    }
}

void CommandEdit::dropEvent(QDropEvent *e)
{
    QList <QUrl> url;
    QString fileName;
    QString filePath;

    url = e->mimeData()->urls();

    if(url.isEmpty()==true)
        return;

    if(url.size()>1)
    {
        return;
    }

    fileName = url[0].fileName();
    filePath = url[0].path();

    qDebug()<<filePath.remove(0,1);


    QRegExp rx("\\S*\\.(cfw|hex)");
    if(rx.exactMatch(fileName)==false)
    {
        QMessageBox::information(this,"info",QStringLiteral("不支持该种类型文件"));
        return;
    }

    QTextCursor cursor = this->textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    setTextCursor(cursor);
    insertPlainText("-> ");

    this->insertPlainText("upgrade "+filePath);
}

