//==============================================================================
// Plugin information
//==============================================================================

#ifndef PLUGININFO_H
#define PLUGININFO_H

//==============================================================================

#include <QtPlugin>

//==============================================================================

#include <QMap>
#include <QStringList>

//==============================================================================

namespace OpenCOR {

//==============================================================================

#define PLUGININFO_FUNC extern "C" Q_DECL_EXPORT void *
// Note: each plugin has a function called <PLUGIN_NAME>PluginInfo which can be
//       used to retrieve some information about the plugin itself. This is done
//       by returning a PluginInfo object. This therefore requires C++ support,
//       so logically we would use extern "C++", but this would result in the
//       function name being mangled. So, to avoid this problem, we use extern
//       "C" which ensures that the function name remains intact. Now, because
//       it's C and not C++, MSVC will generate a warning if we return a
//       PluginInfo object, so we return a void pointer which means that we must
//       free it once we have used...

//==============================================================================

typedef QMap<QString, QString> Descriptions;

//==============================================================================

class PluginInfo
{
public:
    enum InterfaceVersion {
        UndefinedInterfaceVersion,
        InterfaceVersion001
    };

    enum Type {
        UndefinedType,
        General,
        Console,
        Gui
    };

    enum Category {
        Organisation,
        Editing,
        Simulation,
        Analysis,
        Miscellaneous,
        Api,
        ThirdParty
    };

    explicit PluginInfo(const InterfaceVersion &pInterfaceVersion = UndefinedInterfaceVersion,
                        const Type &pType = UndefinedType,
                        const Category &pCategory = Miscellaneous,
                        const bool &pManageable = false,
                        const QStringList &pDependencies = QStringList(),
                        const Descriptions &pDescriptions = Descriptions());

    InterfaceVersion interfaceVersion() const;

    Type type() const;
    Category category() const;
    bool manageable() const;

    QStringList dependencies() const;
    QStringList fullDependencies() const;
    void setFullDependencies(const QStringList &pFullDependencies);

    QString description(const QString &pLocale) const;
    Descriptions descriptions() const;

private:
    InterfaceVersion mInterfaceVersion;

    Type mType;
    Category mCategory;
    bool mManageable;

    QStringList mDependencies;
    QStringList mFullDependencies;

    Descriptions mDescriptions;
};

//==============================================================================

}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
