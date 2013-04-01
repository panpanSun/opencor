//==============================================================================
// File browser widget
//==============================================================================

#include "filebrowserwidget.h"

//==============================================================================

#include <QApplication>
#include <QHeaderView>
#include <QHelpEvent>
#include <QSettings>

//==============================================================================

namespace OpenCOR {
namespace FileBrowser {

//==============================================================================

static const QString HomeFolder = QDir::homePath();

//==============================================================================

FileBrowserWidget::FileBrowserWidget(QWidget *pParent) :
    TreeViewWidget(pParent),
    mNeedDefColWidth(true),
    mInitPathDirs(QStringList()),
    mInitPathDir(QString()),
    mInitPath(QString()),
    mPreviousItems(QStringList()),
    mNextItems(QStringList())
{
    // Create an instance of the file system model that we want to view

    mModel = new FileBrowserModel(this);

    // Set some properties for the file browser widget itself

    setDragDropMode(QAbstractItemView::DragOnly);
    setFrameShape(QFrame::StyledPanel);
    setModel(mModel);
    setSortingEnabled(true);

    header()->setSectionsMovable(false);

    // Some connections

    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(itemChanged(const QModelIndex &, const QModelIndex &)));

    connect(mModel, SIGNAL(directoryLoaded(const QString &)),
            this, SLOT(directoryLoaded(const QString &)));
}

//==============================================================================

static const QString SettingsColumnWidth = "ColumnWidth%1";
static const QString SettingsInitialPath = "InitialPath";
static const QString SettingsSortColumn  = "SortColumn";
static const QString SettingsSortOrder   = "SortOrder";

//==============================================================================

void FileBrowserWidget::loadSettings(QSettings *pSettings)
{
    // We are about to begin loading the settings, so we don't want to keep
    // track of the change of item

    disconnect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
               this, SLOT(itemChanged(const QModelIndex &, const QModelIndex &)));

    // Retrieve the width of each column

    QString columnWidthKey;

    for (int i = 0, iMax = header()->count(); i < iMax; ++i) {
        columnWidthKey = SettingsColumnWidth.arg(i);

        mNeedDefColWidth =     mNeedDefColWidth
                           && !pSettings->contains(columnWidthKey);

        setColumnWidth(i, pSettings->value(columnWidthKey,
                                           columnWidth(i)).toInt());
    }

    // Retrieve the sorting information

    sortByColumn(pSettings->value(SettingsSortColumn, 0).toInt(),
                 Qt::SortOrder(pSettings->value(SettingsSortOrder,
                                                Qt::AscendingOrder).toInt()));

    // Retrieve the initial path

    mInitPath = pSettings->value(SettingsInitialPath,
                                 QDir::homePath()).toString();

    QFileInfo initPathFileInfo = mInitPath;

    if (!initPathFileInfo.exists()) {
        // The initial path doesn't exist, so just revert to the home path

        mInitPathDir = QDir::homePath();
        mInitPath    = "";
    } else {
        // The initial path exists, so retrieve some information about the
        // folder and/or file (depending on whether the initial path refers to a
        // file)
        // Note: indeed, should mInitPath refer to a file, then to directly set
        //       the current index of the tree view widget to that of a file
        //       won't give us the expected behaviour (i.e. the parent folder
        //       being open and expanded, and the file selected), so instead
        //       one must set the current index to that of the parent folder
        //       and then select the file

        if (initPathFileInfo.isDir()) {
            // We are dealing with a folder, so...

            mInitPathDir = initPathFileInfo.canonicalFilePath();
            mInitPath    = "";
        } else {
            // We are dealing with a file, so...

            mInitPathDir = initPathFileInfo.canonicalPath();
            mInitPath    = initPathFileInfo.canonicalFilePath();
        }
    }

    // On Windows, if mInitPathDir refers to the root of a particular drive
    // (e.g. the C: drive), then it won't include a trailing separator (i.e.
    // "C:" instead of "C:/") and this causes problems below (when wanting to
    // retrieve the different folders), so we must make sure that that
    // mInitPathDir contains a trailing separator
    // Note: this is clearly not needed on Linux and OS X, but it doesn't harm
    //       doing it for these platforms too, so...

    mInitPathDir = QDir(mInitPathDir+QDir::separator()).canonicalPath();

    // Create a list of the different folders that need to be loaded to consider
    // that the loading of the settings is finished (see just below and the
    // directoryLoaded slot)

