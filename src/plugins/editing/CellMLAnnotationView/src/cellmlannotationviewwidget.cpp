//==============================================================================
// CellML annotation view widget
//==============================================================================

#include "borderedwidget.h"
#include "cellmlannotationviewcellmllistwidget.h"
#include "cellmlannotationviewmetadatanormalviewdetailswidget.h"
#include "cellmlannotationviewmetadatadetailswidget.h"
#include "cellmlannotationviewmetadataeditdetailswidget.h"
#include "cellmlannotationviewmetadataviewdetailswidget.h"
#include "cellmlannotationviewplugin.h"
#include "cellmlannotationviewwidget.h"
#include "cellmlfilemanager.h"
#include "coreutils.h"
#include "treeviewwidget.h"

//==============================================================================

#include "ui_cellmlannotationviewwidget.h"

//==============================================================================

#include <QComboBox>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QWebView>

//==============================================================================

namespace OpenCOR {
namespace CellMLAnnotationView {

//==============================================================================

CellmlAnnotationViewWidget::CellmlAnnotationViewWidget(CellMLAnnotationViewPlugin *pPluginParent,
                                                       const QString &pFileName,
                                                       QWidget *pParent) :
    QSplitter(pParent),
    CommonWidget(pParent),
    mGui(new Ui::CellmlAnnotationViewWidget),
    mPluginParent(pPluginParent),
    oldWebViewUrls(QMap<QWebView *, QUrl>())
{
    // Set up the GUI

    mGui->setupUi(this);

    // Retrieve some SVG diagrams

    QFile modelQualifierFile(":modelQualifier");
    QFile biologyQualifierFile(":biologyQualifier");

    modelQualifierFile.open(QIODevice::ReadOnly);
    biologyQualifierFile.open(QIODevice::ReadOnly);

    mModelQualifierSvg   = QString(modelQualifierFile.readAll());
    mBiologyQualifierSvg = QString(biologyQualifierFile.readAll());

    modelQualifierFile.close();
    biologyQualifierFile.close();

    // Retrieve our output template

    QFile qualifierInformationFile(":qualifierInformation");

    qualifierInformationFile.open(QIODevice::ReadOnly);

    mQualifierInformationTemplate = QString(qualifierInformationFile.readAll());

    qualifierInformationFile.close();

    // Retrieve and load, in case it's necessary, the requested CellML file

    mCellmlFile = CellMLSupport::CellmlFileManager::instance()->cellmlFile(pFileName);

    mCellmlFile->load();

    // Customise our GUI which consists of two main parts:
    //
    //  1) A couple of lists (for CellML elements and metadata, resp.); and
    //  2) Some details (for a CellML element or metadata).
    //
    // These two main parts are widgets of ourselves and moving the splitter
    // will result in the splitter of other CellML files' view to be moved too

    // Create our two main parts

    mCellmlList      = new CellmlAnnotationViewCellmlListWidget(this);
    mMetadataDetails = new CellmlAnnotationViewMetadataDetailsWidget(this);

    // Populate ourselves

    addWidget(new Core::BorderedWidget(mCellmlList,
                                       false, false, false, true));
    addWidget(mMetadataDetails);

    // Keep track of our splitter being moved

    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(emitSplitterMoved()));

    // A connection to let our details widget know that we want to see the
    // metadata details of some CellML element

    connect(mCellmlList, SIGNAL(metadataDetailsRequested(iface::cellml_api::CellMLElement *)),
            mMetadataDetails, SLOT(updateGui(iface::cellml_api::CellMLElement *)));

    // A connection to handle the fact that an RDF triple has been added

    connect(mMetadataDetails->metadataEditDetails(), SIGNAL(rdfTripleAdded(CellMLSupport::CellmlFileRdfTriple *)),
            this, SLOT(addRdfTriple(CellMLSupport::CellmlFileRdfTriple *)));

    // Make our CellML list widget our focus proxy

    setFocusProxy(mCellmlList);

    // Select the first item from our lists
    // Note: our CellML list is our primary list, so we must select the first
    //       item of our lists in a reverse order, i.e. finish with our CellML
    //       list...

