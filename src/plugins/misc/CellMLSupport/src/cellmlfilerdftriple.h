//==============================================================================
// CellML file RDF triple
//==============================================================================

#ifndef CELLMLFILERDFTRIPLE_H
#define CELLMLFILERDFTRIPLE_H

//==============================================================================

#include "cellmlfilerdftripleelement.h"
#include "cellmlsupportglobal.h"

//==============================================================================

#include <QList>
#include <QSet>
#include <QUrl>
#include <QVector>

//==============================================================================

namespace OpenCOR {
namespace CellMLSupport {

//==============================================================================

static const QString ResourceRegExp = "[0-9a-z]+((-|\\.)[0-9a-z]+)?";
static const QString IdRegExp = "[0-9A-Za-z\\.%-_:]+(:[0-9A-Za-z\\.%-_:]+)?";

//==============================================================================

class CellmlFile;

//==============================================================================

class CELLMLSUPPORT_EXPORT CellmlFileRdfTriple
{
public:
    enum Type {
        Unknown,
        BioModelsDotNetQualifier,
        Empty
    };

    enum ModelQualifier {
        ModelUnknown,
        ModelIs,
        ModelIsDerivedFrom,
        ModelIsDescribedBy,
        FirstModelQualifier = ModelIs,
        LastModelQualifier = ModelIsDescribedBy
    };

    enum BioQualifier {
        BioUnknown,
        BioEncodes,
        BioHasPart,
        BioHasProperty,
        BioHasVersion,
        BioIs,
        BioIsDescribedBy,
        BioIsEncodedBy,
        BioIsHomologTo,
        BioIsPartOf,
        BioIsPropertyOf,
        BioIsVersionOf,
        BioOccursIn,
        BioHasTaxon,
        FirstBioQualifier = BioEncodes,
        LastBioQualifier = BioHasTaxon
    };

    explicit CellmlFileRdfTriple(CellmlFile *pCellmlFile,
                                 iface::rdf_api::Triple *pRdfTriple);
    explicit CellmlFileRdfTriple(CellmlFile *pCellmlFile,
                                 const QString pSubject,
                                 const ModelQualifier &pModelQualifier,
                                 const QString &pResource, const QString &pId);
    explicit CellmlFileRdfTriple(CellmlFile *pCellmlFile,
                                 const QString pSubject,
                                 const BioQualifier &pBioQualifier,
                                 const QString &pResource, const QString &pId);
    ~CellmlFileRdfTriple();

    iface::rdf_api::Triple * rdfTriple() const;
    void setRdfTriple(iface::rdf_api::Triple *pRdfTriple);

    CellmlFileRdfTripleElement * subject() const;
    CellmlFileRdfTripleElement * predicate() const;
    CellmlFileRdfTripleElement * object() const;

    Type type() const;

    QString metadataId() const;

    QString qualifierAsString() const;

    ModelQualifier modelQualifier() const;
    QString modelQualifierAsString() const;
    static QString modelQualifierAsString(const ModelQualifier &pModelQualifier);

    BioQualifier bioQualifier() const;
    QString bioQualifierAsString() const;
    static QString bioQualifierAsString(const BioQualifier &pBioQualifier);

    static QStringList qualifiersAsStringList();

    QString resource() const;
    QString id() const;

    static bool decodeTerm(const QString &pTerm, QString &pResource, QString &pId);

private:
    CellmlFile *mCellmlFile;

    ObjRef<iface::rdf_api::Triple> mRdfTriple;

    CellmlFileRdfTripleElement *mSubject;
    CellmlFileRdfTripleElement *mPredicate;
    CellmlFileRdfTripleElement *mObject;

    Type mType;

    ModelQualifier mModelQualifier;
    BioQualifier mBioQualifier;

    QString mResource;
    QString mId;

    void constructor(CellmlFile *pCellmlFile,
                     iface::rdf_api::Triple *pRdfTriple, const Type &pType,
                     const ModelQualifier &pModelQualifier,
                     const BioQualifier &pBioQualifier,
                     const QString &pResource, const QString &pId);
};

//==============================================================================

class CELLMLSUPPORT_EXPORT CellmlFileRdfTriples: public QList<CellmlFileRdfTriple *>
{
public:
    explicit CellmlFileRdfTriples(CellmlFile *pCellmlFile);

    CellmlFileRdfTriple::Type type() const;

    CellmlFileRdfTriples contains(iface::cellml_api::CellMLElement *pElement) const;

    CellmlFileRdfTriple * add(CellmlFileRdfTriple *pRdfTriple);

    void remove(CellmlFileRdfTriple *pRdfTriple);
    void remove(iface::cellml_api::CellMLElement *pElement);
    void removeAll();

private:
    CellmlFile *mCellmlFile;

    void recursiveContains(CellmlFileRdfTriples &pRdfTriples,
                           CellmlFileRdfTriple *pRdfTriple) const;

    void removeRdfTriples(const CellmlFileRdfTriples &pRdfTriples);
};

//==============================================================================

}   // namespace CellMLSupport
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
