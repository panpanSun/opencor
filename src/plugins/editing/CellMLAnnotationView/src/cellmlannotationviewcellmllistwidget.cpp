//==============================================================================
// CellML annotation view CellML list widget
//==============================================================================

#include "cellmlannotationviewcellmllistwidget.h"
#include "cellmlannotationviewwidget.h"
#include "treeviewwidget.h"

//==============================================================================

#include "ui_cellmlannotationviewcellmllistwidget.h"

//==============================================================================

#include <QMenu>
#include <QStandardItemModel>

//==============================================================================

namespace OpenCOR {
namespace CellMLAnnotationView {

//==============================================================================

void CellmlAnnotationViewCellmlElementItemDelegate::paint(QPainter *pPainter,
                                                          const QStyleOptionViewItem &pOption,
                                                          const QModelIndex &pIndex) const
{
    // Paint the item as normal, except for error/warning/category items

    CellmlAnnotationViewCellmlElementItem *cellmlElementItem = static_cast<CellmlAnnotationViewCellmlElementItem *>(qobject_cast<const QStandardItemModel *>(pIndex.model())->itemFromIndex(pIndex));

    QStyleOptionViewItemV4 option(pOption);

    initStyleOption(&option, pIndex);

    if (   (cellmlElementItem->type() == CellmlAnnotationViewCellmlElementItem::Error)
        || (cellmlElementItem->type() == CellmlAnnotationViewCellmlElementItem::Warning)) {
        // This is an error/warning item, so prevent it from hoverable and make
        // it look enabled since it's actually disabled (so we can't select it),
        // yet we want to see as if it was enabled, so...

        option.state &= ~QStyle::State_MouseOver;
        option.state |=  QStyle::State_Enabled;
    } else if (cellmlElementItem->isCategory()) {
        // We are dealing with a category so show it bold

        option.font.setBold(cellmlElementItem->isCategory());
    }

    QStyledItemDelegate::paint(pPainter, option, pIndex);
}

//==============================================================================

CellmlAnnotationViewCellmlElementItem::CellmlAnnotationViewCellmlElementItem(const bool &pError, const QString &pText) :
    QStandardItem(pText),
    mCategory(false),
    mType(pError?Error:Warning),
    mNumber(-1),
    mElement(0)
{
    // Constructor for either an error or a warning

    // Disable the item and use its text as a tooltip (in case it's too long and
    // doesn't fit within the allocated space we have)
    // Note: the item will get 're-enabled' by our item delegate...

    setEnabled(false);
    setToolTip(pText);

    // Set the icon for the item

    setIcon(mType);
}

//==============================================================================

CellmlAnnotationViewCellmlElementItem::CellmlAnnotationViewCellmlElementItem(const Type &pType,
                                                                             const QString &pText) :
    QStandardItem(pText),
    mCategory(true),
    mType(pType),
    mNumber(-1),
    mElement(0)
{
    // Constructor for a category

    // Use its text as a tooltip (in case it's too long and doesn't fit within
    // the allocated space we have)

    setToolTip(pText);

    // Set the icon for the item

    setIcon(pType);
}

//==============================================================================

CellmlAnnotationViewCellmlElementItem::CellmlAnnotationViewCellmlElementItem(const Type &pType,
                                                                             CellMLSupport::CellmlFileElement *pElement,
                                                                             const int pNumber) :
    QStandardItem(),
    mCategory(false),
    mType(pType),
    mNumber(pNumber),
    mElement(pElement)
{
    static const QChar RightArrow = QChar(0x2192);

    // Set the text for some types

    switch (pType) {
    case Import:
        setText(static_cast<CellMLSupport::CellmlFileImport *>(pElement)->xlinkHref());

        break;
    case RelationshipReference:
        setText(static_cast<CellMLSupport::CellmlFileRelationshipReference *>(pElement)->relationship());

        break;
    case Group:
        setText(QObject::tr("Group #%1").arg(QString::number(pNumber)));

        break;
    case ComponentReference:
        setText(static_cast<CellMLSupport::CellmlFileComponentReference *>(pElement)->component());

        break;
    case Connection:
        setText(QObject::tr("Connection #%1").arg(QString::number(pNumber)));

        break;
    case ComponentMapping: {
        CellMLSupport::CellmlFileMapComponents *mapComponents = static_cast<CellMLSupport::CellmlFileMapComponents *>(pElement);

        setText(QString("%1 %2 %3").arg(mapComponents->firstComponent(),
                                        RightArrow,
                                        mapComponents->secondComponent()));

        break;
    }
    case VariableMapping: {
        CellMLSupport::CellmlFileMapVariablesItem *mapVariables = static_cast<CellMLSupport::CellmlFileMapVariablesItem *>(pElement);

        setText(QString("%1 %2 %3").arg(mapVariables->firstVariable(),
                                        RightArrow,
                                        mapVariables->secondVariable()));

        break;
    }
    default:
        // Another type of element which has a name

        setText(static_cast<CellMLSupport::CellmlFileNamedElement *>(pElement)->name());
    }

    // Use its text as a tooltip (in case it's too long and doesn't fit within
    // the allocated space we have)

    setToolTip(text());

    // Set the icon for the item

    setIcon(pType);
}

//==============================================================================

void CellmlAnnotationViewCellmlElementItem::setIcon(const Type &pType)
{
    // Determine the icon to be used for the item

    switch (pType) {
    case Error:
        QStandardItem::setIcon(QIcon(":CellMLSupport_errorNode"));

        break;
    case Warning:
        QStandardItem::setIcon(QIcon(":CellMLSupport_warningNode"));

        break;
    case Model:
        QStandardItem::setIcon(QIcon(":CellMLSupport_modelNode"));

        break;
    case Import:
        QStandardItem::setIcon(QIcon(":CellMLSupport_importNode"));

        break;
    case ImportUnit:
    case Unit:
        QStandardItem::setIcon(QIcon(":CellMLSupport_unitNode"));

        break;
    case UnitElement:
        QStandardItem::setIcon(QIcon(":CellMLSupport_unitElementNode"));

        break;
    case ImportComponent:
    case Component:
    case ComponentMapping:
        QStandardItem::setIcon(QIcon(":CellMLSupport_componentNode"));

        break;
    case Variable:
    case VariableMapping:
        QStandardItem::setIcon(QIcon(":CellMLSupport_variableNode"));

        break;
    case Group:
        QStandardItem::setIcon(QIcon(":CellMLSupport_groupNode"));

        break;
    case RelationshipReference:
        QStandardItem::setIcon(QIcon(":CellMLSupport_relationshipNode"));

        break;
    case ComponentReference:
        QStandardItem::setIcon(QIcon(":CellMLSupport_componentNode"));

        break;
    case Connection:
        QStandardItem::setIcon(QIcon(":CellMLSupport_connectionNode"));

        break;
    default:
        // Type which doesn't require an icon, so...

        ;
    }
}

//==============================================================================

bool CellmlAnnotationViewCellmlElementItem::isCategory() const
{
    // Return whether the CellML element item is a category

    return mCategory;
}

//==============================================================================

int CellmlAnnotationViewCellmlElementItem::type() const
{
    // Return the CellML element item's type

    return mType;
}

//==============================================================================

int CellmlAnnotationViewCellmlElementItem::number() const
{
    // Return the CellML element item's number

    return mNumber;
}

//==============================================================================

CellMLSupport::CellmlFileElement * CellmlAnnotationViewCellmlElementItem::element() const
{
    // Return the CellML element item's element

    return mElement;
}

//==============================================================================

CellmlAnnotationViewCellmlListWidget::CellmlAnnotationViewCellmlListWidget(CellmlAnnotationViewWidget *pParent) :
    Widget(pParent),
    mCellmlFile(pParent->cellmlFile()),
    mGui(new Ui::CellmlAnnotationViewCellmlListWidget),
    mIndexes(QList<QModelIndex>())
{
    // Set up the GUI

    mGui->setupUi(this);

    // Create and customise our tree view widget which will contain all of the
    // imports, units, components, groups and connections from a CellML file

    mTreeViewWidget = new Core::TreeViewWidget(pParent);
    mModel          = new QStandardItemModel(mTreeViewWidget);
    mItemDelegate   = new CellmlAnnotationViewCellmlElementItemDelegate();

    mTreeViewWidget->setModel(mModel);
    mTreeViewWidget->setItemDelegate(mItemDelegate);

    mTreeViewWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTreeViewWidget->setHeaderHidden(true);
    mTreeViewWidget->setRootIsDecorated(false);
    mTreeViewWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    // Note: the selection mode we are opting for means that there is always
    //       going to be a CellML element which is selected, so it's something
    //       that we must keep in mind when showing the context menu...

    // Populate ourselves

    mGui->layout->addWidget(mTreeViewWidget);

    // We want a context menu for our tree view widget

    mTreeViewWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mTreeViewWidget, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showCustomContextMenu(const QPoint &)));

