//==============================================================================
// Property editor widget
//==============================================================================

#include "propertyeditorwidget.h"

//==============================================================================

#include <float.h>

//==============================================================================

#include <QHeaderView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QSettings>
#include <QStandardItem>
#include <QVariant>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

NumberEditorWidget::NumberEditorWidget(QWidget *pParent) :
    QLineEdit(pParent)
{
#ifdef Q_OS_MAC
    setAttribute(Qt::WA_MacShowFocusRect, 0);
    // Note: the above removes the focus border since it messes up the look of
    //       our editor
#endif
    setFrame(QFrame::NoFrame);
}

//==============================================================================

void NumberEditorWidget::keyPressEvent(QKeyEvent *pEvent)
{
    // Check some key combinations

    if (   !(pEvent->modifiers() & Qt::ShiftModifier)
        && !(pEvent->modifiers() & Qt::ControlModifier)
        && !(pEvent->modifiers() & Qt::AltModifier)
        && !(pEvent->modifiers() & Qt::MetaModifier)) {
        // None of the modifiers is selected

        if (pEvent->key() == Qt::Key_Up) {
            // The user wants to go to the previous property

            emit goToPreviousPropertyRequested();

            // Accept the event

            pEvent->accept();
        } else if (pEvent->key() == Qt::Key_Down) {
            // The user wants to go to the next property

            emit goToNextPropertyRequested();

            // Accept the event

            pEvent->accept();
        } else {
            // Default handling of the event

            QLineEdit::keyPressEvent(pEvent);
        }
    } else {
        // Default handling of the event

        QLineEdit::keyPressEvent(pEvent);
    }
}

//==============================================================================

IntegerEditorWidget::IntegerEditorWidget(QWidget *pParent) :
    NumberEditorWidget(pParent)
{
    // Set a validator which accepts any integer

    setValidator(new QRegExpValidator(QRegExp("^[+-]?[0-9]*([eE][+-]?[0-9]+)?$"), this));
}

//==============================================================================

DoubleEditorWidget::DoubleEditorWidget(QWidget *pParent) :
    NumberEditorWidget(pParent)
{
    // Set a validator which accepts any double

    setValidator(new QRegExpValidator(QRegExp("^[+-]?[0-9]*\\.?[0-9]+([eE][+-]?[0-9]+)?$"), this));
}

//==============================================================================

ListEditorWidget::ListEditorWidget(QWidget *pParent) :
    QComboBox(pParent)
{
}

//==============================================================================

void ListEditorWidget::keyPressEvent(QKeyEvent *pEvent)
{
    // Check some key combinations

    if (   !(pEvent->modifiers() & Qt::ShiftModifier)
        && !(pEvent->modifiers() & Qt::ControlModifier)
        && !(pEvent->modifiers() & Qt::AltModifier)
        && !(pEvent->modifiers() & Qt::MetaModifier)) {
        // None of the modifiers is selected

        if (pEvent->key() == Qt::Key_Up) {
            // The user wants to go to the previous property

            emit goToPreviousPropertyRequested();

            // Accept the event

            pEvent->accept();
        } else if (pEvent->key() == Qt::Key_Down) {
            // The user wants to go to the next property

            emit goToNextPropertyRequested();

            // Accept the event

            pEvent->accept();
        } else {
            // Default handling of the event

            QComboBox::keyPressEvent(pEvent);
        }
    } else {
        // Default handling of the event

        QComboBox::keyPressEvent(pEvent);
    }
}

//==============================================================================

void ListEditorWidget::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
    // We want to go to the next item in the list (and go back to the first one
    // if we are at the end of the list), so determine the new current index

    int newCurrentIndex = currentIndex()+1;

    if (newCurrentIndex == count())
        newCurrentIndex = 0;

    // Set the new current index

    setCurrentIndex(newCurrentIndex);

    // Accept the event

    pEvent->accept();
}

//==============================================================================