    mInitPathDirs << mInitPathDir;

    QDir initPathDir = mInitPathDir;

    while (initPathDir.cdUp())
        mInitPathDirs << initPathDir.canonicalPath();

    // Set the current index to that of the folder (and file, if it exists) we
    // are interested in
    // Note: this will result in the directoryLoaded signal being emitted and,
    //       us, to take advantage of it to scroll to the right directory/file

    setCurrentIndex(mModel->index(mInitPathDir));

    if (!mInitPath.isEmpty())
        // The initial path is that of a file, so...

        setCurrentIndex(mModel->index(mInitPath));

    // Make sure that the current path is expanded
    // Note: this is important in case the current path is that of the C: drive
    //       or the root of the file system, in which case this won't result in
    //       the directoryLoaded signal being emitted

    if (!isExpanded(currentIndex()))
        setExpanded(currentIndex(), true);

    // Let the user know of a few default things about ourselves by emitting a
    // few signals

    emitItemChangedRelatedSignals();
}

//==============================================================================

void FileBrowserWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of the width of each column

    for (int i = 0, iMax = header()->count(); i < iMax; ++i)
        pSettings->setValue(SettingsColumnWidth.arg(i), columnWidth(i));

    // Keep track of the sorting information

    pSettings->setValue(SettingsSortColumn,
                        header()->sortIndicatorSection());
    pSettings->setValue(SettingsSortOrder, header()->sortIndicatorOrder());

    // Keep track of what will be our future initial folder/file path

    pSettings->setValue(SettingsInitialPath,
                        mModel->filePath(currentIndex()));
}

//==============================================================================

QStringList FileBrowserWidget::selectedFiles() const
{
    // Retrieve all the files that are currently selected
    // Note: if there is a non-file among the selected items, then we return an
    //       empty list

    QStringList res;
    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();

    for (int i = 0, iMax = selectedIndexes.count(); i < iMax; ++i) {
        QString fileName = pathOf(selectedIndexes[i]);
        QFileInfo fileInfo = fileName;

        if (fileInfo.isFile()) {
            if (fileInfo.isSymLink()) {
                // The current item is a symbolic link, so retrieve its target
                // and check that it exists, and if it does then add it to the
                // list

                fileName = fileInfo.symLinkTarget();

                if (QFileInfo(fileName).exists())
                    res << fileName;
            } else {
                // The current item is a file, so just add to the list

                res << fileName;
            }
        }
    }

    // Remove duplicates (which might be present as a result of symbolic links)

    res.removeDuplicates();

    return res;
}

//==============================================================================

void FileBrowserWidget::deselectFolders() const
{
    // Check whether more than one item is selected with the view of dragging
    // them and, if so, unselect any folder item since we don't allow them to be
    // dragged

    if (selectionModel()->selectedRows().count() > 1) {
        QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();

        for (int i = 0, iMax = selectedIndexes.count(); i < iMax; ++i) {
            QFileInfo fileInfo = pathOf(selectedIndexes[i]);

            if (fileInfo.isDir() || !fileInfo.exists())
                // Either we are dealing with a directory or an entry that
                // doesn't actually exist (e.g. on Windows, it could be a drive
                // for a removable device with the device not being present)

                selectionModel()->select(selectedIndexes[i],
                                         QItemSelectionModel::Deselect);
        }
    }
}

//==============================================================================

void FileBrowserWidget::emitItemChangedRelatedSignals()
{
    // Let the user know whether the path of the new item is not that of our
    // home folder, as well as whether we could go to the parent item

    emit notHomeFolder(currentPath() != HomeFolder);
    emit goToParentFolderEnabled(!currentPathParent().isEmpty());

    // Let the user know whether we can go to the previous/next file/folder

    emit goToPreviousFileOrFolderEnabled(mPreviousItems.count());
    emit goToNextFileOrFolderEnabled(mNextItems.count());
}

//==============================================================================

void FileBrowserWidget::updateItems(const QString &pItemPath,
                                    QStringList &pItems) const
{
    // Remove any instance of pItemPath in pItems

    pItems.removeAll(pItemPath);

    // Because of the above, we may have two or more consective identital items
    // in the list, so we must reduce that to one

    if (pItems.count() > 1) {
        QStringList newItems;
        QString prevItem = pItems.first();

        newItems << prevItem;

        for (int i = 1, iMax = pItems.count(); i < iMax; ++i) {
            QString crtItem = pItems[i];

            if (crtItem != prevItem) {
                // The current and previous items are different, so we want to
                // keep track of it and add it to our new list

                newItems << crtItem;

                prevItem = crtItem;
            }
        }

        // Update the old list of items with our new one

        pItems = newItems;
    }
}

