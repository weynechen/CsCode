#ifndef CODEEDIT_H
#define CODEEDIT_H
#include <QPlainTextEdit>
#include "codehlighter.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class LineNumberArea;


class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = 0);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent * e);
    void dragEnterEvent(QDragEnterEvent * e);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
private:
    QWidget *lineNumberArea;
    CodeHlighter *codeHlighter;
};



class LineNumberArea : public QWidget
{
    Q_OBJECT
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const Q_DECL_OVERRIDE {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};



#endif // CODEEDIT_H