void ListEditorWidget::mousePressEvent(QMouseEvent *pEvent)
{
    // Check whether the user clicked on the arrow and, if so, allow the default
    // handling of the event (so that the list of items gets displayed) while do
    // nothing if the user clicked somewhere else (this to so that if the user
    // double clicks on the widget, then we can select the next item)
    // Note: we would normally call style()->hitTestComplexControl() and, if it
    //       returns QStyle::SC_ComboBoxArrow, then allow the default handling
    //       of the event, but if this works fine on Windows and Linux, it just
    //       doesn't work on OS X. Indeed, no matter where we are over the
    //       widget, style()->hitTestComplexControl() will always (and as
    //       expected; [QtSources]/qtbase/src/widgets/styles/qmacstyle_mac.mm)
    //       return QStyle::SC_ComboBoxArrow. So, to get the behaviour we are
    //       after, we do what is done in
    //       [QtSources]/qtbase/src/widgets/styles/qcommonstyle.cpp...

    // Retrieve our style option

    QStyleOptionComboBox styleOption;

    initStyleOption(&styleOption);

    // Go through the different sub controls (starting with the arrow) and
    // determine over which one we are

    uint subControl = QStyle::SC_ComboBoxArrow;
    QRect subControlRect;

    while (subControl) {
        // Retrieve the sub control's region

        subControlRect = style()->subControlRect(QStyle::CC_ComboBox,
                                                 &styleOption,
                                                 QStyle::SubControl(subControl),
                                                 this);

        // Check whether the sub control exists (i.e. its region is valid) and,
        // if so, whether we ore over it

        if (subControlRect.isValid() && subControlRect.contains(pEvent->pos()))
            // The sub control exists and we are over it, so...

            break;

        // Either the sub control doesn't exist or we are not over it, so try
        // the next sub control

        subControl >>= 1;
    }

    // Check whether the 'current' sub control is the arrow we are after

    if (QStyle::SubControl(subControl) == QStyle::SC_ComboBoxArrow)
        // It is the arrow we are after, so...

        QComboBox::mousePressEvent(pEvent);

    // Accept the event

    pEvent->accept();
}

//==============================================================================

QWidget * PropertyItemDelegate::createEditor(QWidget *pParent,
                                             const QStyleOptionViewItem &pOption,
                                             const QModelIndex &pIndex) const
{
    Q_UNUSED(pOption);

    // Create and return an editor for our item, based on its type

    QWidget *editor;
    PropertyItem *propertyItem = static_cast<PropertyItem *>(qobject_cast<const QStandardItemModel *>(pIndex.model())->itemFromIndex(pIndex));

    switch (propertyItem->type()) {
    case PropertyItem::Integer:
        editor = new IntegerEditorWidget(pParent);

        break;
    case PropertyItem::Double:
        editor = new DoubleEditorWidget(pParent);

        break;
    case PropertyItem::List: {
        ListEditorWidget *listEditor = new ListEditorWidget(pParent);

        listEditor->addItems(propertyItem->list());

        editor = listEditor;

        // Propagate the signal telling us about the list property value having
        // changed

        connect(listEditor, SIGNAL(currentIndexChanged(const QString &)),
                this, SIGNAL(listPropertyChanged(const QString &)));

        break;
    }
    default:
        // PropertyItem::Section

        return 0;
    }

    // Propagate a few signals

    connect(editor, SIGNAL(goToPreviousPropertyRequested()),
            this, SIGNAL(goToPreviousPropertyRequested()));
    connect(editor, SIGNAL(goToNextPropertyRequested()),
            this, SIGNAL(goToNextPropertyRequested()));

    // Let people know that there is a new editor

    emit openEditor(editor);

    // Return the editor

    return editor;
}

//==============================================================================

bool PropertyItemDelegate::eventFilter(QObject *pObject, QEvent *pEvent)
{
    // We want to handle key events ourselves, so...

    if (pEvent->type() == QEvent::KeyPress)
        return false;
    else
        return QStyledItemDelegate::eventFilter(pObject, pEvent);
}

//==============================================================================

void PropertyItemDelegate::paint(QPainter *pPainter,
                                 const QStyleOptionViewItem &pOption,
                                 const QModelIndex &pIndex) const
{
    // Paint the item as normal, except if it is a section item

    PropertyItem *propertyItem = static_cast<PropertyItem *>(qobject_cast<const QStandardItemModel *>(pIndex.model())->itemFromIndex(pIndex));

    QStyleOptionViewItemV4 option(pOption);

    initStyleOption(&option, pIndex);

    if (propertyItem->type() == PropertyItem::Section) {
        // Make our section item bold

        option.font.setBold(true);
    }

    QStyledItemDelegate::paint(pPainter, option, pIndex);
}

//==============================================================================

PropertyItem::PropertyItem(const Type &pType, const bool &pEditable) :
    QStandardItem(),
    mType(pType),
    mList(QStringList()),
    mEmptyListValue(QString("???"))
{
    // Disable the editing of the property item if it is of section or string
    // type

    if (!pEditable)
        setFlags(flags() & ~Qt::ItemIsEditable);
}

//==============================================================================

int PropertyItem::type() const
{
    // Return the property item's type

    return mType;
}

//==============================================================================

QStringList PropertyItem::list() const
{
    // Return the property item's list

    return mList;
}

//==============================================================================