//==============================================================================

void FileBrowserWidget::goToOtherItem(QStringList &pItems,
                                      QStringList &pOtherItems)
{
    if (pItems.isEmpty())
        return;

    // Go to the previous/next item and move the last item from our list of
    // items to our list of other items

    // First, we must stop keeping track of the change of item otherwise it's
    // going to mess things up

    disconnect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
               this, SLOT(itemChanged(const QModelIndex &, const QModelIndex &)));

    // Retrieve our current path and add it to our list of other items

    QString crtPath = currentPath();

    pOtherItems << crtPath;

    // Retrieve the new path and check whether it still exists

    QString newItemPath = pItems.last();

    while (   !pItems.isEmpty()
           && !QFileInfo(newItemPath).exists()) {
        // The new item doesn't exist anymore, so remove it from our list of
        // items and other items

        updateItems(newItemPath, pItems);
        updateItems(newItemPath, pOtherItems);

        // Try with the next new item

        if (!pItems.isEmpty()) {
            // The list is not empty, so make the last item our next new item

            newItemPath = pItems.last();

            // The next new item cannot, however, be the same as the current
            // path

            while (!pItems.isEmpty() && (newItemPath == crtPath)) {
                pItems.removeLast();

                if (!pItems.isEmpty())
                    newItemPath = pItems.last();
                else
                    newItemPath = "";
            }
        } else {
            // The list is empty, so...

            newItemPath = "";
        }
    }

    if (!newItemPath.isEmpty()) {
        // We have a valid new item, so go to its path and remove it from our
        // list of items

        goToPath(newItemPath);

        pItems.removeLast();
    }

    // Make sure that the last item in the lists of items and other items isn't
    // that of the current path (this may happen if some items got deleted by
    // the user)

    crtPath = currentPath();

    if (!pItems.isEmpty() && (pItems.last() == crtPath))
        pItems.removeLast();

    if (!pOtherItems.isEmpty() && (pOtherItems.last() == crtPath))
        pOtherItems.removeLast();

    // Now that we are done, we can once again keep track of the change of item

    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(itemChanged(const QModelIndex &, const QModelIndex &)));

    // Let the user know about a few item changed related things

    emitItemChangedRelatedSignals();
}

//==============================================================================

void FileBrowserWidget::keyPressEvent(QKeyEvent *pEvent)
{
    // Default handling of the event

    TreeViewWidget::keyPressEvent(pEvent);

    // Deselect folders, if required

    deselectFolders();

    // Let people know about a key having been pressed with the view of opening
    // one or several files

    QStringList crtSelectedFiles = selectedFiles();

    if (   crtSelectedFiles.count()
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
       && (   (pEvent->key() == Qt::Key_Enter)
           || (pEvent->key() == Qt::Key_Return))
#elif defined(Q_OS_MAC)
        && (   (pEvent->modifiers() & Qt::ControlModifier)
            && (pEvent->key() == Qt::Key_Down))
#else
    #error Unsupported platform
#endif
       )
        // There are some files that are selected and we want to open them, so
        // let people know about it

        emit filesOpened(crtSelectedFiles);
}

//==============================================================================

void FileBrowserWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
    // Default handling of the event

    TreeViewWidget::mouseMoveEvent(pEvent);

    // Deselect folders, if required

    deselectFolders();
}

//==============================================================================

void FileBrowserWidget::mousePressEvent(QMouseEvent *pEvent)
{
    // Default handling of the event

    TreeViewWidget::mousePressEvent(pEvent);

    // Deselect folders, if required

    deselectFolders();
}

//==============================================================================

bool FileBrowserWidget::viewportEvent(QEvent *pEvent)
{
    if (pEvent->type() == QEvent::ToolTip) {
        // We need to show a tool tip, so make sure that it's up to date by
        // setting it to the path of the current item

        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(pEvent);

        setToolTip(QDir::toNativeSeparators(mModel->filePath(indexAt(helpEvent->pos()))));
    }

    // Default handling of the event

    return TreeViewWidget::viewportEvent(pEvent);
}

