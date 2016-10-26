#ifndef COMMANDEDIT_H
#define COMMANDEDIT_H

#include <QPlainTextEdit>
#include "commandhlighter.h"

class CommandEdit : public QPlainTextEdit
{
  Q_OBJECT
public:
  CommandEdit(QWidget *parent = 0);
protected:
  virtual void keyPressEvent(QKeyEvent *e);
   void dropEvent(QDropEvent *e);


private:
  commandHlighter *mCmdLighter;
  QStringList mCommandStr;
  int mCommandCounter;

signals:
  void command(QString str);
};

#endif // COMMANDEDIT_H