void PropertyItem::setList(const QStringList &pList)
{
    // Set the value of our list, if appropriate

    if ((mType == List) && (pList != mList)) {
        // Keep track of the list

        mList = pList;

        // Use the first item of our list as the default value, assuming the
        // list is not empty

        PropertyEditorWidget::setPropertyItem(this,
                                              pList.isEmpty()?
                                                  mEmptyListValue:
                                                  pList.first());
    }
}

//==============================================================================

QString PropertyItem::emptyListValue() const
{
    // Return the property item's empty list value

    return mEmptyListValue;
}

//==============================================================================

void PropertyItem::setEmptyListValue(const QString &pEmptyListValue)
{
    // Set the value of our empty list value, if needed

    if ((mType == List) && pEmptyListValue.compare(mEmptyListValue))
        mEmptyListValue = pEmptyListValue;
}

//==============================================================================

Property::Property(const PropertyItem::Type &pType, const bool &pEditable,
                   const bool &pCheckable) :
    mName(new PropertyItem((pType == PropertyItem::Section)?pType:PropertyItem::String, false)),
    mValue(new PropertyItem(pType, pEditable)),
    mUnit(new PropertyItem(PropertyItem::String, false))
{
    // Make the property checkable, if needed

    mName->setCheckable(pCheckable);
}

//==============================================================================

Property::Property(PropertyItem *pName, PropertyItem *pValue,
                   PropertyItem *pUnit) :
    mName(pName),
    mValue(pValue),
    mUnit(pUnit)
{
}

//==============================================================================

PropertyItem * Property::name() const
{
    // Return our name item

    return mName;
}

//==============================================================================

PropertyItem * Property::value() const
{
    // Return our value item

    return mValue;
}

//==============================================================================

PropertyItem * Property::unit() const
{
    // Return our unit item

    return mUnit;
}

//==============================================================================

QList<QStandardItem *> Property::items() const
{
    // Return our items as a list

    return QList<QStandardItem *>() << mName << mValue << mUnit;
}

//==============================================================================

PropertyEditorWidgetGuiStateProperty::PropertyEditorWidgetGuiStateProperty(Property *pProperty,
                                                                           const bool &pHidden,
                                                                           const bool &pExpanded,
                                                                           const QString &pValue) :
    mProperty(pProperty),
    mHidden(pHidden),
    mExpanded(pExpanded),
    mValue(pValue)
{
}

//==============================================================================

Property * PropertyEditorWidgetGuiStateProperty::property() const
{
    // Return our property

    return mProperty;
}

//==============================================================================

bool PropertyEditorWidgetGuiStateProperty::isHidden() const
{
    // Return whether our property is hidden

    return mHidden;
}

//==============================================================================

bool PropertyEditorWidgetGuiStateProperty::isExpanded() const
{
    // Return whether our property is expanded

    return mExpanded;
}

//==============================================================================

QString PropertyEditorWidgetGuiStateProperty::value() const
{
    // Return our property's value

    return mValue;
}

//==============================================================================

PropertyEditorWidgetGuiState::PropertyEditorWidgetGuiState(const QModelIndex &pCurrentProperty) :
    mProperties(PropertyEditorWidgetGuiStateProperties()),
    mCurrentProperty(pCurrentProperty)
{
}

//==============================================================================

PropertyEditorWidgetGuiState::~PropertyEditorWidgetGuiState()
{
    // Delete some internal objects

    foreach (PropertyEditorWidgetGuiStateProperty *property, mProperties)
        delete property;
}

//==============================================================================

PropertyEditorWidgetGuiStateProperties PropertyEditorWidgetGuiState::properties() const
{
    // Return our properties

    return mProperties;
}

//==============================================================================

void PropertyEditorWidgetGuiState::addProperty(PropertyEditorWidgetGuiStateProperty *pProperty)
{
    // Add the property to our list

    mProperties << pProperty;
}

//==============================================================================

QModelIndex PropertyEditorWidgetGuiState::currentProperty() const
{
    // Return our current property

    return mCurrentProperty;
}

//==============================================================================