    // Some connections to handle the expansion/collapse of a CellML element

    connect(mTreeViewWidget, SIGNAL(expanded(const QModelIndex &)),
            this, SLOT(resizeTreeViewToContents()));
    connect(mTreeViewWidget, SIGNAL(collapsed(const QModelIndex &)),
            this, SLOT(resizeTreeViewToContents()));

    // Some connections to handle the change of CellML element

    connect(mTreeViewWidget->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(updateMetadataDetails(const QModelIndex &, const QModelIndex &)));

    // Populate our tree view widget

    populateModel();

    // Make our tree view widget our focus proxy

    setFocusProxy(mTreeViewWidget);

    // Expand our tree view widget enough so that we can see the meaningful
    // parts of the CellML file

    mTreeViewWidget->expandToDepth(1);

    // Resize our tree view widget, just to be on the safe side

    resizeTreeViewToContents();
}

//==============================================================================

CellmlAnnotationViewCellmlListWidget::~CellmlAnnotationViewCellmlListWidget()
{
    // Delete the GUI

    delete mGui;
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::retranslateUi()
{
    // Retranslate our GUI

    mGui->retranslateUi(this);

    // Retranslate some of the CellML elements in our tree view widget

    retranslateDataItem(static_cast<CellmlAnnotationViewCellmlElementItem *>(mModel->invisibleRootItem()));
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::retranslateDataItem(CellmlAnnotationViewCellmlElementItem *pCellmlElementItem)
{
    // Retranslate some of the CellML element's children

    for (int i = 0, iMax = pCellmlElementItem->rowCount(); i < iMax; ++i)
        retranslateDataItem(static_cast<CellmlAnnotationViewCellmlElementItem *>(pCellmlElementItem->child(i)));

    // Check whether we are dealing with a category

    if (pCellmlElementItem->isCategory())
        // We are dealing with a category, so retranslate its type

        switch (pCellmlElementItem->type()) {
        case CellmlAnnotationViewCellmlElementItem::Import:
            pCellmlElementItem->setText(tr("Imports"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Unit:
            pCellmlElementItem->setText(tr("Units"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Component:
            pCellmlElementItem->setText(tr("Components"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Variable:
            pCellmlElementItem->setText(tr("Variables"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Group:
            pCellmlElementItem->setText(tr("Groups"));

            break;
        case CellmlAnnotationViewCellmlElementItem::RelationshipReference:
            pCellmlElementItem->setText(tr("Relationship References"));

            break;
        case CellmlAnnotationViewCellmlElementItem::ComponentReference:
            pCellmlElementItem->setText(tr("Component References"));

            break;
        case CellmlAnnotationViewCellmlElementItem::Connection:
            pCellmlElementItem->setText(tr("Connections"));

            break;
        default:
            // Not a type we can retranslate, so do nothing...

            ;
        }
    else
        // We are not dealing with a category, so check the type and see whether
        // a CellML element needs retranslating

        switch (pCellmlElementItem->type()) {
        case CellmlAnnotationViewCellmlElementItem::Group:
            pCellmlElementItem->setText(tr("Group #%1").arg(pCellmlElementItem->number()));

            break;
        case CellmlAnnotationViewCellmlElementItem::Connection:
            pCellmlElementItem->setText(tr("Connection #%1").arg(pCellmlElementItem->number()));

            break;
        default:
            // Not a sub-type we can retranslate, so do nothing...

            ;
        }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::populateModel()
{
    // Make sure that the CellML file was properly loaded

    CellMLSupport::CellmlFileIssues issues = mCellmlFile->issues();
    int issuesCount = issues.count();

    if (issuesCount) {
        // Something went wrong while trying to load the CellML file, so report
        // the issue(s) and leave

        for (int i = 0; i < issuesCount; ++i)
            mModel->invisibleRootItem()->appendRow(new CellmlAnnotationViewCellmlElementItem(issues[i].type() == CellMLSupport::CellmlFileIssue::Error,
                                                                                             QString("[%1:%2] %3").arg(QString::number(issues[i].line()),
                                                                                                                       QString::number(issues[i].column()),
                                                                                                                       issues[i].formattedMessage())));

        return;
    }

    // Retrieve the model's root

    CellmlAnnotationViewCellmlElementItem *modelItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Model,
                                                                                                 mCellmlFile->model());

    mModel->invisibleRootItem()->appendRow(modelItem);

    // Retrieve the model's imports

    if (mCellmlFile->imports()->count()) {
        // Imports category

        CellmlAnnotationViewCellmlElementItem *importsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Import,
                                                                                                       tr("Imports"));

        modelItem->appendRow(importsItem);

        // Retrieve the model's imports themselves

        foreach (CellMLSupport::CellmlFileImport *import,
                 *mCellmlFile->imports()) {
            // A model import

            CellmlAnnotationViewCellmlElementItem *importItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Import,
                                                                                                          import);

            importsItem->appendRow(importItem);

            // Retrieve the model's import's units

            if (import->units().count()) {
                // Units category

                CellmlAnnotationViewCellmlElementItem *unitsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ImportUnit,
                                                                                                             tr("Units"));

                importItem->appendRow(unitsItem);

                // Retrieve the model's import's units themselves

                foreach (CellMLSupport::CellmlFileImportUnit *unit,
                         import->units())
                    // A model's import's unit

                    unitsItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ImportUnit,
                                                                                   unit));
            }

            // Retrieve the model's import's components

            if (import->components().count()) {
                // Components category

                CellmlAnnotationViewCellmlElementItem *componentsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ImportComponent,
                                                                                                                  tr("Components"));

                importItem->appendRow(componentsItem);

                // Retrieve the model's import's components themselves

                foreach (CellMLSupport::CellmlFileImportComponent *component,
                         import->components())
                    // A model's import's component

                    componentsItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ImportComponent,
                                                                                        component));
            }
        }
    }

    // Retrieve the model's units

    populateUnitsModel(modelItem, mCellmlFile->units());

    // Retrieve the model's components

    if (mCellmlFile->components()->count()) {
        // Components category

        CellmlAnnotationViewCellmlElementItem *componentsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Component,
                                                                                                          tr("Components"));

        modelItem->appendRow(componentsItem);

        // Retrieve the model's components themselves

        foreach (CellMLSupport::CellmlFileComponent *component,
                 *mCellmlFile->components()) {
            // A model's component

            CellmlAnnotationViewCellmlElementItem *componentItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Component,
                                                                                                             component);

            componentsItem->appendRow(componentItem);

            // Retrieve the model's component's units

            populateUnitsModel(componentItem, component->units());

            // Retrieve the model's component's variables

            if (component->variables()->count()) {
                // Variables category

                CellmlAnnotationViewCellmlElementItem *variablesItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Variable,
                                                                                                                 tr("Variables"));

                componentItem->appendRow(variablesItem);

                // Retrieve the model's component's variables themselves, but
                // only add a variable if neither its public nor its private
                // interface is equal to "in"

                foreach (CellMLSupport::CellmlFileVariable *variable,
                         *component->variables())
                    if (   (variable->publicInterface()  != CellMLSupport::CellmlFileVariable::In)
                        && (variable->privateInterface() != CellMLSupport::CellmlFileVariable::In))
                        variablesItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Variable,
                                                                                           variable));
            }
        }
    }

    // Retrieve the model's groups

    if (mCellmlFile->groups()->count()) {
        // Groups category

        CellmlAnnotationViewCellmlElementItem *groupsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Group,
                                                                                                      tr("Groups"));

        modelItem->appendRow(groupsItem);

        // Retrieve the model's groups themselves

        int counter = 0;

        foreach (CellMLSupport::CellmlFileGroup *group,
                 *mCellmlFile->groups()) {
            // A model's group

            CellmlAnnotationViewCellmlElementItem *groupItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Group,
                                                                                                         group,
                                                                                                         ++counter);

            groupsItem->appendRow(groupItem);

            // Retrieve the model's group's relationship references

            if (group->relationshipReferences().count()) {
                // Relationship references category

                CellmlAnnotationViewCellmlElementItem *relationshipReferencesItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::RelationshipReference,
                                                                                                                              tr("Relationship References"));

                groupItem->appendRow(relationshipReferencesItem);

                // Retrieve the model's group's relationship references
                // themselves

                foreach (CellMLSupport::CellmlFileRelationshipReference *relationshipReference,
                         group->relationshipReferences())
                    relationshipReferencesItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::RelationshipReference,
                                                                                                    relationshipReference));
            }

            // Retrieve the model's group's component references

            if (group->componentReferences().count()) {
                // Component references category

                CellmlAnnotationViewCellmlElementItem *componentReferencesItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ComponentReference,
                                                                                                                           tr("Component References"));

                groupItem->appendRow(componentReferencesItem);

                // Retrieve the model's group's relationship references
                // themselves

                foreach (CellMLSupport::CellmlFileComponentReference *componentReference,
                         group->componentReferences())
                    populateComponentReferenceModel(componentReferencesItem,
                                                    componentReference);
            }
        }
    }

    // Retrieve the model's connections

    if (mCellmlFile->connections()->count()) {
        // Connections category

        CellmlAnnotationViewCellmlElementItem *connectionsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Connection,
                                                                                                           tr("Connections"));

        modelItem->appendRow(connectionsItem);

        // Retrieve the model's connections themselves

        int counter = 0;

        foreach (CellMLSupport::CellmlFileConnection *connection,
                 *mCellmlFile->connections()) {
            // A model's connection

            CellmlAnnotationViewCellmlElementItem *connectionItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Connection,
                                                                                                              connection,
                                                                                                              ++counter);

            connectionsItem->appendRow(connectionItem);

            // Component mapping

            connectionItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ComponentMapping,
                                                                                connection->componentMapping()));

            // Variable mappings

            foreach (CellMLSupport::CellmlFileMapVariablesItem *mapVariablesItem,
                     connection->variableMappings())
                connectionItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::VariableMapping,
                                                                                    mapVariablesItem));
        }
    }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::populateUnitsModel(CellmlAnnotationViewCellmlElementItem *pCellmlElementItem,
                                                              CellMLSupport::CellmlFileUnits *pUnits)
{
    // Retrieve the units

    if (pUnits->count()) {
        // Units category

        CellmlAnnotationViewCellmlElementItem *unitsItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Unit,
                                                                                                     tr("Units"));

        pCellmlElementItem->appendRow(unitsItem);

        // Retrieve the units themselves

        foreach (CellMLSupport::CellmlFileUnit *unit, *pUnits) {
            CellmlAnnotationViewCellmlElementItem *unitItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::Unit,
                                                                                                        unit);

            unitsItem->appendRow(unitItem);

            // Retrieve the unit's unit elements

            if (unit->unitElements().count())
                foreach (CellMLSupport::CellmlFileUnitElement *unitElement,
                         unit->unitElements())
                    unitItem->appendRow(new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::UnitElement,
                                                                                  unitElement));

        }
    }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::populateComponentReferenceModel(CellmlAnnotationViewCellmlElementItem *pCellmlElementItem,
                                                                           CellMLSupport::CellmlFileComponentReference *pComponentReference)
{
    CellmlAnnotationViewCellmlElementItem *componentReferenceItem = new CellmlAnnotationViewCellmlElementItem(CellmlAnnotationViewCellmlElementItem::ComponentReference,
                                                                                                              pComponentReference);

    pCellmlElementItem->appendRow(componentReferenceItem);

    // Retrieve the component reference's children

    foreach (CellMLSupport::CellmlFileComponentReference *componentReference,
             pComponentReference->componentReferences())
        populateComponentReferenceModel(componentReferenceItem, componentReference);
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::resizeTreeViewToContents()
{
    // Resize our tree view widget so that its contents is visible

    mTreeViewWidget->resizeColumnToContents(0);
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::updateMetadataDetails(const QModelIndex &pNewIndex,
                                                                 const QModelIndex &pOldIndex)
{
    Q_UNUSED(pOldIndex);

    // Make sure that we have a valid new index

    if (!pNewIndex.isValid())
        return;

    // Keep track of the fact that there is a CellML element to update

    mIndexes << pNewIndex;

    // Make sure that we are not already updating a CellML element by checking
    // that the CellML file for which we want to update an element is not in our
    // list of CellML files being updated

    static QStringList cellmlFileBeingUpdated;

    QString cellmlFileName = mCellmlFile->fileName();

    if (cellmlFileBeingUpdated.contains(cellmlFileName))
        return;

    cellmlFileBeingUpdated << cellmlFileName;

    // Loop while there are CellML elements to update
    // Note: this is done because a CellML element may take time to update and
    //       we may end up in a situation where several CellML elements need
    //       updating, so...

    while (mIndexes.count()) {
        // Retrieve the first CellML element to update

        QModelIndex crtIndex = mIndexes.first();

        mIndexes.removeFirst();

        // Let people know that we request to see some metadata details

        emit metadataDetailsRequested(static_cast<CellmlAnnotationViewCellmlElementItem *>(mModel->itemFromIndex(crtIndex))->element());
    }

    // We are done, so...

    cellmlFileBeingUpdated.removeAt(cellmlFileBeingUpdated.indexOf(cellmlFileName));
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::showCustomContextMenu(const QPoint &pPosition) const
{
    Q_UNUSED(pPosition);

    // Determine whether to show the context menu based on whether we are over
    // an item

    CellmlAnnotationViewCellmlElementItem *posItem = static_cast<CellmlAnnotationViewCellmlElementItem *>(mModel->itemFromIndex(mTreeViewWidget->indexAt(mTreeViewWidget->mapFromGlobal(QCursor::pos()-mTreeViewWidget->pos()))));

    if (posItem) {
        // We are over an item, so create a custom context menu which items
        // match the contents of our tool bar widget

        // Update the enabled status of our actions

        if (posItem->hasChildren()) {
            mGui->actionExpandAll->setEnabled(!indexIsAllExpanded(mTreeViewWidget->currentIndex()));
            mGui->actionCollapseAll->setEnabled(mTreeViewWidget->isExpanded(mTreeViewWidget->currentIndex()));
        }

        if (!posItem->isCategory()) {
            mGui->actionRemoveCurrentMetadata->setEnabled(posItem->element()->rdfTriples().count());
            mGui->actionRemoveAllMetadata->setEnabled(mCellmlFile->rdfTriples()->count());
        }

        // Create and show the context menu, if it isn't empty

        QMenu menu;

        if (posItem->hasChildren()) {
            menu.addAction(mGui->actionExpandAll);
            menu.addAction(mGui->actionCollapseAll);
            menu.addSeparator();
        }

        if (!posItem->isCategory()) {
            menu.addAction(mGui->actionRemoveCurrentMetadata);
            menu.addAction(mGui->actionRemoveAllMetadata);
        }

        if (!menu.isEmpty())
            menu.exec(QCursor::pos());
    }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::on_actionExpandAll_triggered()
{
    // Expand all the CellML elements below the current one
    // Note: we disable and then re-enable updates before expanding all the
    //       index since it may end up in quite a few updates...

    mTreeViewWidget->setUpdatesEnabled(false);
        qApp->setOverrideCursor(Qt::WaitCursor);

        indexExpandAll(mTreeViewWidget->currentIndex());

        qApp->restoreOverrideCursor();
    mTreeViewWidget->setUpdatesEnabled(true);
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::on_actionCollapseAll_triggered()
{
    // Collapse all the CellML elements below the current one
    // Note: see the note in on_actionExpandAll_triggered() above...

    mTreeViewWidget->setUpdatesEnabled(false);
        qApp->setOverrideCursor(Qt::WaitCursor);

        indexCollapseAll(mTreeViewWidget->currentIndex());

        qApp->restoreOverrideCursor();
    mTreeViewWidget->setUpdatesEnabled(true);
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::on_actionRemoveCurrentMetadata_triggered()
{
    // Remove the metadata associated with the current node

    currentCellmlElementItem()->element()->removeAllMetadata();

    // Re-update the metadata details view now that the current node doesn't
    // have any metadata associated with it

    updateMetadataDetails(mTreeViewWidget->currentIndex(), QModelIndex());
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::on_actionRemoveAllMetadata_triggered()
{
    // Remove all the metadata associated with the CellML file

    mCellmlFile->rdfTriples()->removeAll();

    // Re-update the metadata details view now that the CellML file doesn't have
    // any metadata associated with it

    updateMetadataDetails(mTreeViewWidget->currentIndex(), QModelIndex());
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::indexExpandAll(const QModelIndex &pIndex) const
{
    // Recursively expand all the CellML elements below the current one
    // Note: the test with against pIndex.child(0, 0) is to ensure that we are
    //       not trying to expand an index which item has no children. Indeed,
    //       a call to expand() is quite expensive, so the fewer of those we
    //       make the better...

    if (pIndex.child(0, 0).isValid()) {
        mTreeViewWidget->expand(pIndex);

        QStandardItem *item = mModel->itemFromIndex(pIndex);

        for (int i = 0, iMax = item->rowCount(); i < iMax; ++i)
            indexExpandAll(item->child(i)->index());
    }
}

//==============================================================================

void CellmlAnnotationViewCellmlListWidget::indexCollapseAll(const QModelIndex &pIndex) const
{
    // Recursively collapse all the CellML elements below the current one
    // Note: see the note in indexExpandAll() above...

    if (pIndex.child(0, 0).isValid()) {
        QStandardItem *item = mModel->itemFromIndex(pIndex);

        for (int i = 0, iMax = item->rowCount(); i < iMax; ++i)
            indexCollapseAll(item->child(i)->index());

        mTreeViewWidget->collapse(pIndex);
    }
}

//==============================================================================

bool CellmlAnnotationViewCellmlListWidget::indexIsAllExpanded(const QModelIndex &pIndex) const
{
    // Recursively check that the current CellML element and all of its children
    // are expanded
    // Note: see the note in indexExpandAll() above...

    if (pIndex.child(0, 0).isValid()) {
        QStandardItem *item = mModel->itemFromIndex(pIndex);

        for (int i = 0, iMax = item->rowCount(); i < iMax; ++i)
            if (!indexIsAllExpanded(item->child(i)->index()))
                return false;

        return mTreeViewWidget->isExpanded(pIndex);
    } else {
        return true;
    }
}

//==============================================================================

Core::TreeViewWidget * CellmlAnnotationViewCellmlListWidget::treeViewWidget() const
{
    // Return our tree view widget

    return mTreeViewWidget;
}

//==============================================================================

CellmlAnnotationViewCellmlElementItem * CellmlAnnotationViewCellmlListWidget::currentCellmlElementItem() const
{
    // Return the current CellML element item

    return static_cast<CellmlAnnotationViewCellmlElementItem *>(mModel->itemFromIndex(mTreeViewWidget->currentIndex()));
}

//==============================================================================

}   // namespace CellMLAnnotationView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