    mCellmlList->treeViewWidget()->selectFirstItem();
}

//==============================================================================

CellmlAnnotationViewWidget::~CellmlAnnotationViewWidget()
{
    // Delete the GUI

    delete mGui;
}

//==============================================================================

void CellmlAnnotationViewWidget::retranslateUi()
{
    // Retranslate our lists and details widgets

    mCellmlList->retranslateUi();
    mMetadataDetails->retranslateUi();
}

//==============================================================================

QString CellmlAnnotationViewWidget::pluginViewName() const
{
    // Return our pointer to the plugin parent

    return mPluginParent->viewName();
}

//==============================================================================

CellMLSupport::CellmlFile * CellmlAnnotationViewWidget::cellmlFile() const
{
    // Return the CellML file

    return mCellmlFile;
}

//==============================================================================

void CellmlAnnotationViewWidget::updateSizes(const QList<int> &pSizes)
{
    // The splitter of another CellmlAnnotationViewWidget object has been moved,
    // so update our sizes

    setSizes(pSizes);
}

//==============================================================================

void CellmlAnnotationViewWidget::emitSplitterMoved()
{
    // Let people know that our splitter has been moved

    emit splitterMoved(sizes());
}

//==============================================================================

CellmlAnnotationViewCellmlListWidget * CellmlAnnotationViewWidget::cellmlList() const
{
    // Return our CellML list widget

    return mCellmlList;
}

//==============================================================================

CellmlAnnotationViewMetadataDetailsWidget * CellmlAnnotationViewWidget::metadataDetails() const
{
    // Return our metadata details widget

    return mMetadataDetails;
}

//==============================================================================

void CellmlAnnotationViewWidget::updateWebViewerWithQualifierDetails(QWebView *pWebView,
                                                                     const QString &pQualifier,
                                                                     const bool &pRetranslate)
{
    Q_UNUSED(pRetranslate);

    // The user requested a qualifier to be looked up, so generate a web page
    // containing some information about the qualifier
    // Note: ideally, there would be a way to refer to a particular qualifier
    //       using http://biomodels.net/qualifiers/, but that would require
    //       anchors and they don't have any, so instead we use the information
    //       which can be found on that site and present it to the user in the
    //       form of a web page...

    if (pQualifier.isEmpty())
        return;

    // Generate the web page containing some information about the qualifier

    QString qualifierSvg;
    QString shortDescription;
    QString longDescription;

    if (!pQualifier.compare("model:is")) {
        qualifierSvg = mModelQualifierSvg;

        shortDescription = tr("Identity");
        longDescription  = tr("The modelling object represented by the model element is identical with the subject of the referenced resource (\"Modelling Object B\"). For instance, this qualifier might be used to link an encoded model to a database of models.");
    } else if (!pQualifier.compare("model:isDerivedFrom")) {
        qualifierSvg = mModelQualifierSvg;

        shortDescription = tr("Origin");
        longDescription  = tr("The modelling object represented by the model element is derived from the modelling object represented by the referenced resource (\"Modelling Object B\"). This relation may be used, for instance, to express a refinement or adaptation in usage for a previously described modelling component.");
    } else if (!pQualifier.compare("model:isDescribedBy")) {
        qualifierSvg = mModelQualifierSvg;

        shortDescription = tr("Description");
        longDescription  = tr("The modelling object represented by the model element is described by the subject of the referenced resource (\"Modelling Object B\"). This relation might be used to link a model or a kinetic law to the literature that describes it.");
    } else if (!pQualifier.compare("bio:encodes")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Encodement");
        longDescription  = tr("The biological entity represented by the model element encodes, directly or transitively, the subject of the referenced resource (\"Biological Entity B\"). This relation may be used to express, for example, that a specific DNA sequence encodes a particular protein.");
    } else if (!pQualifier.compare("bio:hasPart")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Part");
        longDescription  = tr("The biological entity represented by the model element includes the subject of the referenced resource (\"Biological Entity B\"), either physically or logically. This relation might be used to link a complex to the description of its components.");
    } else if (!pQualifier.compare("bio:hasProperty")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Property");
        longDescription  = tr("The subject of the referenced resource (\"Biological Entity B\") is a property of the biological entity represented by the model element. This relation might be used when a biological entity exhibits a certain enzymatic activity or exerts a specific function.");
    } else if (!pQualifier.compare("bio:hasVersion")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Version");
        longDescription  = tr("The subject of the referenced resource (\"Biological Entity B\") is a version or an instance of the biological entity represented by the model element. This relation may be used to represent an isoform or modified form of a biological entity.");
    } else if (!pQualifier.compare("bio:is")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Indentity");
        longDescription  = tr("The biological entity represented by the model element has identity with the subject of the referenced resource (\"Biological Entity B\"). This relation might be used to link a reaction to its exact counterpart in a database, for instance.");
    } else if (!pQualifier.compare("bio:isDescribedBy")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Description");
        longDescription  = tr("The biological entity represented by the model element is described by the subject of the referenced resource (\"Biological Entity B\"). This relation should be used, for instance, to link a species or a parameter to the literature that describes the concentration of that species or the value of that parameter.");
    } else if (!pQualifier.compare("bio:isEncodedBy")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Encoder");
        longDescription  = tr("The biological entity represented by the model element is encoded, directly or transitively, by the subject of the referenced resource (\"Biological Entity B\"). This relation may be used to express, for example, that a protein is encoded by a specific DNA sequence.");
    } else if (!pQualifier.compare("bio:isHomologTo")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Homolog");
        longDescription  = tr("The biological entity represented by the model element is homologous to the subject of the referenced resource (\"Biological Entity B\"). This relation can be used to represent biological entities that share a common ancestor.");
    } else if (!pQualifier.compare("bio:isPartOf")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Parthood");
        longDescription  = tr("The biological entity represented by the model element is a physical or logical part of the subject of the referenced resource (\"Biological Entity B\"). This relation may be used to link a model component to a description of the complex in which it is a part.");
    } else if (!pQualifier.compare("bio:isPropertyOf")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Property bearer");
        longDescription  = tr("The biological entity represented by the model element is a property of the referenced resource (\"Biological Entity B\").");
    } else if (!pQualifier.compare("bio:isVersionOf")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Hypernym");
        longDescription  = tr("The biological entity represented by the model element is a version or an instance of the subject of the referenced resource (\"Biological Entity B\"). This relation may be used to represent, for example, the 'superclass' or 'parent' form of a particular biological entity.");
    } else if (!pQualifier.compare("bio:occursIn")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Container");
        longDescription  = tr("The biological entity represented by the model element is physically limited to a location, which is the subject of the referenced resource (\"Biological Entity B\"). This relation may be used to ascribe a compartmental location, within which a reaction takes place.");
    } else if (!pQualifier.compare("bio:hasTaxon")) {
        qualifierSvg = mBiologyQualifierSvg;

        shortDescription = tr("Taxon");
        longDescription  = tr("The biological entity represented by the model element is taxonomically restricted, where the restriction is the subject of the referenced resource (\"Biological Entity B\"). This relation may be used to ascribe a species restriction to a biochemical reaction.");
    } else {
        qualifierSvg = "";

        shortDescription = tr("Unknown");
        longDescription  = tr("Unknown");
    }

    // Show the information

    oldWebViewUrls.insert(pWebView, QUrl());

    pWebView->setHtml(mQualifierInformationTemplate.arg(pQualifier,
                                                        qualifierSvg,
                                                        shortDescription,
                                                        longDescription,
                                                        Core::copyright()));
}

//==============================================================================

void CellmlAnnotationViewWidget::updateWebViewerWithResourceDetails(QWebView *pWebView,
                                                                    const QString &pResource,
                                                                    const bool &pRetranslate)
{
    // The user requested a resource to be looked up, so retrieve it using
    // identifiers.org, but only if we are not retranslating since the looking
    // up would already be correct

    if (!pRetranslate) {
        // Note: updating the URL of the web view results in it being refreshed,
        //       even if the URL is the same as the one before in which case it
        //       looks like some kind of a big flickering. We therefore want to
        //       avoid setting the URL if it's the same as the previous one.
        //       Normally, we would check the (current) URL of the web view, but
        //       this won't work if the URL redirects to another (as is the case
        //       with identifiers.org URLs), so instead we keep track of the URL
        //       ourselves...

        QUrl oldUrl = oldWebViewUrls.value(pWebView);
        QUrl newUrl = "http://identifiers.org/"+pResource+"/?redirect=true";
        //---GRY--- NOTE THAT redirect=true DOESN'T WORK AT THE MOMENT, SO WE DO
        //          END UP WITH A FRAME, BUT THE identifiers.org GUYS ARE GOING
        //          TO 'FIX' IT, SO WE SHOULD BE READY FOR WHEN IT'S DONE...

        if (newUrl != oldUrl) {
            oldWebViewUrls.insert(pWebView, newUrl);

            pWebView->setUrl(newUrl);
        }
    }
}

//==============================================================================

void CellmlAnnotationViewWidget::updateWebViewerWithIdDetails(QWebView *pWebView,
                                                              const QString &pResource,
                                                              const QString &pId,
                                                              const bool &pRetranslate)
{
    // The user requested a resource id to be looked up, so retrieve it using
    // identifiers.org, but only if we are not retranslating since the looking
    // up would already be correct

    if (!pRetranslate) {
        // Note: see comment in updateWebViewerWithResourceDetails()...

        QUrl oldUrl = oldWebViewUrls.value(pWebView);
        QUrl newUrl = "http://identifiers.org/"+pResource+"/"+pId+"?profile=most_reliable&redirect=true";

        if (newUrl != oldUrl) {
            oldWebViewUrls.insert(pWebView, newUrl);

            pWebView->setUrl(newUrl);
        }
    }
}

//==============================================================================

void CellmlAnnotationViewWidget::addRdfTriple(CellMLSupport::CellmlFileRdfTriple *pRdfTriple) const
{
    // Add the given RDF triple to our details widget

    mMetadataDetails->addRdfTriple(pRdfTriple);
}

//==============================================================================

}   // namespace CellMLAnnotationView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