void PropertyEditorWidget::constructor(const bool &pShowUnits,
                                       const bool &pAutoUpdateHeight)
{
    // Some initialisations

    mShowUnits = pShowUnits;
    mAutoUpdateHeight = pAutoUpdateHeight;

    mProperties = Properties();

    mProperty       = 0;
    mPropertyEditor = 0;

    mPropertiesChecked = QMap<Property *, bool>();

    // Customise ourselves

    setRootIsDecorated(false);

    // Create and set our data model

    mModel = new QStandardItemModel(this);

    setModel(mModel);

    // Create our item delegate and set it, after making sure that we handle a
    // few of its signals

    PropertyItemDelegate *propertyItemDelegate = new PropertyItemDelegate();

    connect(mModel, SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(emitPropertyChecked(QStandardItem *)));

    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(editorClosed()));

    connect(propertyItemDelegate, SIGNAL(openEditor(QWidget *)),
            this, SLOT(editorOpened(QWidget *)));
    connect(propertyItemDelegate, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)),
            this, SLOT(editorClosed()));

    connect(propertyItemDelegate, SIGNAL(goToPreviousPropertyRequested()),
            this, SLOT(goToPreviousProperty()));
    connect(propertyItemDelegate, SIGNAL(goToNextPropertyRequested()),
            this, SLOT(goToNextProperty()));

    connect(propertyItemDelegate, SIGNAL(listPropertyChanged(const QString &)),
            this, SIGNAL(listPropertyChanged(const QString &)));

    setItemDelegate(propertyItemDelegate);

    // Resize our height in case some data has changed or one of the properties
    // gets expanded/collapsed

    connect(mModel, SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(updateHeight()));

    connect(this, SIGNAL(collapsed(const QModelIndex &)),
            this, SLOT(updateHeight()));
    connect(this, SIGNAL(expanded(const QModelIndex &)),
            this, SLOT(updateHeight()));

    // Further customise ourselves

    setSelectionMode(QAbstractItemView::SingleSelection);

    header()->setSectionsMovable(false);

    // Retranslate ourselves

    retranslateUi();

    // Initialise our height

    updateHeight();
}

//==============================================================================

PropertyEditorWidget::PropertyEditorWidget(const bool &pShowUnits,
                                           const bool &pAutoUpdateHeight,
                                           QWidget *pParent) :
    TreeViewWidget(pParent)
{
    // Construct our object

    constructor(pShowUnits, pAutoUpdateHeight);
}

//==============================================================================

PropertyEditorWidget::PropertyEditorWidget(const bool &pAutoUpdateHeight,
                                           QWidget *pParent) :
    TreeViewWidget(pParent)
{
    // Construct our object

    constructor(true, pAutoUpdateHeight);
}

//==============================================================================

PropertyEditorWidget::PropertyEditorWidget(QWidget *pParent) :
    TreeViewWidget(pParent)
{
    // Construct our object

    constructor();
}

//==============================================================================

PropertyEditorWidget::~PropertyEditorWidget()
{
    // Delete some internal objects

    foreach (Property *property, mProperties)
        delete property;
}

//==============================================================================

void PropertyEditorWidget::retranslateEmptyListProperties(QStandardItem *pItem)
{
    // Retranslate the current item, should it be an empty list

    QModelIndex index = pItem->index();

    if (index.isValid()) {
        // The index is valid (i.e. it's not our invisible root item), soc heck
        // whether the property value is of list type and whether its list is
        // empty and, if so, then set its text value accordingly

        PropertyItem *propertyValue = property(index)->value();

        if (   propertyValue
            && (propertyValue->type() == PropertyItem::List)
            && (propertyValue->list().isEmpty()))
            setPropertyItem(propertyValue, propertyValue->emptyListValue());
    }

    // Retranslate the current item's children, if any

    for (int i = 0, iMax = pItem->rowCount(); i < iMax; ++i)
        retranslateEmptyListProperties(pItem->child(i));
}

//==============================================================================

void PropertyEditorWidget::retranslateUi()
{
    // Retranslate our header labels

    if (mShowUnits)
        mModel->setHorizontalHeaderLabels(QStringList() << tr("Property")
                                                        << tr("Value")
                                                        << tr("Unit"));
    else
        mModel->setHorizontalHeaderLabels(QStringList() << tr("Property")
                                                        << tr("Value"));

    // 'Retranslate' the value of all empty list properties

    retranslateEmptyListProperties(mModel->invisibleRootItem());
}

//==============================================================================

static const QString SettingsColumnWidth = "ColumnWidth%1";

//==============================================================================

void PropertyEditorWidget::loadSettings(QSettings *pSettings)
{
    // Retrieve the width of each column

    for (int i = 0, iMax = header()->count(); i < iMax; ++i)
        setColumnWidth(i, pSettings->value(SettingsColumnWidth.arg(i),
                                           columnWidth(i)).toInt());
}

//==============================================================================

void PropertyEditorWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of the width of each column

    for (int i = 0, iMax = header()->count(); i < iMax; ++i)
        pSettings->setValue(SettingsColumnWidth.arg(i), columnWidth(i));
}

//==============================================================================

int PropertyEditorWidget::childrenRowHeight(const QStandardItem *pItem) const
{
    // Return the total height of the given index's children

    int res = 0;

    if (pItem->hasChildren())
        for (int i = 0, iMax = pItem->rowCount(); i < iMax; ++i) {
            QStandardItem *childItem = pItem->child(i, 0);
            int childIndexHeight     = rowHeight(childItem->index());

            if (childIndexHeight)
                res += childIndexHeight+childrenRowHeight(childItem);
        }

    return res;
}

