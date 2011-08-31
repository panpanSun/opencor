/* This file is automatically generated from TeLICeMS.idl
 * DO NOT EDIT DIRECTLY OR CHANGES WILL BE LOST!
 */
#ifndef _GUARD_IFACE_TeLICeMS
#define _GUARD_IFACE_TeLICeMS
#include "cda_compiler_support.h"
#ifdef MODULE_CONTAINS_TeLICeMS
#define PUBLIC_TeLICeMS_PRE CDA_EXPORT_PRE
#define PUBLIC_TeLICeMS_POST CDA_EXPORT_POST
#else
#define PUBLIC_TeLICeMS_PRE CDA_IMPORT_PRE
#define PUBLIC_TeLICeMS_POST CDA_IMPORT_POST
#endif
#include "Ifacexpcom.hxx"
#include "IfaceDOM_APISPEC.hxx"
#include "IfaceMathML_content_APISPEC.hxx"
#include "IfaceCellML_APISPEC.hxx"
namespace iface
{
  namespace cellml_services
  {
    PUBLIC_TeLICeMS_PRE 
    class  PUBLIC_TeLICeMS_POST TeLICeMResult
     : public virtual iface::XPCOM::IObject
    {
    public:
      virtual ~TeLICeMResult() {}
      virtual wchar_t* errorMessage() throw(std::exception&)  WARN_IF_RETURN_UNUSED = 0;
    };
    PUBLIC_TeLICeMS_PRE 
    class  PUBLIC_TeLICeMS_POST TeLICeMModelResult
     : public virtual iface::cellml_services::TeLICeMResult
    {
    public:
      virtual ~TeLICeMModelResult() {}
      virtual iface::cellml_api::Model* modelResult() throw(std::exception&)  WARN_IF_RETURN_UNUSED = 0;
    };
    PUBLIC_TeLICeMS_PRE 
    class  PUBLIC_TeLICeMS_POST TeLICeMMathResult
     : public virtual iface::cellml_services::TeLICeMResult
    {
    public:
      virtual ~TeLICeMMathResult() {}
      virtual iface::mathml_dom::MathMLElement* mathResult() throw(std::exception&)  WARN_IF_RETURN_UNUSED = 0;
    };
    PUBLIC_TeLICeMS_PRE 
    class  PUBLIC_TeLICeMS_POST TeLICeMService
     : public virtual iface::XPCOM::IObject
    {
    public:
      virtual ~TeLICeMService() {}
      virtual iface::cellml_services::TeLICeMModelResult* parseModel(const wchar_t* aModelText) throw(std::exception&) WARN_IF_RETURN_UNUSED = 0;
      virtual iface::cellml_services::TeLICeMMathResult* parseMaths(iface::dom::Document* aDoc, const wchar_t* aMathText) throw(std::exception&) WARN_IF_RETURN_UNUSED = 0;
      virtual wchar_t* showModel(iface::cellml_api::Model* aModel) throw(std::exception&) WARN_IF_RETURN_UNUSED = 0;
      virtual wchar_t* showMaths(iface::mathml_dom::MathMLContentElement* aEl) throw(std::exception&) WARN_IF_RETURN_UNUSED = 0;
    };
  };
};
#undef PUBLIC_TeLICeMS_PRE
#undef PUBLIC_TeLICeMS_POST
#endif // guard