//==============================================================================

void FileBrowserWidget::itemChanged(const QModelIndex &,
                                    const QModelIndex &pPrevItem)
{
    // A new item has been selected, so we need to keep track of the old one in
    // case we want to go back to it

    mPreviousItems << pathOf(pPrevItem);

    // Reset the list of next items since that list doesn't make sense anymore

    mNextItems.clear();

    // Let the user know about a few item changed related things

    emitItemChangedRelatedSignals();
}

//==============================================================================

void FileBrowserWidget::directoryLoaded(const QString &pPath)
{
    static bool needInitializing = true;

    if (needInitializing
        && (   ( mInitPath.isEmpty() && mInitPathDir.contains(pPath))
            || (!mInitPath.isEmpty() && mInitPath.contains(pPath)))) {
        // mModel is still loading the initial path, so we try to expand it and
        // scroll to it, but first we process any pending event (indeed, though
        // Windows doesn't need this, Linux and OS X definitely do and it can't
        // harm having it for all three environments, so...)

        qApp->processEvents();

        QModelIndex initPathDirIndex = mModel->index(mInitPathDir);

        setExpanded(initPathDirIndex, true);
        scrollTo(initPathDirIndex);
        setCurrentIndex(initPathDirIndex);

        if (!mInitPath.isEmpty()) {
            // The initial path is that of a file and it exists, so select it

            QModelIndex initPathIndex = mModel->index(mInitPath);

            scrollTo(initPathIndex);
            setCurrentIndex(initPathIndex);
        }

        // Set the default width of the columns, if needed, so that it fits
        // their contents

        if (mNeedDefColWidth)
            resizeColumnsToContents();

        // Remove the loaded directory from mInitPathDirs

        mInitPathDirs.removeOne(QDir(pPath+QDir::separator()).canonicalPath());
        // Note: it is very important, on Windows, to add QDir::separator() to
        //       pPath. Indeed, say that mInitPathDir is on the C: drive, then
        //       eventually pPath will be equal to "C:" while mInitPathDirs will
        //       know about "C:/", so... (note: this is clearly not needed on
        //       Linux and OS X, but it doesn't harm adding it for these
        //       platforms too, so...)

        // Check whether we are done initializing

        if (mInitPathDirs.isEmpty()) {
            // We are now done loading the settings for the file browser widget,
            // so we can now keep track of the change of item

            connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
                    this, SLOT(itemChanged(const QModelIndex &, const QModelIndex &)));

            // We are done initalising, so...

            needInitializing = false;
        }
    }
}

//==============================================================================

void FileBrowserWidget::goToPath(const QString &pPath, const bool &pExpand)
{
    // Set the current index to that of the provided path

    QModelIndex pathIndex = mModel->index(pPath);

    if ((pathIndex.isValid()) && (pathIndex != currentIndex())) {
        // The path exists, so we can go to it

        if (pExpand)
            setExpanded(pathIndex, true);

        setCurrentIndex(pathIndex);
    }
}

//==============================================================================

void FileBrowserWidget::goToHomeFolder()
{
    // Go to the home folder

    goToPath(HomeFolder, true);
}

//==============================================================================

void FileBrowserWidget::goToParentFolder()
{
    // Go to the parent folder

    goToPath(currentPathParent());
}

//==============================================================================

void FileBrowserWidget::goToPreviousFileOrFolder()
{
    // Go to the previous file/folder

    goToOtherItem(mPreviousItems, mNextItems);
}

//==============================================================================

void FileBrowserWidget::goToNextFileOrFolder()
{
    // Go to the next file/folder

    goToOtherItem(mNextItems, mPreviousItems);
}

//==============================================================================

QString FileBrowserWidget::currentPath() const
{
    // Return the current path

    return mModel->filePath(currentIndex());
}

//==============================================================================

QString FileBrowserWidget::currentPathParent() const
{
    // Return the current path parent, if any

    QModelIndex crtIndexParent = currentIndex().parent();

    return crtIndexParent.isValid()?mModel->filePath(crtIndexParent):QString();
}

//==============================================================================

QString FileBrowserWidget::pathOf(const QModelIndex &pIndex) const
{
    // Return the file path of pIndex, if it exists

    return pIndex.isValid()?mModel->filePath(pIndex):QString();
}

//==============================================================================

}   // namespace FileBrowser
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