//==============================================================================

QSize PropertyEditorWidget::sizeHint() const
{
    // Return either our default/ideal size, depending on the case

    if (mAutoUpdateHeight) {
        // We automatically resize our height, so determine our ideal size which
        // is based on the width of our different columns, and the height of our
        // header and our different rows

        int hintWidth  = 0;
        int hintHeight = header()->height();

        for (int i = 0, iMax = header()->count(); i < iMax; ++i)
            hintWidth += columnWidth(i);

        for (int i = 0, iMax = mModel->rowCount(); i < iMax; ++i) {
            QStandardItem *rowItem = mModel->item(i, 0);
            int rowItemHeight      = rowHeight(rowItem->index());

            if (rowItemHeight)
                // Our current row has some height, meaning that it is visible,
                // so we can its height to ou hintHeight, as well as retrieve
                // the total height of our row's children

                hintHeight += rowItemHeight+childrenRowHeight(rowItem);
        }

        return QSize(hintWidth, hintHeight);
    } else {
        // We don't automatically resize our height, so our ideal size is our
        // maximum size...

        return maximumSize();
    }
}

//==============================================================================

void PropertyEditorWidget::selectFirstProperty()
{
    // Convenience function to select the first property, i.e. the first item

    selectFirstItem();
}

//==============================================================================

PropertyEditorWidgetGuiState * PropertyEditorWidget::guiState()
{
    // Finish the property editing, if any

    finishPropertyEditing();

    // Retrieve our GUI state

    PropertyEditorWidgetGuiState *res = new PropertyEditorWidgetGuiState(currentIndex());

    // Retrieve the hidden state, expanded state and value of our different
    // properties

    foreach (Property *property, mProperties)
        res->addProperty(new PropertyEditorWidgetGuiStateProperty(property,
                                                                  isRowHidden(property->name()->row(),
                                                                              property->name()->parent()?
                                                                                  property->name()->parent()->index():
                                                                                  mModel->invisibleRootItem()->index()),
                                                                  isExpanded(property->name()->index()),
                                                                  property->value()?
                                                                      property->value()->text():
                                                                      QString()));

    // Return our GUI state

    return res;
}

//==============================================================================

void PropertyEditorWidget::setGuiState(PropertyEditorWidgetGuiState *pGuiState)
{
    // Set our GUI state

    // Set the hidden state, expanded state and value of our different
    // properties

    foreach (PropertyEditorWidgetGuiStateProperty *guiStateProperty, pGuiState->properties()) {
        setRowHidden(guiStateProperty->property()->name()->row(),
                     guiStateProperty->property()->name()->parent()?
                         guiStateProperty->property()->name()->parent()->index():
                         mModel->invisibleRootItem()->index(),
                     guiStateProperty->isHidden());

        setExpanded(guiStateProperty->property()->name()->index(),
                    guiStateProperty->isExpanded());

        PropertyItem *propertyValue = guiStateProperty->property()->value();

        if (propertyValue)
            setPropertyItem(propertyValue, guiStateProperty->value());
    }

    // Set our current index, if it is valid

    if (pGuiState->currentProperty().isValid())
        setCurrentIndex(pGuiState->currentProperty());
}

//==============================================================================

Property * PropertyEditorWidget::addProperty(const PropertyItem::Type &pType,
                                             const bool &pEditable,
                                             const bool &pCheckable,
                                             Property *pParent)
{
    // Determine our new property's information

    Property *res = new Property(pType, pEditable, pCheckable);

    // Populate our data model with our new property

    if (pParent) {
        // We want to add a child property

        pParent->name()->appendRow(res->items());

        // If we want to see the child property, we need root decoration

        setRootIsDecorated(true);
    } else {
        // We want to add a root property

        mModel->invisibleRootItem()->appendRow(res->items());
    }

    // Span ourselves if we are of section type

    if (pType == PropertyItem::Section)
        setFirstColumnSpanned(res->name()->row(),
                              pParent?pParent->name()->index():mModel->invisibleRootItem()->index(),
                              true);

    // Keep track of our new property

    mProperties << res;

    // Return our new property's information

    return res;
}

//==============================================================================

Property * PropertyEditorWidget::addSectionProperty(Property *pParent)
{
    // Add a section property

    Property *res = addProperty(PropertyItem::Section, false, false, pParent);

    // Return our section property information

    return res;
}

//==============================================================================

Property * PropertyEditorWidget::addIntegerProperty(const bool &pEditable,
                                                    Property *pParent)
{
    // Add an integer property and return its information

    return addProperty(PropertyItem::Integer, pEditable, false, pParent);
}

