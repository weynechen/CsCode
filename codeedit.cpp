#include "codeedit.h"
#include <QtWidgets>
#include <QPainter>
#include "QPalette"


CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    QFont f("Courier", 12);

    this->setFont(f);
    this->setReadOnly(false);
    //    QPalette p;
    //    p.setColor(QPalette::Base,QColor(199,237,204));
    //    this->setPalette(p);
    this->setLineWrapMode(QPlainTextEdit::NoWrap);
    //添加代码编辑区语法高亮
    codeHlighter = new CodeHlighter(this->document());

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}


void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    QPlainTextEdit::keyPressEvent(e);
}


/*!
 * @brief
 * @note
 * @param
 * @retval : line number;
 */
int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());

    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('A')) * digits;
    //    qDebug()<<space;
    if (space < 33)
    {
        space = 33;
    }
    return space;
}


//![extraAreaWidth]

//![slotUpdateExtraAreaWidth]

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


//![slotUpdateExtraAreaWidth]

//![slotUpdateRequest]

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy)
    {
        lineNumberArea->scroll(0, dy);
    }
    else
    {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth(0);
    }
}


//![slotUpdateRequest]

//![resizeEvent]

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();

    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


void CodeEditor::dropEvent(QDropEvent *e)
{
    Q_UNUSED(e);
    // qDebug()<<e->mimeData()->text();
}


void CodeEditor::dragEnterEvent(QDragEnterEvent *e)
{
    Q_UNUSED(e);
    //    //if(e->mimeData()->hasText())
    //    if(e->mimeData()->hasFormat(""))
    //            e->acceptProposedAction();
}


//![resizeEvent]

//![cursorPositionChanged]

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(0xa0, 0xa0, 0xa4, 50);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}


//![cursorPositionChanged]

//![extraAreaPaintEvent_0]

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);

    painter.fillRect(event->rect(), QColor(0xC0, 0xC0, 0xC0, 127));

    //![extraAreaPaintEvent_0]

    //![extraAreaPaintEvent_1]
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();
    //![extraAreaPaintEvent_1]

    //![extraAreaPaintEvent_2]
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && (bottom >= event->rect().top()))
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(0, 0, 0, 170));
            painter.drawText(0, top + 3, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignHCenter, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}


//![extraAreaPaintEvent_2]
