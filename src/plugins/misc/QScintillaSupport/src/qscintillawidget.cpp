//==============================================================================
// QScintillaWidget class
//==============================================================================

#include "commonwidget.h"
#include "filemanager.h"
#include "qscintillawidget.h"

//==============================================================================

#include <QDragEnterEvent>
#include <QMimeData>

//==============================================================================

#include "Qsci/qscilexer.h"

//==============================================================================

namespace OpenCOR {
namespace QScintillaSupport {

//==============================================================================

void QScintillaWidget::constructor(const QString &pContents,
                                   const bool &pReadOnly, QsciLexer *pLexer)
{
    // Remove the frame around our Scintilla editor

    setFrameShape(QFrame::NoFrame);

    // Remove the margin in our Scintilla editor

    setMarginWidth(1, 0);

    // Associate a lexer to our Scintilla editor, should one be available

    if (pLexer) {
        // A lexer was provided, so specify its fonts and associate it with our
        // Scintilla editor

        QFont defaultFont = QFont(Core::DefaultFontFamily, Core::DefaultFontSize);

        pLexer->setDefaultFont(defaultFont);
        pLexer->setFont(defaultFont);

        setLexer(pLexer);

        // Specify the type of tree folding to be used. Some lexers may indeed
        // use that feature, so...

        setFolding(QsciScintilla::BoxedTreeFoldStyle);
    } else {
        // No lexer was provided, so simply specify a default font family and
        // size for our Scintilla editor

        setFont(QFont(Core::DefaultFontFamily, Core::DefaultFontSize));
    }

    // Set the contents of our Scintilla editor and its read-only property

    setText(pContents);
    setReadOnly(pReadOnly);
}

//==============================================================================

QScintillaWidget::QScintillaWidget(const QString &pContents,
                                   const bool &pReadOnly,
                                   QsciLexer *pLexer, QWidget *pParent) :
    QsciScintilla(pParent)
{
    // Construct our object

    constructor(pContents, pReadOnly, pLexer);
}

//==============================================================================

QScintillaWidget::QScintillaWidget(const QString &pContents,
                                   const bool &pReadOnly, QWidget *pParent) :
    QsciScintilla(pParent)
{
    // Construct our object

    constructor(pContents, pReadOnly);
}

//==============================================================================

QScintillaWidget::QScintillaWidget(QsciLexer *pLexer, QWidget *pParent) :
    QsciScintilla(pParent)
{
    // Construct our object

    constructor(QString(), false, pLexer);
}

//==============================================================================

QScintillaWidget::QScintillaWidget(QWidget *pParent) :
    QsciScintilla(pParent)
{
    // Construct our object

    constructor();
}

//==============================================================================

void QScintillaWidget::setContents(const QString &pContents)
{
    // Set our contents

    setText(pContents);
}

//==============================================================================

void QScintillaWidget::dragEnterEvent(QDragEnterEvent *pEvent)
{
    // Accept the proposed action for the event, but only if we are not dropping
    // URIs
    // Note: this is not (currently?) needed on Windows and OS X, but if we
    //       don't have that check on Linux, then to drop some files on our
    //       Scintilla editor will result in the text/plain version of the data
    //       (e.g. file:///home/me/myFile) to be inserted in the text, so...

    if (!pEvent->mimeData()->hasFormat(Core::FileSystemMimeType))
        pEvent->acceptProposedAction();
    else
        pEvent->ignore();
}

//==============================================================================

}   // namespace QScintillaSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