//==============================================================================

Property * PropertyEditorWidget::addDoubleProperty(const bool &pEditable,
                                                   const bool &pCheckable,
                                                   Property *pParent)
{
    // Add a double property and return its information

    return addProperty(PropertyItem::Double, pEditable, pCheckable, pParent);
}

//==============================================================================

Property * PropertyEditorWidget::addListProperty(Property *pParent)
{
    // Add a list property and return its information

    return addProperty(PropertyItem::List, true, false, pParent);
}

//==============================================================================

void PropertyEditorWidget::setPropertyItem(QStandardItem *pPropertyItem,
                                           const QString &pValue)
{
    // Set the value of the given property item, if it exists, and use its value
    // as a tooltip (in case it's too long and doesn't fit within the allocated
    // space we have)

    if (pPropertyItem) {
        // Set the property item itself

        pPropertyItem->setText(pValue);
        pPropertyItem->setToolTip(pValue);
    }
}

//==============================================================================

void PropertyEditorWidget::setStringPropertyItem(QStandardItem *pPropertyItem,
                                                 const QString &pValue)
{
    // Set the value of the given property item, if it exists and is of string
    // type

    if (pPropertyItem &&
        (   (pPropertyItem->type() == PropertyItem::Section)
         || (pPropertyItem->type() == PropertyItem::String)))
        setPropertyItem(pPropertyItem, pValue);
}

//==============================================================================

int PropertyEditorWidget::integerPropertyItem(PropertyItem *pPropertyItem)
{
    // Return the value of the given integer property item, if it exists and is
    // valid

    if (pPropertyItem && (pPropertyItem->type() == PropertyItem::Integer))
        return pPropertyItem->text().toInt();
    else
        // The property item is either not valid or not of integer type, so...

        return 0;
}

//==============================================================================

void PropertyEditorWidget::setIntegerPropertyItem(PropertyItem *pPropertyItem,
                                                  const int &pValue)
{
    // Set the value of the given property item, if it exists and is of integer
    // type

    if (pPropertyItem && (pPropertyItem->type() == PropertyItem::Integer))
        setPropertyItem(pPropertyItem, QString::number(pValue));
}

//==============================================================================

double PropertyEditorWidget::doublePropertyItem(PropertyItem *pPropertyItem)
{
    // Return the value of the given double property item, if it exists and is
    // valid

    if (pPropertyItem && (pPropertyItem->type() == PropertyItem::Double))
        return pPropertyItem->text().toDouble();
    else
        // The property item is either not valid or not of double type, so...

        return 0.0;
}

//==============================================================================

void PropertyEditorWidget::setDoublePropertyItem(PropertyItem *pPropertyItem,
                                                 const double &pValue)
{
    // Set the value of the given property item, if it exists and is of double
    // type

    if (pPropertyItem && (pPropertyItem->type() == PropertyItem::Double))
        setPropertyItem(pPropertyItem, QString::number(pValue));
}

//==============================================================================

void PropertyEditorWidget::keyPressEvent(QKeyEvent *pEvent)
{
    // Check some key combinations

    if (   (pEvent->modifiers() & Qt::ControlModifier)
        && (pEvent->key() == Qt::Key_A)) {
        // The user wants to select everything which we don't want to allow,
        // so just accept the event...

        pEvent->accept();
    } else if (   (pEvent->key() == Qt::Key_Return)
               || (pEvent->key() == Qt::Key_Enter)) {
        // The user wants to start/stop editing the property

        if (mPropertyEditor)
            // We are currently editing a property, so stop editing it

            editProperty(0);
        else
            // We are not currently editing a property, so start editing the
            // current one
            // Note: we could use mProperty, but if it was to be empty then we
            //       would have to use currentIndex().row(), so we might as well
            //       use the latter all the time...

            editProperty(property(currentIndex()));

        // Accept the event

        pEvent->accept();
    } else if (pEvent->key() == Qt::Key_Escape) {
        // The user wants to cancel the editing of the property

        finishPropertyEditing(false);

        // Accept the event

        pEvent->accept();
    } else if (   !(pEvent->modifiers() & Qt::ShiftModifier)
               && !(pEvent->modifiers() & Qt::ControlModifier)
               && !(pEvent->modifiers() & Qt::AltModifier)
               && !(pEvent->modifiers() & Qt::MetaModifier)) {
        // None of the modifiers is selected

        if (pEvent->key() == Qt::Key_Up) {
            // The user wants to go the previous property

            goToPreviousProperty();

            // Accept the event

            pEvent->accept();
        } else if (pEvent->key() == Qt::Key_Down) {
            // The user wants to go to the next property

            goToNextProperty();

            // Accept the event

            pEvent->accept();
        } else {
            // Default handling of the event

            TreeViewWidget::keyPressEvent(pEvent);
        }
    } else {
        // Default handling of the event

        TreeViewWidget::keyPressEvent(pEvent);
    }
}

