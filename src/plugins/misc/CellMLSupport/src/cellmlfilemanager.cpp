//==============================================================================
// CellML file manager
//==============================================================================

#include "cellmlfilemanager.h"
#include "cellmlsupportplugin.h"
#include "coreutils.h"
#include "filemanager.h"

//==============================================================================

#include <QDir>

//==============================================================================

namespace OpenCOR {
namespace CellMLSupport {

//==============================================================================

CellmlFileManager::CellmlFileManager() :
    mCellmlFiles(CellmlFiles())
{
    // Create some connections to keep track of some events related to our
    // 'global' file manager

    Core::FileManager *fileManagerInstance = Core::FileManager::instance();

    connect(fileManagerInstance, SIGNAL(fileManaged(const QString &)),
            this, SLOT(manageFile(const QString &)));
    connect(fileManagerInstance, SIGNAL(fileUnmanaged(const QString &)),
            this, SLOT(unmanageFile(const QString &)));

    connect(fileManagerInstance, SIGNAL(fileRenamed(const QString &, const QString &)),
            this, SLOT(renameFile(const QString &, const QString &)));
}

//==============================================================================

CellmlFileManager::~CellmlFileManager()
{
    // Remove all the managed files

    foreach (CellmlFile *cellmlFile, mCellmlFiles)
        delete cellmlFile;
}

//==============================================================================

CellmlFileManager * CellmlFileManager::instance()
{
    // Return the 'global' instance of our CellML file manager class

    static CellmlFileManager instance;

    return static_cast<CellmlFileManager *>(Core::globalInstance("OpenCOR::CellMLSupport::CellmlFileManager",
                                                                 &instance));
}

//==============================================================================

CellmlFile * CellmlFileManager::cellmlFile(const QString &pFileName)
{
    // Return the CellmlFile object, if any, associated with the requested file

    return mCellmlFiles.value(Core::nativeCanonicalFileName(pFileName));
}

//==============================================================================

void CellmlFileManager::manageFile(const QString &pFileName)
{
    if (isCellmlFile(pFileName))
        // We are dealing with a CellML file, so we can add it to our list of
        // managed CellML files

        mCellmlFiles.insert(pFileName, new CellmlFile(pFileName));
}

//==============================================================================

void CellmlFileManager::unmanageFile(const QString &pFileName)
{
    if (isCellmlFile(pFileName)) {
        // We are dealing with a CellML file, so we can remove it from our list
        // of managed CellML files after having deleted it

        delete cellmlFile(pFileName);

        mCellmlFiles.remove(pFileName);
    }
}

//==============================================================================

void CellmlFileManager::renameFile(const QString &pOldFileName,
                                   const QString &pNewFileName)
{
    // The file has been renamed, so we need to update our CellML files mapping

    mCellmlFiles.insert(pNewFileName, mCellmlFiles.value(pOldFileName));
    mCellmlFiles.remove(pOldFileName);
}

//==============================================================================

}   // namespace CellMLSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