//==============================================================================

void PropertyEditorWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
    // Edit the property, but only if we want to edit a new one

    Property *mouseProperty = property(indexAt(pEvent->pos()));

    if (mouseProperty && (mouseProperty != mProperty))
        editProperty(mouseProperty);

    // Accept the event

    pEvent->accept();
}

//==============================================================================

void PropertyEditorWidget::mousePressEvent(QMouseEvent *pEvent)
{
    // Start/stop the editing of the property

    Property *mouseProperty = property(indexAt(pEvent->pos()));

    if (mPropertyEditor) {
        // We are already editing a property, so either stop its editing or
        // start editing anoher property

        if (mouseProperty != mProperty)
            // We want to edit another property

            editProperty(mouseProperty);
        else
            // We want to stop editing the property

            editProperty(0);
    } else {
        // We are not currently editing a property, so start editing the current
        // one

        editProperty(mouseProperty);
    }

    // Accept the event

    pEvent->accept();
}

//==============================================================================

void PropertyEditorWidget::resizeEvent(QResizeEvent *pEvent)
{
    // Default handling of the event

    TreeViewWidget::resizeEvent(pEvent);

    // Update our height

    updateHeight();
}

//==============================================================================

void PropertyEditorWidget::updateHeight()
{
    // Update our height, so that we don't have any blank space at the bottom,
    // but only if required

    if (mAutoUpdateHeight) {
        QSize idealSize = sizeHint();

        setFixedHeight( idealSize.height()
                       +((width() < idealSize.width())?
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
                            horizontalScrollBar()->height():
#elif defined(Q_OS_LINUX)
                            horizontalScrollBar()->height()+3:
                            // Note: why on earth does Linux require three more
                            //       pixels?!... Indeed, if we don't add them,
                            //       then there won't be enough space to show
                            //       the last property (upon selecting it) and
                            //       the widget will increase its height,
                            //       resulting in some blank space at the bottom
                            //       and a vertical bar being shown! We could
                            //       prevent the vertical bar from ever being
                            //       shown, but not sure what could be done
                            //       about the blank space, hence we 'manually'
                            //       add those three pixels...
#else
    #error Unsupported platform
#endif
                            0));
        // Note: the new height consists of our ideal height to which we add the
        //       height of our horizontal scroll bar, should it be shown (i.e.
        //       if our width is smaller than that of our ideal size)...
    }
}

//==============================================================================

void PropertyEditorWidget::emitPropertyChecked(QStandardItem *pItem)
{
    // Let people know whether our current property is checked

    Property *itemProperty = property(pItem->index());
    bool oldPropertyChecked = mPropertiesChecked.value(itemProperty, false);
    bool newPropertyChecked = itemProperty->name()->checkState() == Qt::Checked;

    if (oldPropertyChecked != newPropertyChecked) {
        // The property's checked status has changed, so let people know about
        // it

        emit propertyChecked(itemProperty, newPropertyChecked);

        // Keep track of the property's new checked status

        mPropertiesChecked.insert(itemProperty, newPropertyChecked);
    }
}

//==============================================================================

void PropertyEditorWidget::editorOpened(QWidget *pEditor)
{
    // Keep track of some information about the property

    mProperty       = currentProperty();
    mPropertyEditor = pEditor;

    // We are starting the editing of a property, so make sure that if we are to
    // edit a list item, then its original value gets properly set
    // Note: indeed, by default the first list item will be selected...

    PropertyItem *propertyValue = mProperty->value();

    if (propertyValue->type() == PropertyItem::List) {
        ListEditorWidget *propertyEditor = static_cast<ListEditorWidget *>(mPropertyEditor);

        for (int i = 0, iMax = propertyValue->list().count(); i < iMax; ++i)
            if (!propertyValue->text().compare(propertyValue->list()[i])) {
                propertyEditor->setCurrentIndex(i);

                break;
            }
    }

    // Next, we need to use the property's editor as our focus proxy and make
    // sure that it immediately gets the focus
    // Note: if we were not to immediately give the editor the focus, then the
    //       central widget would give the focus to the previously focused
    //       widget (see CentralWidget::updateGui()), so...

    setFocusProxy(pEditor);

    pEditor->setFocus();
}

//==============================================================================

void PropertyEditorWidget::editorClosed()
{
    // Make sure that we were editing a property

    if (!mPropertyEditor)
        return;

    // We have stopped editing a property, so make sure that if we were editing
    // a list item, then its value gets properly set

    PropertyItem *propertyValue = mProperty->value();

    if (propertyValue->type() == PropertyItem::List)
        setPropertyItem(propertyValue,
                        propertyValue->list().isEmpty()?
                            propertyValue->emptyListValue():
                            static_cast<ListEditorWidget *>(mPropertyEditor)->currentText());
    else
        // Not a list item, but still need to call setPropertyItem() so that the
        // item's tool tip gets updated

        setPropertyItem(propertyValue, propertyValue->text());

    // Reset our focus proxy and make sure that we get the focus (see
    // editorOpened() above for the reason)

    setFocusProxy(0);

    setFocus();

    // Let people know that the property value has changed

    emit propertyChanged(mProperty);

    // Reset some information about the property

    mProperty       = 0;
    mPropertyEditor = 0;
}

//==============================================================================

void PropertyEditorWidget::editProperty(Property *pProperty,
                                        const bool &pCommitData)
{
    // Check that we are dealing with a 'proper' property item and not a section

    if (pProperty && !pProperty->value())
        return;

    // We want to edit a new property, so first stop the editing of the current
    // one, if any

    if (mPropertyEditor) {
        // A property is currently being edited, so commit its data and then
        // close its corresponding editor

        if (pCommitData)
            commitData(mPropertyEditor);

        closeEditor(mPropertyEditor,
                    pCommitData?
                        QAbstractItemDelegate::SubmitModelCache:
                        QAbstractItemDelegate::RevertModelCache);

        // Update our state
        // Note: this is very important otherwise our state will remain that of
        //       editing...

        setState(NoState);

        // Call editorClosed() to reset a few things

        editorClosed();
    }

    // Now that the editing of our old property has finished, we can start the
    // editing of our new property, if any

    if (pProperty) {
        // There is a new property to edit, so first make sure that it is
        // selected

        QModelIndex propertyIndex = pProperty->value()->index();

        setCurrentIndex(propertyIndex);

        // Now, we can 'properly' edit the property's value, but only if the
        // property's value is actually editable

        if (pProperty->value()->isEditable())
            edit(propertyIndex);
    }
}

//==============================================================================

Properties PropertyEditorWidget::properties() const
{
    // Return our properties

    return mProperties;
}

//==============================================================================

void PropertyEditorWidget::finishPropertyEditing(const bool &pCommitData)
{
    // The user wants to finish the editing of the property

    editProperty(0, pCommitData);
}

//==============================================================================

void PropertyEditorWidget::removeAllProperties()
{
    // Remove all the properties we currently hold

    mModel->removeRows(0, mModel->rowCount());

    // By default, we don't want root decoration

    setRootIsDecorated(false);
}

//==============================================================================

void PropertyEditorWidget::setPropertyVisible(Property *pProperty,
                                              const bool &pVisible)
{
    // Show/hide the property, if not empty

    if (!pProperty)
        return;

    setRowHidden(pProperty->name()->row(),
                 pProperty->name()->parent()?
                     pProperty->name()->parent()->index():
                     mModel->invisibleRootItem()->index(),
                 !pVisible);

    // Make sure that our height is correct

    updateHeight();
}

//==============================================================================

void PropertyEditorWidget::goToNeighbouringProperty(const int &pShift)
{
    // Determine the index of the current index's neighbour

    QModelIndex neighbouringIndex = QModelIndex();

    if (pShift == 1)
        neighbouringIndex = indexBelow(currentIndex());
    else if (pShift == -1)
        neighbouringIndex = indexAbove(currentIndex());

    // Edit the neighbouring property, if any

    if (neighbouringIndex.isValid())
        editProperty(property(neighbouringIndex));
}

//==============================================================================

void PropertyEditorWidget::goToPreviousProperty()
{
    // Go to the previous property

    goToNeighbouringProperty(-1);
}

//==============================================================================

void PropertyEditorWidget::goToNextProperty()
{
    // Go to the next property

    goToNeighbouringProperty(1);
}

//==============================================================================

Property * PropertyEditorWidget::property(const QModelIndex &pIndex) const
{
    // Make sure that the given index is valid

    if (!pIndex.isValid())
        return 0;

    // Return our information about the property at the given index

    foreach (Property *property, mProperties)
        if (   (property->name()->index()  == pIndex)
            || (property->value()->index() == pIndex)
            || (property->unit()->index()  == pIndex))
            return property;

    // Somehow, we couldn't find the property (how is that even possible?!),
    // so...

    return 0;
}

//==============================================================================

Property * PropertyEditorWidget::currentProperty() const
{
    // Return some information about the current property

    return property(currentIndex());
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
